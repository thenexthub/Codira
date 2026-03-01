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

#ifndef LIBPANDABASE_UTILS_BIT_MEMORY_REGION_INL_H
#define LIBPANDABASE_UTILS_BIT_MEMORY_REGION_INL_H

#include "bit_memory_region.h"

#include <iomanip>

namespace panda {

template <typename Base>
void BitMemoryRegion<Base>::Dump(std::ostream &os) const
{
    static constexpr size_t BITS_PER_HEX_DIGIT = 4;
    os << "0x";
    static constexpr size_t BITS_PER_WORD = sizeof(size_t) * BITS_PER_BYTE;
    if (Size() >= BITS_PER_WORD) {
        bool is_zero = true;
        size_t width = BITS_PER_WORD - (BITS_PER_HEX_DIGIT - Size() % BITS_PER_HEX_DIGIT);
        for (ssize_t i = Size() - width; i >= 0; i -= width) {
            auto val = Read(i, width);
            if (val != 0 || !is_zero) {
                if (!is_zero) {
                    os << std::setw(static_cast<int>(width / BITS_PER_HEX_DIGIT)) << std::setfill('0');
                }
                os << std::hex << val;
                is_zero = false;
            }
            if (i == 0) {
                break;
            }
            width = std::min<size_t>(i, BITS_PER_WORD);
        }
        if (is_zero) {
            os << '0';
        }
    } else {
        os << std::hex << ReadAll();
    }
    os << std::dec;
}

template <typename T>
inline std::ostream &operator<<(std::ostream &os, const BitMemoryRegion<T> &region)
{
    region.Dump(os);
    return os;
}

}  // namespace panda

#endif  // LIBPANDABASE_UTILS_BIT_MEMORY_REGION_INL_H
