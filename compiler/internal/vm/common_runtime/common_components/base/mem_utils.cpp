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

#include "common_components/log/log.h"
#include "securec.h"

namespace common {
void MemorySet(uintptr_t dest, size_t size, int c, size_t count)
{
    uintptr_t destAddress = dest;
    while (size != 0) {
        size_t sizePerChunk = size > SECUREC_MEM_MAX_LEN ? SECUREC_MEM_MAX_LEN : size;
        size_t countPerChunk = count > SECUREC_MEM_MAX_LEN ? SECUREC_MEM_MAX_LEN : count;
        LOGE_IF(memset_s(reinterpret_cast<void*>(destAddress), sizePerChunk, c, countPerChunk) != EOK) <<
            "memset_s fail";
        size -= sizePerChunk;
        count -= countPerChunk;
        destAddress += sizePerChunk;
    }
}

void MemoryCopy(uintptr_t dest, size_t size, const uintptr_t src, size_t count)
{
    uintptr_t destAddress = dest;
    uintptr_t srcAddress = src;
    while (size != 0) {
        size_t sizePerChunk = size > SECUREC_MEM_MAX_LEN ? SECUREC_MEM_MAX_LEN : size;
        size_t countPerChunk = count > SECUREC_MEM_MAX_LEN ? SECUREC_MEM_MAX_LEN : count;
        LOGE_IF(memcpy_s(reinterpret_cast<void*>(destAddress), sizePerChunk, reinterpret_cast<void*>(srcAddress),
                         countPerChunk) != EOK) << "memcpy_s fail";
        size -= sizePerChunk;
        count -= countPerChunk;
        destAddress += sizePerChunk;
        srcAddress += sizePerChunk;
    }
}
} // namespace common
