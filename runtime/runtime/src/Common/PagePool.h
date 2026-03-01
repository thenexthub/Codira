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


#ifndef MRT_ALLOCATOR_PAGE_POOL_H
#define MRT_ALLOCATOR_PAGE_POOL_H

#include <atomic>
#include <mutex>
#ifdef _WIN64
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#else
#include <sys/mman.h>
#endif
#include "Base/Globals.h"
#include "Base/SysCall.h"
#include "Heap/Allocator/CartesianTree.h"
#include "securec.h"
#if defined(_WIN64) || defined(__APPLE__)
#include "Base/MemUtils.h"
#endif

namespace MapleRuntime {
// a page pool maintain a pool of free pages, serve page allocation and free
class PagePool {
public:
    explicit PagePool(const char* name) : tag(name) {}
    PagePool(PagePool const&) = delete;
    PagePool& operator=(const PagePool&) = delete;
    ~PagePool() {}
    void Init(uint32_t pageCount)
    {
        totalPageCount = pageCount;
        size_t size = static_cast<size_t>(totalPageCount) * MapleRuntime::MRT_PAGE_SIZE;
        freePagesTree.Init(totalPageCount);
        base = MapMemory(size, tag);
        totalSize = size;
    }
    void Fini()
    {
        freePagesTree.Fini();
#ifdef _WIN64
        CHECK_E(!VirtualFree(base, 0, MEM_RELEASE), "VirtualFree failed in PagePool destruction, errno: %s",
                GetLastError());
#else
        CHECK_E(munmap(base, totalPageCount * MapleRuntime::MRT_PAGE_SIZE) != EOK,
                "munmap failed in PagePool destruction, errno: %d", errno);
#endif
    }

    uint8_t* GetPage(size_t bytes = MapleRuntime::MRT_PAGE_SIZE)
    {
        Index idx = 0;
        size_t count = (bytes + MapleRuntime::MRT_PAGE_SIZE - 1) / MapleRuntime::MRT_PAGE_SIZE;
        size_t pageSize = RoundUp(bytes, MapleRuntime::MRT_PAGE_SIZE);
        CHECK_DETAIL(count < std::numeric_limits<Count>::max(), "native memory out of memory!");
        {
            std::lock_guard<std::mutex> lg(freePagesMutex);
            if (freePagesTree.TakeUnits(static_cast<Count>(count), idx, false)) {
                auto* ret = base + static_cast<size_t>(idx) * MapleRuntime::MRT_PAGE_SIZE;
#ifdef _WIN64
                CHECK_E(UNLIKELY(!VirtualAlloc(ret, pageSize, MEM_COMMIT, PAGE_READWRITE)),
                        "VirtualAlloc commit failed in GetPage, errno: %d", GetLastError());
#endif
                return ret;
            }
            if ((usedZone + pageSize) <= totalSize && base != nullptr) {
                size_t current = usedZone;
                usedZone += pageSize;
#ifdef _WIN64
                CHECK_E(UNLIKELY(!VirtualAlloc(base + current, pageSize, MEM_COMMIT, PAGE_READWRITE)),
                        "VirtualAlloc commit failed in GetPage, errno: %d", GetLastError());
#endif
                return base + current;
            }
        }
        return MapMemory(pageSize, tag, true);
    }

    void ReturnPage(uint8_t* page, size_t bytes = MapleRuntime::MRT_PAGE_SIZE) noexcept
    {
        uint8_t* end = base + totalSize;
        size_t num = (bytes + MapleRuntime::MRT_PAGE_SIZE - 1) / MapleRuntime::MRT_PAGE_SIZE;
        if (page < base || page >= end) {
#ifdef _WIN64
            CHECK_E(UNLIKELY(!VirtualFree(page, 0, MEM_RELEASE)), "VirtualFree failed in ReturnPage, errno: %s",
                    GetLastError());
#else
            CHECK_E(UNLIKELY(munmap(page, num * MapleRuntime::MRT_PAGE_SIZE) != EOK),
                    "munmap failed in ReturnPage, errno: %d", errno);
#endif
            return;
        }
        CHECK_DETAIL(num < std::numeric_limits<Count>::max(), "native memory out of memory!");
        Index idx = static_cast<Index>((page - base) / MapleRuntime::MRT_PAGE_SIZE);
#if defined(_WIN64)
        CHECK_E(UNLIKELY(!VirtualFree(page, num * MapleRuntime::MRT_PAGE_SIZE, MEM_DECOMMIT)),
                "VirtualFree failed in ReturnPage, errno: %s", GetLastError());
#elif defined(__APPLE__)
        MapleRuntime::MemorySet(reinterpret_cast<uintptr_t>(page), num * MapleRuntime::MRT_PAGE_SIZE, 0,
                                num * MapleRuntime::MRT_PAGE_SIZE);
        void* ret = mmap(page, num * MapleRuntime::MRT_PAGE_SIZE,
                         PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
        if (ret == MAP_FAILED) {
            LOG(RTLOG_ERROR, "page pool mmap fixed failed");
        } else if (ret != reinterpret_cast<void*>(page)) {
            LOG(RTLOG_ERROR, "mmap fixed at wrong addr %p->%p", page, ret);
        }
#else
        (void)madvise(page, num * MapleRuntime::MRT_PAGE_SIZE, MADV_DONTNEED);
#endif
        std::lock_guard<std::mutex> lg(freePagesMutex);
        CHECK_DETAIL(freePagesTree.MergeInsert(idx, static_cast<Count>(num), false),
                     "tid %d: failed to return pages to freePagesTree [%u+%u, %u)", GetTid(), idx, num, idx + num);
    }

    // return unused pages to os
    void Trim() const {}

    MRT_EXPORT static PagePool& Instance() noexcept;

protected:
    uint8_t* MapMemory(size_t size, const char* memName, bool isCommit = false) const
    {
#ifdef _WIN64
        void* result = VirtualAlloc(NULL, size, isCommit ? MEM_COMMIT : MEM_RESERVE, PAGE_READWRITE);
        if (result == NULL) {
            LOG(RTLOG_FATAL, "allocate create page failed! Out of Memory!");
        }
        (void)memName;
#else
        void* result = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        CHECK_DETAIL(result != MAP_FAILED, "allocate create page failed! Out of Memory!");
#if defined(__linux__) || defined(__OHOS__) || defined(__ANDROID__)
        (void)madvise(result, size, MADV_NOHUGEPAGE);
#endif
        (void)isCommit;
#endif

#if defined(__linux__) || defined(hongmeng)
        MRT_PRCTL(result, size, memName);
#endif
        return reinterpret_cast<uint8_t*>(result);
    }
    using Index = CartesianTree::Index;
    using Count = CartesianTree::Count;
    std::mutex freePagesMutex;
    CartesianTree freePagesTree;
    uint8_t* base = nullptr; // start address of the mapped pages
    size_t totalSize = 0;    // total size of the mapped pages
    size_t usedZone = 0;     // used zone for native memory pool.
    const char* tag = nullptr;

private:
    std::atomic<uint32_t> smallPageUsed = { 0 };
    uint32_t totalPageCount = 0;
};
} // namespace MapleRuntime
#endif // MRT_ALLOCATOR_PAGE_POOL_H
