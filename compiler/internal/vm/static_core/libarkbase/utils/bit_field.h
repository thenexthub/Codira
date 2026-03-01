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

#ifndef LIBPANDABASE_UTILS_BIT_FIELD_H_
#define LIBPANDABASE_UTILS_BIT_FIELD_H_

#include <limits>
#include "libarkbase/macros.h"

namespace ark {

/*
 * Auxiliary static class that provides access to bits range within an integer value.
 */
template <typename T, size_t START, size_t BITS_NUM = 1>
class BitField {
    static constexpr unsigned BITS_PER_BYTE = 8;

    static_assert(START < sizeof(uint64_t) * BITS_PER_BYTE, "Invalid position");
    static_assert(BITS_NUM <= sizeof(uint64_t) * BITS_PER_BYTE, "Invalid size");
    static_assert(BITS_NUM + START <= sizeof(uint64_t) * BITS_PER_BYTE, "Invalid position + size");

public:
    using ValueType = T;
    static constexpr unsigned START_BIT = START;
    static constexpr unsigned END_BIT = START + BITS_NUM;
    static constexpr unsigned SIZE = BITS_NUM;

    /*
     * This is static class and should not be instantiated.
     */
    BitField() = delete;

    virtual ~BitField() = delete;

    NO_COPY_SEMANTIC(BitField);
    NO_MOVE_SEMANTIC(BitField);

    /*
     * Make BitField type that follows right after current bit range.
     *
     *  If we have
     *   BitField<T, 0, 9>
     * then
     *   BitField<T, 0, 9>::NextField<T,3>
     * will be equal to
     *   BitField<T, 9, 3>
     *
     * It is helpful when we need to specify chain of fields.
     */
    template <typename T2, unsigned BITS_NUM2>
    using NextField = BitField<T2, START + BITS_NUM, BITS_NUM2>;

    /*
     * Make Flag field that follows right after current bit range.
     * Same as NextField, but no need to specify number of bits, it is always 1.
     */
    using NextFlag = BitField<bool, START + BITS_NUM, 1>;

public:
    /*
     * Return maximum value that fits bit range [START_BIT : START_BIT+END_BIT]
     */
    static constexpr uint64_t MaxValue()
    {
        return (1LLU << BITS_NUM) - 1;
    }

    /*
     * Return mask of bit range, f.e. 0b1110 for BitField<T, 1, 3>
     */
    static constexpr uint64_t Mask()
    {
        return MaxValue() << START;
    }

    /*
     * Check if given value fits into the bit field
     */
    static constexpr bool IsValid(T value)
    {
        return (static_cast<uint64_t>(value) & ~MaxValue()) == 0;
    }

    /*
     * Set 'value' to current bit range [START_BIT : START_BIT+END_BIT] within the 'stor' parameter.
     */
    template <typename Stor>
    static constexpr void Set(T value, Stor *stor)
    {
        static_assert(END_BIT <= std::numeric_limits<Stor>::digits);
        *stor = (*stor & ~Mask()) | Encode(value);
    }

    /*
     * Return bit range [START_BIT : START_BIT+END_BIT] value from given integer 'value'
     */
    static constexpr T Get(uint64_t value)
    {
        return static_cast<T>((value >> START) & MaxValue());
    }

    /*
     * Encode 'value' to current bit range [START_BIT : START_BIT+END_BIT] and return it
     */
    static constexpr uint64_t Encode(T value)
    {
        ASSERT(IsValid(value));
        return (static_cast<uint64_t>(value) << START);
    }

    /*
     * Update 'value' to current bit range [START_BIT : START_BIT+END_BIT] and return it
     */
    static constexpr uint64_t Update(uint64_t oldValue, T value)
    {
        return (oldValue & ~Mask()) | Encode(value);
    }

    /*
     * Decode from value
     */
    static constexpr T Decode(uint64_t value)
    {
        return Get(value);
    }
};

}  // namespace ark

#endif  // LIBPANDABASE_UTILS_BIT_FIELD_H_
