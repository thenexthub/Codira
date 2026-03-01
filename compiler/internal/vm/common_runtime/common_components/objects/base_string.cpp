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
#include "common_interfaces/objects/string/base_string.h"

#include "common_interfaces/objects/string/base_string-inl.h"
#include "common_interfaces/objects/utils/utf_utils.h"
#include "common_components/base/utf_helper.h"
#include "common_components/platform/string_hash.h"
#include "common_components/platform/string_hash_helper.h"

#include <codecvt>
#include <locale>

namespace common {
size_t UtfUtils::DebuggerConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len,
                                                  size_t utf8Len, size_t start, bool modify, bool isWriteBuffer)
{
    return utf_helper::DebuggerConvertRegionUtf16ToUtf8(utf16In, utf8Out, utf16Len, utf8Len, start, modify,
                                                        isWriteBuffer);
}

size_t UtfUtils::ConvertRegionUtf16ToLatin1(const uint16_t *utf16In, uint8_t *latin1Out, size_t utf16Len,
                                            size_t latin1Len)
{
    return utf_helper::ConvertRegionUtf16ToLatin1(utf16In, latin1Out, utf16Len, latin1Len);
}

// To change the hash algorithm of BaseString, please modify BaseString::CalculateConcatHashCode
// and BaseStringHashHelper::ComputeHashForDataPlatform simultaneously!!
template <typename T>
uint32_t ComputeHashForDataInternal(const T *data, size_t size, uint32_t hashSeed)
{
    if (size <= static_cast<size_t>(StringHash::MIN_SIZE_FOR_UNROLLING)) {
        uint32_t hash = hashSeed;
        for (uint32_t i = 0; i < size; i++) {
            hash = (hash << static_cast<uint32_t>(StringHash::HASH_SHIFT)) - hash + data[i];
        }
        return hash;
    }
    return StringHashHelper::ComputeHashForDataPlatform(data, size, hashSeed);
}

// static
template <typename T1, typename T2>
uint32_t BaseString::CalculateDataConcatHashCode(const T1 *dataFirst, size_t sizeFirst, const T2 *dataSecond,
                                                 size_t sizeSecond)
{
    uint32_t totalHash = ComputeHashForData(dataFirst, sizeFirst, 0);
    totalHash = ComputeHashForData(dataSecond, sizeSecond, totalHash);
    return MixHashcode(totalHash, NOT_INTEGER);
}

template uint32_t BaseString::CalculateDataConcatHashCode<uint8_t, uint8_t>(const uint8_t *dataFirst, size_t sizeFirst,
                                                                            const uint8_t *dataSecond,
                                                                            size_t sizeSecond);
template uint32_t BaseString::CalculateDataConcatHashCode<uint16_t, uint16_t>(const uint16_t *dataFirst,
                                                                              size_t sizeFirst,
                                                                              const uint16_t *dataSecond,
                                                                              size_t sizeSecond);
template uint32_t BaseString::CalculateDataConcatHashCode<uint8_t, uint16_t>(const uint8_t *dataFirst, size_t sizeFirst,
                                                                             const uint16_t *dataSecond,
                                                                             size_t sizeSecond);
template uint32_t BaseString::CalculateDataConcatHashCode<uint16_t, uint8_t>(const uint16_t *dataFirst,
                                                                             size_t sizeFirst,
                                                                             const uint8_t *dataSecond,
                                                                             size_t sizeSecond);

template <typename T1, typename T2>
bool IsSubStringAtSpan(common::Span<T1> &lhsSp, common::Span<T2> &rhsSp, uint32_t offset)
{
    size_t rhsSize = rhsSp.size();
    DCHECK_CC(rhsSize + offset <= lhsSp.size());
    for (size_t i = 0; i < rhsSize; ++i) {
        auto left = static_cast<int32_t>(lhsSp[offset + static_cast<uint32_t>(i)]);
        auto right = static_cast<int32_t>(rhsSp[i]);
        if (left != right) {
            return false;
        }
    }
    return true;
}

template bool IsSubStringAtSpan<const uint8_t, const uint8_t>(common::Span<const uint8_t> &lhsSp,
                                                              common::Span<const uint8_t> &rhsSp, uint32_t offset);
template bool IsSubStringAtSpan<const uint16_t, const uint16_t>(common::Span<const uint16_t> &lhsSp,
                                                                common::Span<const uint16_t> &rhsSp, uint32_t offset);
template bool IsSubStringAtSpan<const uint8_t, const uint16_t>(common::Span<const uint8_t> &lhsSp,
                                                               common::Span<const uint16_t> &rhsSp, uint32_t offset);
template bool IsSubStringAtSpan<const uint16_t, const uint8_t>(common::Span<const uint16_t> &lhsSp,
                                                               common::Span<const uint8_t> &rhsSp, uint32_t offset);

std::u16string Utf16ToU16String(const uint16_t *utf16Data, uint32_t dataLen)
{
    auto *char16tData = reinterpret_cast<const char16_t *>(utf16Data);
    std::u16string u16str(char16tData, dataLen);
    return u16str;
}

std::u16string Utf8ToU16String(const uint8_t *utf8Data, uint32_t dataLen)
{
    auto *charData = reinterpret_cast<const char *>(utf8Data);
    std::string str(charData, dataLen);
    std::u16string u16str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(str);
    return u16str;
}
}  // namespace common

