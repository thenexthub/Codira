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

#ifndef COMMON_COMPONENTS_PLATFORM_STRING_HASH_H
#define COMMON_COMPONENTS_PLATFORM_STRING_HASH_H

#include <cstddef>
#include <cstdint>

namespace common {
    class StringHash {
    public:
        static constexpr size_t BLOCK_SIZE = 4;
        static constexpr size_t SIMD_U8_LOOP_SIZE = 8;
        static constexpr size_t MIN_SIZE_FOR_UNROLLING = 32;

        static constexpr uint32_t HASH_SHIFT = 5;
        static constexpr uint32_t HASH_MULTIPLY = 31;
        static constexpr uint32_t BLOCK_MULTIPLY = 31 * 31 * 31 * 31;

        static constexpr uint32_t MULTIPLIER[BLOCK_SIZE] = { 31 * 31 * 31, 31 * 31, 31, 1 };

        static constexpr size_t SIZE_1 = 1;
        static constexpr size_t SIZE_2 = 2;
        static constexpr size_t SIZE_3 = 3;
    };
}  // namespace common
#endif  // COMMON_COMPONENTS_PLATFORM_STRING_HASH_H
