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


#ifndef MRT_PAGE_CACHE_H
#define MRT_PAGE_CACHE_H

#include "Base/ImmortalWrapper.h"
#include "MemCommon.h"

namespace MapleRuntime {
class PageCache {
public:
    // Return the singleton object of PageCache
    static PageCache* GetInstance() { return &*instance; }

    // Get a k-page Span
    Span* NewSpan(size_t k);

    std::mutex& GetPageMutex();

    // Pass in a small fixed-length memory block to obtain the Span object corresponding to the page it is located in.
    Span* MapObjectToSpan(void* obj);

    // Try to merge the pages before and after the span to alleviate the external fragmentation problem.
    void ReleaseSpanToPageCache(Span* span);

private:
    friend class ImmortalWrapper<PageCache>;
    std::mutex pageMtx;
    SpanList pageCacheSpans[MAX_NPAGES];
    std::unordered_map<pageID, Span*> idSpanMap; // The mapping between page numbers and span objects.

private:
    PageCache() noexcept {}

    PageCache(const PageCache&) = delete;
    PageCache& operator=(const PageCache&) = delete;

    static ImmortalWrapper<PageCache> instance;
};

class ScopedPageCacheMutex {
public:
    explicit ScopedPageCacheMutex() { PageCache::GetInstance()->GetPageMutex().lock(); }

    ~ScopedPageCacheMutex() { PageCache::GetInstance()->GetPageMutex().unlock(); }
};
} // namespace MapleRuntime
#endif
