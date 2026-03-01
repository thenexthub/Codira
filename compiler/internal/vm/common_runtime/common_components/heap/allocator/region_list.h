/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_LIST_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_LIST_H

#include "common_components/heap/allocator/region_desc.h"

namespace common {
class RegionList {
public:
    RegionList(const char* name) : listName_(name) {}

    void PrependRegion(RegionDesc* region, RegionDesc::RegionType type);
    void PrependRegionLocked(RegionDesc* region, RegionDesc::RegionType type);

    void MergeRegionList(RegionList& regionList, RegionDesc::RegionType regionType);
    void MergeRegionListWithoutHead(RegionList& regionList, RegionDesc::RegionType regionType);

    void DeleteRegion(RegionDesc* del)
    {
        if (del == nullptr) {
            return;
        }

        std::lock_guard<std::mutex> lock(listMutex_);
        DeleteRegionLocked(del);
    }

    bool TryDeleteRegion(RegionDesc* del, RegionDesc::RegionType oldType, RegionDesc::RegionType newType)
    {
        if (del == nullptr) {
            return false;
        }

        CHECK_CC(oldType != newType);
        std::lock_guard<std::mutex> lock(listMutex_);
        if (del->GetRegionType() == oldType) {
            DeleteRegionLocked(del);
            del->SetRegionType(newType);
            return true;
        }
        return false;
    }

    void DumpRegionSummary() const;
#ifndef NDEBUG
    void DumpRegionList(const char*);
#endif

    void DecCounts(size_t nRegion, size_t nUnit)
    {
        if (regionCount_ >= nRegion && unitCount_ >= nUnit) {
            regionCount_ -= nRegion;
            unitCount_ -= nUnit;
        } else {
            LOG_COMMON(FATAL) << "region list error count " <<
                regionCount_ << "-" << nRegion << " " << unitCount_ << "-" << nUnit;
        }
    }

    void IncCounts(size_t nRegion, size_t nUnit)
    {
        CHECK_CC((nRegion <= std::numeric_limits<size_t>::max() - regionCount_) &&
              (nUnit <= std::numeric_limits<size_t>::max() - unitCount_));
        regionCount_ += nRegion;
        unitCount_ += nUnit;
    }

    RegionDesc* GetHeadRegion() const { return listHead_; }

    void ClearList()
    {
        listHead_ = nullptr;
        listTail_ = nullptr;
        regionCount_ = 0;
        unitCount_ = 0;
    }

    RegionDesc* GetTailRegion() { return listTail_; }

    RegionDesc* TakeHeadRegion()
    {
        std::lock_guard<std::mutex> lg(listMutex_);
        if (listHead_ == nullptr) {
            return nullptr;
        }
        RegionDesc* currentHead = listHead_;
        DeleteRegionLocked(currentHead);
        return currentHead;
    }

    size_t GetUnitCount() const { return unitCount_; }

    size_t GetRegionCount() const { return regionCount_; }

    size_t GetAllocatedSize(bool usedPageSize = true) const
    {
        if (usedPageSize) {
            return GetUnitCount() * RegionDesc::UNIT_SIZE;
        }
        return CountAllocatedSize();
    }

    void VisitAllRegions(const std::function<void(RegionDesc*)>& visitor)
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        for (RegionDesc* node = listHead_; node != nullptr; node = node->GetNextRegion()) {
            visitor(node);
        }
    }

    void SetElementType(RegionDesc::RegionType type)
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        for (RegionDesc* node = listHead_; node != nullptr; node = node->GetNextRegion()) {
            node->SetRegionType(type);
        }
    }

    std::mutex& GetListMutex() { return listMutex_; }

    void MoveTo(RegionList& targetList)
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        targetList.AssignWith(*this);
        this->ClearList();
    }

    void CopyListTo(RegionList& dstList)
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        dstList.listHead_ = this->listHead_;
        dstList.listTail_ = this->listTail_;
        dstList.regionCount_ = this->regionCount_;
        dstList.unitCount_ = this->unitCount_;
    }

    uintptr_t AllocFromFreeListInLock()
    {
        RegionDesc* region = GetHeadRegion();
        if (region == nullptr) {
            return 0;
        }
        return region->AllocNonMovableFromFreeList();
    }

protected:
    std::mutex listMutex_;
    size_t regionCount_ = 0;
    size_t unitCount_ = 0;
    RegionDesc* listHead_ = nullptr; // the start region for iteration, i.e., the first region
    RegionDesc* listTail_ = nullptr; // help to merge region list
    const char* listName_ = nullptr;

private:
    void DeleteRegionLocked(RegionDesc* del);

    void AssignWith(const RegionList& srcList)
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        listHead_ = srcList.listHead_;
        listTail_ = srcList.listTail_;
        regionCount_ = srcList.regionCount_;
        unitCount_ = srcList.unitCount_;
    }

    // allocated-size of tl-region list must be calculated on the fly.
    size_t CountAllocatedSize() const
    {
        size_t allocCnt = 0;
        std::lock_guard<std::mutex> lock(const_cast<RegionList*>(this)->listMutex_);
        for (RegionDesc* region = listHead_; region != nullptr; region = region->GetNextRegion()) {
            allocCnt += region->GetRegionAllocatedSize();
        }
        return allocCnt;
    }

#ifndef NDEBUG
    void VerifyRegion(RegionDesc* region)
    {
        RegionDesc* prev = region->GetPrevRegion();
        RegionDesc* next = region->GetNextRegion();
        if (prev != nullptr && prev->GetNextRegion() != region) {
            LOG_COMMON(FATAL) << "illegal region node";
            UNREACHABLE_CC();
        }

        if (next != nullptr && next->GetPrevRegion() != region) {
            LOG_COMMON(FATAL) << "illegal region node";
            UNREACHABLE_CC();
        }
    }
#endif
};

class RegionCache : public RegionList {
public:
    RegionCache(const char* name) : RegionList(name) {}

    bool TryPrependRegion(RegionDesc *region, RegionDesc::RegionType type)
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        if (active_) {
            PrependRegionLocked(region, type);
            return true;
        }
        return false;
    }

    void ActivateRegionCache()
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        active_ = true;
    }

    void DeactivateRegionCache()
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        for (RegionDesc* node = listHead_; node != nullptr; node = node->GetNextRegion()) {
            node->ClearMarkingCopyLine();
        }
        active_ = false;
    }
private:
    bool active_ = false;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_LIST_H
