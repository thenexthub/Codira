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
#include "libarkbase/mem/mem_pool.h"
#include "runtime/mem/region_space-inl.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace ark::mem {

uint32_t Region::GetAllocatedBytes() const
{
    if (!IsTLAB() || IsMixedTLAB()) {
        return top_ - begin_;
    }
    uint32_t allocatedBytes = 0;
    ASSERT(tlabVector_ != nullptr);
    for (auto i : *tlabVector_) {
        allocatedBytes += i->GetOccupiedSize();
    }
    return allocatedBytes;
}

double Region::GetFragmentation() const
{
    ASSERT(Size() >= GetAllocatedBytes());
    return static_cast<double>(Size() - GetAllocatedBytes()) / Size();
}

bool Region::IsInRange(const ObjectHeader *object) const
{
    return ToUintPtr(object) >= begin_ && ToUintPtr(object) < end_;
}

bool Region::IsInAllocRange(const ObjectHeader *object) const
{
    bool inRange = false;
    if (!IsTLAB()) {
        inRange = (ToUintPtr(object) >= begin_ && ToUintPtr(object) < top_);
    } else {
        for (auto i : *tlabVector_) {
            inRange = i->ContainObject(object);
            if (inRange) {
                break;
            }
        }
        if (IsMixedTLAB() && !inRange) {
            inRange = (ToUintPtr(object) >= ToUintPtr(tlabVector_->back()->GetEndAddr()) && ToUintPtr(object) < top_);
        }
    }
    return inRange;
}

InternalAllocatorPtr Region::GetInternalAllocator()
{
    return space_->GetPool()->GetInternalAllocator();
}

void Region::CreateRemSet()
{
    ASSERT(remSet_ == nullptr);
    remSet_ = GetInternalAllocator()->New<RemSetT>();
}

void Region::SetupAtomics()
{
    liveBytes_ = GetInternalAllocator()->New<std::atomic<uint32_t>>();
    pinnedObjects_ = GetInternalAllocator()->New<std::atomic<uint32_t>>();
}

void Region::CreateTLABSupport()
{
    ASSERT(tlabVector_ == nullptr);
    tlabVector_ = GetInternalAllocator()->New<PandaVector<TLAB *>>(GetInternalAllocator()->Adapter());
}

size_t Region::GetRemainingSizeForTLABs() const
{
    ASSERT(IsTLAB());
    ASSERT(!IsMixedTLAB());
    // TLABs are stored one by one.
    uintptr_t lastTlabEndByte = tlabVector_->empty() ? Top() : ToUintPtr(tlabVector_->back()->GetEndAddr());
    ASSERT((lastTlabEndByte <= End()) && (lastTlabEndByte >= Top()));
    return End() - lastTlabEndByte;
}

TLAB *Region::CreateTLAB(size_t size)
{
    ASSERT(IsTLAB());
    ASSERT(!IsMixedTLAB());
    ASSERT(Begin() != 0);
    ASSERT(Top() == Begin());
    size_t remainingSize = GetRemainingSizeForTLABs();
    if (remainingSize < size) {
        return nullptr;
    }
    ASSERT(End() > remainingSize);
    TLAB *tlab = GetInternalAllocator()->New<TLAB>(ToVoidPtr(End() - remainingSize), size);
    tlab->SetZeroedFlag(HasFlag(RegionFlag::IS_ZEROED));
    tlabVector_->push_back(tlab);
    return tlab;
}

MarkBitmap *Region::CreateMarkBitmap()
{
    ASSERT(markBitmap_ == nullptr);
    auto allocator = GetInternalAllocator();
    auto bitmapData = allocator->Alloc(MarkBitmap::GetBitMapSizeInByte(Size()));
    ASSERT(bitmapData != nullptr);
    markBitmap_ = allocator->New<MarkBitmap>(this, Size(), bitmapData);
    ASSERT(markBitmap_ != nullptr);
    markBitmap_->ClearAllBits();
    return markBitmap_;
}

MarkBitmap *Region::CreateLiveBitmap()
{
    ASSERT(liveBitmap_ == nullptr);
    auto allocator = GetInternalAllocator();
    auto bitmapData = allocator->Alloc(MarkBitmap::GetBitMapSizeInByte(Size()));
    ASSERT(bitmapData != nullptr);
    liveBitmap_ = allocator->New<MarkBitmap>(this, Size(), bitmapData);
    ASSERT(liveBitmap_ != nullptr);
    liveBitmap_->ClearAllBits();
    return liveBitmap_;
}

void Region::SwapMarkBitmap()
{
    ASSERT(liveBitmap_ != nullptr);
    ASSERT(markBitmap_ != nullptr);
    std::swap(liveBitmap_, markBitmap_);
}

void Region::CloneMarkBitmapToLiveBitmap()
{
    ASSERT(liveBitmap_ != nullptr);
    ASSERT(markBitmap_ != nullptr);
    markBitmap_->CopyTo(liveBitmap_);
}

void Region::SetMarkBit(ObjectHeader *object)
{
    ASSERT(IsInRange(object));
    markBitmap_->Set(object);
}

uint32_t Region::CalcLiveBytes() const
{
    ASSERT(liveBitmap_ != nullptr);
    uint32_t liveBytes = 0;
    liveBitmap_->IterateOverMarkedChunks<true>(
        [&liveBytes](const void *object) { liveBytes += GetAlignedObjectSize(GetObjectSize(object)); });
    return liveBytes;
}

uint32_t Region::CalcMarkBytes() const
{
    ASSERT(markBitmap_ != nullptr);
    uint32_t liveBytes = 0;
    markBitmap_->IterateOverMarkedChunks(
        [&liveBytes](const void *object) { liveBytes += GetAlignedObjectSize(GetObjectSize(object)); });
    return liveBytes;
}

void Region::Destroy()
{
    auto allocator = GetInternalAllocator();
    if (remSet_ != nullptr) {
        allocator->Delete(remSet_);
        remSet_ = nullptr;
    }
    if (liveBytes_ != nullptr) {
        allocator->Delete(liveBytes_);
        liveBytes_ = nullptr;
    }
    if (pinnedObjects_ != nullptr) {
        allocator->Delete(pinnedObjects_);
        pinnedObjects_ = nullptr;
    }
    if (tlabVector_ != nullptr) {
        for (auto i : *tlabVector_) {
            allocator->Delete(i);
        }
        allocator->Delete(tlabVector_);
        tlabVector_ = nullptr;
    }
    if (liveBitmap_ != nullptr) {
        allocator->Delete(liveBitmap_->GetBitMap().data());
        allocator->Delete(liveBitmap_);
        liveBitmap_ = nullptr;
    }
    if (markBitmap_ != nullptr) {
        allocator->Delete(markBitmap_->GetBitMap().data());
        allocator->Delete(markBitmap_);
        markBitmap_ = nullptr;
    }
}

void RegionBlock::Init(uintptr_t regionsBegin, uintptr_t regionsEnd)
{
    os::memory::LockHolder lock(lock_);
    ASSERT(occupied_.Empty());
    ASSERT(Region::IsAlignment(regionsBegin, regionSize_));
    ASSERT((regionsEnd - regionsBegin) % regionSize_ == 0);
    size_t numRegions = (regionsEnd - regionsBegin) / regionSize_;
    if (numRegions > 0) {
        size_t size = numRegions * sizeof(Region *);
        auto data = reinterpret_cast<Region **>(allocator_->Alloc(size));
        memset_s(data, size, 0, size);
        occupied_ = Span<Region *>(data, numRegions);
        regionsBegin_ = regionsBegin;
        regionsEnd_ = regionsEnd;
    }
}

Region *RegionBlock::AllocRegion()
{
    os::memory::LockHolder lock(lock_);
    // NOTE(yxr) : find a unused region, improve it
    for (size_t i = 0; i < occupied_.Size(); ++i) {
        if (occupied_[i] == nullptr) {
            auto *region = RegionAt(i);
            occupied_[i] = region;
            numUsedRegions_++;
            return region;
        }
    }
    return nullptr;
}

Region *RegionBlock::AllocLargeRegion(size_t largeRegionSize)
{
    os::memory::LockHolder lock(lock_);
    // NOTE(yxr) : search continuous unused regions, improve it
    size_t allocRegionNum = largeRegionSize / regionSize_;
    size_t left = 0;
    while (left + allocRegionNum <= occupied_.Size()) {
        bool found = true;
        size_t right = left;
        while (right < left + allocRegionNum) {
            if (occupied_[right] != nullptr) {
                found = false;
                break;
            }
            ++right;
        }
        if (found) {
            // mark those regions as 'used'
            auto *region = RegionAt(left);
            for (size_t i = 0; i < allocRegionNum; i++) {
                occupied_[left + i] = region;
            }
            numUsedRegions_ += allocRegionNum;
            return region;
        }
        // next round
        left = right + 1;
    }
    return nullptr;
}

void RegionBlock::FreeRegion(Region *region, bool releasePages)
{
    os::memory::LockHolder lock(lock_);
    size_t regionIdx = RegionIndex(region);
    size_t regionNum = region->Size() / regionSize_;
    ASSERT(regionIdx + regionNum <= occupied_.Size());
    for (size_t i = 0; i < regionNum; i++) {
        ASSERT(occupied_[regionIdx + i] == region);
        occupied_[regionIdx + i] = nullptr;
    }
    numUsedRegions_ -= regionNum;
    if (releasePages) {
        os::mem::ReleasePages(ToUintPtr(region), region->End());
    }
}

Region *RegionPool::NewRegion(RegionSpace *space, SpaceType spaceType, AllocatorType allocatorType, size_t regionSize,
                              RegionFlag edenOrOldOrNonmovable, RegionFlag properties, OSPagesAllocPolicy allocPolicy)
{
    // check that the input region_size is aligned
    ASSERT(regionSize % regionSize_ == 0);
    ASSERT(IsYoungRegionFlag(edenOrOldOrNonmovable) || edenOrOldOrNonmovable == RegionFlag::IS_OLD ||
           edenOrOldOrNonmovable == RegionFlag::IS_NONMOVABLE);

    // Ensure leaving enough space so there's always some free regions in heap which we can use for full gc
    if (edenOrOldOrNonmovable == RegionFlag::IS_NONMOVABLE || regionSize > regionSize_) {
        if (!spaces_->CanAllocInSpace(false, regionSize + regionSize_)) {
            return nullptr;
        }
    }

    if (!spaces_->CanAllocInSpace(IsYoungRegionFlag(edenOrOldOrNonmovable), regionSize)) {
        return nullptr;
    }

    // 1.get region from pre-allocated region block(e.g. a big mmaped continuous space)
    void *region = nullptr;
    RegionFlag zeroed = RegionFlag::IS_UNUSED;
    if (block_.GetFreeRegionsNum() > 0) {
        region = (regionSize <= regionSize_) ? block_.AllocRegion() : block_.AllocLargeRegion(regionSize);
    }
    if (region != nullptr) {
        IsYoungRegionFlag(edenOrOldOrNonmovable) ? spaces_->IncreaseYoungOccupiedInSharedPool(regionSize)
                                                 : spaces_->IncreaseTenuredOccupiedInSharedPool(regionSize);
    } else if (extend_) {  // 2.mmap region directly, this is more flexible for memory usage
        Pool pool(0, nullptr);

        if (IsYoungRegionFlag(edenOrOldOrNonmovable)) {
            pool = spaces_->TryAllocPoolForYoung(regionSize, spaceType, allocatorType, this);
        } else {
            pool = spaces_->TryAllocPoolForTenured(regionSize, spaceType, allocatorType, this, allocPolicy);
        }

        if (pool.IsZeroed()) {
            zeroed = RegionFlag::IS_ZEROED;
        }

        region = pool.GetMem();
    }

    if (UNLIKELY(region == nullptr)) {
        return nullptr;
    }
    return NewRegion(region, space, regionSize, edenOrOldOrNonmovable, properties, zeroed);
}

Region *RegionPool::NewRegion(void *region, RegionSpace *space, size_t regionSize, RegionFlag edenOrOldOrNonmovable,
                              RegionFlag properties, RegionFlag zeroed)
{
    ASSERT(Region::IsAlignment(ToUintPtr(region), regionSize_));

    ASAN_UNPOISON_MEMORY_REGION(region, Region::HeadSize());
    auto *ret = new (region) Region(space, ToUintPtr(region) + Region::HeadSize(), ToUintPtr(region) + regionSize);
    // NOTE(dtrubenkov): remove this fast fixup
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    ret->AddFlag(edenOrOldOrNonmovable);
    ret->AddFlag(properties);
    // Don't add zeroed flag for the old region during GC, because GC copies object without memory zeroing,
    // but can "touch" memory that can be available after GC
    if (edenOrOldOrNonmovable != RegionFlag::IS_OLD || properties != RegionFlag::IS_UNUSED) {
        ret->AddFlag(zeroed);
    }
    ret->CreateRemSet();
    ret->SetupAtomics();
    ret->CreateMarkBitmap();
    if (!IsYoungRegionFlag(edenOrOldOrNonmovable)) {
        ret->CreateLiveBitmap();
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    return ret;
}

void RegionPool::PromoteYoungRegion(Region *region)
{
    ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
    if (block_.IsAddrInRange(region)) {
        spaces_->ReduceYoungOccupiedInSharedPool(region->Size());
        spaces_->IncreaseTenuredOccupiedInSharedPool(region->Size());
    } else {
        spaces_->PromoteYoungPool(region->Size());
    }
    // Change region type
    region->AddFlag(RegionFlag::IS_PROMOTED);
    region->RmvFlag(RegionFlag::IS_EDEN);
    region->AddFlag(RegionFlag::IS_OLD);
}

bool RegionPool::HaveTenuredSize(size_t size) const
{
    return spaces_->CanAllocInSpace(GenerationalSpaces::IS_TENURED_SPACE, size);
}

bool RegionPool::HaveFreeRegions(size_t numRegions, size_t regionSize) const
{
    if (block_.GetFreeRegionsNum() >= numRegions) {
        return true;
    }
    numRegions -= block_.GetFreeRegionsNum();
    return PoolManager::GetMmapMemPool()->HaveEnoughPoolsInObjectSpace(numRegions, regionSize);
}

Region *RegionSpace::NewRegion(size_t regionSize, RegionFlag edenOrOldOrNonmovable, RegionFlag properties,
                               OSPagesAllocPolicy allocPolicy)
{
    Region *region = nullptr;
    auto youngRegionFlag = IsYoungRegionFlag(edenOrOldOrNonmovable);
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    if (youngRegionFlag && youngRegionsInUse_.load(std::memory_order_relaxed) >= desiredEdenLength_) {
        return nullptr;
    }
    if (youngRegionFlag && (!emptyYoungRegions_.empty())) {
        region = GetRegionFromEmptyList(emptyYoungRegions_);
        ASAN_UNPOISON_MEMORY_REGION(region, Region::HeadSize());
        ASSERT(regionSize == region->Size());
        regionPool_->NewRegion(region, this, regionSize, edenOrOldOrNonmovable, properties);
    } else if (!youngRegionFlag && (!emptyTenuredRegions_.empty())) {
        region = GetRegionFromEmptyList(emptyTenuredRegions_);
        ASAN_UNPOISON_MEMORY_REGION(region, Region::HeadSize());
        ASSERT(regionSize == region->Size());
        regionPool_->NewRegion(region, this, regionSize, edenOrOldOrNonmovable, properties);
    } else {
        region = regionPool_->NewRegion(this, spaceType_, allocatorType_, regionSize, edenOrOldOrNonmovable, properties,
                                        allocPolicy);
    }
    if (UNLIKELY(region == nullptr)) {
        return nullptr;
    }
    ASAN_POISON_MEMORY_REGION(ToVoidPtr(region->Begin()), region->End() - region->Begin());
    regions_.push_back(region->AsListNode());
    if (youngRegionFlag) {
        // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
        // on other reads or writes
        youngRegionsInUse_.fetch_add(1, std::memory_order_relaxed);
    }
    return region;
}

void RegionSpace::PromoteYoungRegion(Region *region)
{
    ASSERT(region->GetSpace() == this);
    ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
    if (region->IsTLAB()) {
        region->AddFlag(RegionFlag::IS_MIXEDTLAB);
        region->SetTop(ToUintPtr(region->GetLastTLAB()->GetEndAddr()));
    }
    regionPool_->PromoteYoungRegion(region);
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    [[maybe_unused]] auto previousRegionsInUse = youngRegionsInUse_.fetch_sub(1, std::memory_order_relaxed);
    ASSERT(previousRegionsInUse > 0);
}

void RegionSpace::FreeAllRegions()
{
    // delete all regions
    IterateRegions([this](Region *region) { FreeRegion(region); });
    ReleaseEmptyRegions<RegionFlag::IS_EDEN, OSPagesPolicy::IMMEDIATE_RETURN>();
    ReleaseEmptyRegions<RegionFlag::IS_OLD, OSPagesPolicy::IMMEDIATE_RETURN>();
}

Region *RegionSpace::GetRegionFromEmptyList(DList &regionList)
{
    Region *region = Region::AsRegion(&(*regionList.begin()));
    regionList.erase(regionList.begin());
    ASSERT(region != nullptr);
    return region;
}

}  // namespace ark::mem
