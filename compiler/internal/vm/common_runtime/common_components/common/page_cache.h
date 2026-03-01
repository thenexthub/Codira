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

#ifndef COMMON_COMPONENTS_COMMON_PAGE_CACHE_H
#define COMMON_COMPONENTS_COMMON_PAGE_CACHE_H

#include "common_components/common/mem_common.h"

namespace common {
class PageCache {
public:
    // Return the singleton object of PageCache
    static PageCache* GetInstance() { return &instance_; }

    // Get a k-page Span
    Span* NewSpan(size_t k);

    std::mutex& GetPageMutex();

    // Pass in a small fixed-length memory block to obtain the Span object corresponding to the page it is located in.
    Span* MapObjectToSpan(void* obj);

    // Try to merge the pages before and after the span to alleviate the external fragmentation problem.
    void ReleaseSpanToPageCache(Span* span);

private:
    std::mutex pageMtx_;
    SpanList pageCacheSpans_[MAX_NPAGES];
    std::unordered_map<pageID, Span*> idSpanMap_; // The mapping between page numbers and span objects.

private:
    PageCache() noexcept {}

    PageCache(const PageCache&) = delete;
    PageCache& operator=(const PageCache&) = delete;

    static PageCache instance_;
};

class ScopedPageCacheMutex {
public:
    explicit ScopedPageCacheMutex() { PageCache::GetInstance()->GetPageMutex().lock(); }

    ~ScopedPageCacheMutex() { PageCache::GetInstance()->GetPageMutex().unlock(); }
};
} // namespace common
#endif
