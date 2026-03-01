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

#ifndef COMMON_COMPONENTS_HEAP_COLLECTOR_REGION_BITMAP_H
#define COMMON_COMPONENTS_HEAP_COLLECTOR_REGION_BITMAP_H

#include "common_components/base/immortal_wrapper.h"
#include "common_components/base/mem_utils.h"
#include "common_components/base/sys_call.h"
#include "common_components/heap/heap.h"
#if defined(__linux__) || defined(PANDA_TARGET_OHOS) || defined(__APPLE__)
#include <sys/mman.h>
#endif

namespace common {
static constexpr size_t kBitsPerByte = 8;
static constexpr size_t kMarkedBytesPerBit = 8;
static constexpr size_t kBitsPerWord = sizeof(uint64_t) * kBitsPerByte;
static constexpr size_t kBytesPerWord = sizeof(uint64_t) / sizeof(uint8_t);
struct RegionBitmap {
    static constexpr uint8_t factor = 16;
    std::atomic<uint16_t> partLiveBytes[factor];
    std::atomic<size_t> liveBytes;

    // 1 bit marks 8 bytes in region, 64 bits per word.
    // word count = region size / (8 * 64) = region size / 512, should be dynamically decided at runtime.
    std::atomic<size_t> wordCnt;
    std::atomic<uint64_t> markWords[0];

    static size_t GetRegionBitmapSize(size_t regionSize)
    {
        CHECK_CC(regionSize % (kMarkedBytesPerBit * kBitsPerWord) == 0);
        return sizeof(RegionBitmap) + ((regionSize / (kMarkedBytesPerBit * kBitsPerWord)) * sizeof(uint64_t));
    }

    explicit RegionBitmap(size_t regionSize) : liveBytes(0), wordCnt(regionSize / (kMarkedBytesPerBit * kBitsPerWord))
    {}

    bool MarkBits(size_t start)
    {
        size_t headWordIdx = (start / kMarkedBytesPerBit) / kBitsPerWord;
        size_t headMaskBitStart = (start / kMarkedBytesPerBit) % kBitsPerWord;
        uint64_t headMaskBits = static_cast<uint64_t>(1) << headMaskBitStart;
        uint64_t old = markWords[headWordIdx].load();
        bool isMarked = ((old & headMaskBits) != 0);
        if (!isMarked) {
            old = markWords[headWordIdx].fetch_or(headMaskBits);
            isMarked = ((old & headMaskBits) != 0);
            return isMarked;
        }
        return isMarked;
    }

    bool IsMarked(size_t start)
    {
        size_t headWordIdx = (start / kMarkedBytesPerBit) / kBitsPerWord;
        size_t headMaskBitStart = (start / kMarkedBytesPerBit) % kBitsPerWord;
        uint64_t mask = static_cast<uint64_t>(1) << headMaskBitStart;
        bool ret = ((markWords[headWordIdx].load() & mask) != 0);
        return ret;
    }
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_COLLECTOR_REGION_BITMAP_H
