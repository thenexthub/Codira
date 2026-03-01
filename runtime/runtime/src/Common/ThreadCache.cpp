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


#include "ThreadCache.h"

#include "CentralCache.h"

namespace MapleRuntime {
void* ThreadCache::Allocate(size_t bytes)
{
    // ThreadCache can only allocate object space that is less than or equal to 256KB
    CHECK(bytes <= MAX_BYTES);
    // 1. Determine the size of the aligned small fixed-size memory block and the free list bucket it maps to
    size_t index = SizeManager::Index(bytes);
    size_t alignBytes = SizeManager::RoundUp(bytes);
    // 2. If the free list bucket is not empty, retrieve the first small fixed-size memory block.
    //    If the bucket is empty, batch-allocate from CentralCache.
    if (!freeLists[index].Empty()) {
        return freeLists[index].PopFront();
    } else {
        return FetchFromCentralCache(index, alignBytes);
    }
}

// Deallocate a small fixed-size memory block
void ThreadCache::Deallocate(void* ptr, size_t bytes)
{
    CHECK(ptr != nullptr);
    CHECK(bytes <= MAX_BYTES);
    // 1. Calculate the free list bucket it maps to
    size_t index = SizeManager::Index(bytes);
    // 2. Insert the small fixed-size memory block at the head of the free list bucket
    freeLists[index].PushFront(ptr);
    // 3. When the free list length is greater than or equal to the batch allocation size,
    //    return all small fixed-size memory blocks in the current free list to CentralCache
    if (freeLists[index].GetSize() >= freeLists[index].GetAdjustSize()) {
        ReturnToCentralCache(freeLists[index], bytes);
    }
}

// Fetch a "batch" of small fixed-size memory blocks from the central cache and return one block to the caller
void* ThreadCache::FetchFromCentralCache(size_t index, size_t alignBytes)
{
    // Slow-start feedback adjustment algorithm:
    // 1. Initially, do not request too many blocks from CentralCache at once, as too many may not be fully utilized.
    // 2. Gradually increase the batch size until the upper limit is reached.
    // 3. The smaller the alignBytes, the larger the batch size requested from CentralCache.
    // 4. The larger the alignBytes, the smaller the batch size requested from CentralCache.

    CHECK(alignBytes >= MIN_ALIGN_NUM);

    // 1. Calculate the number of small fixed-size memory blocks to request from CentralCache
    size_t batchNum = std::min(freeLists[index].GetAdjustSize(), SizeManager::LimitSize(alignBytes));
    if (freeLists[index].GetAdjustSize() == batchNum) {
        freeLists[index].GetAdjustSize() += 1;
    }
    // 2. Request a batch of small fixed-size memory blocks from CentralCache using output parameters
    void* begin = nullptr;
    void* end = nullptr;
    size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(begin, end, batchNum, alignBytes);
    // 3. Return the first small fixed-size memory block to the caller
    //    and link the remaining blocks to the free list bucket
    CHECK(actualNum > 0);
    if (actualNum == 1) {
        CHECK(begin == end);
        return begin;
    } else {
        freeLists[index].PushAtFront(NextObj(begin), end, actualNum - 1);
        return begin;
    }
}

// Return a list of memory blocks to CentralCache
void ThreadCache::ReturnToCentralCache(FreeList& list, size_t bytes)
{
    CHECK(bytes > 0);

    void* start = nullptr;
    // 1. Strip the contiguous small fixed-size memory blocks from the list
    list.PopAtFront(start, list.GetSize());
    // 2. Release the stripped memory block list to CentralCache
    CentralCache::GetInstance()->ReleaseListToSpans(start, SizeManager::Index(bytes));
}
} // namespace MapleRuntime
