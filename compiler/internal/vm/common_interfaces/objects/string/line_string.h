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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_H
#define COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_H

#include "objects/string/base_string.h"

namespace common {
/*
 +-----------------------------+ <-- offset 0
 |      BaseObject fields      |
 +-----------------------------+ <-- offset = BaseObjectSize()
 | Padding (uint64_t)          |
 +-----------------------------+
 | LengthAndFlags (uint32_t)   |
 +-----------------------------+
 | RawHashcode (uint32_t)      |
 +-----------------------------+ <-- offset = BaseString::SIZE
 |     String Data (UTF8/16)   | <-- DATA_OFFSET = BaseString::SIZE
 |     ...                     |
 +-----------------------------+
 */
// The LineString abstract class captures sequential string values, only LineString can store chars data

/**
 * @class LineString
 * @brief LineString represents sequential flat string layout with actual character data.
 *
 * Derived from BaseString, this class is used to store and manage character sequences
 * directly within the object memory layout. It supports UTF-8 and UTF-16 encodings.
 */
class LineString : public BaseString {
public:
    BASE_CAST_CHECK(LineString, IsLineString);
    NO_MOVE_SEMANTIC_CC(LineString);
    NO_COPY_SEMANTIC_CC(LineString);

    static constexpr size_t ALIGNMENT_8_BYTES = 8;
    static constexpr uint32_t MAX_LENGTH = 0x0FFFFFF0U;  // Maximum of the string, (1 << 28) -16
    static constexpr uint32_t INIT_LENGTH_TIMES = 4;
    // DATA_OFFSET: the string data stored after the string header.
    // Data can be stored in utf8 or utf16 form according to compressed bit.
    static constexpr size_t DATA_OFFSET = BaseString::SIZE;  // DATA_OFFSET equal to Empty String size

    void Trim(uint32_t newLength);

    /**
     * @brief Create a line string from UTF-8 encoded data.
     * @tparam Allocator Callable allocator for allocating BaseObject.
     * @param allocator Allocator instance.
     * @param utf8Data Pointer to UTF-8 data.
     * @param utf8Len Length in bytes.
     * @param canBeCompress Whether data is ASCII-only and compressible.
     * @return Pointer to created LineString instance.
     */
    template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *> = 0>
    static LineString *CreateFromUtf8(Allocator &&allocator, const uint8_t *utf8Data, uint32_t utf8Len,
                                      bool canBeCompress);

    /**
     * @brief Create a line string from a compressed UTF-8 substring.
     * @tparam Allocator Callable allocator for allocating BaseObject.
     * @param allocator Allocator instance.
     * @param string Source string handle.
     * @param offset Start position in UTF-8 bytes.
     * @param utf8Len Number of bytes to extract.
     * @return Pointer to created LineString instance.
     */
    template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *> = 0>
    static LineString *CreateFromUtf8CompressedSubString(Allocator &&allocator, const ReadOnlyHandle<BaseString> string,
                                                         uint32_t offset, uint32_t utf8Len);

    /**
     * @brief Create a LineString instance.
     * @tparam Allocator Callable allocator.
     * @param allocator Allocator object.
     * @param length String length in code units.
     * @param compressed Whether to use UTF-8 compression.
     * @return LineString pointer.
     */
    template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *> = 0>
    static LineString *Create(Allocator &&allocator, size_t length, bool compressed);

    /**
     * @brief Create a line string from UTF-16 encoded data.
     * @tparam Allocator Callable allocator for allocating BaseObject.
     * @param allocator Allocator instance.
     * @param utf16Data Pointer to UTF-16 data.
     * @param utf16Len Length in code units.
     * @param canBeCompress Whether string can be compressed.
     * @return Pointer to created LineString instance.
     */
    template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *> = 0>
    static LineString *CreateFromUtf16(Allocator &&allocator, const uint16_t *utf16Data, uint32_t utf16Len,
                                       bool canBeCompress);

    /**
     * @brief Get the pointer to the character buffer (UTF-16 view).
     * @return Pointer to character buffer.
     */
    uint16_t *GetData() const;
    /**
     * @brief Retrieve string's raw UTF-8 data.
     * @return Pointer to UTF-8 buffer.
     */
    const uint8_t *GetDataUtf8() const;

    /**
     * @brief Retrieve string's raw UTF-16 data.
     * @return Pointer to UTF-16 buffer.
     */
    const uint16_t *GetDataUtf16() const;

    /**
     * @brief Retrieve a writable UTF-8 buffer for direct mutation.
     * @return Writable pointer to UTF-8 data.
     * @note Requires IsLineString and UTF-8 encoding.
     */
    uint8_t *GetDataUtf8Writable() const;

    /**
     * @brief Retrieve a writable UTF-16 buffer for direct mutation.
     * @return Writable pointer to UTF-16 data.
     * @note Requires IsLineString and UTF-16 encoding.
     */
    uint16_t *GetDataUtf16Writable() const;

    /**
     * @brief Get character at specific index.
     * @tparam VERIFY Whether bounds checking should be performed.
     * @param index Index into the character buffer.
     * @return UTF-16 code unit at the given index.
     */
    template <bool VERIFY = true>
    uint16_t Get(int32_t index) const;

    /**
     * @brief Set character value at a specific index.
     * @param index Position in the string.
     * @param src Character to store.
     */
    void Set(uint32_t index, uint16_t src);

    /**
     * @brief Compute memory size for a UTF-8 encoded string.
     * @param utf8Len Length in bytes.
     * @return Total memory footprint in bytes.
     */
    static size_t ComputeSizeUtf8(uint32_t utf8Len);

    /**
     * @brief Compute memory size for a UTF-16 encoded string.
     * @param utf16Len Length in UTF-16 code units.
     * @return Total memory footprint in bytes.
     */
    static size_t ComputeSizeUtf16(uint32_t utf16Len);

    /**
     * @brief Compute total memory size of a given BaseString instance.
     * @param str BaseString pointer.
     * @return Size in bytes including metadata and payload.
     */
    static size_t ObjectSize(const BaseString *str);

    /**
     * @brief Get size of the string payload only.
     * @param str BaseString pointer.
     * @return Size in bytes of the character buffer.
     */
    static size_t DataSize(BaseString *str);

    /**
     * @brief Write a portion of another string's data into this string.
     *
     * This method copies a slice of characters from a source string (`src`) into the current string,
     * starting at the specified offset. It is typically used to fill flat string buffers during construction.
     *
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A memory read barrier for safely accessing the source string.
     * @param src Source string to copy from.
     * @param start Start index within the source string (in characters).
     * @param destSize Total capacity available in the destination string.
     * @param length Number of characters to copy from the source.
     */
    template <typename ReadBarrier>
    void WriteData(ReadBarrier &&readBarrier, BaseString *src, uint32_t start, uint32_t destSize, uint32_t length);

    /**
     * @brief Check if a given BaseString is eligible for compression.
     * @param string Pointer to BaseString.
     * @return true if compressible.
     */
    static bool CanBeCompressed(const LineString *string);
};


inline uint16_t *LineString::GetData() const
{
    return reinterpret_cast<uint16_t *>(reinterpret_cast<uintptr_t>(this) + DATA_OFFSET);
}

inline const uint8_t *LineString::GetDataUtf8() const
{
    DCHECK_CC(IsUtf8() && "BaseString: Read data as utf8 for utf16 string");
    return reinterpret_cast<uint8_t *>(GetData());
}

inline const uint16_t *LineString::GetDataUtf16() const
{
    DCHECK_CC(IsUtf16() && "BaseString: Read data as utf16 for utf8 string");
    return GetData();
}

inline uint8_t *LineString::GetDataUtf8Writable() const
{
    DCHECK_CC(IsUtf8() && "BaseString: Read data as utf8 for utf16 string");
    return reinterpret_cast<uint8_t *>(GetData());
}

inline uint16_t *LineString::GetDataUtf16Writable() const
{
    DCHECK_CC(IsUtf16() && "BaseString: Read data as utf16 for utf8 string");
    return GetData();
}
}  // namespace common
// NOLINTEND(cppcoreguidelines-special-member-functions)

#endif //COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_H

