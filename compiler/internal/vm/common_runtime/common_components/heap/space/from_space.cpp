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

#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/heap/space/from_space.h"
#include "common_components/heap/space/old_space.h"
#include "common_components/heap/collector/collector_resources.h"
#include "common_components/taskpool/taskpool.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
void FromSpace::DumpRegionStats() const
{
    size_t fromRegions = fromRegionList_.GetRegionCount();
    size_t fromUnits = fromRegionList_.GetUnitCount();
    size_t fromSize = fromUnits * RegionDesc::UNIT_SIZE;
    size_t allocFromSize = fromRegionList_.GetAllocatedSize();

    size_t exemptedFromRegions = exemptedFromRegionList_.GetRegionCount();
    size_t exemptedFromUnits = exemptedFromRegionList_.GetUnitCount();
    size_t exemptedFromSize = exemptedFromUnits * RegionDesc::UNIT_SIZE;
    size_t allocExemptedFromSize = exemptedFromRegionList_.GetAllocatedSize();
    size_t units = fromUnits + exemptedFromUnits;

    VLOG(DEBUG, "\tfrom space units: %zu (%zu B)", units, units * RegionDesc::UNIT_SIZE);
    VLOG(DEBUG, "\t  from-regions %zu: %zu units (%zu B, alloc %zu)", fromRegions, fromUnits, fromSize, allocFromSize);
    VLOG(DEBUG, "\t  exempted from-regions %zu: %zu units (%zu B, alloc %zu)",
        exemptedFromRegions, exemptedFromUnits, exemptedFromSize, allocExemptedFromSize);
}

// forward only regions whose garbage bytes is greater than or equal to exemptedRegionThreshold.
void FromSpace::ExemptFromRegions()
{
    size_t forwardBytes = 0;
    size_t floatingGarbage = 0;
    size_t oldFromBytes = fromRegionList_.GetUnitCount() * RegionDesc::UNIT_SIZE;
    RegionDesc* fromRegion = fromRegionList_.GetHeadRegion();
    while (fromRegion != nullptr) {
        size_t threshold = static_cast<size_t>(exemptedRegionThreshold_ * fromRegion->GetRegionSize());
        size_t liveBytes = fromRegion->GetLiveByteCount();
        long rawPtrCnt = fromRegion->GetRawPointerObjectCount();
        if (liveBytes > threshold) { // ignore this region
            RegionDesc* del = fromRegion;
            DLOG(REGION, "region %p @0x%zx+%zu exempted by forwarding: %zu units, %u live bytes", del,
                del->GetRegionStart(), del->GetRegionAllocatedSize(),
                del->GetUnitCount(), del->GetLiveByteCount());

            fromRegion = fromRegion->GetNextRegion();
            if (fromRegionList_.TryDeleteRegion(del, RegionDesc::RegionType::FROM_REGION,
                                                RegionDesc::RegionType::EXEMPTED_FROM_REGION)) {
                ExemptFromRegion(del);
            }
            floatingGarbage += (del->GetRegionSize() - del->GetLiveByteCount());
        } else if (rawPtrCnt > 0) { // LCOV_EXCL_BR_LINE
            RegionDesc* del = fromRegion;
            DLOG(REGION, "region %p @0x%zx+%zu pinned by forwarding: %zu units, %u live bytes rawPtr cnt %u",
                del, del->GetRegionStart(), del->GetRegionAllocatedSize(),
                del->GetUnitCount(), del->GetLiveByteCount(), rawPtrCnt);

            fromRegion = fromRegion->GetNextRegion();
            if (fromRegionList_.TryDeleteRegion(del, RegionDesc::RegionType::FROM_REGION, // LCOV_EXCL_BR_LINE
                                                RegionDesc::RegionType::RAW_POINTER_REGION)) { // LCOV_EXCL_BR_LINE
                heap_.AddRawPointerRegion(del);
            }
            floatingGarbage += (del->GetRegionSize() - del->GetLiveByteCount());
        } else {
            forwardBytes += fromRegion->GetLiveByteCount();
            fromRegion = fromRegion->GetNextRegion();
        }
    }

    size_t newFromBytes = fromRegionList_.GetUnitCount() * RegionDesc::UNIT_SIZE;
    size_t exemptedFromBytes = exemptedFromRegionList_.GetUnitCount() * RegionDesc::UNIT_SIZE;
    VLOG(DEBUG, "exempt from-space: %zu B - %zu B -> %zu B, %zu B floating garbage, %zu B to forward",
         oldFromBytes, exemptedFromBytes, newFromBytes, floatingGarbage, forwardBytes);
}

class CopyTask : public Task {
public:
    CopyTask(int32_t id, FromSpace& fromSpace, RegionDesc* region, size_t regionCnt, TaskPackMonitor &monitor)
        : Task(id), fromSpace_(fromSpace), startRegion_(region), regionCount_(regionCnt), monitor_(monitor) {}

    ~CopyTask() override = default;

    bool Run([[maybe_unused]] uint32_t threadIndex) override
    {
        // set current thread as a gc thread.
        ThreadLocal::SetThreadType(ThreadType::GC_THREAD);
        fromSpace_.ParallelCopyFromRegions(startRegion_, regionCount_);
        monitor_.NotifyFinishOne();
        ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
        return true;
    }

private:
    FromSpace &fromSpace_;
    RegionDesc *startRegion_ {nullptr};
    size_t regionCount_;
    TaskPackMonitor &monitor_;
};

void FromSpace::ParallelCopyFromRegions(RegionDesc *startRegion, size_t regionCnt)
{
    RegionDesc *currentRegion = startRegion;
    for (size_t count = 0; (count < regionCnt) && currentRegion != nullptr; ++count) {
        RegionDesc *region = currentRegion;
        currentRegion = currentRegion->GetNextRegion();
        heap_.CopyRegion(region);
    }

    AllocationBuffer* allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY_CC(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions(); // clear thread local region for gc threads.
    }
}

void FromSpace::CopyFromRegions()
{
    // iterate each region in fromRegionList
    RegionDesc* fromRegion = fromRegionList_.GetHeadRegion();
    while (fromRegion != nullptr) {
        ASSERT_LOGF(fromRegion->IsValidRegion(), "region is not head when get head region of from region list");
        RegionDesc* region = fromRegion;
        fromRegion = fromRegion->GetNextRegion();
        heap_.CopyRegion(region);
    }

    VLOG(DEBUG, "forward %zu from-region units", fromRegionList_.GetUnitCount());

    AllocationBuffer* allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY_CC(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions(); // clear region for next GC
    }
}

void FromSpace::CopyFromRegions(Taskpool* threadPool)
{
    if (threadPool != nullptr) {
        uint32_t parallel = Heap::GetHeap().GetCollectorResources().GetGCThreadCount(true) - 1;
        uint32_t threadNum = parallel + 1;
        // We won't change fromRegionList during gc, so we can use it without lock.
        size_t totalRegionCount = fromRegionList_.GetRegionCount();
        if (UNLIKELY_CC(totalRegionCount == 0)) {
            return;
        }
        size_t regionCntEachTask = totalRegionCount / static_cast<size_t>(threadNum);
        size_t leftRegionCnt = totalRegionCount - regionCntEachTask * parallel;
        RegionDesc* region = fromRegionList_.GetHeadRegion();
        TaskPackMonitor monitor(parallel, parallel);
        for (uint32_t i = 0; i < parallel; ++i) {
            ASSERT_LOGF(region != nullptr, "from region list records wrong region info");
            RegionDesc* startRegion = region;
            for (size_t count = 0; count < regionCntEachTask; ++count) {
                region = region->GetNextRegion();
            }
            threadPool->PostTask(std::make_unique<CopyTask>(0, *this, startRegion, regionCntEachTask, monitor));
        }
        ParallelCopyFromRegions(region, leftRegionCnt);
        monitor.WaitAllFinished();
    } else {
        CopyFromRegions();
    }
}

void FromSpace::GetPromotedTo(OldSpace& mspace)
{
    mspace.PromoteRegionList(exemptedFromRegionList_);
}
} // namespace common
