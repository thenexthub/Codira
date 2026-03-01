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

// NOLINTBEGIN(cppcoreguidelines-special-member-functions)

#ifndef COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_H
#define COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_H

#include <vector>

#include "common_interfaces/objects/string/base_string.h"
#include "common_interfaces/base/bit_field.h"

namespace common {
/*
 +-----------------------------+ <-- offset 0
 |      BaseObject fields      |
 +-----------------------------+
 | Padding (uint64_t)          |
 +-----------------------------+
 | LengthAndFlags (uint32_t)   |
 +-----------------------------+
 | RawHashcode (uint32_t)      |
 +-----------------------------+ <-- offset = BaseString::SIZE
 | Parent (BaseString *)       | <-- PARENT_OFFSET
 +-----------------------------+
 | StartIndexAndFlags (uint32_t) | <-- STARTINDEX_AND_FLAGS_OFFSET
 +-----------------------------+ <-- SIZE
*/
/*
 +-------------------------------+
 | StartIndexAndFlags (uint32_t) |
 +-------------------------------+
 Bit layout:
   [0]         : HasBackingStoreBit         (1 bit)
   [1]         : ReserveBit                 (1 bit)
   [2 - 31]    : StartIndexBits             (30 bits)
 */
// The substrings of another string use SlicedString to describe.

/**
 * @class SlicedString
 * @brief Represents a substring that references a slice of another BaseString.
 *
 * Used for substring operations, SlicedString holds a reference to a parent BaseString
 * and an offset indicating the starting index of the slice.
 */
class SlicedString : public BaseString {
public:
    BASE_CAST_CHECK(SlicedString, IsSlicedString);
    NO_MOVE_SEMANTIC_CC(SlicedString);
    NO_COPY_SEMANTIC_CC(SlicedString);
    static constexpr uint32_t MIN_SLICED_STRING_LENGTH = 13;
    static constexpr size_t PARENT_OFFSET = BaseString::SIZE;
    static constexpr uint32_t START_INDEX_BITS_NUM = 30U;
    static constexpr uint32_t REF_FIELDS_COUNT = 1;

    using HasBackingStoreBit = BitField<bool, 0>;                                  // 1
    using ReserveBit = HasBackingStoreBit::NextFlag;                               // 1
    using StartIndexBits = ReserveBit::NextField<uint32_t, START_INDEX_BITS_NUM>;  // 30
    static_assert(StartIndexBits::START_BIT + StartIndexBits::SIZE == sizeof(uint32_t) * BITS_PER_BYTE,
                  "StartIndexBits does not match the field size");
    // NOLINTNEXTLINE(misc-redundant-expression)
    static_assert(StartIndexBits::SIZE == LengthBits::SIZE, "The size of startIndex should be same with Length");

    POINTER_FIELD(Parent, PARENT_OFFSET, STARTINDEX_AND_FLAGS_OFFSET)
    PRIMITIVE_FIELD(StartIndexAndFlags, uint32_t, STARTINDEX_AND_FLAGS_OFFSET, SIZE);

    /**
     * @brief Create a SlicedString backed by a parent BaseString.
     * @tparam Allocator Callable allocator.
     * @tparam WriteBarrier A callable used to manage memory writes.
     * @param allocator Allocator instance.
     * @param writeBarrier Memory write barrier.
     * @param parent Read-only handle to parent string.
     * @return SlicedString pointer.
     */
    template <typename Allocator, typename WriteBarrier,
              objects_traits::enable_if_is_allocate<Allocator, BaseObject *> = 0,
              objects_traits::enable_if_is_write_barrier<WriteBarrier> = 0>
    static SlicedString *Create(Allocator &&allocator, WriteBarrier &&writeBarrier, ReadOnlyHandle<BaseString> parent);
    /**
     * @brief Get the start index of the sliced region.
     * @return Start index into parent string.
     */
    uint32_t GetStartIndex() const;

    /**
     * @brief Set the start index for this slice.
     * @param startIndex Index value to set.
     */
    void SetStartIndex(uint32_t startIndex);

    /**
     * @brief Check if the string has an allocated backing store.
     * @return true if the string has its own buffer; false if it references parent.
     */
    bool GetHasBackingStore() const;

    /**
     * @brief Set whether this sliced string has a backing store.
     * @param hasBackingStore true if buffer is independently allocated.
     */
    void SetHasBackingStore(bool hasBackingStore);

    /**
     * @brief Get UTF-16 character at given index with optional bounds check.
     *
     * Computes the actual index relative to the parent BaseString.
     *
     * @tparam verify If true, index is verified for validity.
     * @tparam ReadBarrier Callable for safe memory access.
     * @param readBarrier Memory read barrier.
     * @param index Index into the sliced string (not the parent).
     * @return UTF-16 character code unit.
     */
    template <bool VERIFY = true, typename ReadBarrier>
    uint16_t Get(ReadBarrier &&readBarrier, int32_t index) const;
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_H

// NOLINTEND(cppcoreguidelines-special-member-functions)
