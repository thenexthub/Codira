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
#include "common_interfaces/base/common.h"
#ifdef _WIN64
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#endif

#include "common_components/base/immortal_wrapper.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/heap/collector/region_bitmap.h"
#include "common_components/heap/collector/copy_data_manager.h"
#include "common_components/platform/os.h"

namespace common {

static ImmortalWrapper<HeapBitmapManager> forwardDataManager;
HeapBitmapManager& HeapBitmapManager::GetHeapBitmapManager() { return *forwardDataManager; }

void HeapBitmapManager::InitializeHeapBitmap()
{
    DCHECK_CC(!initialized);
    size_t maxHeapBytes = Heap::GetHeap().GetMaxCapacity();
    size_t heapBitmapSize = RoundUp(GetHeapBitmapSize(maxHeapBytes), COMMON_PAGE_SIZE);
    allHeapBitmapSize_ = heapBitmapSize;

#ifdef _WIN64
    void* startAddress = VirtualAlloc(NULL, allHeapBitmapSize_, MEM_RESERVE, PAGE_READWRITE);
    if (startAddress == NULL) { //LCOV_EXCL_BR_LINE
        LOG_COMMON(FATAL) << "failed to initialize HeapBitmapManager";
        UNREACHABLE_CC();
    }
#else
    void* startAddress = mmap(nullptr, allHeapBitmapSize_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (startAddress == MAP_FAILED) { //LCOV_EXCL_BR_LINE
        LOG_COMMON(FATAL) << "failed to initialize HeapBitmapManager";
        UNREACHABLE_CC();
    } else {
#ifndef __APPLE__
        (void)madvise(startAddress, allHeapBitmapSize_, MADV_NOHUGEPAGE);
#endif
    }
#endif

    heapBitmapStart_ = reinterpret_cast<uintptr_t>(startAddress);
    heapBitmap_[0].InitializeMemory(heapBitmapStart_, heapBitmapSize, regionUnitCount_);

    os::PrctlSetVMA(startAddress, allHeapBitmapSize_, "ArkTS Heap CMCGC HeapBitMap");
    initialized = true;
}

void HeapBitmapManager::DestroyHeapBitmap()
{
#ifdef _WIN64
    if (!VirtualFree(reinterpret_cast<void*>(heapBitmapStart_), 0, MEM_RELEASE)) {
        LOG_COMMON(ERROR) << "VirtualFree error for HeapBitmapManager";
    }
#else
    if (munmap(reinterpret_cast<void*>(heapBitmapStart_), allHeapBitmapSize_) != 0) {
        LOG_COMMON(ERROR) << "munmap error for HeapBitmapManager";
    }
#endif
    initialized = false;
}

} // namespace common
