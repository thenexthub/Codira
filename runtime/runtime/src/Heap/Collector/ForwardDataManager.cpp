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


#ifdef _WIN64
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#endif

#include "Base/ImmortalWrapper.h"
#include "Heap/Allocator/RegionInfo.h"
#include "LiveInfo.h"
#include "ForwardDataManager.h"

namespace MapleRuntime {

static ImmortalWrapper<ForwardDataManager> forwardDataManager;
ForwardDataManager& ForwardDataManager::GetForwardDataManager() { return *forwardDataManager; }

void ForwardDataManager::InitializeForwardData()
{
    size_t maxHeapBytes = Heap::GetHeap().GetMaxCapacity();
    size_t liveInfoSize = RoundUp(GetLiveInfoDataSize(maxHeapBytes), MapleRuntime::MRT_PAGE_SIZE);
    // 2: forwadData is the twice size of liveInfo
    forwardDataSize = liveInfoSize * 2;

#ifdef _WIN64
    void* startAddress = VirtualAlloc(NULL, forwardDataSize, MEM_RESERVE, PAGE_READWRITE);
    if (startAddress == NULL) {
        LOG(RTLOG_FATAL, "failed to initialize ForwardDataManager");
    }
#else
    void* startAddress = mmap(nullptr, forwardDataSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (startAddress == MAP_FAILED) {
        LOG(RTLOG_FATAL, "failed to initialize ForwardDataManager");
    } else {
#ifndef __APPLE__
        (void)madvise(startAddress, forwardDataSize, MADV_NOHUGEPAGE);
        MRT_PRCTL(startAddress, forwardDataSize, "forward_data");
#endif
    }
#endif

    forwardDataStart = reinterpret_cast<uintptr_t>(startAddress);
    liveInfoData[0].InitializeMemory(forwardDataStart, liveInfoSize, regionUnitCount);
    liveInfoData[1].InitializeMemory(forwardDataStart + liveInfoSize, liveInfoSize, regionUnitCount);
}
void ForwardDataManager::ForwardDataSpace::UnbindPreviousLiveInfo()
{
    auto& zone = allocZone[ForwardDataSpace::Zone::ZoneType::LIVE_INFO];
    size_t start = zone.zoneStartAddress;
    size_t pos = zone.zonePosition.load();
    for (size_t current = start; current < pos; current += sizeof(LiveInfo)) {
        LiveInfo* currentLiveInfo = reinterpret_cast<LiveInfo*>(current);
        RegionInfo* bindedRegion = currentLiveInfo->bindedRegion;
        CHECK(bindedRegion != nullptr);
        bindedRegion->CheckAndClearLiveInfo(currentLiveInfo);
    }
}
} // namespace MapleRuntime
