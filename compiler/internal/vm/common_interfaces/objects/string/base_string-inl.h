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

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers,
// readability-else-after-return, hicpp-signed-bitwise)

#ifndef COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_IMPL_H
#define COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_IMPL_H

#include <vector>

#include "objects/string/base_string.h"
#include "objects/string/line_string-inl.h"
#include "objects/string/sliced_string-inl.h"
#include "objects/string/tree_string-inl.h"
#include "objects/utils/utf_utils.h"
#include "objects/utils/span.h"
#include "securec.h"

namespace common {
std::u16string Utf16ToU16String(const uint16_t *utf16Data, uint32_t dataLen);
std::u16string Utf8ToU16String(const uint8_t *utf8Data, uint32_t dataLen);

template <typename T1, typename T2>
int32_t CompareStringSpan(Span<T1> &lhsSp, Span<T2> &rhsSp, int32_t count);
template <typename T1, typename T2>
bool IsSubStringAtSpan(Span<T1> &lhsSp, Span<T2> &rhsSp, uint32_t offset);

template <typename ReadBarrier>
uint32_t BaseString::ComputeHashcode(ReadBarrier &&readBarrier) const
{
#if defined(PANDA_32_BIT_MANAGED_POINTER)
    return ComputeRawHashcode32bits(readBarrier);
#else
    auto [hash, isInteger] = ComputeRawHashcode(readBarrier);
    return MixHashcode(hash, isInteger);
#endif
}

template <typename ReadBarrier>
uint32_t BaseString::ComputeRawHashcode32bits(ReadBarrier &&readBarrier) const
{
    uint32_t length = GetLength();
    if (length == 0) {
        return 0;
    }

    if (IsUtf8()) {
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        // String can not convert to integer number, using normal hashcode computing algorithm.
        return ComputeHashForData(data, length, 0);
    }
    std::vector<uint16_t> buf;
    const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
    // If rawSeed has certain value, and second string uses UTF16 encoding,
    // then merged string can not be small integer number.
    return ComputeHashForData(data, length, 0);
}

template <typename ReadBarrier>
std::pair<uint32_t, bool> BaseString::ComputeRawHashcode(ReadBarrier &&readBarrier) const
{
    uint32_t hash = 0;
    uint32_t length = GetLength();
    if (length == 0) {
        return {hash, false};
    }

    if (IsUtf8()) {
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        // String using UTF8 encoding, and length smaller than 10, try to compute integer hash.
        if (length < MAX_ELEMENT_INDEX_LEN && this->HashIntegerString(data, length, &hash, 0)) {
            return {hash, true};
        }
        // String can not convert to integer number, using normal hashcode computing algorithm.
        hash = ComputeHashForData(data, length, 0);
        return {hash, false};
    } else {
        std::vector<uint16_t> buf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        // If rawSeed has certain value, and second string uses UTF16 encoding,
        // then merged string can not be small integer number.
        hash = ComputeHashForData(data, length, 0);
        return {hash, false};
    }
}

template <typename T>
inline static bool IsDecimalDigitChar(const T c)
{
    return (c >= '0' && c <= '9');
}

inline bool ComputeIntegerHash(uint32_t *num, uint8_t c)
{
    if (!IsDecimalDigitChar(c)) {
        return false;
    }
    int charDate = c - '0';
    *num = (*num) * 10 + charDate;  // 10: decimal factor
    return true;
}

template <typename T>
bool BaseString::HashIntegerString(const T *data, size_t size, uint32_t *hash, uint32_t hashSeed)
{
    if (hashSeed == 0) {
        if (IsDecimalDigitChar(data[0]) && data[0] != '0') {
            uint32_t num = data[0] - '0';
            uint32_t i = 1;
            do {
                // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
                // CC-OFFNXT(G.FUN.01-CPP) solid logic
                if (i == size) {
                    // compute mix hash
                    if (num <= MAX_INTEGER_HASH_NUMBER) {
                        *hash = MixHashcode(num, IS_INTEGER);
                        return true;
                    }
                    return false;
                }
            } while (ComputeIntegerHash(&num, data[i++]));
        }
        if (size == 1 && (data[0] == '0')) {
            *hash = MixHashcode(0, IS_INTEGER);
            return true;
        }
    } else {
        if (IsDecimalDigitChar(data[0])) {
            uint32_t num = hashSeed * 10 + (data[0] - '0');  // 10: decimal factor
            uint32_t i = 1;
            do {
                // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
                // CC-OFFNXT(G.FUN.01-CPP) solid logic
                if (i == size) {
                    // compute mix hash
                    if (num <= MAX_INTEGER_HASH_NUMBER) {
                        *hash = MixHashcode(num, IS_INTEGER);
                        return true;
                    }
                    return false;
                }
            } while (ComputeIntegerHash(&num, data[i++]));
        }
    }
    return false;
}

template <typename ReadBarrier>
bool BaseString::EqualToSplicedString(ReadBarrier &&readBarrier, const BaseString *str1, const BaseString *str2)
{
    DCHECK_CC(!IsTreeString());
    DCHECK_CC(!str1->IsTreeString() && !str2->IsTreeString());
    if (GetLength() != str1->GetLength() + str2->GetLength()) {
        return false;
    }
    if (IsUtf16()) {
        std::vector<uint16_t> buf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        if (BaseString::StringsAreEqualUtf16(std::forward<ReadBarrier>(readBarrier), str1, data, str1->GetLength())) {
            return BaseString::StringsAreEqualUtf16(std::forward<ReadBarrier>(readBarrier), str2,
                                                    data + str1->GetLength(), str2->GetLength());
        }
    } else {
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        if (BaseString::StringIsEqualUint8Data(std::forward<ReadBarrier>(readBarrier), str1, data, str1->GetLength(),
                                               this->IsUtf8())) {
            return BaseString::StringIsEqualUint8Data(std::forward<ReadBarrier>(readBarrier), str2,
                                                      data + str1->GetLength(), str2->GetLength(), this->IsUtf8());
        }
    }
    return false;
}

template <typename ReadBarrier>
std::u16string BaseString::ToU16String(ReadBarrier &&readBarrier, uint32_t len)
{
    uint32_t length = len > 0 ? len : GetLength();
    std::u16string result;
    if (IsUtf16()) {
        std::vector<uint16_t> buf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        result = Utf16ToU16String(data, length);
    } else {
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        result = Utf8ToU16String(data, length);
    }
    return result;
}

template <typename ReadBarrier>
const uint8_t *BaseString::GetNonTreeUtf8Data(ReadBarrier &&readBarrier, const BaseString *src)
{
    DCHECK_CC(src->IsUtf8());
    DCHECK_CC(!src->IsTreeString());
    if (src->IsSlicedString()) {
        const SlicedString *str = SlicedString::ConstCast(src);
        return LineString::Cast(str->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)))->GetDataUtf8() +
               str->GetStartIndex();
    }
    DCHECK_CC(src->IsLineString());
    return LineString::ConstCast(src)->GetDataUtf8();
}

template <typename ReadBarrier>
const uint16_t *BaseString::GetNonTreeUtf16Data(ReadBarrier &&readBarrier, const BaseString *src)
{
    DCHECK_CC(src->IsUtf16());
    DCHECK_CC(!src->IsTreeString());
    if (src->IsSlicedString()) {
        const SlicedString *str = SlicedString::ConstCast(src);
        return LineString::Cast(str->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)))->GetDataUtf16() +
               str->GetStartIndex();
    }
    DCHECK_CC(src->IsLineString());
    return LineString::ConstCast(src)->GetDataUtf16();
}

/* static */
template <typename ReadBarrier>
bool BaseString::StringsAreEqualDiffUtfEncoding(ReadBarrier &&readBarrier, BaseString *left, BaseString *right)
{
    std::vector<uint16_t> bufLeftUft16;
    std::vector<uint16_t> bufRightUft16;
    std::vector<uint8_t> bufLeftUft8;
    std::vector<uint8_t> bufRightUft8;
    int32_t lhsCount = static_cast<int32_t>(left->GetLength());
    int32_t rhsCount = static_cast<int32_t>(right->GetLength());
    if (!left->IsUtf16() && !right->IsUtf16()) {
        const uint8_t *data1 = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), left, bufLeftUft8);
        const uint8_t *data2 = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), right, bufRightUft8);
        Span<const uint8_t> lhsSp(data1, lhsCount);
        Span<const uint8_t> rhsSp(data2, rhsCount);
        return BaseString::StringsAreEquals(lhsSp, rhsSp);
    }
    if (!left->IsUtf16()) {
        const uint8_t *data1 = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), left, bufLeftUft8);
        const uint16_t *data2 =
            BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), right, bufRightUft16);
        Span<const uint8_t> lhsSp(data1, lhsCount);
        Span<const uint16_t> rhsSp(data2, rhsCount);
        return BaseString::StringsAreEquals(lhsSp, rhsSp);
    }
    if (!right->IsUtf16()) {
        const uint16_t *data1 =
            BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), left, bufLeftUft16);
        const uint8_t *data2 = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), right, bufRightUft8);
        Span<const uint16_t> lhsSp(data1, lhsCount);
        Span<const uint8_t> rhsSp(data2, rhsCount);
        return BaseString::StringsAreEquals(lhsSp, rhsSp);
    }
    const uint16_t *data1 = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), left, bufLeftUft16);
    const uint16_t *data2 = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), right, bufRightUft16);
    Span<const uint16_t> lhsSp(data1, lhsCount);
    Span<const uint16_t> rhsSp(data2, rhsCount);
    return StringsAreEquals(lhsSp, rhsSp);
}

/* static */
template <typename ReadBarrier>
bool BaseString::StringsAreEqual(ReadBarrier &&readBarrier, BaseString *str1, BaseString *str2)
{
    DCHECK_CC(str1 != nullptr);
    DCHECK_CC(str2 != nullptr);
    if (str1 == str2) {
        return true;
    }
    uint32_t str1Len = str1->GetLength();
    if (str1Len != str2->GetLength()) {
        return false;
    }
    if (str1Len == 0) {
        return true;
    }

    uint32_t str1Hash;
    uint32_t str2Hash;
    if (str1->TryGetHashCode(&str1Hash) && str2->TryGetHashCode(&str2Hash)) {
        if (str1Hash != str2Hash) {
            return false;
        }
    }
    return StringsAreEqualDiffUtfEncoding(std::forward<ReadBarrier>(readBarrier), str1, str2);
}

/* static */
template <typename ReadBarrier>
bool BaseString::StringIsEqualUint8Data(ReadBarrier &&readBarrier, const BaseString *str1, const uint8_t *dataAddr,
                                        uint32_t dataLen, bool canBeCompressToUtf8)
{
    if (!str1->IsSlicedString() && canBeCompressToUtf8 != str1->IsUtf8()) {
        return false;
    }
    if (canBeCompressToUtf8 && str1->GetLength() != dataLen) {
        return false;
    }
    if (str1->IsUtf8()) {
        std::vector<uint8_t> buf;
        Span<const uint8_t> data1(BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), str1, buf),
                                  dataLen);
        Span<const uint8_t> data2(dataAddr, dataLen);
        return BaseString::StringsAreEquals(data1, data2);
    }
    std::vector<uint16_t> buf;
    uint32_t length = str1->GetLength();
    const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), str1, buf);
    return IsUtf8EqualsUtf16(dataAddr, dataLen, data, length);
}

/* static */
template <typename ReadBarrier>
bool BaseString::StringsAreEqualUtf16(ReadBarrier &&readBarrier, const BaseString *str1, const uint16_t *utf16Data,
                                      uint32_t utf16Len)
{
    uint32_t length = str1->GetLength();
    if (length != utf16Len) {
        return false;
    }
    if (str1->IsUtf8()) {
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), str1, buf);
        return IsUtf8EqualsUtf16(data, length, utf16Data, utf16Len);
    }
    std::vector<uint16_t> buf;
    Span<const uint16_t> data1(BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), str1, buf), length);
    Span<const uint16_t> data2(utf16Data, utf16Len);
    return BaseString::StringsAreEquals(data1, data2);
}

#include <vector>
#include "securec.h"

template <typename ReadBarrier>
size_t BaseString::GetUtf8Length(ReadBarrier &&readBarrier, bool modify, bool isGetBufferSize) const
{
    if (!IsUtf16()) {
        return GetLength() + 1;  // add place for zero in the end
    }
    std::vector<uint16_t> tmpBuf;
    const uint16_t *data = GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    return UtfUtils::Utf16ToUtf8Size(data, GetLength(), modify, isGetBufferSize);
}

template <typename ReadBarrier, typename Vec,
          std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint16_t>, int>>
const uint16_t *BaseString::GetUtf16DataFlat(ReadBarrier &&readBarrier, const BaseString *src, Vec &buf)
{
    DCHECK_CC(src->IsUtf16());
    uint32_t length = src->GetLength();
    if (src->IsTreeString()) {
        if (src->IsFlat(std::forward<ReadBarrier>(readBarrier))) {
            src = BaseString::Cast(
                TreeString::ConstCast(src)->GetLeftSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
        } else {
            buf.reserve(length);
            WriteToFlat(std::forward<ReadBarrier>(readBarrier), src, buf.data(), length);
            return buf.data();
        }
    } else if (src->IsSlicedString()) {
        const SlicedString *str = SlicedString::ConstCast(src);
        return LineString::Cast(str->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)))->GetDataUtf16() +
               str->GetStartIndex();
    }
    return LineString::ConstCast(src)->GetDataUtf16();
}

constexpr bool BaseString::IsStringType(ObjectType type)
{
    return (type >= ObjectType::STRING_FIRST && type <= ObjectType::STRING_LAST);
}

inline ObjectType BaseString::GetStringType() const
{
    ObjectType type = GetBaseClass()->GetObjectType();
    DCHECK_CC(IsStringType(type) && ("Invalid ObjectType"));
    return type;
}

template <bool VERIFY, typename ReadBarrier>
uint16_t BaseString::At(ReadBarrier &&readBarrier, int32_t index) const
{
    auto length = static_cast<int32_t>(GetLength());
    if constexpr (VERIFY) {
        if ((index < 0) || (index >= length)) {
            return 0;
        }
    }
    switch (GetStringType()) {
        case ObjectType::LINE_STRING:
            return LineString::ConstCast(this)->Get<VERIFY>(index);
        case ObjectType::SLICED_STRING:
            return SlicedString::ConstCast(this)->Get<VERIFY>(std::forward<ReadBarrier>(readBarrier), index);
        case ObjectType::TREE_STRING:
            return TreeString::ConstCast(this)->Get<VERIFY>(std::forward<ReadBarrier>(readBarrier), index);
        default:
            UNREACHABLE_CC();
    }
}

template <typename ReadBarrier>
bool BaseString::IsFlat(ReadBarrier &&readBarrier) const
{
    if (!this->IsTreeString()) {
        return true;
    }
    return TreeString::ConstCast(this)->IsFlat(std::forward<ReadBarrier>(readBarrier));
}

template <typename Char, typename ReadBarrier>
// CC-OFFNXT(huge_depth, huge_method, G.FUN.01-CPP) solid logic
void BaseString::WriteToFlat(ReadBarrier &&readBarrier, const BaseString *src, Char *buf, uint32_t maxLength)
{
    // DISALLOW_GARBAGE_COLLECTION;
    uint32_t length = src->GetLength();
    if (length == 0) {
        return;
    }
    while (true) {
        DCHECK_CC(length <= maxLength && length > 0);
        DCHECK_CC(length <= src->GetLength());
        switch (src->GetStringType()) {
            case ObjectType::LINE_STRING: {
                if (src->IsUtf8()) {
                    CopyChars(buf, LineString::ConstCast(src)->GetDataUtf8(), length);
                } else {
                    CopyChars(buf, LineString::ConstCast(src)->GetDataUtf16(), length);
                }
                return;
            }
            case ObjectType::TREE_STRING: {
                const TreeString *treeSrc = TreeString::ConstCast(src);
                BaseString *left =
                    BaseString::Cast(treeSrc->GetLeftSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                BaseString *right =
                    BaseString::Cast(treeSrc->GetRightSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                uint32_t leftLength = left->GetLength();
                uint32_t rightLength = right->GetLength();
                // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
                if (rightLength >= leftLength) {
                    // right string is longer. So recurse over left.
                    WriteToFlat(std::forward<ReadBarrier>(readBarrier), left, buf, maxLength);
                    // CC-OFFNXT(G.FUN.01-CPP) solid logic
                    if (left == right) {
                        CopyChars(buf + leftLength, buf, leftLength);
                        return;
                    }
                    buf += leftLength;
                    maxLength -= leftLength;
                    src = right;
                    length -= leftLength;
                } else {
                    // left string is longer.  So recurse over right.
                    // if src{left:A,right:B} is half flat to {left:A+B,right:empty} by another thread
                    // but the other thread didn't end, and this thread get  {left:A+B,right:B}
                    // it may cause write buffer overflower in line 424, buf + leftLength is overflower.
                    // so use 'length > leftLength' instead of 'rightLength > 0'
                    // CC-OFFNXT(G.FUN.01-CPP) solid logic
                    if (length > leftLength) {
                        if (rightLength == 1) {
                            buf[leftLength] =
                                static_cast<Char>(right->At<false>(std::forward<ReadBarrier>(readBarrier), 0));
                        } else if ((right->IsLineString()) && right->IsUtf8()) {
                            CopyChars(buf + leftLength, LineString::Cast(right)->GetDataUtf8(), rightLength);
                        } else {
                            WriteToFlat(std::forward<ReadBarrier>(readBarrier), right, buf + leftLength,
                                        maxLength - leftLength);
                        }
                        length -= rightLength;
                    }
                    maxLength = leftLength;
                    src = left;
                }
                continue;
            }
            case ObjectType::SLICED_STRING: {
                BaseString *parent = BaseString::Cast(
                    SlicedString::ConstCast(src)->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                if (src->IsUtf8()) {
                    CopyChars(buf,
                              LineString::Cast(parent)->GetDataUtf8() + SlicedString::ConstCast(src)->GetStartIndex(),
                              length);
                } else {
                    CopyChars(buf,
                              LineString::Cast(parent)->GetDataUtf16() + SlicedString::ConstCast(src)->GetStartIndex(),
                              length);
                }
                return;
            }
            default:
                UNREACHABLE_CC();
        }
    }
}

template <typename Char, typename ReadBarrier>
void BaseString::WriteToFlatWithPos(ReadBarrier &&readBarrier, BaseString *src, Char *buf, uint32_t length,
                                    uint32_t pos)
{
    // DISALLOW_GARBAGE_COLLECTION;
    [[maybe_unused]] uint32_t maxLength = src->GetLength();
    if (length == 0) {
        return;
    }
    while (true) {
        DCHECK_CC(length + pos <= maxLength && length > 0);
        DCHECK_CC(length <= src->GetLength());
        DCHECK_CC(pos >= 0);
        switch (src->GetStringType()) {
            case ObjectType::LINE_STRING: {
                if (src->IsUtf8()) {
                    CopyChars(buf, LineString::Cast(src)->GetDataUtf8() + pos, length);
                } else {
                    CopyChars(buf, LineString::Cast(src)->GetDataUtf16() + pos, length);
                }
                return;
            }
            case ObjectType::TREE_STRING: {
                TreeString *treeSrc = TreeString::Cast(src);
                BaseString *left =
                    BaseString::Cast(treeSrc->GetLeftSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                DCHECK_CC(left->IsLineString());
                src = left;
                continue;
            }
            case ObjectType::SLICED_STRING: {
                BaseString *parent = BaseString::Cast(
                    SlicedString::Cast(src)->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                if (src->IsUtf8()) {
                    CopyChars(buf,
                              LineString::Cast(parent)->GetDataUtf8() + SlicedString::Cast(src)->GetStartIndex() + pos,
                              length);
                } else {
                    CopyChars(buf,
                              LineString::Cast(parent)->GetDataUtf16() + SlicedString::Cast(src)->GetStartIndex() + pos,
                              length);
                }
                return;
            }
            default:
                UNREACHABLE_CC();
        }
    }
}

// It allows user to copy into buffer even if maxLength < length
template <typename ReadBarrier>
size_t BaseString::WriteUtf8(ReadBarrier &&readBarrier, uint8_t *buf, size_t maxLength, bool isWriteBuffer) const
{
    if (maxLength == 0) {
        return 1;  // maxLength was -1 at napi
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    buf[maxLength - 1] = '\0';
    return CopyDataRegionUtf8(std::forward<ReadBarrier>(readBarrier), buf, 0, GetLength(), maxLength, true,
                              isWriteBuffer) +
           1;
}

// It allows user to copy into buffer even if maxLength < length
template <typename ReadBarrier>
size_t BaseString::WriteUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t targetLength, uint32_t bufLength) const
{
    if (bufLength == 0) {
        return 0;
    }
    // Returns a number representing a valid backrest length.
    return CopyDataToUtf16(std::forward<ReadBarrier>(readBarrier), buf, targetLength, bufLength);
}

template <typename ReadBarrier>
size_t BaseString::WriteOneByte(ReadBarrier &&readBarrier, uint8_t *buf, size_t maxLength) const
{
    if (maxLength == 0) {
        return 0;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    buf[maxLength - 1] = '\0';
    uint32_t length = GetLength();
    if (!IsUtf16()) {
        std::vector<uint8_t> tmpBuf;
        const uint8_t *data = GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        if (length > maxLength) {
            length = maxLength;
        }
        if (memcpy_s(buf, maxLength, data, length) != EOK) {
            UNREACHABLE_CC();
        }
        return length;
    }

    std::vector<uint16_t> tmpBuf;
    const uint16_t *data = GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    if (length > maxLength) {
        return UtfUtils::ConvertRegionUtf16ToLatin1(data, buf, maxLength, maxLength);
    }
    return UtfUtils::ConvertRegionUtf16ToLatin1(data, buf, length, maxLength);
}

template <typename ReadBarrier>
uint32_t BaseString::CopyDataUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t maxLength) const
{
    uint32_t length = GetLength();
    if (length > maxLength) {
        return 0;
    }
    if (IsUtf16()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::vector<uint16_t> tmpBuf;
        const uint16_t *data = GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        if (memcpy_s(buf, maxLength * sizeof(uint16_t), data, length * sizeof(uint16_t)) != EOK) {
            UNREACHABLE_CC();
        }
        return length;
    }
    std::vector<uint8_t> tmpBuf;
    const uint8_t *data = GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    return UtfUtils::ConvertRegionUtf8ToUtf16(data, buf, length, maxLength);
}

template <typename ReadBarrier, typename Vec,
          std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int>>
Span<const uint8_t> BaseString::ToUtf8Span(ReadBarrier &&readBarrier, Vec &buf, bool modify, bool cesu8)
{
    Span<const uint8_t> str;
    uint32_t strLen = GetLength();
    if (UNLIKELY_CC(IsUtf16())) {
        using U16Vec = objects_traits::vector_with_same_alloc_t<Vec, uint16_t>;
        U16Vec tmpBuf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        DCHECK_CC(UtfUtils::Utf16ToUtf8Size(data, strLen, modify, false, cesu8) > 0);
        size_t len = UtfUtils::Utf16ToUtf8Size(data, strLen, modify, false, cesu8) - 1;
        buf.reserve(len);
        len = UtfUtils::ConvertRegionUtf16ToUtf8(data, buf.data(), strLen, len, 0, modify, false, cesu8);
        str = Span<const uint8_t>(buf.data(), len);
    } else {
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        str = Span<const uint8_t>(data, strLen);
    }
    return str;
}

template <typename ReadBarrier, typename Vec,
          std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int>>
Span<const uint8_t> BaseString::DebuggerToUtf8Span(ReadBarrier &&readBarrier, Vec &buf, bool modify)
{
    Span<const uint8_t> str;
    uint32_t strLen = GetLength();
    if (UNLIKELY_CC(IsUtf16())) {
        using U16Vec = objects_traits::vector_with_same_alloc_t<Vec, uint16_t>;
        U16Vec tmpBuf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        size_t len = UtfUtils::Utf16ToUtf8Size(data, strLen, modify) - 1;
        buf.reserve(len);
        len = UtfUtils::DebuggerConvertRegionUtf16ToUtf8(data, buf.data(), strLen, len, 0, modify);
        str = Span<const uint8_t>(buf.data(), len);
    } else {
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        str = Span<const uint8_t>(data, strLen);
    }
    return str;
}

// single char copy for loop
template <typename DstType, typename SrcType>
void BaseString::CopyChars(DstType *dst, SrcType *src, uint32_t count)
{
    Span<SrcType> srcSp(src, count);
    Span<DstType> dstSp(dst, count);
    for (uint32_t i = 0; i < count; i++) {
        dstSp[i] = srcSp[i];
    }
}

template <typename ReadBarrier, typename Vec,
          std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int>>
const uint8_t *BaseString::GetUtf8DataFlat(ReadBarrier &&readBarrier, const BaseString *src, Vec &buf)
{
    DCHECK_CC(src->IsUtf8());
    uint32_t length = src->GetLength();
    if (src->IsTreeString()) {
        if (src->IsFlat(std::forward<ReadBarrier>(readBarrier))) {
            src = BaseString::Cast(
                TreeString::ConstCast(src)->GetLeftSubString<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
        } else {
            buf.reserve(length);
            WriteToFlat(std::forward<ReadBarrier>(readBarrier), src, buf.data(), length);
            return buf.data();
        }
    } else if (src->IsSlicedString()) {
        const SlicedString *str = SlicedString::ConstCast(src);
        return LineString::Cast(str->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)))->GetDataUtf8() +
               str->GetStartIndex();
    }
    return LineString::ConstCast(src)->GetDataUtf8();
}

template <typename ReadBarrier>
size_t BaseString::CopyDataRegionUtf8(ReadBarrier &&readBarrier, uint8_t *buf, size_t start, size_t length,
                                      size_t maxLength, bool modify, bool isWriteBuffer) const
{
    uint32_t len = GetLength();
    if (start + length > len) {
        return 0;
    }
    constexpr size_t TWO_TIMES = 2;
    if (!IsUtf16()) {
        if (length > (std::numeric_limits<size_t>::max() / TWO_TIMES - 1)) {
            // 2: half
            UNREACHABLE_CC();
        }
        std::vector<uint8_t> tmpBuf;
        const uint8_t *data = GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf) + start;
        // Only copy maxLength number of chars into buffer if length > maxLength
        auto dataLen = std::min(length, maxLength);
        std::copy(data, data + dataLen, buf);
        return dataLen;
    }
    std::vector<uint16_t> tmpBuf;
    const uint16_t *data = GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    if (length > maxLength) {
        return UtfUtils::ConvertRegionUtf16ToUtf8(data, buf, maxLength, maxLength, start, modify, isWriteBuffer);
    }
    return UtfUtils::ConvertRegionUtf16ToUtf8(data, buf, length, maxLength, start, modify, isWriteBuffer);
}

template <typename ReadBarrier>
size_t BaseString::CopyDataToUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t length, uint32_t bufLength) const
{
    if (IsUtf16()) {
        std::vector<uint16_t> tmpBuf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        if (length > bufLength) {
            if (memcpy_s(buf, bufLength * sizeof(uint16_t), data, bufLength * sizeof(uint16_t)) != EOK) {
                UNREACHABLE_CC();
            }
            return bufLength;
        }
        if (memcpy_s(buf, bufLength * sizeof(uint16_t), data, length * sizeof(uint16_t)) != EOK) {
            UNREACHABLE_CC();
        }
        return length;
    }
    std::vector<uint8_t> tmpBuf;
    const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    if (length > bufLength) {
        return UtfUtils::ConvertRegionUtf8ToUtf16(data, buf, bufLength, bufLength);
    }
    return UtfUtils::ConvertRegionUtf8ToUtf16(data, buf, length, bufLength);
}

#if defined(PANDA_32_BIT_MANAGED_POINTER)
// To change the hash algorithm of BaseString, please modify BaseString::CalculateConcatHashCode
// and BaseStringHashHelper::ComputeHashForDataPlatform simultaneously!!
template <typename T>
uint32_t BaseString::ComputeHashForData(const T *data, size_t size, uint32_t hashSeed)
{
    uint32_t hash = hashSeed;
    for (uint32_t i = 0; i < size; i++) {
        hash = (hash << static_cast<uint32_t>(HASH_SHIFT)) - hash + data[i];
    }
    return hash;
}
#endif

inline bool BaseString::IsASCIICharacter(uint16_t data)
{
    if (data == 0) {
        return false;
    }
    // \0 is not considered ASCII in Ecma-Modified-UTF8 [only modify '\u0000']
    return data <= UtfUtils::UTF8_1B_MAX;
}

template <typename T>
bool BaseString::MemCopyChars(Span<T> &dst, size_t dstMax, Span<const T> &src, size_t count)
{
    DCHECK_CC(dstMax >= count);
    DCHECK_CC(dst.Size() >= src.Size());
    if (memcpy_s(dst.data(), dstMax, src.data(), count) != EOK) {
        // LOG_FULL(FATAL) << "memcpy_s failed";
        UNREACHABLE_CC();
    }
    return true;
}

template <typename T1, typename T2>
int32_t BaseString::LastIndexOf(Span<const T1> &lhsSp, Span<const T2> &rhsSp, int32_t pos)
{
    int rhsSize = static_cast<int>(rhsSp.size());
    DCHECK_CC(rhsSize > 0);
    auto first = rhsSp[0];
    for (int32_t i = pos; i >= 0; i--) {
        if (lhsSp[i] != first) {
            continue;
        }
        /* Found first character, now look at the rest of rhsSp */
        int j = 1;
        while (j < rhsSize) {
            if (rhsSp[j] != lhsSp[i + j]) {
                break;
            }
            j++;
        }
        if (j == rhsSize) {
            return i;
        }
    }
    return -1;
}

/* static */
template <typename T1, typename T2>
int32_t BaseString::IndexOf(Span<const T1> &lhsSp, Span<const T2> &rhsSp, int32_t pos, int32_t max)
{
    auto first = static_cast<int32_t>(rhsSp[0]);
    for (int32_t i = pos; i <= max; i++) {
        if (static_cast<int32_t>(lhsSp[i]) != first) {
            i++;
            while (i <= max && static_cast<int32_t>(lhsSp[i]) != first) {
                i++;
            }
        }
        /* Found first character, now look at the rest of rhsSp */
        if (i <= max) {
            int j = i + 1;
            int end = j + static_cast<int>(rhsSp.size()) - 1;

            for (int k = 1; j < end && static_cast<int32_t>(lhsSp[j]) == static_cast<int32_t>(rhsSp[k]); j++, k++) {
            }
            if (j == end) {
                /* Found whole string. */
                return i;
            }
        }
    }
    return -1;
}

// static
// CC-OFFNXT(C_RULE_ID_INLINE_FUNCTION_SIZE) Perf critical common runtime code stub
// CC-OFFNXT(G.FUD.06) perf critical
inline bool BaseString::CanBeCompressed(const uint8_t *utf8Data, uint32_t utf8Len)
{
    uint32_t index = 0;
    for (; index + 4 <= utf8Len; index += 4) {  // 4: process the data in chunks of 4 elements to improve speed
        // Check if all four characters in the current block are ASCII characters
        if (!IsASCIICharacter(utf8Data[index]) ||
            !IsASCIICharacter(utf8Data[index + 1]) ||  // 1: the second element of the block
            !IsASCIICharacter(utf8Data[index + 2]) ||  // 2: the third element of the block
            !IsASCIICharacter(utf8Data[index + 3])) {  // 3: the fourth element of the block
            return false;
        }
    }
    // Check remaining characters if they are ASCII
    for (; index < utf8Len; ++index) {
        if (!IsASCIICharacter(utf8Data[index])) {
            return false;
        }
    }
    return true;
}

/* static */
// CC-OFFNXT(C_RULE_ID_INLINE_FUNCTION_SIZE) Perf critical common runtime code stub
// CC-OFFNXT(G.FUD.06) perf critical
inline bool BaseString::CanBeCompressed(const uint16_t *utf16Data, uint32_t utf16Len)
{
    uint32_t index = 0;
    for (; index + 4 <= utf16Len; index += 4) {  // 4: process the data in chunks of 4 elements to improve speed
        // Check if all four characters in the current block are ASCII characters
        if (!IsASCIICharacter(utf16Data[index]) ||
            !IsASCIICharacter(utf16Data[index + 1]) ||  // 1: the second element of the block
            !IsASCIICharacter(utf16Data[index + 2]) ||  // 2: the third element of the block
            !IsASCIICharacter(utf16Data[index + 3])) {  // 3: the fourth element of the block
            return false;
        }
    }
    // Check remaining characters if they are ASCII
    for (; index < utf16Len; ++index) {
        if (!IsASCIICharacter(utf16Data[index])) {
            return false;
        }
    }
    return true;
}

template <typename T1, typename T2>
int32_t CompareStringSpan(Span<T1> &lhsSp, Span<T2> &rhsSp, int32_t count)
{
    for (int32_t i = 0; i < count; ++i) {
        auto left = static_cast<int32_t>(lhsSp[i]);
        auto right = static_cast<int32_t>(rhsSp[i]);
        if (left != right) {
            return left - right;
        }
    }
    return 0;
}

#if defined(PANDA_32_BIT_MANAGED_POINTER)
inline uint32_t BaseString::ComputeHashcodeUtf8(const uint8_t *utf8Data, size_t utf8Len, bool canBeCompress)
{
    if (canBeCompress) {
        return ComputeHashForData(utf8Data, utf8Len, 0);
    }
    auto utf16Len = UtfUtils::Utf8ToUtf16Size(utf8Data, utf8Len);
    std::vector<uint16_t> tmpBuffer(utf16Len);
    [[maybe_unused]] auto len = UtfUtils::ConvertRegionUtf8ToUtf16(utf8Data, tmpBuffer.data(), utf8Len, utf16Len);
    DCHECK_CC(len == utf16Len);
    return ComputeHashForData(tmpBuffer.data(), utf16Len, 0);
}
#else
inline uint32_t BaseString::ComputeHashcodeUtf8(const uint8_t *utf8Data, size_t utf8Len, bool canBeCompress)
{
    if (utf8Len == 0) {
        return MixHashcode(0, NOT_INTEGER);
    }
    if (canBeCompress) {
        uint32_t mixHash = 0;
        // String using UTF8 encoding, and length smaller than 10, try to compute integer hash.
        if (utf8Len < MAX_ELEMENT_INDEX_LEN && HashIntegerString(utf8Data, utf8Len, &mixHash, 0)) {
            return mixHash;
        }
        uint32_t hash = ComputeHashForData(utf8Data, utf8Len, 0);
        return MixHashcode(hash, NOT_INTEGER);
    }
    auto utf16Len = UtfUtils::Utf8ToUtf16Size(utf8Data, utf8Len);
    std::vector<uint16_t> tmpBuffer(utf16Len);
    [[maybe_unused]] auto len = UtfUtils::ConvertRegionUtf8ToUtf16(utf8Data, tmpBuffer.data(), utf8Len, utf16Len);
    DCHECK_CC(len == utf16Len);
    uint32_t hash = ComputeHashForData(tmpBuffer.data(), utf16Len, 0);
    return MixHashcode(hash, NOT_INTEGER);
}
#endif

#if defined(PANDA_32_BIT_MANAGED_POINTER)
inline uint32_t BaseString::ComputeHashcodeUtf16(const uint16_t *utf16Data, uint32_t length)
{
    return ComputeHashForData(utf16Data, length, 0);
}
#else
/* static */
inline uint32_t BaseString::ComputeHashcodeUtf16(const uint16_t *utf16Data, uint32_t length)
{
    if (length == 0) {
        return MixHashcode(0, NOT_INTEGER);
    }
    uint32_t mixHash = 0;
    // String length smaller than 10, try to compute integer hash.
    if (length < MAX_ELEMENT_INDEX_LEN && HashIntegerString(utf16Data, length, &mixHash, 0)) {
        return mixHash;
    }
    uint32_t hash = ComputeHashForData(utf16Data, length, 0);
    return MixHashcode(hash, NOT_INTEGER);
}
#endif

static size_t FixUtf8Len(const uint8_t *utf8, size_t utf8Len)
{
    constexpr size_t TWO_BYTES_LENGTH = 2;
    constexpr size_t THREE_BYTES_LENGTH = 3;
    size_t trimSize = 0;
    if (utf8Len >= 1 && utf8[utf8Len - 1] >= 0xC0) {
        // The last one char claim there are more than 1 byte next to it, it's invalid, so drop the last one.
        trimSize = 1;
    }
    if (utf8Len >= TWO_BYTES_LENGTH && utf8[utf8Len - TWO_BYTES_LENGTH] >= 0xE0) {
        // The second to last char claim there are more than 2 bytes next to it, it's invalid, so drop the last two.
        trimSize = TWO_BYTES_LENGTH;
    }
    if (utf8Len >= THREE_BYTES_LENGTH && utf8[utf8Len - THREE_BYTES_LENGTH] >= 0xF0) {
        // The third to last char claim there are more than 3 bytes next to it, it's invalid, so drop the last three.
        trimSize = THREE_BYTES_LENGTH;
    }
    return utf8Len - trimSize;
}

/* static */
// CC-OFFNXT(C_RULE_ID_INLINE_FUNCTION_SIZE) Perf critical common runtime code stub
// CC-OFFNXT(G.FUD.06) perf critical
// CC-OFFNXT(huge_cyclomatic_complexity[C], huge_method[C], huge_depth[C], G.FUN.01-CPP) solid logic
inline bool BaseString::IsUtf8EqualsUtf16(const uint8_t *utf8Data, size_t utf8Len, const uint16_t *utf16Data,
                                          uint32_t utf16Len)
{
    constexpr size_t LOW_3BITS = 0x7;
    constexpr size_t LOW_4BITS = 0xF;
    constexpr size_t LOW_5BITS = 0x1F;
    constexpr size_t LOW_6BITS = 0x3F;
    constexpr size_t L_SURROGATE_START = 0xDC00;
    constexpr size_t H_SURROGATE_START = 0xD800;
    constexpr size_t SURROGATE_RAIR_START = 0x10000;
    constexpr size_t OFFSET_18POS = 18;
    constexpr size_t OFFSET_12POS = 12;
    constexpr size_t OFFSET_10POS = 10;
    constexpr size_t OFFSET_6POS = 6;
    size_t safeUtf8Len = FixUtf8Len(utf8Data, utf8Len);
    const uint8_t *utf8End = utf8Data + utf8Len;
    const uint8_t *utf8SafeEnd = utf8Data + safeUtf8Len;
    const uint16_t *utf16End = utf16Data + utf16Len;
    while (utf8Data < utf8SafeEnd && utf16Data < utf16End) {
        uint8_t src = *utf8Data;
        switch (src & 0xF0) {  // NOLINT(hicpp-signed-bitwise)
            case 0xF0: {
                const uint8_t c2 = *(++utf8Data);
                const uint8_t c3 = *(++utf8Data);
                const uint8_t c4 = *(++utf8Data);
                uint32_t codePoint = ((src & LOW_3BITS) << OFFSET_18POS) | ((c2 & LOW_6BITS) << OFFSET_12POS) |
                                     ((c3 & LOW_6BITS) << OFFSET_6POS) | (c4 & LOW_6BITS);
                if (codePoint >= SURROGATE_RAIR_START) {
                    // CC-OFFNXT(G.FUN.01-CPP) solid logic
                    if (utf16Data >= utf16End - 1) {
                        return false;
                    }
                    codePoint -= SURROGATE_RAIR_START;
                    // CC-OFFNXT(G.FUN.01-CPP) solid logic
                    if (*utf16Data++ != static_cast<uint16_t>((codePoint >> OFFSET_10POS) | H_SURROGATE_START)) {
                        return false;
                        // CC-OFFNXT(G.FUN.01-CPP) solid logic
                        // NOLINT(hicpp-signed-bitwise)
                    } else if (*utf16Data++ != static_cast<uint16_t>((codePoint & 0x3FF) | L_SURROGATE_START)) {
                        return false;
                    }
                } else {
                    // CC-OFFNXT(G.FUN.01-CPP) solid logic
                    if (*utf16Data++ != static_cast<uint16_t>(codePoint)) {
                        return false;
                    }
                }
                utf8Data++;
                break;
            }
            case 0xE0: {
                const uint8_t c2 = *(++utf8Data);
                const uint8_t c3 = *(++utf8Data);
                if (*utf16Data++ != static_cast<uint16_t>(((src & LOW_4BITS) << OFFSET_12POS) |
                                                          ((c2 & LOW_6BITS) << OFFSET_6POS) | (c3 & LOW_6BITS))) {
                    return false;
                }
                utf8Data++;
                break;
            }
            case 0xD0:
            case 0xC0: {
                const uint8_t c2 = *(++utf8Data);
                if (*utf16Data++ != static_cast<uint16_t>(((src & LOW_5BITS) << OFFSET_6POS) | (c2 & LOW_6BITS))) {
                    return false;
                }
                utf8Data++;
                break;
            }
            default:
                do {
                    // CC-OFFNXT(G.FUN.01-CPP) solid logic
                    if (*utf16Data++ != static_cast<uint16_t>(*utf8Data++)) {
                        return false;
                    }
                } while (utf8Data < utf8SafeEnd && utf16Data < utf16End && *utf8Data < 0x80);
                break;
        }
    }
    // The remain chars should be treated as single byte char.
    while (utf8Data < utf8End && utf16Data < utf16End) {
        // CC-OFFNXT(G.FUN.01-CPP) solid logic
        if (*utf16Data++ != static_cast<uint16_t>(*utf8Data++)) {
            return false;
        }
    }
    return utf8Data == utf8End && utf16Data == utf16End;
}
}  // namespace common

#endif  // COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_IMPL_H
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers, readability-else-after-return,
// hicpp-signed-bitwise)

