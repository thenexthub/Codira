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


#ifndef MRT_FORWARD_DATA_MANAGER_H
#define MRT_FORWARD_DATA_MANAGER_H

#include "Base/ImmortalWrapper.h"
#include "Heap/Heap.h"
#if defined(__linux__) || defined(hongmeng) || defined(__APPLE__)
#include <sys/mman.h>
#endif
#include "Base/Log.h"
#include "Base/LogFile.h"
#include "Base/MemUtils.h"
#include "Base/SysCall.h"
#include "LiveInfo.h"

#ifdef _WIN64
#include "Base/AtomicSpinLock.h"
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#else
#include <sys/mman.h>
#endif

namespace MapleRuntime {

class ForwardDataManager {
    class ForwardDataSpace {
    public:
        struct Zone {
            enum ZoneType : size_t {
                LIVE_INFO,
                BIT_MAP,
                TOTAL_NUM,
            };
            uintptr_t zoneStartAddress = 0;
            std::atomic<uintptr_t> zonePosition;
        };
        ForwardDataSpace() = default;
        void InitializeMemory(uintptr_t start, size_t sz, size_t unitCount)
        {
            startAddress = start;
            size = sz;
            InitZones(unitCount);
        }
        void InitZones(size_t unitCount)
        {
            uintptr_t start = startAddress;
            allocZone[Zone::ZoneType::LIVE_INFO].zoneStartAddress = start;
            allocZone[Zone::ZoneType::LIVE_INFO].zonePosition = start;
#if defined(_WIN64)
            lastCommitEndAddr[Zone::ZoneType::LIVE_INFO].store(start);
#endif
            start += unitCount * sizeof(LiveInfo);
            allocZone[Zone::ZoneType::BIT_MAP].zoneStartAddress = start;
            allocZone[Zone::ZoneType::BIT_MAP].zonePosition = start;
#if defined(_WIN64)
            lastCommitEndAddr[Zone::ZoneType::BIT_MAP].store(start);
#endif
        }
        uintptr_t Allocate(Zone::ZoneType type, size_t sz)
        {
#if defined(_WIN64)
            allocSpinLock.Lock();
            uintptr_t startAddr = allocZone[type].zonePosition.fetch_add(sz);
            uintptr_t endAddr = startAddr + sz;
            uintptr_t lastAddr = lastCommitEndAddr[type].load(std::memory_order_relaxed);
            if (endAddr <= lastAddr) {
                allocSpinLock.Unlock();
                return startAddr;
            }
            size_t pageSize = RoundUp(sz, MapleRuntime::MRT_PAGE_SIZE);
            CHECK_E(UNLIKELY(!VirtualAlloc(reinterpret_cast<void*>(lastAddr), pageSize, MEM_COMMIT, PAGE_READWRITE)),
                    "VirtualAlloc commit failed in Allocate, errno: %d", GetLastError());
            lastCommitEndAddr[type].store(lastAddr + pageSize);
            allocSpinLock.Unlock();
            return startAddr;
#else
            return allocZone[type].zonePosition.fetch_add(sz);
#endif
        }
        void ReleaseMemory()
        {
#if defined(_WIN64)
            CHECK_E(UNLIKELY(!VirtualFree(reinterpret_cast<void*>(startAddress), size, MEM_DECOMMIT)),
                    "VirtualFree failed in ReturnPage, errno: %s", GetLastError());
#elif defined(__APPLE__)
            MapleRuntime::MemorySet(startAddress, size, 0, size);
            void* ret = mmap(reinterpret_cast<void*>(startAddress), size,
                            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1 , 0);
            if (ret == MAP_FAILED) {
                LOG(RTLOG_ERROR, "forwarding fata mmap ixed failed");
            } else if (ret != reinterpret_cast<void*>(startAddress)) {
                LOG(RTLOG_ERROR, "mmap fixed at wrong addr %p -> %p", startAddress, ret);
            }
#else
            if (madvise(reinterpret_cast<void*>(startAddress), size, MADV_DONTNEED) == 0) {
                DLOG(REGION, "release forward-data @[%#zx+%zu, %#zx)", startAddress, size, startAddress + size);
            } else {
                MapleRuntime::MemorySet(startAddress, size, 0, size);
                DLOG(REGION, "clear forward-data @[%#zx+%zu, %#zx)", startAddress, size, startAddress + size);
            }
#endif
            for (size_t i = Zone::ZoneType::LIVE_INFO; i < Zone::ZoneType::TOTAL_NUM; ++i) {
                allocZone[i].zonePosition = allocZone[i].zoneStartAddress;
#if defined(_WIN64)
                lastCommitEndAddr[i].store(allocZone[i].zoneStartAddress);
#endif
            }
        }

        void UnbindPreviousLiveInfo();

    private:
        Zone allocZone[Zone::TOTAL_NUM];
        uintptr_t startAddress = 0;
        size_t size = 0;
#if defined(_WIN64)
        std::atomic<uintptr_t> lastCommitEndAddr[Zone::TOTAL_NUM];
        AtomicSpinLock allocSpinLock;
#endif
    };

public:
    ForwardDataManager() = default;
    ~ForwardDataManager()
    {
#ifdef _WIN64
        if (!VirtualFree(reinterpret_cast<void*>(forwardDataStart), 0, MEM_RELEASE)) {
            LOG(RTLOG_ERROR, "VirtualFree error for ForwardDataManager");
        }
#else
        if (munmap(reinterpret_cast<void*>(forwardDataStart), forwardDataSize) != 0) {
            LOG(RTLOG_ERROR, "munmap error for ForwardDataManager");
        }
#endif
    }

    static ForwardDataManager& GetForwardDataManager();

    void InitializeForwardData();

    void ClearPreviousForwardData() { liveInfoData[GetPreviousTagID()].ReleaseMemory(); }

    RegionBitmap* AllocateRegionBitmap(size_t regionSize)
    {
        uintptr_t addr = liveInfoData[currentTagID].Allocate(ForwardDataSpace::Zone::ZoneType::BIT_MAP,
                                                             RegionBitmap::GetRegionBitmapSize(regionSize));
        RegionBitmap* bitmap = reinterpret_cast<RegionBitmap*>(addr);
        CHECK(bitmap != nullptr);
        new (bitmap) RegionBitmap(regionSize);
        return bitmap;
    }

    LiveInfo* AllocateLiveInfo()
    {
        return reinterpret_cast<LiveInfo*>(
            liveInfoData[currentTagID].Allocate(ForwardDataSpace::Zone::ZoneType::LIVE_INFO, sizeof(LiveInfo)));
    }

    uint16_t GetPreviousTagID() const { return currentTagID ^ 1; }

    void SetTagID(uint16_t id) { currentTagID = id; }

    void UnbindPreviousLiveInfo() { liveInfoData[GetPreviousTagID()].UnbindPreviousLiveInfo(); }

private:
    size_t GetLiveInfoDataSize(size_t heapSize)
    {
        const size_t REGION_UNIT_SIZE = MapleRuntime::MRT_PAGE_SIZE; // must be equal to RegionInfo::UNIT_SIZE
        heapSize = RoundUp<size_t>(heapSize, REGION_UNIT_SIZE);
        size_t unitCnt = heapSize / REGION_UNIT_SIZE;
        regionUnitCount = unitCnt;
        // 64: bitmap 1 bit marks the 64 bits in region.
        constexpr uint8_t bitMarksSize = 64;
        // 3 bitmap for each region: markBitmap,resurrectBitmap, enqueueBitmap.
        constexpr uint8_t bitmapNum = 3;
        return unitCnt * sizeof(LiveInfo) +
            unitCnt * (sizeof(RegionBitmap) + (REGION_UNIT_SIZE / bitMarksSize)) * bitmapNum;
    }
    ForwardDataSpace liveInfoData[2];
    size_t regionUnitCount = 0;
    uintptr_t forwardDataStart = 0;
    size_t forwardDataSize = 0;
    uint16_t currentTagID = 0; // propagate from collector.
};
} // namespace MapleRuntime
#endif // MRT_FORWARD_DATA_MANAGER_H
