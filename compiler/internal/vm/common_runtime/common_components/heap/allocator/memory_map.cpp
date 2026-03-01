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
#include "common_components/heap/allocator/memory_map.h"

#include <algorithm>
#include "base/globals.h"
#ifdef _WIN64
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#endif

#include "common_components/platform/os.h"
#include "common_components/common_runtime/hooks.h"
#include "common_components/base/sys_call.h"
#include "common_components/log/log.h"

#include "securec.h"

namespace common {
using namespace std;

// not thread safe, do not call from multiple threads
MemoryMap* MemoryMap::MapMemory(size_t reqSize, size_t initSize, const Option& opt)
{
    void* mappedAddr = nullptr;
    reqSize = AllocUtilRndUp<size_t>(reqSize, ALLOC_UTIL_PAGE_SIZE);

#ifdef _WIN64
    // Windows don`t support to map a non-access memory, and virtualProtect() only can change page protections on
    // memory blocks allocated by GlobalAlloc(), HeapAlloc(), or LocalAlloc(). For mapped views, this value must be
    // compatible with the access protection specified when the view was mapped, which means we can`t crete a non-access
    // map view, than change the protections like linux procedure. Here we just map a PAGE_EXECUTE_READWRITE at
    // beginning.
    mappedAddr = VirtualAlloc(NULL, reqSize, MEM_RESERVE, PAGE_READWRITE);
#else
    DLOG(ALLOC, "MemoryMap::MapMemory size %zu", reqSize);
    mappedAddr = mmap(opt.reqBase, reqSize, PROT_NONE, opt.flags, -1, 0);
#endif

    bool failure = false;
#if defined(_WIN64) || defined(__APPLE__)
    if (mappedAddr != NULL) {
#else
    if (mappedAddr != MAP_FAILED) {
        (void)madvise(mappedAddr, reqSize, MADV_NOHUGEPAGE);
        COMMON_PRCTL(mappedAddr, reqSize, opt.tag);
#endif
        // if protAll, all memory is protected at creation, and we never change it (save time)
        size_t protSize = opt.protAll ? reqSize : initSize;
        if (!ProtectMemInternal(mappedAddr, protSize, opt.prot)) {
            failure = true;
            LOG_COMMON(ERROR) << "MemoryMap::MapMemory mprotect failed";
            ALLOCUTIL_MEM_UNMAP(mappedAddr, reqSize);
        }
    } else {
        failure = true;
    }
    LOGF_CHECK(!failure) << "MemoryMap::MapMemory failed reqSize: " << reqSize << " initSize: " << initSize;

    DLOG(ALLOC, "MemoryMap::MapMemory size %zu successful at %p", reqSize, mappedAddr);
    MemoryMap* memMap = new (std::nothrow) MemoryMap(mappedAddr, initSize, reqSize);
    LOGF_CHECK(memMap != nullptr) << "new MemoryMap failed";

    os::PrctlSetVMA(mappedAddr, reqSize, (std::string("ArkTS Heap CMCGC ") + opt.tag).c_str());
    return memMap;
}

MemoryMap* MemoryMap::MapMemoryAlignInner4G(uint64_t reqSize, uint64_t initSize, const Option& opt)
{
    static constexpr uint64_t MAX_SUPPORT_CAPACITY = 4ULL * GB;

    void* mappedAddr = nullptr;
    reqSize = AllocUtilRndUp<size_t>(reqSize, ALLOC_UTIL_PAGE_SIZE);
#ifdef PANDA_TARGET_64
    size_t needReqSize = reqSize * 2;
#else
    size_t needReqSize = reqSize;
#endif

#ifdef _WIN64
    // Windows don`t support to map a non-access memory, and virtualProtect() only can change page protections on
    // memory blocks allocated by GlobalAlloc(), HeapAlloc(), or LocalAlloc(). For mapped views, this value must be
    // compatible with the access protection specified when the view was mapped, which means we can`t crete a non-access
    // map view, than change the protections like linux procedure. Here we just map a PAGE_EXECUTE_READWRITE at
    // beginning.
    mappedAddr = VirtualAlloc(NULL, needReqSize, MEM_RESERVE, PAGE_READWRITE);
#else
    DLOG(ALLOC, "MemMap::MapMemory size %zu", needReqSize);
    mappedAddr = mmap(opt.reqBase, needReqSize, PROT_NONE, opt.flags, -1, 0);
#endif

    uintptr_t baseAddr = 0x0;
#ifdef PANDA_TARGET_64
    uintptr_t startAddr = reinterpret_cast<uintptr_t>(mappedAddr);
    uintptr_t alignAddr = AllocUtilRndUp(startAddr, MAX_SUPPORT_CAPACITY);

    size_t leftSize = alignAddr - startAddr;
    uintptr_t remainderAddr = alignAddr;
    if (leftSize > needReqSize) {
        remainderAddr = startAddr;
    } else if (leftSize > reqSize) {
        remainderAddr = alignAddr - reqSize;
    }
    mappedAddr = reinterpret_cast<void *>(remainderAddr);
    baseAddr = remainderAddr & (0xFFFFFFFFULL << 32);

    auto leftUnmapAddr = reinterpret_cast<void *>(startAddr);
    size_t leftUnmapSize = remainderAddr - reinterpret_cast<uintptr_t>(leftUnmapAddr);

    auto rightUnmapAddr = reinterpret_cast<void *>(remainderAddr + reqSize);
    size_t rightUnmapSize = (startAddr + needReqSize) - reinterpret_cast<uintptr_t>(rightUnmapAddr);
#ifdef _WIN64
    VirtualFree(leftUnmapAddr, leftUnmapSize, MEM_DECOMMIT);
    VirtualFree(rightUnmapAddr, rightUnmapSize, MEM_DECOMMIT);
#else
    munmap(leftUnmapAddr, leftUnmapSize);
    munmap(rightUnmapAddr, rightUnmapSize);
#endif
#endif
    SetBaseAddress(baseAddr);

    bool failure = false;
#if defined(_WIN64) || defined(__APPLE__)
    if (mappedAddr != NULL) {
#else
    if (mappedAddr != MAP_FAILED) {
        (void)madvise(mappedAddr, reqSize, MADV_NOHUGEPAGE);
        COMMON_PRCTL(mappedAddr, reqSize, opt.tag);
#endif
        // if protAll, all memory is protected at creation, and we never change it (save time)
        size_t protSize = opt.protAll ? reqSize : initSize;
        if (!ProtectMemInternal(mappedAddr, protSize, opt.prot)) {
            failure = true;
            LOG_COMMON(ERROR) << "MemMap::MapMemory mprotect failed";
            ALLOCUTIL_MEM_UNMAP(mappedAddr, reqSize);
        }
    } else {
        failure = true;
    }
    LOGF_CHECK(!failure) << "MemMap::MapMemory failed reqSize: " << reqSize << " initSize: " << initSize;

    DLOG(ALLOC, "MemMap::MapMemory size %zu successful at %p", reqSize, mappedAddr);
    MemoryMap* memMap = new (std::nothrow) MemoryMap(mappedAddr, initSize, reqSize);
    LOGF_CHECK(memMap != nullptr) << "new MemMap failed";

    os::PrctlSetVMA(mappedAddr, needReqSize, (std::string("ArkTS Heap CMCGC ") + opt.tag).c_str());
    return memMap;
}

#ifdef _WIN64
void MemoryMap::CommitMemory(void* addr, size_t size)
{
    LOGE_IF(UNLIKELY_CC(!VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE))) <<
        "VirtualAlloc commit failed in GetPage, errno: " << GetLastError();
}
#endif

MemoryMap::MemoryMap(void* baseAddr, size_t initSize, size_t mappedSize)
    : memBaseAddr_(baseAddr), memCurrSize_(initSize), memMappedSize_(mappedSize)
{
    memCurrEndAddr_ = reinterpret_cast<void*>(reinterpret_cast<HeapAddress>(memBaseAddr_) + memCurrSize_);
    memMappedEndAddr_ = reinterpret_cast<void*>(reinterpret_cast<HeapAddress>(memBaseAddr_) + memMappedSize_);
}

bool MemoryMap::ProtectMemInternal(void* addr, size_t size, int prot)
{
    DLOG(ALLOC, "MemoryMap::ProtectMem %p, size %zu, prot %d", addr, size, prot);
#ifdef _WIN64
    return true;
#else
    int ret = mprotect(addr, size, prot);
    return (ret == 0);
#endif
}
MemoryMap::~MemoryMap()
{
    ALLOCUTIL_MEM_UNMAP(memBaseAddr_, memMappedSize_);
    memBaseAddr_ = nullptr;
    memCurrEndAddr_ = nullptr;
    memMappedEndAddr_ = nullptr;
}
} // namespace common
