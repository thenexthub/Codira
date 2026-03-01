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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//             modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

#ifndef COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_DECLARE_H
#define COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_DECLARE_H

#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/objects/utils/field_macro.h"
#include "common_interfaces/objects/utils/objects_traits.h"
#include "common_interfaces/objects/readonly_handle.h"
#include "common_interfaces/base/bit_field.h"
#include "common_interfaces/objects/utils/span.h"

#include <type_traits>
#include <vector>
#include <cstring>

namespace common {
class LineString;
class TreeString;
class SlicedString;

/*
 +-----------------------------+ <-- offset 0
 |      BaseObject fields      |
 +-----------------------------+ <-- offset = BaseObjectSize()
 | Padding (uint64_t)          | <-- PADDING_OFFSET
 +-----------------------------+
 | LengthAndFlags (uint32_t)   | <-- LENGTH_AND_FLAGS_OFFSET
 +-----------------------------+
 | RawHashcode (uint32_t)      | <-- RAW_HASHCODE_OFFSET
 +-----------------------------+ <-- SIZE (== BaseString::SIZE)
 */
/*
 +-----------------------------+
 |   LengthAndFlags (uint32_t) |
 +-----------------------------+
 Bit layout:
   [0]         : CompressedStatusBit         (1 bit)
   [1]         : IsInternBit                 (1 bit)
   [2 - 31]    : LengthBits                  (30 bits)
 */

/**
 * @class BaseString
 * @brief Base class for internal string representation supporting multiple encodings and layouts.
 *
 * Provides common interface for string types like LineString, TreeString, and SlicedString.
 */
class BaseString : public BaseObject {
public:
    BASE_CAST_CHECK(BaseString, IsString);
    NO_MOVE_SEMANTIC_CC(BaseString);
    NO_COPY_SEMANTIC_CC(BaseString);
    static constexpr uint32_t RAW_HASH_LENGTH = 31;
    static constexpr uint32_t IS_INTEGER_MASK = 1U << RAW_HASH_LENGTH;
    static constexpr uint32_t MAX_INTEGER_HASH_NUMBER = 0x3B9AC9FF;
    static constexpr uint32_t MAX_CACHED_INTEGER_SIZE = 9;
    static constexpr size_t MAX_STRING_LENGTH = 0x40000000U;  // 30 bits for string length, 2 bits for special meaning
    static constexpr uint32_t MAX_ELEMENT_INDEX_LEN = 10;
    static constexpr size_t HASH_SHIFT = 5;
#if (defined(PANDA_TARGET_64) && !defined(PANDA_32_BIT_MANAGED_POINTER))
    static constexpr size_t PADDING_OFFSET = BaseObjectSize();
#else
    static constexpr size_t LENGTH_AND_FLAGS_OFFSET = BaseObjectSize();
#endif
    static constexpr uint32_t STRING_LENGTH_BITS_NUM = 30;

    enum CompressedStatus {
        STRING_COMPRESSED,
        STRING_UNCOMPRESSED,
    };

    enum TrimMode : uint8_t {
        TRIM,
        TRIM_START,
        TRIM_END,
    };

    enum IsIntegerStatus {
        NOT_INTEGER = 0,
        IS_INTEGER,
    };

    enum ConcatOptStatus {
        BEGIN_STRING_ADD = 1,
        IN_STRING_ADD,
        CONFIRMED_IN_STRING_ADD,
        END_STRING_ADD,
        INVALID_STRING_ADD,
    };

    using CompressedStatusBit = BitField<CompressedStatus, 0>;                    // 1
    using IsInternBit = CompressedStatusBit::NextFlag;                            // 1
    using LengthBits = IsInternBit::NextField<uint32_t, STRING_LENGTH_BITS_NUM>;  // 30
    static_assert(LengthBits::START_BIT + LengthBits::SIZE == sizeof(uint32_t) * BITS_PER_BYTE,
                  "LengthBits does not match the field size");

#if (defined(PANDA_TARGET_64) && !defined(PANDA_32_BIT_MANAGED_POINTER))
    // When enable cmcgc, the ObjectHeader in 1.2 is 128 bits, to align with it, a 64bits padding is needed.
    PRIMITIVE_FIELD(padding, uint64_t, PADDING_OFFSET, LENGTH_AND_FLAGS_OFFSET)
#endif
    PRIMITIVE_FIELD(LengthAndFlags, uint32_t, LENGTH_AND_FLAGS_OFFSET, MIX_HASHCODE_OFFSET)

    using RawHashcode = BitField<uint32_t, 0, RAW_HASH_LENGTH>;       // 31
    using IsIntegerBit = RawHashcode::NextField<IsIntegerStatus, 1>;  // 1
    // In last bit of mix_hash we store if this string is small-integer number or not.
    PRIMITIVE_FIELD(MixHashcode, uint32_t, MIX_HASHCODE_OFFSET, SIZE)

    static inline uint32_t MixHashcode(uint32_t hashcode, bool isInteger);

    template <typename ReadBarrier>
    inline bool IsInteger(ReadBarrier &&readBarrier)
    {
        uint32_t hashcode = GetHashcode(std::forward<ReadBarrier>(readBarrier));
        return IsIntegerBit::Decode(hashcode) == IS_INTEGER;
    }

    bool IsString() const
    {
        return GetBaseClass()->IsString();
    }
    /**
     * @brief Return whether the underlying string is a LineString.
     * @return true if LineString, false otherwise.
     */
    bool IsLineString() const
    {
        return GetBaseClass()->IsLineString();
    }

    /**
     * @brief Return whether the string is stored as a TreeString.
     * @return true if TreeString, false otherwise.
     */
    bool IsTreeString() const
    {
        return GetBaseClass()->IsTreeString();
    }

    /**
     * @brief Return whether the string is stored as a SlicedString.
     * @return true if SlicedString, false otherwise.
     */
    bool IsSlicedString() const
    {
        return GetBaseClass()->IsSlicedString();
    }

    /**
     * @brief Return whether the string is stored using UTF-8 encoding.
     * @return true if UTF-8, false if UTF-16.
     */
    bool IsUtf8() const;

    /**
     * @brief Return whether the string is stored using UTF-16 encoding.
     * @return true if UTF-16, false if UTF-8.
     */
    bool IsUtf16() const;

    /**
     * @brief Return string length in code units.
     * @return Number of UTF-8 or UTF-16 units.
     */
    uint32_t GetLength() const;

    /**
     * @brief Initialize flags and length fields of the string.
     * @param length Length of the string.
     * @param compressed Whether it is UTF-8 (true) or UTF-16 (false).
     * @param isIntern Whether string is interned in string table.
     */
    void InitLengthAndFlags(uint32_t length, bool compressed = false, bool isIntern = false);

    /**
     * @brief Get the length of the string when encoded in UTF-8.
     *
     * This method traverses the string to compute the number of bytes required for its UTF-8 representation.
     * It can also be used to compute a safe buffer size for encoding.
     *
     * @tparam ReadBarrier A callable used to handle memory access barriers.
     * @param readBarrier A memory read barrier used to access underlying string data.
     * @param modify Whether the internal cached state may be modified during computation.
     * @param isGetBufferSize Whether the result is intended to reserve a buffer (may affect padding behavior).
     * @return Number of bytes required to encode the string in UTF-8.
     */
    template <typename ReadBarrier>
    size_t GetUtf8Length(ReadBarrier &&readBarrier, bool modify = true, bool isGetBufferSize = false) const;

    /**
     * @brief Mark this string as interned.
     */
    void SetIsInternString();

    /**
     * @brief Check if string is interned.
     * @return true if interned, false otherwise.
     */
    bool IsInternString() const;

    /**
     * @brief Clear the interned string flag.
     */
    void ClearInternStringFlag();

    /**
     * @brief Try to get cached hashcode.
     * @param hash Output pointer to store hash if available.
     * @return true if hashcode is present, false otherwise.
     */
    bool TryGetHashCode(uint32_t *hash) const;

    /**
     * @brief Compute hashcode on-demand if not already cached.
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A barrier callable object.
     * @return The hashcode.
     */
    template <typename ReadBarrier>
    uint32_t PUBLIC_API GetHashcode(ReadBarrier &&readBarrier);

    template <class ReadBarrier>
    uint32_t ComputeHashcode(ReadBarrier &&readBarrier) const;

    template <typename ReadBarrier>
    uint32_t ComputeRawHashcode32bits(ReadBarrier &&readBarrier) const;

    /**
     * @brief Compute the raw hashcode based on content.
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A barrier callable object.
     * @return The computed hash value and if this string is an integer.
     */
    template <typename ReadBarrier>
    std::pair<uint32_t, bool> ComputeRawHashcode(ReadBarrier &&readBarrier) const;

    /**
     * @brief Retrieve character at a specific index.
     * @tparam verify Enable index boundary checks.
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A barrier callable object.
     * @param index The index to retrieve.
     * @return UTF-16 code unit.
     */
    template <bool verify = true, typename ReadBarrier>
    uint16_t At(ReadBarrier &&readBarrier, int32_t index) const;

    /**
     * @brief Compare the string content with a virtual concatenation of str1 + str2.
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A barrier callable object.
     * @param str1 Left part string.
     * @param str2 Right part string.
     * @return true if content equals str1+str2.
     */
    template <typename ReadBarrier>
    bool EqualToSplicedString(ReadBarrier &&readBarrier, const BaseString *str1, const BaseString *str2);

    /**
     * @brief Write string content to a UTF-8 buffer.
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A barrier callable object.
     * @param buf Output buffer.
     * @param maxLength Maximum number of bytes to write.
     * @param isWriteBuffer Whether the target is a real output buffer or dummy.
     * @return Number of bytes written (excluding null terminator).
     */
    template <typename ReadBarrier>
    size_t WriteUtf8(ReadBarrier &&readBarrier, uint8_t *buf, size_t maxLength, bool isWriteBuffer = false) const;

    /**
     * @brief Copy the UTF-16 encoded content of the string into the provided buffer.
     *
     * This method traverses the string structure and copies the characters into the target UTF-16 buffer.
     * It handles any decoding or materialization needed depending on the internal representation.
     *
     * @tparam ReadBarrier A callable used to handle memory access barriers.
     * @param readBarrier A memory read barrier for accessing string data.
     * @param buf Destination buffer to copy UTF-16 data into.
     * @param length Number of characters to copy from the string.
     * @param bufLength Maximum capacity (in code units) of the destination buffer.
     * @return Number of UTF-16 code units actually copied.
     */
    template <typename ReadBarrier>
    size_t CopyDataToUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t length, uint32_t bufLength) const;

    /**
     * @brief Write the UTF-16 encoded content of the string into the provided buffer, possibly truncated.
     *
     * Unlike full-copy functions, this method allows writing even if the target buffer is smaller than
     * the total string length, enabling partial extraction or safe truncation.
     *
     * @tparam ReadBarrier A callable used to handle memory access barriers.
     * @param readBarrier A memory read barrier for accessing string data.
     * @param buf Destination buffer to receive UTF-16 data.
     * @param targetLength Intended number of UTF-16 code units to write from the string.
     * @param bufLength Actual size of the destination buffer (in UTF-16 code units).
     * @return Number of UTF-16 code units actually written into the buffer.
     */
    template <typename ReadBarrier>
    size_t WriteUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t targetLength, uint32_t bufLength) const;

    /**
     * @brief Write the string content into a UTF-8/one-byte-per-char buffer with possible truncation.
     *
     * This function writes at most `maxLength` bytes of data into the target buffer, using single-byte
     * (Latin1/CESU-8) encoding if applicable. It is used for compact output or debug printing.
     *
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A memory read barrier used during data access.
     * @param buf Target buffer for writing one-byte string data.
     * @param maxLength Maximum number of bytes to write.
     * @return Number of bytes actually written into the buffer.
     */
    template <typename ReadBarrier>
    size_t WriteOneByte(ReadBarrier &&readBarrier, uint8_t *buf, size_t maxLength) const;

    /**
     * @brief Copy a specific region of the string into a UTF-8 buffer with optional modification and truncation.
     *
     * This function extracts a substring starting at the given position and writes it in UTF-8 encoding
     * to the provided buffer. It supports partial copying, internal caching, and write-buffer scenarios.
     *
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A memory read barrier for accessing string data.
     * @param buf Destination buffer to receive UTF-8 encoded bytes.
     * @param start Start index within the string (in character index, not byte).
     * @param length Number of characters to convert and copy.
     * @param maxLength Maximum number of bytes allowed to be written into the buffer.
     * @param modify Whether this copy operation is allowed to mutate internal cached state.
     * @param isWriteBuffer Whether the buffer is being used as a writable backend storage.
     * @return Number of UTF-8 bytes written into the buffer.
     */
    template <typename ReadBarrier>
    size_t CopyDataRegionUtf8(ReadBarrier &&readBarrier, uint8_t *buf, size_t start, size_t length, size_t maxLength,
                              bool modify = true, bool isWriteBuffer = false) const;

    /**
     * @brief Copy the entire string content into a UTF-16 buffer up to a maximum number of characters.
     *
     * This function reads the string and writes its UTF-16 representation into the provided buffer,
     * copying at most `maxLength` code units. It is safe for partial copying.
     *
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A memory read barrier for accessing string content.
     * @param buf Target buffer to store UTF-16 code units.
     * @param maxLength Maximum number of UTF-16 code units to write.
     * @return Actual number of UTF-16 code units written.
     */
    template <typename ReadBarrier>
    uint32_t CopyDataUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t maxLength) const;

    /**
     * @brief Convert internal string data to std::u16string.
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A barrier object.
     * @param len Optional length override.
     * @return UTF-16 representation of this string.
     */
    template <typename ReadBarrier>
    std::u16string ToU16String(ReadBarrier &&readBarrier, uint32_t len = 0);

    /**
     * @brief Convert string to UTF-8 and return it as a span backed by a user buffer.
     * @tparam ReadBarrier A callable used to manage memory access.
     * @tparam Vec A std::vector<uint8_t>-compatible type.
     * @param readBarrier A barrier object.
     * @param buf Vector used as backing storage.
     * @param modify Whether UTF-8 conversion may mutate internal state.
     * @param cesu8 Whether to use CESU-8 encoding.
     * @return A span of UTF-8 bytes.
     */
    template <typename ReadBarrier, typename Vec,
              std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int> = 0>
    Span<const uint8_t> ToUtf8Span(ReadBarrier &&readBarrier, Vec &buf, bool modify = true, bool cesu8 = false);

    /**
     * @brief Convert string to UTF-8 in debugger mode (non-CESU-8).
     * @tparam ReadBarrier A callable used to manage memory access.
     * @tparam Vec A std::vector<uint8_t>-compatible type.
     * @param readBarrier A barrier object.
     * @param buf Vector used as backing storage.
     * @param modify Whether UTF-8 conversion may mutate internal state.
     * @return A span of UTF-8 bytes.
     */
    template <typename ReadBarrier, typename Vec,
              std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int> = 0>
    Span<const uint8_t> DebuggerToUtf8Span(ReadBarrier &&readBarrier, Vec &buf, bool modify = true);

    /**
     * @brief Check whether the string is in a flat (contiguous) representation.
     *
     * A flat string has its character data stored in a single contiguous buffer without slicing or tree structure.
     *
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier A memory read barrier used to inspect internal layout.
     * @return true if the string is flat and directly addressable; false otherwise.
     */
    template <typename ReadBarrier>
    bool IsFlat(ReadBarrier &&readBarrier) const;

    /**
     * @brief Get the actual string object type (e.g., LineString, TreeString, etc.).
     *
     * This function returns the internal representation type of the string,
     * which can be used to distinguish between different string layouts or implementations.
     *
     * @return ObjectType enum value representing the string's runtime type.
     */
    ObjectType GetStringType() const;

    /**
     * @brief Compute the hash code of two concatenated string fragments.
     *
     * This function calculates a combined hash value as if the two buffers were joined
     * into a single logical string. It avoids allocating or copying by computing the hash
     * directly over the two parts.
     *
     * @tparam T1 Type of the first character buffer (e.g., uint8_t or uint16_t).
     * @tparam T2 Type of the second character buffer.
     * @param dataFirst Pointer to the first string buffer.
     * @param sizeFirst Length of the first buffer (in code units).
     * @param dataSecond Pointer to the second string buffer.
     * @param sizeSecond Length of the second buffer (in code units).
     * @return Hash code of the virtual concatenation of the two buffers.
     */
    template <class T1, class T2>
    static uint32_t CalculateDataConcatHashCode(const T1 *dataFirst, size_t sizeFirst, const T2 *dataSecond,
                                                size_t sizeSecond);

    template <typename T>
    static bool HashIntegerString(const T *data, size_t size, uint32_t *hash, uint32_t hashSeed);

    /**
     * @brief Compute the hash code of a UTF-8 encoded string buffer.
     *
     * This function computes a hash based on the UTF-8 byte sequence, optionally
     * optimizing for compressible (ASCII-only) content.
     *
     * @param utf8Data Pointer to UTF-8 encoded string data.
     * @param utf8Len Length of the data in bytes.
     * @param canBeCompress Whether the data is compressible (i.e., ASCII-only), which may affect hashing.
     * @return Computed 32-bit hash code.
     */
    static uint32_t ComputeHashcodeUtf8(const uint8_t *utf8Data, size_t utf8Len, bool canBeCompress);

    /**
     * @brief Compute the hash code of a UTF-16 encoded string buffer.
     *
     * This function generates a 32-bit hash code from a UTF-16 sequence, typically
     * for use in hash-based string tables or interning mechanisms.
     *
     * @param utf16Data Pointer to UTF-16 encoded data.
     * @param length Number of UTF-16 code units to include in the hash.
     * @return Computed 32-bit hash code.
     */
    static uint32_t ComputeHashcodeUtf16(const uint16_t *utf16Data, uint32_t length);

    /**
     * @brief Check if two UTF-like spans are equal.
     *
     * Compares two spans character-by-character and assumes that they should have the same length.
     * Intended for fast comparison of raw string segments in internal routines.
     *
     * @tparam T Element type of the first span (e.g. uint8_t or uint16_t).
     * @tparam T1 Element type of the second span.
     * @param str1 First character span.
     * @param str2 Second character span.
     * @return true if the spans are equal in length and content; false otherwise.
     */
    template <typename T, typename T1>
    static bool StringsAreEquals(Span<const T> &str1, Span<const T1> &str2);

    /**
     * @brief Compare a UTF-8 string with a UTF-16 string for equality.
     *
     * This method checks whether the UTF-8 and UTF-16 encoded strings represent the same sequence
     * of characters, accounting for encoding differences.
     *
     * @param utf8Data Pointer to UTF-8 encoded data.
     * @param utf8Len Length of the UTF-8 data in bytes.
     * @param utf16Data Pointer to UTF-16 encoded data.
     * @param utf16Len Length of the UTF-16 data in code units.
     * @return true if both strings represent the same character sequence; false otherwise.
     */
    static bool IsUtf8EqualsUtf16(const uint8_t *utf8Data, size_t utf8Len, const uint16_t *utf16Data,
                                  uint32_t utf16Len);

    /**
     * @brief Compare two BaseString objects by their byte representation.
     *
     * This comparison checks whether the contents of the two strings are byte-wise equal.
     * It does not normalize or canonicalize Unicode sequences. It is intended for internal fast equality checks.
     *
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier Barrier used to safely access memory.
     * @param str1 First string to compare.
     * @param str2 Second string to compare.
     * @return true if the strings are equal in byte content; false otherwise.
     */
    template <typename ReadBarrier>
    static bool StringsAreEqual(ReadBarrier &&readBarrier, BaseString *str1, BaseString *str2);

    /**
     * @brief Compare two BaseString objects with potentially different UTF encodings.
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier Barrier used to safely access memory.
     * @param str1 First string to compare.
     * @param str2 Second string to compare.
     * @return true if the strings are equal in character content; false otherwise.
     */
    template <typename ReadBarrier>
    static bool StringsAreEqualDiffUtfEncoding(ReadBarrier &&readBarrier, BaseString *str1, BaseString *str2);

    /**
     * @brief Compare a BaseString with raw UTF-8 data for equality.
     *
     * Performs a character-by-character comparison between the BaseString and a UTF-8 byte array.
     * Does not modify the internal structure of str1. Less efficient if str1 is not flat.
     * It also allows comparing against compressible (ASCII-only) UTF-8 data.
     *
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier Barrier used to safely access memory.
     * @param str1 Pointer to the BaseString instance.
     * @param dataAddr Pointer to raw UTF-8 data.
     * @param dataLen Length of UTF-8 data in bytes.
     * @param canBeCompress Whether the UTF-8 data is compressible (ASCII-only).
     * @return true if contents are equal; false otherwise.
     */
    template <typename ReadBarrier>
    static bool StringIsEqualUint8Data(ReadBarrier &&readBarrier, const BaseString *str1, const uint8_t *dataAddr,
                                       uint32_t dataLen, bool canBeCompress);

    /**
     * @brief Compare a BaseString with UTF-16 data for equality.
     *
     * Performs a character-by-character comparison between the BaseString and a UTF-16 sequence.
     * It does not normalize Unicode or change internal structure. May be inefficient if str1 is not flat.
     *
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier Barrier used to safely access memory.
     * @param str1 Pointer to the BaseString instance.
     * @param utf16Data Pointer to UTF-16 data.
     * @param utf16Len Length of UTF-16 data in code units.
     * @return true if contents are equal; false otherwise.
     */
    template <typename ReadBarrier>
    static bool StringsAreEqualUtf16(ReadBarrier &&readBarrier, const BaseString *str1, const uint16_t *utf16Data,
                                     uint32_t utf16Len);

    /**
     * @brief Copy characters from a source span to a destination span.
     *
     * Copies up to `count` characters from `src` to `dst`, not exceeding `dstMax`.
     * Safe for use with UTF-8 or UTF-16 spans.
     *
     * @tparam T Character type (uint8_t or uint16_t).
     * @param dst Destination span to write to.
     * @param dstMax Maximum writable elements in destination.
     * @param src Source span to read from.
     * @param count Number of characters to copy.
     * @return true if all `count` characters were copied; false if `dstMax` was insufficient.
     */
    template <typename T>
    static bool MemCopyChars(Span<T> &dst, size_t dstMax, Span<const T> &src, size_t count);

    /**
     * @brief Determine whether string data is eligible for UTF-8 compression.
     * @param utf8Data Pointer to UTF-8 data.
     * @param utf8Len Length in bytes.
     * @return true if compressible, false otherwise.
     */
    static bool CanBeCompressed(const uint8_t *utf8Data, uint32_t utf8Len);

    /**
     * @brief Determine whether UTF-16 data can be losslessly compressed to UTF-8.
     * @param utf16Data Pointer to UTF-16 data.
     * @param utf16Len Length in code units.
     * @return true if compressible, false otherwise.
     */
    static bool CanBeCompressed(const uint16_t *utf16Data, uint32_t utf16Len);

    /**
     * @brief Copy characters from a source to destination buffer.
     *
     * Performs a loop-based copy of `count` characters from `src` to `dst`.
     * Supports conversion between character types (e.g., uint8_t to uint16_t).
     *
     * @tparam DstType Destination character type.
     * @tparam SrcType Source character type.
     * @param dst Pointer to destination character buffer.
     * @param src Pointer to source character buffer.
     * @param count Number of characters to copy.
     */
    template <typename DstType, typename SrcType>
    static void CopyChars(DstType *dst, SrcType *src, uint32_t count);

    /**
     * @brief Compute hash value over a UTF-like character buffer.
     *
     * This function computes a hash code from the given character data using a provided seed.
     * It is used internally to generate consistent hashes for strings, enabling deduplication
     * or fast lookup in hash maps.
     *
     * @tparam T Character type (e.g., uint8_t or uint16_t).
     * @param data Pointer to the character buffer.
     * @param size Number of characters in the buffer.
     * @param hashSeed Initial hash seed for mixing.
     * @return 32-bit computed hash value.
     */
    template <typename T>
    static PUBLIC_API uint32_t ComputeHashForData(const T *data, size_t size, uint32_t hashSeed);

    /**
     * @brief Check whether a UTF-16 code unit is an ASCII character.
     *
     * Determines whether the given 16-bit character is within the standard ASCII range (0x01–0x7F).
     *
     * @param data The UTF-16 code unit to check.
     * @return true if the character is ASCII; false otherwise.
     */
    static bool IsASCIICharacter(uint16_t data);

    /**
     * @brief Find the first occurrence of a substring within another span.
     *
     * Performs forward search of `rhsSp` within `lhsSp`, starting at `pos`.
     *
     * @tparam T1 Character type of the main string.
     * @tparam T2 Character type of the search string.
     * @param lhsSp Span representing the main string.
     * @param rhsSp Span representing the substring to search for.
     * @param pos Position to start the search.
     * @param max Maximum valid index to search within lhsSp.
     * @return The index of the first match, or -1 if not found.
     */
    template <typename T1, typename T2>
    static int32_t IndexOf(Span<const T1> &lhsSp, Span<const T2> &rhsSp, int32_t pos, int32_t max);

    /**
     * @brief Find the last occurrence of a substring within another span.
     *
     * Performs reverse search of `rhsSp` within `lhsSp`, starting at `pos`.
     *
     * @tparam T1 Character type of the main string.
     * @tparam T2 Character type of the search string.
     * @param lhsSp Span representing the main string.
     * @param rhsSp Span representing the substring to search for.
     * @param pos Position to start the reverse search.
     * @return The index of the last match, or -1 if not found.
     */
    template <typename T1, typename T2>
    static int32_t LastIndexOf(Span<const T1> &lhsSp, Span<const T2> &rhsSp, int32_t pos);

    /**
     * @brief Write characters from a BaseString into a buffer.
     *
     * Writes characters into a flat buffer with given max capacity.
     *
     * @tparam Char Character type (uint8_t or uint16_t).
     * @tparam ReadBarrier A callable used to manage memory access.
     * @param readBarrier Memory access handler.
     * @param src Source string.
     * @param buf Destination character buffer.
     * @param maxLength Maximum number of characters to write.
     */
    template <typename Char, typename ReadBarrier>
    static void WriteToFlat(ReadBarrier &&readBarrier, const BaseString *src, Char *buf, uint32_t maxLength);

    /**
     * @brief Write characters from a BaseString into a buffer at a given offset.
     *
     * Similar to WriteToFlat, but begins writing at a specified offset within the string.
     *
     * @tparam Char Character type.
     * @tparam ReadBarrier Memory access handler.
     * @param readBarrier Barrier callable.
     * @param src Source string.
     * @param buf Destination buffer.
     * @param length Number of characters to write.
     * @param pos Offset in the source string.
     */
    template <typename Char, typename ReadBarrier>
    static void WriteToFlatWithPos(ReadBarrier &&readBarrier, BaseString *src, Char *buf, uint32_t length,
                                   uint32_t pos);

    /**
     * @brief Get flat UTF-8 data from BaseString, using a backing vector.
     *
     * Converts the given BaseString into a flat UTF-8 byte sequence. If the internal string
     * is not flat or not in UTF-8 format, conversion is performed and stored into the
     * provided buffer.
     *
     * @tparam ReadBarrier A callable used to safely access memory.
     * @tparam Vec A std::vector<uint8_t>-compatible container.
     * @param readBarrier Memory access handler.
     * @param src Source BaseString to flatten.
     * @param buf Vector used for temporary storage if flattening is needed.
     * @return Pointer to UTF-8 encoded byte array.
     */
    template <typename ReadBarrier, typename Vec,
              std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int> = 0>
    static const uint8_t *GetUtf8DataFlat(ReadBarrier &&readBarrier, const BaseString *src, Vec &buf);

    /**
     * @brief Get UTF-8 data from a non-tree BaseString.
     *
     * @param readBarrier Barrier.
     * @param src Source string.
     * @return Pointer to UTF-8 byte array.
     */
    template <typename ReadBarrier>
    static const uint8_t *GetNonTreeUtf8Data(ReadBarrier &&readBarrier, const BaseString *src);

    /**
     * @brief Get flat UTF-16 data from BaseString, using a backing vector.
     *
     * @tparam ReadBarrier Memory access handler.
     * @tparam Vec A std::vector<uint16_t> compatible type.
     * @param readBarrier Barrier.
     * @param src Source string.
     * @param buf Vector for temporary storage.
     * @return Pointer to UTF-16 character array.
     */
    template <typename ReadBarrier, typename Vec,
              std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint16_t>, int> = 0>
    static const uint16_t *GetUtf16DataFlat(ReadBarrier &&readBarrier, const BaseString *src, Vec &buf);

    /**
     * @brief Get UTF-16 data from a non-tree BaseString.
     *
     * @param readBarrier Barrier.
     * @param src Source string.
     * @return Pointer to UTF-16 character array.
     */
    template <typename ReadBarrier>
    static const uint16_t *GetNonTreeUtf16Data(ReadBarrier &&readBarrier, const BaseString *src);

private:
    static constexpr bool IsStringType(ObjectType type);
};

inline bool BaseString::IsUtf8() const
{
    uint32_t bits = GetLengthAndFlags();
    return CompressedStatusBit::Decode(bits) == STRING_COMPRESSED;
}

inline bool BaseString::IsUtf16() const
{
    uint32_t bits = GetLengthAndFlags();
    return CompressedStatusBit::Decode(bits) == STRING_UNCOMPRESSED;
}

inline uint32_t BaseString::GetLength() const
{
    uint32_t bits = GetLengthAndFlags();
    return LengthBits::Decode(bits);
}

inline void BaseString::InitLengthAndFlags(uint32_t length, bool compressed, bool isIntern)
{
    DCHECK_CC(length < BaseString::MAX_STRING_LENGTH);
    uint32_t newVal = 0;
    newVal = IsInternBit::Update(newVal, isIntern);
    newVal = CompressedStatusBit::Update(newVal, (compressed ? STRING_COMPRESSED : STRING_UNCOMPRESSED));
    newVal = LengthBits::Update(newVal, length);
    SetLengthAndFlags(newVal);
}

inline void BaseString::SetIsInternString()
{
    uint32_t bits = GetLengthAndFlags();
    uint32_t newVal = IsInternBit::Update(bits, true);
    SetLengthAndFlags(newVal);
}

inline bool BaseString::IsInternString() const
{
    uint32_t bits = GetLengthAndFlags();
    return IsInternBit::Decode(bits);
}

inline void BaseString::ClearInternStringFlag()
{
    uint32_t bits = GetLengthAndFlags();
    uint32_t newVal = IsInternBit::Update(bits, false);
    SetLengthAndFlags(newVal);
}

inline bool BaseString::TryGetHashCode(uint32_t *hash) const
{
    uint32_t hashcode = GetMixHashcode();
    if (hashcode == 0 && GetLength() != 0) {
        return false;
    }
    *hash = hashcode;
    return true;
}

// not change this data structure.
// if string is not flat, this func has low efficiency.
template <typename ReadBarrier>
uint32_t PUBLIC_API BaseString::GetHashcode(ReadBarrier &&readBarrier)
{
    uint32_t hashcode = GetMixHashcode();
    // GetLength() == 0 means it's an empty array.No need to computeHashCode again when hashseed is 0.
    if (hashcode == 0 && GetLength() != 0) {
        hashcode = ComputeHashcode(std::forward<ReadBarrier>(readBarrier));
        SetMixHashcode(hashcode);
    }
    return hashcode;
}

// Check that two spans are equal. Should have the same length.
/* static */
template <typename T, typename T1>
bool BaseString::StringsAreEquals(Span<const T> &str1, Span<const T1> &str2)
{
    DCHECK_CC(str1.Size() <= str2.Size());
    size_t size = str1.Size();
    if constexpr (std::is_same_v<T, T1>) {
        return !memcmp(str1.data(), str2.data(), size * sizeof(T));
    } else {
        for (size_t i = 0; i < size; i++) {
            auto left = static_cast<uint16_t>(str1[i]);
            auto right = static_cast<uint16_t>(str2[i]);
            if (left != right) {
                return false;
            }
        }
        return true;
    }
}

inline uint32_t BaseString::MixHashcode(uint32_t hashcode, bool isInteger)
{
    return isInteger ? (hashcode | IS_INTEGER_MASK) : (hashcode & (~IS_INTEGER_MASK));
}
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_DECLARE_H

// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//             modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

