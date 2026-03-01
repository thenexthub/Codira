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

#ifndef PANDA_RUNTIME_CORETYPES_COMMON_STRING_H_
#define PANDA_RUNTIME_CORETYPES_COMMON_STRING_H_

#include <cstdint>
#include "common_interfaces/objects/string/base_string-inl.h"
#include "common_interfaces/objects/string/line_string-inl.h"
#include "common_interfaces/objects/base_class.h"
#include "libarkbase/utils/utf.h"
#include "runtime/include/language_context.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/object_accessor.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::coretypes {
class String : public ObjectHeader {
public:
    static constexpr uint32_t STRING_LENGTH_SHIFT = common::BaseString::LengthBits::START_BIT;

    static void SetCompressedStringsEnabled(bool val)
    {
        LineString::SetCompressedStringsEnabled(val);
    }

    static bool GetCompressedStringsEnabled()
    {
        return LineString::GetCompressedStringsEnabled();
    }

    PANDA_PUBLIC_API static String *CreateFromMUtf8(const uint8_t *mutf8Data, size_t mutf8Length, uint32_t utf16Length,
                                                    bool canBeCompressed, const LanguageContext &ctx, PandaVM *vm,
                                                    bool movable = true, bool pinned = false);

    PANDA_PUBLIC_API static String *CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                    bool canBeCompressed, const LanguageContext &ctx, PandaVM *vm,
                                                    bool movable = true, bool pinned = false);

    PANDA_PUBLIC_API static String *CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                    const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                    bool pinned = false);
    PANDA_PUBLIC_API static String *CreateFromMUtf8(const uint8_t *mutf8Data, const LanguageContext &ctx, PandaVM *vm,
                                                    bool movable = true, bool pinned = false);
    PANDA_PUBLIC_API static String *CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length,
                                                    uint32_t utf16Length, const LanguageContext &ctx, PandaVM *vm,
                                                    bool movable, bool pinned);

    PANDA_PUBLIC_API static String *CreateFromUtf8(const uint8_t *utf8Data, uint32_t utf8Length,
                                                   const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                   bool pinned = false);

    PANDA_PUBLIC_API static String *CreateFromUtf16(const uint16_t *utf16Data, uint32_t utf16Length,
                                                    bool canBeCompressed, const LanguageContext &ctx, PandaVM *vm,
                                                    bool movable = true, bool pinned = false);

    PANDA_PUBLIC_API static String *CreateFromUtf16(const uint16_t *utf16Data, uint32_t utf16Length,
                                                    const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                    bool pinned = false);
    PANDA_PUBLIC_API static String *CreateEmptyString(const LanguageContext &ctx, PandaVM *vm);

    /**
     * @brief create string from another string
     * @param [in] str : original string
     * @return The string created
     */
    PANDA_PUBLIC_API static String *CreateFromString(String *str, const LanguageContext &ctx, PandaVM *vm);

    /**
     * @brief extract an char array from src string with utf16Length chars , positioned from the start char from src
     * string
     * @param [in] src : original string
     * @param [in] start : the start index in original string
     * @param [in] utf16Length : how many number of chars to extract
     * @return the char array extracted
     */
    PANDA_PUBLIC_API static Array *GetChars(String *src, uint32_t start, uint32_t utf16Length,
                                            const LanguageContext &ctx);

    /**
     * @brief Alloc a LineString with specified chars
     * @param [in]length : number of chars stored in LineString
     * @param [in]compressed : true if compressedStringsEnabled_ and all ASCII chars stored , indicates that can be
     * stored one byte one char in mutf8 format
     * @return The LineString created
     */
    static String *AllocLineStringObject(ManagedThread *thread, size_t length, bool compressed,
                                         const LanguageContext &ctx, PandaVM *vm = nullptr, bool movable = true,
                                         bool pinned = false);

    /**
     * @brief make slice of src string
     * @param [in]src : parent string , will flatten it
     * @param [in]start : startIndex to the parent string
     * @param [in]length : number of chars to get
     * @return The SlicedString created
     */
    static String *GetSlicedString(String *src, uint32_t start, uint32_t length, const LanguageContext &ctx,
                                   PandaVM *vm, bool movable = true, bool pinned = false);

    /**
     * @brief Create TreeString from given child Strings
     * @param [in]left : left child string
     * @param [in]right : right child string
     * @param [in]length : equals left.length + right.length , indicates the whole number of chars in tree
     * @param [in]compressed : true if all chars stored in tree are ASCII
     * @return The TreeString created
     */
    static String *CreateTreeString(ManagedThread *thread, VMHandle<String> &leftHandle, VMHandle<String> &rightHandle,
                                    uint32_t length, bool compressed, const LanguageContext &ctx, PandaVM *vm,
                                    bool movable = true, bool pinned = false);

    /**
     * @brief Concat two compressed Strings
     * @param [in]str1Handle : the first string to be concated
     * @param [in]str2Handle : the second string to be concated
     * @return The concated string
     */
    static String *ConcatLineStringCompressed(VMHandle<String> &str1Handle, VMHandle<String> &str2Handle,
                                              const LanguageContext &ctx, PandaVM *vm);

    /**
     * @brief Concat two uncompressed Strings
     * @param [in]str1Handle : the first string to be concated
     * @param [in]str2Handle : the second string to be concated
     * @return The concated string
     */
    static String *ConcatLineStringUnCompressed(VMHandle<String> &str1Handle, VMHandle<String> &str2Handle,
                                                const LanguageContext &ctx, PandaVM *vm);

    static String *Concat(String *str1, String *str2, const LanguageContext &ctx, PandaVM *vm);

    PANDA_PUBLIC_API static String *CreateNewStringFromChars(uint32_t offset, uint32_t length, Array *chararray,
                                                             const LanguageContext &ctx, PandaVM *vm);

    PANDA_PUBLIC_API static String *CreateNewStringFromBytes(uint32_t offset, uint32_t length, uint32_t highByte,
                                                             Array *bytearray, const LanguageContext &ctx, PandaVM *vm);

    static String *FastSubString(String *src, uint32_t start, uint32_t utf16Length, const LanguageContext &ctx,
                                 PandaVM *vm = nullptr);
    static String *FastSubUtf16String(String *src, uint32_t start, uint32_t length, const LanguageContext &ctx);
    static String *FastSubUtf8String(String *src, uint32_t start, uint32_t length, const LanguageContext &ctx);

    static String *DoReplace(String *src, uint16_t oldC, uint16_t newC, const LanguageContext &ctx, PandaVM *vm);

    static bool CanBeCompressedUtf16(const uint16_t *utf16Data, uint32_t utf16Length, uint16_t non);
    static bool CanBeCompressedMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length, uint16_t non);
    static bool CanBeCompressedMUtf8(const uint8_t *mutf8Data);

    int32_t IndexOf(String *rhs, const LanguageContext &ctx, int pos = 0);
    int32_t LastIndexOf(String *rhs, const LanguageContext &ctx, int pos = INT32_MAX);

    static int32_t IndexOf(VMHandle<String> &receiver, VMHandle<String> &search, const LanguageContext &ctx,
                           int pos = 0);

    static int32_t LastIndexOf(VMHandle<String> &receiver, VMHandle<String> &search, const LanguageContext &ctx,
                               int pos);

    static int32_t Compare(VMHandle<String> &left, VMHandle<String> &right, const LanguageContext &ctx);

    /**
     * @brief compare this string to another string specified by rstr
     * @param [in] rstr : another string
     * @return negative if this string less than rstr string according to dictionary order
     *         zero if this string equals to rstr string
     *         positive if this string greater than rstr string
     */
    PANDA_PUBLIC_API int32_t Compare(String *rstr, const LanguageContext &ctx);

    static bool StringsAreEqual(String *str1, String *str2);
    PANDA_PUBLIC_API static bool StringsAreEqualMUtf8(String *str1, const uint8_t *mutf8Data, uint32_t utf16Length);
    static bool StringsAreEqualMUtf8(String *str1, const uint8_t *mutf8Data, uint32_t utf16Length,
                                     bool canBeCompressed);

    PANDA_PUBLIC_API static bool StringsAreEqualUtf16(String *str1, const uint16_t *utf16Data, uint32_t utf16DataLength)
    {
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        return common::BaseString::StringsAreEqualUtf16(readBarrier, str1->ToString(), utf16Data, utf16DataLength);
    }

    static bool IsMutf8EqualsUtf16(const uint8_t *utf8Data, const uint16_t *utf16Data, uint32_t utf16DataLength);

    static uint32_t ComputeHashcodeMutf8(const uint8_t *mutf8Data, [[maybe_unused]] uint32_t utf16Length,
                                         bool canBeCompressed)
    {
        return common::BaseString::ComputeHashcodeUtf8(mutf8Data, utf::Mutf8Size(mutf8Data), canBeCompressed);
    }

    static uint32_t ComputeHashcodeMutf8(const uint8_t *mutf8Data, uint32_t utf16Length)
    {
        bool canBeCompressed = CanBeCompressedMUtf8(mutf8Data);
        return ComputeHashcodeMutf8(mutf8Data, utf16Length, canBeCompressed);
    }

    static uint32_t ComputeHashcodeUtf16(const uint16_t *utf16Data, uint32_t length)
    {
        return common::BaseString::ComputeHashcodeUtf16(utf16Data, length);
    }

    static std::pair<int32_t, int32_t> NormalizeSubStringIndexes(int32_t beginIndex, int32_t endIndex,
                                                                 const coretypes::String *str)
    {
        auto strLen = str->GetLength();
        std::pair<int32_t, int32_t> normIndexes = {beginIndex, endIndex};

        // If begin_index < 0, then it is assumed to be equal to zero.
        if (normIndexes.first < 0) {
            normIndexes.first = 0;
        } else if (static_cast<decltype(strLen)>(normIndexes.first) > strLen) {
            // If begin_index > str_len, then it is assumed to be equal to str_len.
            normIndexes.first = static_cast<int32_t>(strLen);
        }
        // If end_index < 0, then it is assumed to be equal to zero.
        if (normIndexes.second < 0) {
            normIndexes.second = 0;
        } else if (static_cast<decltype(strLen)>(normIndexes.second) > strLen) {
            // If end_index > str_len, then it is assumed to be equal to str_len.
            normIndexes.second = static_cast<int32_t>(strLen);
        }
        // If begin_index > end_index, then these are swapped.
        if (normIndexes.first > normIndexes.second) {
            std::swap(normIndexes.first, normIndexes.second);
        }
        ASSERT((normIndexes.second - normIndexes.first) >= 0);
        return normIndexes;
    }

    Array *ToCharArray(const LanguageContext &ctx)
    {
        return GetChars(this, 0, GetLength(), ctx);
    }

    inline size_t CopyDataMUtf8(uint8_t *buf, size_t maxLength, bool isCString)
    {
        // May need alternative implementation like copydatautf8 in 1.0
        if (isCString) {
            ASSERT(maxLength != 0);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            buf[maxLength - 1] = '\0';
            return CopyDataRegionMUtf8(buf, 0, GetLength(), maxLength) + 1;  // add place for zero at the end
        }

        return CopyDataRegionMUtf8(buf, 0, GetLength(), maxLength);
    }

    size_t CopyDataRegionMUtf8(uint8_t *buf, size_t start, size_t length, size_t maxLength)
    {
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        // check the difference between CopyDataRegionUtf8 and CopyDataRegionMUtf8
        return ToString()->CopyDataRegionUtf8(std::move(readBarrier), buf, start, length, maxLength);
    }

    uint32_t CopyDataUtf16(uint16_t *buf, uint32_t maxLength)
    {
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        return ToString()->CopyDataUtf16(std::move(readBarrier), buf, maxLength);
    }

    /**
     * @brief copy data from this string to buf
     * @param [in] buf : dest buf
     * @param [in] start : start char offset in this string
     * @param [in] length : how many chars to copy
     * @param [in] maxLength : max chars of dest buf
     * @return how many bytes to copy
     */
    uint32_t CopyDataRegionUtf16(uint16_t *buf, uint32_t start, uint32_t length, uint32_t maxLength)
    {
        if (length > maxLength) {
            return 0;
        }
        uint32_t len = GetLength();
        if (start + length > len) {
            return 0;
        }
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        if (IsUtf16()) {
            std::vector<uint16_t> tmpBuf;
            const uint16_t *data = ToString()->GetUtf16DataFlat(std::move(readBarrier), ToString(), tmpBuf);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(buf, sizeof(uint16_t) * maxLength, data + start, ComputeDataSizeUtf16(length)) != EOK) {
                LOG(FATAL, RUNTIME) << __func__ << " length is higher than buf size";
            }
        } else {
            std::vector<uint8_t> tmpBuf;
            const uint8_t *data = ToString()->GetUtf8DataFlat(std::move(readBarrier), ToString(), tmpBuf);
            const uint8_t *src8 = data + start;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            for (uint32_t i = 0; i < length; ++i) {
                buf[i] = src8[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }
        }
        return length;
    }

    static String *Cast(ObjectHeader *object)
    {
        return static_cast<String *>(object);
    }

    static String *Cast(common::BaseString *str)
    {
        return reinterpret_cast<String *>(str);
    }

    static String *Cast(LineString *str)
    {
        return reinterpret_cast<String *>(str);
    }

    static LineString *Cast(String *str)
    {
        return reinterpret_cast<LineString *>(str);
    }

    static size_t ComputeSizeMUtf8(uint32_t mutf8Length)
    {
        return LineString::ComputeSizeMUtf8(mutf8Length);
    }

    static size_t ComputeDataSizeUtf16(uint32_t length)
    {
        return length * sizeof(uint16_t);
    }

    static size_t ComputeSizeUtf16(uint32_t utf16Length)
    {
        return LineString::ComputeSizeUtf16(utf16Length);
    }

    bool IsEmpty() const
    {
        // do not shift right length because it is always zero for empty string
        return GetLength() == 0;
    }

    uint32_t GetHashcode()
    {
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };

        return ToString()->GetHashcode(std::move(readBarrier));
    }

    common::BaseString *ToString()
    {
        return common::BaseString::Cast(reinterpret_cast<common::BaseObject *>(this));
    }

    const common::BaseString *ToStringConst() const
    {
        return common::BaseString::ConstCast(reinterpret_cast<const common::BaseObject *>(this));
    }

    static constexpr uint32_t GetLengthOffset()
    {
        return common::BaseString::LENGTH_AND_FLAGS_OFFSET;
    }

    static constexpr uint32_t GetHashcodeOffset()
    {
        return common::BaseString::MIX_HASHCODE_OFFSET;
    }

    static constexpr uint32_t GetDataOffset()
    {
        return common::LineString::DATA_OFFSET;
    }

    uint32_t GetLength() const
    {
        return ToStringConst()->GetLength();
    }

    size_t GetUtf16Length() const
    {
        return GetLength();
    }

    size_t GetMUtf8Length()
    {
        if (!IsUtf16()) {
            return GetLength() + 1;
        }
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        return ToString()->GetUtf8Length(std::move(readBarrier), true, false);
    }

    size_t GetUtf8Length()
    {
        if (!IsUtf16()) {
            return GetLength();
        }
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        return ToString()->GetUtf8Length(std::move(readBarrier), false, true) - 1;
    }

    /**
     * @brief Check whether a UTF-16 code unit is an ASCII character.
     *
     * Determines whether the given 16-bit character is within the standard ASCII range (0x01–0x7F).
     *
     * @param data The UTF-16 code unit to check.
     * @return true if the character is ASCII; false otherwise.
     */
    static bool IsASCIICharacter(uint16_t data)
    {
        return common::BaseString::IsASCIICharacter(data);
    }

    bool IsUtf16() const
    {
        return ToStringConst()->IsUtf16();
    }

    bool IsUtf8() const
    {
        return IsMUtf8();
    }

    bool IsMUtf8() const
    {
        return !IsUtf16();
    }

    bool IsLineString() const
    {
        return ToStringConst()->IsLineString();
    }

    bool IsTreeString() const
    {
        return ToStringConst()->IsTreeString();
    }

    bool IsSlicedString() const
    {
        return ToStringConst()->IsSlicedString();
    }

    common::LineString *ToLineString()
    {
        ASSERT(IsLineString());
        return common::LineString::Cast(ToString());
    }

    const common::SlicedString *ToSlicedString() const
    {
        ASSERT(IsSlicedString());
        return common::SlicedString::ConstCast(ToStringConst());
    }

    common::TreeString *ToTreeString()
    {
        ASSERT(IsTreeString());
        return common::TreeString::Cast(ToString());
    }

    uint16_t *GetDataUtf16()
    {
        ASSERT_PRINT(IsUtf16(), "String: Read data as utf16 for mutf8 string");
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
        };
        std::vector<uint16_t> tmpBuf;
        return const_cast<uint16_t *>(
            common::BaseString::GetUtf16DataFlat(std::move(readBarrier), ToStringConst(), tmpBuf));
    }

    uint16_t *GetTreeStringDataUtf16(PandaVector<uint16_t> &buf)
    {
        ASSERT_PRINT(IsTreeString(), "TreeString: only treeString can use utf16 interface");
        ASSERT_PRINT(IsUtf16(), "String: Read data as utf16 for mutf8 string");
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
        };
        return const_cast<uint16_t *>(
            common::BaseString::GetUtf16DataFlat(std::move(readBarrier), ToStringConst(), buf));
    }

    uint8_t *GetDataUtf8()
    {
        ASSERT_PRINT(IsUtf8(), "String: Read data as utf8 for utf16 string");
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
        };
        std::vector<uint8_t> tmpBuf;
        return const_cast<uint8_t *>(
            common::BaseString::GetUtf8DataFlat(std::move(readBarrier), ToStringConst(), tmpBuf));
    }

    uint8_t *GetTreeStringDataUtf8(PandaVector<uint8_t> &buf)
    {
        ASSERT_PRINT(IsTreeString(), "TreeString: only treeString can use utf8 interface");
        ASSERT_PRINT(IsUtf8(), "String: Read data as utf8 for utf16 string");
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
        };
        return const_cast<uint8_t *>(common::BaseString::GetUtf8DataFlat(std::move(readBarrier), ToStringConst(), buf));
    }

    uint8_t *GetDataMUtf8()
    {
        ASSERT_PRINT(IsUtf8(), "String: Read data as mutf8 for utf16 string");
        return GetDataUtf8();
    }

    uint8_t *GetTreeStringDataMUtf8(PandaVector<uint8_t> &buf)
    {
        return GetTreeStringDataUtf8(buf);
    }

    inline uint8_t *GetDataUtf8Writable()
    {
        ASSERT_PRINT(IsUtf8(), "String: Read data as utf8 for utf16 string");
        return ToLineString()->GetDataUtf8Writable();
    }

    inline uint16_t *GetDataUtf16Writable()
    {
        ASSERT_PRINT(IsUtf16(), "String: Read data as utf16 for mutf8 string");
        return ToLineString()->GetDataUtf16Writable();
    }

    size_t ObjectSize() const
    {
        if (IsLineString()) {
            return common::LineString::ObjectSize(ToStringConst());
        }
        if (IsSlicedString()) {
            return common::SlicedString::SIZE;
        }
        if (IsTreeString()) {
            return common::TreeString::SIZE;
        }
        UNREACHABLE();
    }

    /**
     * @brief extract char from this string positioned at index
     * @param [in] index : index to positioned
     * @return the char
     */
    template <bool VERIFY = true>
    uint16_t At(int32_t index)
    {
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(obj, offset));
        };
        return ToString()->At<VERIFY>(std::move(readBarrier), index);
    }

    /**
     * @brief copy data from src to dest , dest is specified by this line string
     * @param [in] src : original data
     * @param [in] start : write to dest positioned at start offset
     * @param [in] destSize :  dest max size
     * @param [in] length : how many chars to copy
     */
    void WriteData(String *src, uint32_t start, uint32_t destSize, uint32_t length)
    {
        ASSERT(IsLineString());
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        ToLineString()->WriteData(std::move(readBarrier), src->ToString(), start, destSize, length);
    }

    /**
     * @brief copy data from this string to buf
     * @param [in] buf : dest buf
     * @param [in] start : start offset in this string
     * @param [in] length : number of bytes to convert and copy
     * @param [in] maxLength : max length of dest buf
     * @note if this string is utf16, then it will convert the whole string to buffer
     * @return how many bytes to copy
     */
    size_t CopyDataRegionUtf8(uint8_t *buf, size_t start, size_t length, size_t maxLength)
    {
        if (length > maxLength) {
            return 0;
        }
        uint32_t len = GetUtf8Length();
        if (start + length > len) {
            return 0;
        }
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        if (!IsUtf16()) {
            return ToString()->CopyDataRegionUtf8(std::move(readBarrier), buf, start, length, maxLength);
        }
        length = this->GetUtf16Length();
        std::vector<uint16_t> tmpBuf;
        const uint16_t *data = ToString()->GetUtf16DataFlat(std::move(readBarrier), ToString(), tmpBuf);
        return ark::utf::ConvertRegionUtf16ToUtf8(data, buf, length, maxLength, start, false);
    }

protected:
    /**
     * @brief Alloc a SlicedString
     * @return The SlicedString created
     */
    static String *AllocSlicedStringObject(const LanguageContext &ctx, PandaVM *vm = nullptr, bool movable = true,
                                           bool pinned = false);

    /**
     * @brief Alloc a TreeString
     * @return The TreeString created
     */
    static String *AllocTreeStringObject(ManagedThread *thread, const LanguageContext &ctx, PandaVM *vm = nullptr,
                                         bool movable = true, bool pinned = false);

private:
    // In last bit of length_ we store if this string is compressed or not.
    // In last second bit of length_ we store if this string is intern or not.
    FIELD_UNUSED uint32_t length_;
    FIELD_UNUSED uint32_t hashcode_;
};

constexpr uint32_t STRING_LENGTH_OFFSET = sizeof(ObjectHeader);
static_assert(STRING_LENGTH_OFFSET == ark::coretypes::String::GetLengthOffset());

constexpr uint32_t STRING_HASHCODE_OFFSET = STRING_LENGTH_OFFSET + sizeof(uint32_t);
static_assert(STRING_HASHCODE_OFFSET == ark::coretypes::String::GetHashcodeOffset());

constexpr uint32_t STRING_DATA_OFFSET = STRING_HASHCODE_OFFSET + sizeof(uint32_t);
static_assert(STRING_DATA_OFFSET == ark::coretypes::String::GetDataOffset());
constexpr uint32_t STRING_TYPE_LINE = static_cast<uint32_t>(common::ObjectType::LINE_STRING);
constexpr uint32_t STRING_TYPE_SLICE = static_cast<uint32_t>(common::ObjectType::SLICED_STRING);
constexpr uint32_t STRING_TYPE_TREE = static_cast<uint32_t>(common::ObjectType::TREE_STRING);

}  // namespace ark::coretypes

#endif  // PANDA_RUNTIME_CORETYPES_COMMON_STRING_H_
