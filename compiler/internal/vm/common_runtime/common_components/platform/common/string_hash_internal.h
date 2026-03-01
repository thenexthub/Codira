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

#ifndef COMMON_COMPONENTS_PLATFORM_STRING_HASH_COMMON_H
#define COMMON_COMPONENTS_PLATFORM_STRING_HASH_COMMON_H

#include <cstdint>

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
};
}  // namespace common
#endif  // COMMON_COMPONENTS_PLATFORM_STRING_HASH_COMMON_H