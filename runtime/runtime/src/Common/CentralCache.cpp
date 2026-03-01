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


#include "CentralCache.h"

#include "PageCache.h"

namespace MapleRuntime {
ImmortalWrapper<CentralCache> CentralCache::instance;

// Retrieve a non-empty Span from a specific SpanList.
Span* CentralCache::GetOneSpan(SpanList& list, size_t alignBytes)
{
    CHECK(alignBytes >= MIN_ALIGN_NUM);
    // 1. First, search within freelist.
    Span* it = list.Begin();
    while (it != list.End()) {
        if (it->freeBlocks != nullptr) {
            return it;
        } else {
            it = it->next;
        }
    }
    // 2. No non-empty Span available; request a Span of a specific page size from the PageCache.
    Span* span = nullptr;
    {
        ScopedPageCacheMutex mtx;
        span = PageCache::GetInstance()->NewSpan(SizeManager::CalculatePageNum(alignBytes));
        span->isUse = true;
    }

    char* start = reinterpret_cast<char*>(span->pageId << PAGE_SHIFT);
    size_t bytes = span->pageNum << PAGE_SHIFT;
    char* end = start + bytes;

    span->freeBlocks = start;
    start += alignBytes;
    void* tail = span->freeBlocks;
    while (start < end) {
        NextObj(tail) = start;
        tail = start;
        start += alignBytes;
    }
    NextObj(tail) = nullptr;
    list.PushFront(span);

    return span;
}

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t alignBytes)
{
    CHECK(batchNum > 0);
    CHECK(alignBytes >= MIN_ALIGN_NUM);

    size_t index = SizeManager::Index(alignBytes);

    centralCacheSpans[index].GetSpanListMutex().lock();

    // Retrieve batchNum small fixed-size memory blocks from the Span.
    Span* span = GetOneSpan(centralCacheSpans[index], alignBytes);
    CHECK(span != nullptr);
    CHECK(span->freeBlocks);
    start = span->freeBlocks;
    end = span->freeBlocks;
    size_t actualNum = 1;
    while (actualNum < batchNum && NextObj(end)) {
        ++actualNum;
        end = NextObj(end);
    }
    span->freeBlocks = NextObj(end);
    NextObj(end) = nullptr;

    span->useCount += actualNum;

    centralCacheSpans[index].GetSpanListMutex().unlock();

    return actualNum;
}

// Reclaim all small fixed-size memory blocks from the free list back to their respective Spans.
void CentralCache::ReleaseListToSpans(void* start, size_t index)
{
    CHECK(start != nullptr);
    // All small fixed-size memory blocks in the free list originate from the Span stored in centralCacheSpans[index].
    centralCacheSpans[index].GetSpanListMutex().lock();

    // Insert each small fixed-size memory block at the head of the freeBlocks of the corresponding Span.
    while (start) {
        void* next = NextObj(start);
        Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
        if (span == nullptr) {
            start = next;
            continue;
        }
        NextObj(start) = span->freeBlocks;
        span->freeBlocks = start;
        --span->useCount;

        // useCount = 0 means that the Span can be reclaimed.
        if (span->useCount == 0) {
            centralCacheSpans[index].Erase(span);
            span->freeBlocks = nullptr;
            span->next = nullptr;
            span->prev = nullptr;

            PageCache::GetInstance()->ReleaseSpanToPageCache(span);
        }

        start = next;
    }

    centralCacheSpans[index].GetSpanListMutex().unlock();
}
} // namespace MapleRuntime
