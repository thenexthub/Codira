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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_H
#define COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_H

#include "common_interfaces/objects/string/base_string.h"

namespace common {
/*
 +--------------------------------+ <-- offset 0
 |      BaseObject fields         |
 +--------------------------------+
 | Padding (uint64_t)             |
 +--------------------------------+
 | LengthAndFlags (uint32_t)      |
 +--------------------------------+
 | RawHashcode (uint32_t)         |
 +--------------------------------+ <-- offset = BaseString::SIZE
 | LeftSubString (BaseString *)   | <-- LEFT_OFFSET
 +--------------------------------+
 | RightSubString (BaseString *)  | <-- RIGHT_OFFSET
 +--------------------------------+ <-- SIZE
*/
/**
 * @class TreeString
 * @brief Represents a concatenated string composed of two BaseString components.
 *
 * Used for efficient concatenation of two substrings without allocating a new flat buffer.
 * TreeString keeps references to both left-hand and right-hand BaseStrings and calculates
 * character data on demand.
 */
class TreeString : public BaseString {
public:
    BASE_CAST_CHECK(TreeString, IsTreeString);

    NO_MOVE_SEMANTIC_CC(TreeString);
    NO_COPY_SEMANTIC_CC(TreeString);
    // Minimum length for a tree string
    static constexpr uint32_t MIN_TREE_STRING_LENGTH = 13;
    static constexpr size_t LEFT_OFFSET = BaseString::SIZE;
    static constexpr uint32_t REF_FIELDS_COUNT = 2;

    POINTER_FIELD(LeftSubString, LEFT_OFFSET, RIGHT_OFFSET)
    POINTER_FIELD(RightSubString, RIGHT_OFFSET, SIZE)

    /**
     * @brief Create a TreeString by joining two substrings.
     * @tparam Allocator Callable allocator.
     * @tparam WriteBarrier A callable used to manage memory writes.
     * @param allocator Allocator instance.
     * @param writeBarrier Memory write barrier.
     * @param left Left-hand string.
     * @param right Right-hand string.
     * @param length Total length of joined string.
     * @param compressed Whether string is compressed (UTF-8).
     * @return TreeString pointer.
     */
    template <typename Allocator, typename WriteBarrier,
              objects_traits::enable_if_is_allocate<Allocator, BaseObject *> = 0,
              objects_traits::enable_if_is_write_barrier<WriteBarrier> = 0>
    static TreeString *Create(Allocator &&allocator, WriteBarrier &&writeBarrier, ReadOnlyHandle<BaseString> left,
                              ReadOnlyHandle<BaseString> right, uint32_t length, bool compressed);

    /**
     * @brief Check if the TreeString can be flattened to a single buffer.
     *
     * A TreeString is flat if both its components are flat and directly accessible.
     *
     * @tparam ReadBarrier Callable for safe memory access.
     * @param readBarrier Barrier function for accessing memory.
     * @return true if the string is flat; false otherwise.
     */
    template <typename ReadBarrier>
    bool IsFlat(ReadBarrier &&readBarrier) const;

    /**
     * @brief Get UTF-16 character at given index with optional bounds check.
     * @tparam verify Whether to perform bounds checking.
     * @tparam ReadBarrier Callable for memory access.
     * @param readBarrier Barrier function.
     * @param index Index in the TreeString.
     * @return UTF-16 code unit at specified index.
     */
    template <bool VERIFY = true, typename ReadBarrier>
    uint16_t Get(ReadBarrier &&readBarrier, int32_t index) const;
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_H

// NOLINTEND(cppcoreguidelines-special-member-functions)
