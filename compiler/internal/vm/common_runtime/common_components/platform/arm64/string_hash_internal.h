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

#ifndef COMMON_COMPONENTS_PLATFORM_STRING_HASH_ARM64_H
#define COMMON_COMPONENTS_PLATFORM_STRING_HASH_ARM64_H

#include <cstdint>

#include <arm_neon.h>

#include "common_components/base/config.h"
#include "common_components/platform/string_hash.h"

namespace common {
class StringHashInternal {
friend class StringHashHelper;
private:
    template <typename T>
    static uint32_t ComputeHashForDataOfLongString(const T *data, size_t size,
                                                   uint32_t hashSeed)
    {
        constexpr uint32_t blockSize = StringHash::BLOCK_SIZE;
        constexpr uint32_t scale = StringHash::BLOCK_MULTIPLY;
        uint32_t hash[blockSize] = {};
        uint32_t index = 0;
        uint32_t remainder = size & (blockSize - 1);
        switch (remainder) {
#define CASE(N) case (N): \
    hash[blockSize - (N)] = data[index++] * StringHash::MULTIPLIER[blockSize - (N)]; [[fallthrough]]
            CASE(StringHash::SIZE_3);
            CASE(StringHash::SIZE_2);
            CASE(StringHash::SIZE_1);
#undef CASE
            default:
                break;
        }
        hash[0] += hashSeed * StringHash::MULTIPLIER[blockSize - 1 - remainder];

        uint32_t dataMul[blockSize] = {};
        for (; index < size; index += blockSize) {
            for (size_t i = 0; i < blockSize; i++) {
                dataMul[i] = data[index + i] * StringHash::MULTIPLIER[i];
                hash[i] = hash[i] * scale + dataMul[i];
            }
        }
        uint32_t hashTotal = 0;
        for (size_t i = 0; i < blockSize; i++) {
            hashTotal += hash[i];
        }
        return hashTotal;
    }

    template <>
    uint32_t ComputeHashForDataOfLongString<uint8_t>(const uint8_t *data,
        size_t size, uint32_t hashSeed)
    {
        const uint32x4_t multiplierVec = vld1q_u32(StringHash::MULTIPLIER);
        constexpr uint32_t multiplierHash = StringHash::MULTIPLIER[0] * StringHash::MULTIPLIER[2];

        uint32_t hash = hashSeed;
        const uint8_t *dataEnd = data + size;
        const uint8_t *vecEnd = data + (size & (~15));
        const uint8_t *p = data;
        constexpr size_t UINT8_LOOP_SIZE = 16; // neon 128bit / uint8_t 8bit = 16
        for (; p < vecEnd; p += UINT8_LOOP_SIZE) {
            uint8x16_t dataVec8 = vld1q_u8(p);
            uint16x8_t dataVec16_1 = vmovl_u8(vget_low_u16(dataVec8));
            uint16x8_t dataVec16_2 = vmovl_u8(vget_high_u16(dataVec8));
            uint32x4_t dataVec32_1 = vmovl_u16(vget_low_u16(dataVec16_1));
            uint32x4_t dataVec32_3 = vmovl_u16(vget_low_u16(dataVec16_2));
            uint32x4_t dataVec32_2 = vmovl_u16(vget_high_u16(dataVec16_1));
            uint32x4_t dataVec32_4 = vmovl_u16(vget_high_u16(dataVec16_2));

            dataVec32_1 = vmulq_u32(dataVec32_1, multiplierVec);
            hash = hash * multiplierHash + vaddvq_u32(dataVec32_1);

            dataVec32_2 = vmulq_u32(dataVec32_2, multiplierVec);
            hash = hash * multiplierHash + vaddvq_u32(dataVec32_2);

            dataVec32_3 = vmulq_u32(dataVec32_3, multiplierVec);
            hash = hash * multiplierHash + vaddvq_u32(dataVec32_3);

            dataVec32_4 = vmulq_u32(dataVec32_4, multiplierVec);
            hash = hash * multiplierHash + vaddvq_u32(dataVec32_4);
        }

        for (; p < dataEnd; p++) {
            hash = (hash << static_cast<uint32_t>(StringHash::HASH_SHIFT)) - hash + *p;
        }
        return hash;
    }

    template <>
    uint32_t ComputeHashForDataOfLongString<uint16_t>(const uint16_t *data,
        size_t size, uint32_t hashSeed)
    {
        const uint32x4_t multiplierVec = vld1q_u32(StringHash::MULTIPLIER);
        constexpr uint32_t multiplierHash = StringHash::MULTIPLIER[0] * StringHash::MULTIPLIER[2];

        uint32_t hash = hashSeed;
        const uint16_t *dataEnd = data + size;
        const uint16_t *vecEnd = data + (size & (~7));
        const uint16_t *p = data;
        constexpr size_t UINT16_LOOP_SIZE = 8; // neon 128bit / uint16_t 16bit = 8
        for (; p < vecEnd; p += UINT16_LOOP_SIZE) {
            uint16x8_t dataVec16 = vld1q_u16(p);
            uint32x4_t dataVec32_1 = vmovl_u16(vget_low_u16(dataVec16));
            dataVec32_1 = vmulq_u32(dataVec32_1, multiplierVec);
            hash = hash * multiplierHash + vaddvq_u32(dataVec32_1);

            uint32x4_t dataVec32_2 = vmovl_u16(vget_high_u16(dataVec16));
            dataVec32_2 = vmulq_u32(dataVec32_2, multiplierVec);
            hash = hash * multiplierHash + vaddvq_u32(dataVec32_2);
        }

        for (; p < dataEnd; p++) {
            hash = (hash << static_cast<uint32_t>(StringHash::HASH_SHIFT)) - hash + *p;
        }
        return hash;
    }
};
}  // namespace common
#endif  // COMMON_COMPONENTS_PLATFORM_STRING_HASH_ARM64_H