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

#ifndef COMMON_COMPONENTS_COMMON_MEM_COMMON_H
#define COMMON_COMPONENTS_COMMON_MEM_COMMON_H

#include <algorithm>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "common_components/common/page_pool.h"

namespace common {
using pageID = unsigned long long;
// The maximum number of pages that PageCache can allocate
constexpr size_t MAX_NPAGES = 129;
// Calculate the page number and the starting address of the page using bitwise shift operations
constexpr size_t PAGE_SHIFT = 12;
// The maximum memory space that ThreadCache can allocate
constexpr size_t MAX_BYTES = 256 * 1024;

enum AlignNmu { ALIGN_8 = 8, ALIGN_16 = 16, ALIGN_128 = 128, ALIGN_1024 = 1024, ALIGN_8192 = 8 * 1028 };

enum MemberSize {
    ALING_8_BYTE = 128,
    ALING_16_BYTE = 1024,
    ALING_128_BYTE = 8 * 1024,
    ALING_1024_BYTE = 64 * 1024,
    ALING_8192_BYTE = 256 * 1024,
};

enum AlignShift { ALIGN_8_SIFT = 3, ALIGN_16_SIFT = 4, ALIGN_128_SIFT = 7, ALIGN_1024_SIFT = 10, ALIGN_8192_SIFT = 13 };

// Allocate memory resources from the system
inline void* SystemAlloc(size_t kpage)
{
    return PagePool::Instance().GetPage(kpage * COMMON_PAGE_SIZE);
}

// Return a reference to the first 4 or 8 bytes of the passed-in space
inline void*& NextObj(void* obj)
{
    CHECK_CC(obj != nullptr);
    return *(void**)obj;
}

// The free list used to store the small fixed-size memory blocks after splitting.
class FreeList {
public:
    void PushFront(void* obj)
    {
        CHECK_CC(obj != nullptr);

        NextObj(obj) = freeList_;
        freeList_ = obj;
        ++size_;
    }
    void* PopFront()
    {
        CHECK_CC(!Empty());

        void* obj = freeList_;
        freeList_ = NextObj(obj);
        --size_;

        return obj;
    }

    void PushAtFront(void* start, void* end, size_t n)
    {
        CHECK_CC(start != nullptr);
        CHECK_CC(end != nullptr);
        CHECK_CC(n > 0);

        NextObj(end) = freeList_;
        freeList_ = start;
        size_ += n;
    }

    void PopAtFront(void*& start, size_t n)
    {
        CHECK_CC(n <= size_);

        start = freeList_;
        void* end = freeList_;
        for (size_t i = 0; i < n - 1; ++i) {
            end = NextObj(end);
        }
        freeList_ = NextObj(end);
        NextObj(end) = nullptr;
        size_ -= n;
    }

    bool Empty() { return size_ == 0; }

    size_t& GetAdjustSize() { return adjustSize_; }

    size_t GetSize() { return size_; }

private:
    size_t size_ = 0;       // The number of small fixed-size memory blocks
    size_t adjustSize_ = 1; // The slow-start adjustment value for requesting memory from the central cache.
    void* freeList_ = nullptr;
};

// Span manages a large block of memory in units of pages.
struct Span {
    size_t pageNum = 0; // number of page
    pageID pageId = 0;  // page if of first page
    Span* next = nullptr;
    Span* prev = nullptr;
    size_t useCount = 0;
    void* freeBlocks = nullptr;
    bool isUse = false;
};

// The doubly linked circular list with a header node for managing spans.
class SpanList {
public:
    SpanList()
    {
        head_->prev = head_;
        head_->next = head_;
    }

    ~SpanList()
    {
        while (!Empty()) {
            Span* span = PopFront();
            delete span;
        }
        delete head_;
    }

    void Insert(Span* pos, Span* newSpan)
    {
        CHECK_CC(pos != nullptr);
        CHECK_CC(newSpan != nullptr);
        Span* prev = pos->prev;
        prev->next = newSpan;
        newSpan->prev = prev;
        newSpan->next = pos;
        pos->prev = newSpan;
    }

    void Erase(Span* pos)
    {
        CHECK_CC(pos != nullptr);
        CHECK_CC(pos != head_); // The sentinel header node must not be deleted.
        Span* prev = pos->prev;
        Span* next = pos->next;
        prev->next = next;
        next->prev = prev;
    }

    Span* Begin() { return head_->next; }

    Span* End() { return head_; }

    bool Empty() { return Begin() == head_; }

    Span* PopFront()
    {
        CHECK_CC(!Empty());

        Span* ret = Begin();
        Erase(Begin());
        return ret;
    }

    void PushFront(Span* span)
    {
        CHECK_CC(span != nullptr);

        Insert(Begin(), span);
    }

    std::mutex& GetSpanListMutex() { return mtx_; }

private:
    Span* head_ = new Span;
    std::mutex mtx_;
};
} // namespace common
#endif
