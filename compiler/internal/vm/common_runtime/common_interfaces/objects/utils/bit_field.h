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

#ifndef COMMON_INTERFACES_OBJECTS_UTILS_BIT_FIELD_H
#define COMMON_INTERFACES_OBJECTS_UTILS_BIT_FIELD_H

#include <limits>
#include <cstdint>
#include "common_interfaces/base/common.h"

namespace panda::common {
/*
 * Auxiliary static class that provides access to bits range within an integer value.
 */
// NOLINTNEXTLINE(readability-identifier-naming)
template <typename T, size_t start, size_t bits_num = 1>
class BitField {
    static constexpr unsigned BITS_PER_BYTE = 8;

    static_assert(start < sizeof(uint64_t) * BITS_PER_BYTE, "Invalid position");
    static_assert(bits_num != 0U, "Invalid size");
    static_assert(bits_num <= sizeof(uint64_t) * BITS_PER_BYTE, "Invalid size");
    static_assert(bits_num + start <= sizeof(uint64_t) * BITS_PER_BYTE, "Invalid position + size");

public:
    using ValueType = T;
    static constexpr unsigned START_BIT = start;
    static constexpr unsigned END_BIT = start + bits_num;
    static constexpr unsigned SIZE = bits_num;

    /*
     * This is static class and should not be instantiated.
     */
    BitField() = delete;

    virtual ~BitField() = delete;

    NO_COPY_SEMANTIC_CC(BitField);
    NO_MOVE_SEMANTIC_CC(BitField);

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
    // NOLINTNEXTLINE(readability-identifier-naming)
    template <typename T2, unsigned bits_num2>
    using NextField = BitField<T2, start + bits_num, bits_num2>;

    /*
     * Make Flag field that follows right after current bit range.
     * Same as NextField, but no need to specify number of bits, it is always 1.
     */
    using NextFlag = BitField<bool, start + bits_num, 1>;

public:
    /*
     * Return maximum value that fits bit range [START_BIT : START_BIT+END_BIT]
     */
    static constexpr uint64_t MaxValue()
    {
        return (1LLU << bits_num) - 1;
    }

    /*
     * Return mask of bit range, f.e. 0b1110 for BitField<T, 1, 3>
     */
    static constexpr uint64_t Mask()
    {
        return MaxValue() << start;
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
        return static_cast<T>((value >> start) & MaxValue());
    }

    /*
     * Encode 'value' to current bit range [START_BIT : START_BIT+END_BIT] and return it
     */
    static constexpr uint64_t Encode(T value)
    {
        ASSERT_COMMON(IsValid(value));
        return (static_cast<uint64_t>(value) << start);
    }

    /*
     * Update 'value' to current bit range [START_BIT : START_BIT+END_BIT] and return it
     */
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr uint64_t Update(uint64_t old_value, T value)
    {
        return (old_value & ~Mask()) | Encode(value);
    }

    /*
     * Decode from value
     */
    static constexpr T Decode(uint64_t value)
    {
        return Get(value);
    }
};
} // namespace panda::common
#endif  // COMMON_INTERFACES_OBJECTS_UTILS_BIT_FIELD_H
