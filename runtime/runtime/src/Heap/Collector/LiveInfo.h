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

#ifndef MRT_LIVE_INFO_H
#define MRT_LIVE_INFO_H
#include "Base/ImmortalWrapper.h"
#include "Base/Log.h"
#include "Base/MemUtils.h"
#include "Base/SysCall.h"
#include "Heap/Heap.h"
#if defined(__linux__) || defined(hongmeng) || defined(__APPLE__)
#include <sys/mman.h>
#endif

namespace MapleRuntime {
constexpr size_t kBitsPerByte = 8;
constexpr size_t kMarkedBytesPerBit = 8;
constexpr size_t kBitsPerWord = sizeof(uint64_t) * kBitsPerByte;
class RegionInfo;
struct RegionBitmap {
    static constexpr uint8_t factor = 16;
    std::atomic<uint16_t> partLiveBytes[factor];
    std::atomic<size_t> liveBytes;
    // 1 bit marks the 64 bits in region.
    // element count = region size / (8 * 64) = region size / 512, should be dynamically decided at runtime.
    std::atomic<size_t> wordCnt;
    std::atomic<uint64_t> markWords[0];

    static size_t GetRegionBitmapSize(size_t regionSize)
    {
        return sizeof(RegionBitmap) + ((regionSize / (kMarkedBytesPerBit * kBitsPerWord)) * sizeof(uint64_t));
    }

    struct BitMaskInfo {
        size_t headWordIdx;
        uint64_t headMaskBits;
        size_t tailWordCnt; // count of mask words excludes the start mask
        uint64_t lastMaskBits;
    };

    static void GetBitMaskInfo(size_t start, size_t byteCnt, BitMaskInfo& maskInfo)
    {
        size_t headWordIdx = (start / kMarkedBytesPerBit) / kBitsPerWord;
        size_t headMaskBitStart = (start / kMarkedBytesPerBit) % kBitsPerWord;
        maskInfo.headWordIdx = headWordIdx;

        size_t headBitCnt = kBitsPerWord - headMaskBitStart;
        size_t maskBitCnt = byteCnt / kMarkedBytesPerBit;
        if (maskBitCnt >= headBitCnt) {
            size_t tailBitCnt = maskBitCnt - headBitCnt;
            size_t tailWordCnt = (tailBitCnt + kBitsPerWord - 1) / kBitsPerWord;
            size_t lastBitCnt = tailBitCnt % kBitsPerWord;
            uint64_t lastMaskBits = (static_cast<uint64_t>(1) << lastBitCnt) - 1;
            maskInfo.headMaskBits = ~((static_cast<uint64_t>(1) << headMaskBitStart) - 1);
            maskInfo.tailWordCnt = tailWordCnt;
            maskInfo.lastMaskBits = lastMaskBits;
        } else {
            size_t headMaskBitEnd = headMaskBitStart + maskBitCnt;
            uint64_t headMaskBits = (static_cast<uint64_t>(1) << headMaskBitEnd) - 1;
            maskInfo.headMaskBits = (headMaskBits >> headMaskBitStart) << headMaskBitStart;
            maskInfo.tailWordCnt = 0;
            maskInfo.lastMaskBits = 0;
        }
    }

    bool ApplyBitMaskInfo(const BitMaskInfo& maskInfo, size_t byteCnt, size_t regionSize)
    {
        uint64_t old = markWords[maskInfo.headWordIdx].load();
        bool isMarked = ((old & maskInfo.headMaskBits) != 0);
        if (isMarked) {
            return isMarked;
        }
        old = markWords[maskInfo.headWordIdx].fetch_or(maskInfo.headMaskBits);
        isMarked = ((old & maskInfo.headMaskBits) != 0);
        if (isMarked) {
            return isMarked;
        }
        size_t markWordSize = regionSize / (kMarkedBytesPerBit * kBitsPerWord);
        uint8_t calFactor = factor > markWordSize ? markWordSize : factor;
        if (markWordSize % calFactor) {
            // The markWordSize needs to be rounded up to ensure it is divisible by calFactor.
            markWordSize = markWordSize + calFactor - markWordSize % calFactor;
        }
        partLiveBytes[maskInfo.headWordIdx / (markWordSize / calFactor)].fetch_add(
            __builtin_popcountll(maskInfo.headMaskBits));
        liveBytes.fetch_add(byteCnt);

        if (maskInfo.tailWordCnt > 0) {
            size_t lastWordIdx = maskInfo.headWordIdx + maskInfo.tailWordCnt;
            if (maskInfo.lastMaskBits != 0) {
                for (size_t idx = maskInfo.headWordIdx + 1; idx < lastWordIdx; idx++) {
                    uint64_t zeros = markWords[idx].fetch_or(~static_cast<uint64_t>(0));
                    partLiveBytes[idx / (markWordSize / calFactor)].fetch_add(kBitsPerWord);
                    DCHECK(zeros == 0);
                }
                markWords[lastWordIdx].fetch_or(maskInfo.lastMaskBits);
                partLiveBytes[lastWordIdx / (markWordSize / calFactor)].fetch_add(
                    __builtin_popcountll(maskInfo.lastMaskBits));
            } else {
                for (size_t idx = maskInfo.headWordIdx + 1; idx <= lastWordIdx; idx++) {
                    uint64_t zeros = markWords[idx].fetch_or(~static_cast<uint64_t>(0));
                    partLiveBytes[idx / (markWordSize / calFactor)].fetch_add(kBitsPerWord);
                    DCHECK(zeros == 0);
                }
            }
        }
        return isMarked;
    }

    explicit RegionBitmap(size_t regionSize) : liveBytes(0), wordCnt(regionSize / (kMarkedBytesPerBit * kBitsPerWord))
    {}

    bool MarkBits(size_t start, size_t byteCnt, size_t regionSize)
    {
        BitMaskInfo maskInfo;
        GetBitMaskInfo(start, byteCnt, maskInfo);
        return ApplyBitMaskInfo(maskInfo, byteCnt, regionSize);
    }

    bool IsMarked(size_t start)
    {
        size_t headWordIdx = (start / kMarkedBytesPerBit) / kBitsPerWord;
        size_t headMaskBitStart = (start / kMarkedBytesPerBit) % kBitsPerWord;
        uint64_t mask = static_cast<uint64_t>(1) << headMaskBitStart;
        bool ret = ((markWords[headWordIdx].load() & mask) != 0);
        return ret;
    }

    struct PreMaskInfo {
        int8_t partIndex;
        uint64_t mask;
        ssize_t StepSize;
        ssize_t index;
    };

    static void GetPreMaskInfo(size_t offset, size_t regionSize, PreMaskInfo& maskInfo)
    {
        maskInfo.index = offset / (kBitsPerWord * kMarkedBytesPerBit);
        size_t markWordSize = regionSize / (kMarkedBytesPerBit * kBitsPerWord);
        uint8_t calFactor = factor > markWordSize ? markWordSize : factor;
        if (markWordSize % calFactor) {
            // The markWordSize needs to be rounded up to ensure it is divisible by calFactor.
            markWordSize = markWordSize + calFactor - markWordSize % calFactor;
        }
        maskInfo.partIndex = maskInfo.index / (markWordSize / calFactor) - 1;
        size_t bitIndex = (offset / kMarkedBytesPerBit) % kBitsPerWord;
        maskInfo.mask = (static_cast<uint64_t>(1) << bitIndex) - 1;
        maskInfo.StepSize = markWordSize / calFactor;
    }

    uint64_t GetPreLiveBytes(const PreMaskInfo& maskInfo)
    {
        uint64_t preLiveBits = 0;
        ssize_t partStartIndex = 0;
        int8_t partIndex = maskInfo.partIndex;
        while (partIndex >= 0) {
            preLiveBits += partLiveBytes[partIndex--];
            partStartIndex += maskInfo.StepSize;
        }
        ssize_t index = maskInfo.index;
        size_t liveBits = __builtin_popcountll(markWords[index].load() & maskInfo.mask);

        if (index == partStartIndex) {
            return (preLiveBits + liveBits) * kMarkedBytesPerBit;
        }
        index--;
        while (index >= partStartIndex) {
            uint64_t makeBit = markWords[index].load();
            liveBits += __builtin_popcountll(makeBit);
            index--;
        }
        return (preLiveBits + liveBits) * kMarkedBytesPerBit;
    }
};
struct LiveInfo {
    static constexpr MAddress TEMPORARY_PTR = 0x1234;
    RegionInfo* bindedRegion = nullptr;
    RegionBitmap* markBitmap = nullptr;
    RegionBitmap* resurrectBitmap = nullptr;
    RegionBitmap* enqueueBitmap = nullptr;

    uint64_t GetPreLiveBytes(size_t offset, size_t regionSize)
    {
        RegionBitmap::PreMaskInfo maskInfo;
        RegionBitmap::GetPreMaskInfo(offset, regionSize, maskInfo);
        uint64_t liveBytes = 0;
        if (markBitmap != nullptr) {
            liveBytes += markBitmap->GetPreLiveBytes(maskInfo);
        }
        if (resurrectBitmap != nullptr) {
            liveBytes += resurrectBitmap->GetPreLiveBytes(maskInfo);
        }
        return liveBytes;
    }
};

struct RouteInfo {
    static constexpr uint32_t INVALID_VALUE = std::numeric_limits<uint32_t>::max();
    uintptr_t toRegion1StartAddress = 0;
    uint32_t toRegion1UsedBytes = 0;
    uint32_t toRegion2Idx = 0;

    uintptr_t GetRoute(uint64_t preLiveBytes);

    void SetRouteInfo(uintptr_t to1, uint32_t to1used = 0, uint32_t to2 = INVALID_VALUE)
    {
        toRegion1StartAddress = to1;
        toRegion1UsedBytes = to1used;
        toRegion2Idx = to2;
    }
};
} // namespace MapleRuntime
#endif // MRT_LIVE_INFO_H
