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

#ifndef COMMON_COMPONENTS_HEAP_COLLECTOR_COPY_DATA_MANAGER_H
#define COMMON_COMPONENTS_HEAP_COLLECTOR_COPY_DATA_MANAGER_H

#include "common_interfaces/base/common.h"
#include "common_components/base/immortal_wrapper.h"
#include "common_components/heap/heap.h"
#if defined(__linux__) || defined(PANDA_TARGET_OHOS) || defined(__APPLE__)
#include <sys/mman.h>
#endif

#include "common_components/base/mem_utils.h"
#include "common_components/base/sys_call.h"
#include "common_components/heap/collector/region_bitmap.h"
#include "common_components/log/log.h"

#ifdef _WIN64
#include "common_components/base/atomic_spin_lock.h"
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#else
#include <sys/mman.h>
#endif

namespace common {

class HeapBitmapManager {
    class Memory {
    public:
        struct Zone {
            enum ZoneType : size_t {
                BIT_MAP,
                ZONE_TYPE_CNT,
            };
            uintptr_t zoneStartAddress = 0;
            std::atomic<uintptr_t> zonePosition;
        };
        Memory() = default;
        void InitializeMemory(uintptr_t start, size_t sz, size_t unitCount)
        {
            startAddress_ = start;
            size_ = sz;
            InitZones(unitCount);
        }
        void InitZones(size_t unitCount)
        {
            uintptr_t start = startAddress_;
            allocZone_[Zone::ZoneType::BIT_MAP].zoneStartAddress = start;
            allocZone_[Zone::ZoneType::BIT_MAP].zonePosition = start;
#if defined(_WIN64)
            lastCommitEndAddr[Zone::ZoneType::BIT_MAP].store(start);
#endif
        }
        uintptr_t Allocate(Zone::ZoneType type, size_t sz)
        {
#if defined(_WIN64)
            allocSpinLock.Lock();
            uintptr_t startAddr = allocZone_[type].zonePosition.fetch_add(sz);
            uintptr_t endAddr = startAddr + sz;
            uintptr_t lastAddr = lastCommitEndAddr[type].load(std::memory_order_relaxed);
            if (endAddr <= lastAddr) {
                allocSpinLock.Unlock();
                return startAddr;
            }
            size_t pageSize = RoundUp(sz, COMMON_PAGE_SIZE);
            LOGE_IF(UNLIKELY_CC(!VirtualAlloc(reinterpret_cast<void*>(lastAddr), pageSize, MEM_COMMIT,
                PAGE_READWRITE))) << "VirtualAlloc commit failed in Allocate, errno: " << GetLastError();
            lastCommitEndAddr[type].store(lastAddr + pageSize);
            allocSpinLock.Unlock();
            return startAddr;
#else
            auto address = allocZone_[type].zonePosition.fetch_add(sz);
            MemorySet(address, sz, 0, sz);
            return address;
#endif
        }
        void ReleaseMemory()
        {
#if defined(_WIN64)
            LOGE_IF(UNLIKELY_CC(!VirtualFree(reinterpret_cast<void*>(startAddress_), size_, MEM_DECOMMIT))) <<
                "VirtualFree failed in ReturnPage, errno: " << GetLastError();
#elif defined(__APPLE__)
            (void)madvise(reinterpret_cast<void*>(startAddress_), size_, MADV_DONTNEED);
#else
            DLOG(REGION, "clear copy-data @[%#zx+%zu, %#zx)", startAddress_, size_, startAddress_ + size_);
            if (madvise(reinterpret_cast<void*>(startAddress_), size_, MADV_DONTNEED) == 0) {
                DLOG(REGION, "release copy-data @[%#zx+%zu, %#zx)", startAddress_, size_, startAddress_ + size_);
            }
#endif
            for (size_t i = 0; i < Zone::ZoneType::ZONE_TYPE_CNT; ++i) {
                allocZone_[i].zonePosition = allocZone_[i].zoneStartAddress;
#if defined(_WIN64)
                lastCommitEndAddr[i].store(allocZone_[i].zoneStartAddress);
#endif
            }
        }

    private:
        Zone allocZone_[Zone::ZONE_TYPE_CNT];
        uintptr_t startAddress_ = 0;
        size_t size_ = 0;
#if defined(_WIN64)
        std::atomic<uintptr_t> lastCommitEndAddr[Zone::ZONE_TYPE_CNT];
        AtomicSpinLock allocSpinLock;
#endif
    };

public:
    HeapBitmapManager() = default;
    ~HeapBitmapManager()
    {
        CHECK_CC(!initialized);
    }

    static HeapBitmapManager& GetHeapBitmapManager();

    void InitializeHeapBitmap();
    void DestroyHeapBitmap();

    void ClearHeapBitmap() { heapBitmap_[0].ReleaseMemory(); }

    RegionBitmap* AllocateRegionBitmap(size_t regionSize)
    {
        uintptr_t addr =
            heapBitmap_[0].Allocate(Memory::Zone::ZoneType::BIT_MAP, RegionBitmap::GetRegionBitmapSize(regionSize));
        RegionBitmap* bitmap = reinterpret_cast<RegionBitmap*>(addr);
        CHECK_CC(bitmap != nullptr);
        new (bitmap) RegionBitmap(regionSize);
        return bitmap;
    }

private:
    size_t GetHeapBitmapSize(size_t heapSize)
    {
        const size_t REGION_UNIT_SIZE = COMMON_PAGE_SIZE; // must be equal to RegionDesc::UNIT_SIZE
        heapSize = RoundUp<size_t>(heapSize, REGION_UNIT_SIZE);
        size_t unitCnt = heapSize / REGION_UNIT_SIZE;
        regionUnitCount_ = unitCnt;
        // 64: 1 bit in bitmap marks 8bytes in region, i.e., 1 byte in bitmap marks 64 bytes in region.
        constexpr uint8_t bitMarksSize = 64;
        // 3 bitmaps for each region: markBitmap,resurrectBitmap, enqueueBitmap.
        constexpr uint8_t nBitmapTypes = 3;
        return unitCnt * (sizeof(RegionBitmap) + (REGION_UNIT_SIZE / bitMarksSize)) * nBitmapTypes;
    }

    Memory heapBitmap_[1];
    size_t regionUnitCount_ = 0;
    uintptr_t heapBitmapStart_ = 0;
    size_t allHeapBitmapSize_ = 0;
    bool initialized = false;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_COLLECTOR_COPY_DATA_MANAGER_H
