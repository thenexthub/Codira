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
#ifndef PANDA_RUNTIME_CORETYPES_LINE_STRING_H_
#define PANDA_RUNTIME_CORETYPES_LINE_STRING_H_

#include <securec.h>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "common_interfaces/objects/string/base_string-inl.h"
#include "libarkbase/utils/utf.h"
#include "runtime/include/language_context.h"
#include "runtime/include/object_header.h"
#include "runtime/include/exceptions.h"

namespace ark::coretypes {

class Array;
class LineString : public ObjectHeader {
public:
    static constexpr uint32_t STRING_LENGTH_SHIFT = common::BaseString::LengthBits::START_BIT;

    static LineString *Cast(ObjectHeader *object)
    {
        // NOTE(linxiang) to do assert
        return static_cast<LineString *>(object);
    }

    PANDA_PUBLIC_API static LineString *CreateFromMUtf8(const uint8_t *mutf8Data, size_t mutf8Length,
                                                        uint32_t utf16Length, bool canBeCompressed,
                                                        const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                        bool pinned = false);

    PANDA_PUBLIC_API static LineString *CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                        bool canBeCompressed, const LanguageContext &ctx, PandaVM *vm,
                                                        bool movable = true, bool pinned = false);

    PANDA_PUBLIC_API static LineString *CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                        const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                        bool pinned = false);

    PANDA_PUBLIC_API static LineString *CreateFromMUtf8(const uint8_t *mutf8Data, const LanguageContext &ctx,
                                                        PandaVM *vm, bool movable = true, bool pinned = false);

    static LineString *CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length, uint32_t utf16Length,
                                       const LanguageContext &ctx, PandaVM *vm, bool movable, bool pinned);

    PANDA_PUBLIC_API static LineString *CreateFromUtf8(const uint8_t *utf8Data, uint32_t utf8Length,
                                                       const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                       bool pinned = false);

    PANDA_PUBLIC_API static LineString *CreateFromUtf16(const uint16_t *utf16Data, uint32_t utf16Length,
                                                        bool canBeCompressed, const LanguageContext &ctx, PandaVM *vm,
                                                        bool movable = true, bool pinned = false);

    PANDA_PUBLIC_API static LineString *CreateFromUtf16(const uint16_t *utf16Data, uint32_t utf16Length,
                                                        const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                        bool pinned = false);

    PANDA_PUBLIC_API static LineString *CreateEmptyLineString(const LanguageContext &ctx, PandaVM *vm);

    PANDA_PUBLIC_API static LineString *CreateFromLineString(LineString *str, const LanguageContext &ctx, PandaVM *vm);

    static LineString *ConcatInner(LineString *newStr, LineString *str1, LineString *str2);

    PANDA_PUBLIC_API static LineString *Concat(LineString *jstring1, LineString *jstring2, const LanguageContext &ctx,
                                               PandaVM *vm);

    PANDA_PUBLIC_API static LineString *CreateNewLineStringFromChars(uint32_t offset, uint32_t length, Array *chararray,
                                                                     const LanguageContext &ctx, PandaVM *vm);

    PANDA_PUBLIC_API static LineString *AllocLineStringObject(size_t length, bool compressed,
                                                              const LanguageContext &ctx, PandaVM *vm = nullptr,
                                                              bool movable = true, bool pinned = false);

    static LineString *CreateNewLineStringFromBytes(uint32_t offset, uint32_t length, uint32_t highByte,
                                                    Array *bytearray, const LanguageContext &ctx, PandaVM *vm);

    common::BaseString *ToString()
    {
        return common::BaseString::Cast(reinterpret_cast<common::BaseObject *>(this));
    }

    const common::BaseString *ToStringConst() const
    {
        return common::BaseString::ConstCast(reinterpret_cast<const common::BaseObject *>(this));
    }

    common::LineString *ToLineString()
    {
        return common::LineString::Cast(ToString());
    }

    template <bool VERIFY = true>
    uint16_t At(int32_t index)
    {
        auto length = GetLength();
        if (VERIFY) {
            if ((index < 0) || (index >= static_cast<int32_t>(length))) {
                ark::ThrowStringIndexOutOfBoundsException(index, length);
                return 0;
            }
        }

        return ToLineString()->Get(index);
    }

    PANDA_PUBLIC_API int32_t Compare(LineString *rstr);

    PANDA_PUBLIC_API Array *ToCharArray(const LanguageContext &ctx);

    PANDA_PUBLIC_API static Array *GetChars(LineString *src, uint32_t start, uint32_t utf16Length,
                                            const LanguageContext &ctx);

    bool IsUtf16() const
    {
        return compressedStringsEnabled_ ? ToStringConst()->IsUtf16() : true;
    }

    bool IsMUtf8() const
    {
        return !IsUtf16();
    }

    static size_t ComputeDataSizeUtf16(uint32_t length)
    {
        return length * sizeof(uint16_t);
    }

    /// Methods for uncompressed strings (UTF16)
    static size_t ComputeSizeUtf16(uint32_t utf16Length)
    {
        return common::LineString::ComputeSizeUtf16(utf16Length);
    }

    uint16_t *GetDataUtf16()
    {
        return ToLineString()->GetDataUtf16Writable();
    }

    /// Methods for compresses strings (MUTF8 or LATIN1)
    static size_t ComputeSizeMUtf8(uint32_t mutf8Length)
    {
        return common::LineString::ComputeSizeUtf8(mutf8Length);
    }

    uint8_t *GetDataUtf8()
    {
        return ToLineString()->GetDataUtf8Writable();
    }

    /// It's MUtf8 format, but without 0 in the end.
    uint8_t *GetDataMUtf8()
    {
        return ToLineString()->GetDataUtf8Writable();
    }

    size_t GetMUtf8Length()
    {
        if (!IsUtf16()) {
            return GetLength() + 1;  // add place for zero at the end
        }
        return ark::utf::Utf16ToMUtf8Size(GetDataUtf16(), GetLength());
    }

    size_t GetUtf16Length()
    {
        return GetLength();
    }

    size_t GetUtf8Length()
    {
        if (!IsUtf16()) {
            return GetLength();
        }
        return ark::utf::Utf16ToUtf8Size(GetDataUtf16(), GetLength(), false) - 1;
    }

    size_t CopyDataRegionUtf8(uint8_t *buf, size_t start, size_t length, size_t maxLength)
    {
        if (length > maxLength) {
            return 0;
        }
        uint32_t len = GetUtf8Length();
        if (start + length > len) {
            return 0;
        }
        if (!IsUtf16()) {
            constexpr size_t MAX_LEN = std::numeric_limits<size_t>::max() / 2 - 1;
            if (length > MAX_LEN) {
                LOG(FATAL, RUNTIME) << __func__ << " length is higher than half of size_t::max";
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(buf, sizeof(uint8_t) * (maxLength + 1), GetDataUtf8() + start, length) != EOK) {
                LOG(FATAL, RUNTIME) << __func__ << " length is higher than buf size";
            }
            return length;
        }
        length = GetUtf16Length();
        return ark::utf::ConvertRegionUtf16ToUtf8(GetDataUtf16(), buf, length, maxLength, start, false);
    }

    inline size_t CopyDataMUtf8(uint8_t *buf, size_t maxLength, bool isCString)
    {
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
        if (length > maxLength) {
            return 0;
        }
        uint32_t len = GetLength();
        if (start + length > len) {
            return 0;
        }
        if (!IsUtf16()) {
            constexpr size_t MAX_LEN = std::numeric_limits<size_t>::max() / 2 - 1;
            if (length > MAX_LEN) {
                LOG(FATAL, RUNTIME) << __func__ << " length is higher than half of size_t::max";
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(buf, sizeof(uint8_t) * (maxLength + 1), GetDataMUtf8() + start, length) != EOK) {
                LOG(FATAL, RUNTIME) << __func__ << " length is higher than buf size";
            }
            return length;
        }
        return ark::utf::ConvertRegionUtf16ToMUtf8(GetDataUtf16(), buf, length, maxLength - 1, start);
    }

    inline uint32_t CopyDataUtf16(uint16_t *buf, uint32_t maxLength)
    {
        return CopyDataRegionUtf16(buf, 0, GetLength(), maxLength);
    }

    uint32_t CopyDataRegionUtf16(uint16_t *buf, uint32_t start, uint32_t length, uint32_t maxLength)
    {
        if (length > maxLength) {
            return 0;
        }
        uint32_t len = GetLength();
        if (start + length > len) {
            return 0;
        }
        if (IsUtf16()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(buf, sizeof(uint16_t) * maxLength, GetDataUtf16() + start, ComputeDataSizeUtf16(length)) !=
                EOK) {
                LOG(FATAL, RUNTIME) << __func__ << " length is higher than buf size";
            }
        } else {
            uint8_t *src8 = GetDataMUtf8() + start;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            for (uint32_t i = 0; i < length; ++i) {
                buf[i] = src8[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }
        }
        return length;
    }

    uint32_t GetLength() const
    {
        return ToStringConst()->GetLength();
    }

    bool IsEmpty() const
    {
        // do not shift right length because it is always zero for empty string
        return GetLength() == 0;
    }

    size_t ObjectSize() const
    {
        return common::LineString::ObjectSize(ToStringConst());
    }

    uint32_t GetHashcode()
    {
        auto readBarrier = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        return ToString()->GetHashcode(std::move(readBarrier));
    }

    int32_t IndexOf(LineString *rhs, int pos = 0);
    int32_t LastIndexOf(LineString *rhs, int pos = INT32_MAX);

    static constexpr uint32_t GetLengthOffset()
    {
        return common::BaseString::LENGTH_AND_FLAGS_OFFSET;
    }

    static constexpr uint32_t GetDataOffset()
    {
        return common::LineString::DATA_OFFSET;
    }

    static constexpr uint32_t GetHashcodeOffset()
    {
        return common::BaseString::MIX_HASHCODE_OFFSET;
    }

    static constexpr uint32_t GetStringCompressionMask()
    {
        return STRING_COMPRESSED_BIT;
    }

    /// Compares strings by bytes, It doesn't check canonical unicode equivalence.
    PANDA_PUBLIC_API static bool LineStringsAreEqual(LineString *str1, LineString *str2);
    /// Compares strings by bytes, It doesn't check canonical unicode equivalence.
    PANDA_PUBLIC_API static bool LineStringsAreEqualMUtf8(LineString *str1, const uint8_t *mutf8Data,
                                                          uint32_t utf16Length);
    static bool LineStringsAreEqualMUtf8(LineString *str1, const uint8_t *mutf8Data, uint32_t utf16Length,
                                         bool canBeCompressed);
    /// Compares strings by bytes, It doesn't check canonical unicode equivalence.
    PANDA_PUBLIC_API static bool LineStringsAreEqualUtf16(LineString *str1, const uint16_t *utf16Data,
                                                          uint32_t utf16DataLength);
    static LineString *DoReplace(LineString *src, uint16_t oldC, uint16_t newC, const LanguageContext &ctx,
                                 PandaVM *vm);
    static uint32_t ComputeHashcodeMutf8(const uint8_t *mutf8Data, uint32_t length);
    static uint32_t ComputeHashcodeMutf8(const uint8_t *mutf8Data, uint32_t utf16Length, bool canBeCompressed);
    static uint32_t ComputeHashcodeUtf16(const uint16_t *utf16Data, uint32_t length);

    static void SetCompressedStringsEnabled(bool val)
    {
        compressedStringsEnabled_ = val;
    }

    static bool GetCompressedStringsEnabled()
    {
        return compressedStringsEnabled_;
    }

    static std::pair<int32_t, int32_t> NormalizeSubLineStringIndexes(int32_t beginIndex, int32_t endIndex,
                                                                     const coretypes::LineString *str)
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

    static LineString *FastSubLineString(LineString *src, uint32_t start, uint32_t utf16Length,
                                         const LanguageContext &ctx, PandaVM *vm = nullptr);

    static bool CanBeCompressedMUtf8(const uint8_t *mutf8Data);

    static bool IsASCIICharacter(uint16_t data)
    {
        // \0 is not considered ASCII in Modified-UTF8
        return common::BaseString::IsASCIICharacter(data);
    }

protected:
    void SetLength(uint32_t length, bool compressed = false)
    {
        return ToString()->InitLengthAndFlags(length, compressed, false);
    }

    void SetHashcode(uint32_t hashcode)
    {
        ToString()->SetMixHashcode(hashcode);
    }

    uint32_t ComputeHashcode();
    static bool CanBeCompressed(const uint16_t *utf16Data, uint32_t utf16Length);
    static void CopyUtf16AsMUtf8(const uint16_t *utf16From, uint8_t *mutf8To, uint32_t utf16Length);

private:
    PANDA_PUBLIC_API static bool compressedStringsEnabled_;
    static constexpr uint32_t STRING_COMPRESSED_BIT = 0x1;
    static constexpr uint32_t MAX_STRING_LENGTH = (1U << 30U) - 1U;  // reserve 30 free slots for length

    enum CompressedStatus {
        STRING_COMPRESSED,
        STRING_UNCOMPRESSED,
    };

    enum InternStatus {
        NON_INTERNED,
        INTERNED,
    };

    static bool CanBeCompressedMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length);
    static bool CanBeCompressedUtf16(const uint16_t *utf16Data, uint32_t utf16Length, uint16_t non);
    static bool CanBeCompressedMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length, uint16_t non);

    /**
     * str1 should have the same length as mutf16_data.
     * Converts mutf8_data to mutf16 and compare it with given mutf16_data.
     */
    // NOTE(alovkov): move to utils/utf.h without allocation a temporary buffer
    static bool IsMutf8EqualsUtf16(const uint8_t *utf8Data, uint32_t utf8DataLength, const uint16_t *utf16Data,
                                   uint32_t utf16DataLength);

    static bool IsMutf8EqualsUtf16(const uint8_t *utf8Data, const uint16_t *utf16Data, uint32_t utf16DataLength);

    template <typename T>
    /// Check that two spans are equal. Should have the same length.
    static bool LineStringsAreEquals(Span<const T> &str1, Span<const T> &str2);
};

}  // namespace ark::coretypes

#endif  // PANDA_RUNTIME_CORETYPES_LINE_STRING_H_
