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
#ifndef ARK_COMM_RUNTIME_ALLOCATOR_PAGE_POOL_H
#define ARK_COMM_RUNTIME_ALLOCATOR_PAGE_POOL_H

#include <atomic>
#include <mutex>
#ifdef _WIN64
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memoryapi.h>
#else
#include <sys/mman.h>
#endif

#include "common_components/base/globals.h"
#include "common_components/base/sys_call.h"
#include "common_components/heap/allocator/treap.h"
#include "common_components/platform/os.h"
#include "securec.h"
#if defined(_WIN64) || defined(__APPLE__)
#include "common_components/base/mem_utils.h"
#endif

namespace common {
// a page pool maintain a pool of free pages, serve page allocation and free
class PagePool {
public:
    explicit PagePool(const char* name) : tag_(name) {}
    PagePool(PagePool const&) = delete;
    PagePool& operator=(const PagePool&) = delete;
    ~PagePool() {}
    void Init(uint32_t pageCount)
    {
        totalPageCount_ = pageCount;
        smallPageUsed_ = 0;
        usedZone_ = 0;
        size_t size = static_cast<size_t>(totalPageCount_) * COMMON_PAGE_SIZE;
        freePagesTree_.Init(totalPageCount_);
        base_ = MapMemory(size, tag_);
        totalSize_ = size;
    }
    void Fini()
    {
        freePagesTree_.Fini();
#ifdef _WIN64
        LOGE_IF(!VirtualFree(base_, 0, MEM_RELEASE)) <<
                "VirtualFree failed in PagePool destruction, errno: " << GetLastError();
#else
        LOGE_IF(munmap(base_, totalPageCount_ * COMMON_PAGE_SIZE) != EOK) <<
                "munmap failed in PagePool destruction, errno: " << errno;
#endif
    }

    uint8_t* GetPage(size_t bytes = COMMON_PAGE_SIZE)
    {
        uint32_t idx = 0;
        size_t count = (bytes + COMMON_PAGE_SIZE - 1) / COMMON_PAGE_SIZE;
        size_t pageSize = RoundUp(bytes, COMMON_PAGE_SIZE);
        LOGF_CHECK(count < std::numeric_limits<uint32_t>::max()) << "native memory out of memory!";
        {
            std::lock_guard<std::mutex> lg(freePagesMutex_);
            if (freePagesTree_.TakeUnits(static_cast<uint32_t>(count), idx, false)) {
                auto* ret = base_ + static_cast<size_t>(idx) * COMMON_PAGE_SIZE;
#ifdef _WIN64
                LOGE_IF(UNLIKELY_CC(!VirtualAlloc(ret, pageSize, MEM_COMMIT, PAGE_READWRITE))) <<
                    "VirtualAlloc commit failed in GetPage, errno: " << GetLastError();
#endif
                return ret;
            }
            if ((usedZone_ + pageSize) <= totalSize_ && base_ != nullptr) {
                size_t current = usedZone_;
                usedZone_ += pageSize;
#ifdef _WIN64
                LOGE_IF(UNLIKELY_CC(!VirtualAlloc(base_ + current, pageSize, MEM_COMMIT, PAGE_READWRITE))) <<
                    "VirtualAlloc commit failed in GetPage, errno: " << GetLastError();
#endif
                return base_ + current;
            }
        }
        return MapMemory(pageSize, tag_, true);
    }

    void ReturnPage(uint8_t* page, size_t bytes = COMMON_PAGE_SIZE) noexcept
    {
        uint8_t* end = base_ + totalSize_;
        size_t num = (bytes + COMMON_PAGE_SIZE - 1) / COMMON_PAGE_SIZE;
        if (page < base_ || page >= end) {
#ifdef _WIN64
            LOGE_IF(UNLIKELY_CC(!VirtualFree(page, 0, MEM_RELEASE))) <<
                "VirtualFree failed in ReturnPage, errno: " << GetLastError();
#else
            LOGE_IF(UNLIKELY_CC(munmap(page, num * COMMON_PAGE_SIZE) != EOK)) <<
                "munmap failed in ReturnPage, errno: " << errno;
#endif
            return;
        }
        LOGF_CHECK(num < std::numeric_limits<uint32_t>::max()) << "native memory out of memory!";
        uint32_t idx = static_cast<uint32_t>((page - base_) / COMMON_PAGE_SIZE);
#if defined(_WIN64)
        LOGE_IF(UNLIKELY_CC(!VirtualFree(page, num * COMMON_PAGE_SIZE, MEM_DECOMMIT))) <<
            "VirtualFree failed in ReturnPage, errno: " << GetLastError();
#elif defined(__APPLE__)
        MemorySet(reinterpret_cast<uintptr_t>(page), num * COMMON_PAGE_SIZE, 0,
                  num * COMMON_PAGE_SIZE);
        (void)madvise(page, num * COMMON_PAGE_SIZE, MADV_DONTNEED);
#else
        (void)madvise(page, num * COMMON_PAGE_SIZE, MADV_DONTNEED);
#endif
        std::lock_guard<std::mutex> lg(freePagesMutex_);
        LOGF_CHECK(freePagesTree_.MergeInsert(idx, static_cast<uint32_t>(num), false)) <<
            "tid " << GetTid() << ": failed to return pages to freePagesTree [" <<
            idx << "+" << num << ", " << (idx + num) << ")";
    }

    // return unused pages to os
    void Trim() const {}

    PUBLIC_API static PagePool& Instance() noexcept;

protected:
    uint8_t* MapMemory(size_t size, const char* memName, bool isCommit = false) const
    {
#ifdef _WIN64
        void* result = VirtualAlloc(NULL, size, isCommit ? MEM_COMMIT : MEM_RESERVE, PAGE_READWRITE);
        if (result == NULL) { //LCOV_EXCL_BR_LINE
            LOG_COMMON(FATAL) << "allocate create page failed! Out of Memory!";
            UNREACHABLE_CC();
        }
        (void)memName;
#else
        void* result = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        LOGF_CHECK(result != MAP_FAILED) << "allocate create page failed! Out of Memory!";
#if defined(__linux__) || defined(PANDA_TARGET_OHOS)
        (void)madvise(result, size, MADV_NOHUGEPAGE);
#endif
        (void)isCommit;
#endif

#if defined(__linux__) || defined(PANDA_TARGET_OHOS)
        COMMON_PRCTL(result, size, memName);
#endif
        os::PrctlSetVMA(result, size, (std::string("ArkTS Heap CMCGC PagePool ") + memName).c_str());
        return reinterpret_cast<uint8_t*>(result);
    }

    std::mutex freePagesMutex_;
    Treap freePagesTree_;
    uint8_t* base_ = nullptr; // start address of the mapped pages
    size_t totalSize_ = 0;    // total size of the mapped pages
    size_t usedZone_ = 0;     // used zone for native memory pool.
    const char* tag_ = nullptr;

private:
    std::atomic<uint32_t> smallPageUsed_ = { 0 };
    uint32_t totalPageCount_ = 0;
};
} // namespace common
#endif // ARK_COMM_RUNTIME_ALLOCATOR_PAGE_POOL_H
