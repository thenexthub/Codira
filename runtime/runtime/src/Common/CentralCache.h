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


#ifndef MRT_CENTRAL_CACHE_H
#define MRT_CENTRAL_CACHE_H

#include "Base/ImmortalWrapper.h"
#include "MemCommon.h"

namespace MapleRuntime {
class CentralCache {
public:
    // Singleton mode
    static CentralCache* GetInstance() { return &*instance; }

    // Obtain a certain number of small fixed-length memory blocks from the central cache.
    size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t alignBytes);

    // Reclaim all small fixed-length memory blocks in the freelist to their corresponding spans.
    void ReleaseListToSpans(void* start, size_t index);

private:
    friend class ImmortalWrapper<CentralCache>;
    // Obtain a non-null span from a specified SpanList.
    Span* GetOneSpan(SpanList& list, size_t alignBytes);

    // Hash Bucket
    SpanList centralCacheSpans[NFREELIST];

private:
    CentralCache() noexcept {}

    CentralCache(const CentralCache&) = delete;
    CentralCache& operator=(const CentralCache&) = delete;

    static ImmortalWrapper<CentralCache> instance;
};
} // namespace MapleRuntime
#endif // MRT_CENTRAL_CACHE_H
