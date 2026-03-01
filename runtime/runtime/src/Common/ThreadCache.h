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


#ifndef MRT_THREAD_CACHE_H
#define MRT_THREAD_CACHE_H
#include "MemCommon.h"

namespace MapleRuntime {
/* The essence of ThreadCache is a hash bucket
  | 8Byte  | -> 8Byte -> 8Byte
  | 16Byte | -> 16Byte
  | 32Byte | -> 32Byte -> 32Byte - 32Byte
  | ...    |
  | ...    |
  | 256KB  | -> 256KB

  The bucket is divided into a total of 208 table items,
  which is used to simplify bucket management while keeping memory fragmentation under control.
  The specific division rules are defined in class SizeManager.
*/
class ThreadCache {
public:
    ThreadCache() noexcept {}
    ThreadCache(const ThreadCache&) noexcept = delete;
    ThreadCache& operator=(const ThreadCache&) noexcept = delete;

    void* Allocate(size_t bytes);
    void Deallocate(void* ptr, size_t bytes);

private:
    // Return a segment of list to CentralCache
    void ReturnToCentralCache(FreeList& list, size_t bytes);
    // Obtain a batch of small fixed-length memory blocks from the CentralCache and return one of them
    void* FetchFromCentralCache(size_t index, size_t alignBytes);

    FreeList freeLists[NFREELIST]; // Hash bucket of small fixed-length memory blocks
};
} // namespace MapleRuntime
#endif
