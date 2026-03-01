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


#ifndef MRT_REGION_LIST_H
#define MRT_REGION_LIST_H

#include "RegionInfo.h"

namespace MapleRuntime {
class RegionList {
public:
    friend void RemoveRegionLocked(RegionList*, RegionInfo*);
    RegionList(const char* name) : listName(name) {}

    void PrependRegion(RegionInfo* region, RegionInfo::RegionType type);
    void PrependRegionLocked(RegionInfo* region, RegionInfo::RegionType type);

    void MergeRegionList(RegionList& regionList, RegionInfo::RegionType regionType);

    void DeleteRegion(RegionInfo* del)
    {
        if (del == nullptr) {
            return;
        }

        std::lock_guard<std::mutex> lock(listMutex);
        DeleteRegionLocked(del);
    }

    bool TryDeleteRegion(RegionInfo* del, RegionInfo::RegionType oldType, RegionInfo::RegionType newType)
    {
        if (del == nullptr) {
            return false;
        }

        CHECK(oldType != newType);
        std::lock_guard<std::mutex> lock(listMutex);
        if (del->GetRegionType() == oldType) {
            DeleteRegionLocked(del);
            del->SetRegionType(newType);
            return true;
        }
        return false;
    }

#ifdef MRT_DEBUG
    void DumpRegionList(const char*);
#endif

    void DecCounts(size_t nRegion, size_t nUnit)
    {
        if (regionCount >= nRegion && unitCount >= nUnit) {
            regionCount -= nRegion;
            unitCount -= nUnit;
        } else {
            LOG(RTLOG_FATAL, "region list %p error count %zu-%zu %zu-%zu", regionCount, nRegion, unitCount, nUnit);
        }
    }

    void IncCounts(size_t nRegion, size_t nUnit)
    {
        CHECK((nRegion <= std::numeric_limits<size_t>::max() - regionCount) &&
              (nUnit <= std::numeric_limits<size_t>::max() - unitCount));
        regionCount += nRegion;
        unitCount += nUnit;
    }

    RegionInfo* GetHeadRegion() const { return listHead; }

    void ClearList()
    {
        listHead = nullptr;
        listTail = nullptr;
        regionCount = 0;
        unitCount = 0;
    }

    RegionInfo* GetTailRegion() { return listTail; }

    RegionInfo* TakeHeadRegion()
    {
        std::lock_guard<std::mutex> lg(listMutex);
        if (listHead == nullptr) { return nullptr; }
        RegionInfo* currentHead = listHead;
        DeleteRegionLocked(currentHead);
        return currentHead;
    }

    RegionInfo* TakeHeadRegion(RegionInfo::RegionType newType)
    {
        std::lock_guard<std::mutex> lg(listMutex);
        if (listHead == nullptr) { return nullptr; }
        RegionInfo* currentHead = listHead;
        DeleteRegionLocked(currentHead);
        currentHead->SetRegionType(newType);
        return currentHead;
    }

    size_t GetUnitCount() const { return unitCount; }

    size_t GetRegionCount() const { return regionCount; }

    size_t GetAllocatedSize(bool count = false) const
    {
        if (!count) {
            return GetUnitCount() * RegionInfo::UNIT_SIZE;
        }
        return CountAllocatedSize();
    }

    void VisitAllRegions(const std::function<void(RegionInfo*)>& visitor)
    {
        std::lock_guard<std::mutex> lock(listMutex);
        RegionInfo* node = listHead;
        RegionInfo* next = node;
        while (node != nullptr) {
            next = node->GetNextRegion();
            visitor(node);
            node = next;
        }
    }

    void VisitAllGhostRegions(const std::function<void(RegionInfo*)>& visitor)
    {
        for (RegionInfo* node = listHead; node != nullptr; node = node->GetNextGhostRegion()) {
            visitor(node);
        }
    }

    void SetElementType(RegionInfo::RegionType type)
    {
        std::lock_guard<std::mutex> lock(listMutex);
        for (RegionInfo* node = listHead; node != nullptr; node = node->GetNextRegion()) {
            node->SetRegionType(type);
        }
    }

    void ClearTraceRegionFlag()
    {
        std::lock_guard<std::mutex> lock(listMutex);
        for (RegionInfo *node = listHead; node != nullptr; node = node->GetNextRegion()) {
            node->SetTraceRegionFlag(0);
        }
    }

    std::mutex& GetListMutex() { return listMutex; }

    void MoveTo(RegionList& targetList)
    {
        std::lock_guard<std::mutex> lock(listMutex);
        targetList.AssignWith(*this);
        this->ClearList();
    }

    void CopyListTo(RegionList& dstList)
    {
        std::lock_guard<std::mutex> lock(listMutex);
        dstList.listHead = this->listHead;
        dstList.listTail = this->listTail;
        dstList.regionCount = this->regionCount;
        dstList.unitCount = this->unitCount;
    }

protected:
    std::mutex listMutex;
    size_t regionCount = 0;
    size_t unitCount = 0;
    RegionInfo* listHead = nullptr; // the start region for iteration, i.e., the first region
    RegionInfo* listTail = nullptr; // help to merge region list
    const char* listName = nullptr;
private:
    void DeleteRegionLocked(RegionInfo* del);

    void AssignWith(const RegionList& srcList)
    {
        std::lock_guard<std::mutex> lock(listMutex);
        listHead = srcList.listHead;
        listTail = srcList.listTail;
        regionCount = srcList.regionCount;
        unitCount = srcList.unitCount;
    }

    // allocated-size of to-region list must be calculated on the fly.
    size_t CountAllocatedSize() const
    {
        size_t allocCnt = 0;
        std::lock_guard<std::mutex> lock(const_cast<RegionList*>(this)->listMutex);
        for (RegionInfo* region = listHead; region != nullptr; region = region->GetNextRegion()) {
            allocCnt += region->GetRegionAllocatedSize();
        }
        return allocCnt;
    }

#ifdef MRT_DEBUG
    void VerifyRegion(RegionInfo* region)
    {
        RegionInfo* prev = region->GetPrevRegion();
        RegionInfo* next = region->GetNextRegion();
        if (prev != nullptr && prev->GetNextRegion() != region) {
            LOG(RTLOG_FATAL, "illegal region node");
        }

        if (next != nullptr && next->GetPrevRegion() != region) {
            LOG(RTLOG_FATAL, "illegal region node");
        }
    }
#endif
};

class RegionCache : public RegionList {
public:
    RegionCache(const char* name) : RegionList(name) {}

    bool TryPrependRegion(RegionInfo *region, RegionInfo::RegionType type)
    {
        std::lock_guard<std::mutex> lock(listMutex);
        if (active) {
            PrependRegionLocked(region, type);
            return true;
        }
        return false;
    }

    void ActivateRegionCache()
    {
        std::lock_guard<std::mutex> lock(listMutex);
        active = true;
    }

    void DeactivateRegionCache()
    {
        std::lock_guard<std::mutex> lock(listMutex);
        for (RegionInfo *node = listHead; node != nullptr; node = node->GetNextRegion()) {
            node->SetTraceRegionFlag(0);
        }
        active = false;
    }
private:
    bool active = false;
};
} // namespace MapleRuntime
#endif // MRT_REGION_LIST_H
