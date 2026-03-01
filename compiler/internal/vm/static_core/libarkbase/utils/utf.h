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

#ifndef PANDA_LIBPANDABASE_UTILS_UTF_H_
#define PANDA_LIBPANDABASE_UTILS_UTF_H_

#include <cstdint>
#include <cstddef>

#include "libarkbase/utils/hash.h"
#include "libarkbase/utils/span.h"

namespace ark::utf {

/*
 * https://en.wikipedia.org/wiki/UTF-8
 *
 * N  Bits for     First        Last        Byte 1      Byte 2      Byte 3      Byte 4
 *    code point   code point   code point
 * 1  7            U+0000       U+007F      0xxxxxxx
 * 2  11           U+0080       U+07FF      110xxxxx    10xxxxxx
 * 3  16           U+0800       U+FFFF      1110xxxx    10xxxxxx    10xxxxxx
 * 4  21           U+10000      U+10FFFF    11110xxx    10xxxxxx    10xxxxxx    10xxxxxx
 */
constexpr size_t MASK1 = 0x80;
constexpr size_t MASK2 = 0x20;
constexpr size_t MASK3 = 0x10;

constexpr size_t MASK_4BIT = 0x0f;
constexpr size_t MASK_5BIT = 0x1f;
constexpr size_t MASK_6BIT = 0x3f;
constexpr size_t MASK_10BIT = 0x03ff;
constexpr size_t MASK_16BIT = 0xffff;

constexpr size_t DATA_WIDTH = 6;
constexpr size_t PAIR_ELEMENT_WIDTH = 16;

constexpr size_t U16_LEAD = 0xd7c0;
constexpr size_t U16_TAIL = 0xdc00;

constexpr uint16_t DECODE_LEAD_LOW = 0xD800;
constexpr uint16_t DECODE_LEAD_HIGH = 0xDBFF;
constexpr uint16_t DECODE_TRAIL_LOW = 0xDC00;
constexpr uint16_t DECODE_TRAIL_HIGH = 0xDFFF;
constexpr uint32_t DECODE_FIRST_FACTOR = 0x400;
constexpr uint32_t DECODE_SECOND_FACTOR = 0x10000;

constexpr uint8_t BIT_MASK_1 = 0x80;
constexpr uint8_t BIT_MASK_2 = 0xC0;
constexpr uint8_t BIT_MASK_3 = 0xE0;
constexpr uint8_t BIT_MASK_4 = 0xF0;
constexpr uint8_t BIT_MASK_5 = 0xF8;

constexpr uint8_t UTF8_1B_MAX = 0x7f;

constexpr uint16_t UTF8_2B_MAX = 0x7ff;
constexpr uint8_t UTF8_2B_FIRST = 0xc0;
constexpr uint8_t UTF8_2B_SECOND = 0x80;
constexpr uint8_t UTF8_2B_THIRD = 0x3f;

constexpr uint8_t UTF8_3B_FIRST = 0xe0;
constexpr uint8_t UTF8_3B_SECOND = 0x80;
constexpr uint8_t UTF8_3B_THIRD = 0x80;

constexpr uint8_t UTF8_4B_FIRST = 0xf0;

enum UtfLength : uint8_t { ONE = 1, TWO = 2, THREE = 3, FOUR = 4 };
enum UtfOffset : uint8_t { SIX = 6, TEN = 10, TWELVE = 12, EIGHTEEN = 18 };

constexpr size_t MAX_BYTES = 4;
struct Utf8Char {
    size_t n;
    std::array<uint8_t, MAX_BYTES> ch;
};

constexpr size_t MAX_U16 = 0xffff;
constexpr size_t CONST_2 = 2;
constexpr size_t CONST_3 = 3;
constexpr size_t CONST_4 = 4;
constexpr size_t CONST_6 = 6;
constexpr size_t CONST_12 = 12;

WEAK_FOR_LTO_START

PANDA_PUBLIC_API std::pair<uint32_t, size_t> ConvertMUtf8ToUtf16Pair(const uint8_t *data, size_t maxBytes = 4);

PANDA_PUBLIC_API bool IsMUtf8OnlySingleBytes(const uint8_t *mutf8In);

PANDA_PUBLIC_API void ConvertMUtf8ToUtf16(const uint8_t *mutf8In, size_t mutf8Len, uint16_t *utf16Out);

PANDA_PUBLIC_API size_t ConvertRegionMUtf8ToUtf16(const uint8_t *mutf8In, uint16_t *utf16Out, size_t mutf8Len,
                                                  size_t utf16Len, size_t start);

PANDA_PUBLIC_API size_t ConvertRegionUtf16ToMUtf8(const uint16_t *utf16In, uint8_t *mutf8Out, size_t utf16Len,
                                                  size_t mutf8Len, size_t start);

PANDA_PUBLIC_API int CompareMUtf8ToMUtf8(const uint8_t *mutf81, const uint8_t *mutf82);

PANDA_PUBLIC_API int CompareUtf8ToUtf8(const uint8_t *utf81, size_t utf81Length, const uint8_t *utf82,
                                       size_t utf82Length);

PANDA_PUBLIC_API bool IsEqual(Span<const uint8_t> utf81, Span<const uint8_t> utf82);

PANDA_PUBLIC_API bool IsEqual(const uint8_t *mutf81, const uint8_t *mutf82);

PANDA_PUBLIC_API size_t MUtf8ToUtf16Size(const uint8_t *mutf8);

PANDA_PUBLIC_API size_t MUtf8ToUtf16Size(const uint8_t *mutf8, size_t mutf8Len);

PANDA_PUBLIC_API size_t Utf16ToMUtf8Size(const uint16_t *mutf16, uint32_t length);

PANDA_PUBLIC_API size_t Mutf8Size(const uint8_t *mutf8);

PANDA_PUBLIC_API bool IsValidModifiedUTF8(const uint8_t *elems);

PANDA_PUBLIC_API uint32_t UTF16Decode(uint16_t lead, uint16_t trail);

PANDA_PUBLIC_API bool IsValidUTF8(const std::vector<uint8_t> &data);

PANDA_PUBLIC_API Utf8Char ConvertUtf16ToUtf8(uint16_t d0, uint16_t d1, bool modify);

PANDA_PUBLIC_API size_t Utf16ToUtf8Size(const uint16_t *utf16, uint32_t length, bool modify = true);

PANDA_PUBLIC_API size_t ConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len,
                                                 size_t utf8Len, size_t start, bool modify = true);

PANDA_PUBLIC_API std::pair<uint32_t, size_t> ConvertUtf8ToUtf16Pair(const uint8_t *data, bool combine = false);

PANDA_PUBLIC_API size_t Utf8ToUtf16Size(const uint8_t *utf8, size_t utf8Len);

PANDA_PUBLIC_API size_t ConvertRegionUtf8ToUtf16(const uint8_t *utf8In, uint16_t *utf16Out, size_t utf8Len,
                                                 size_t utf16Len, size_t start);

PANDA_PUBLIC_API bool IsUTF16SurrogatePair(uint16_t lead);

PANDA_PUBLIC_API void UInt64ToUtf16Array(uint64_t v, uint16_t *outUtf16Buf, uint32_t nDigits, bool negative);

PANDA_PUBLIC_API bool IsWhiteSpaceChar(uint16_t c);

WEAK_FOR_LTO_END

inline const uint8_t *CStringAsMutf8(const char *str)
{
    return reinterpret_cast<const uint8_t *>(str);
}

inline const char *Mutf8AsCString(const uint8_t *mutf8)
{
    return reinterpret_cast<const char *>(mutf8);
}

inline constexpr bool IsAvailableNextUtf16Code(uint16_t val)
{
    return val >= DECODE_LEAD_LOW && val <= DECODE_TRAIL_HIGH;
}

inline Utf8Char ConvertUtf16ToMUtf8(uint16_t d0, uint16_t d1)
{
    return ConvertUtf16ToUtf8(d0, d1, true);
}

struct Mutf8Hash {
    uint32_t operator()(const uint8_t *data) const
    {
        return GetHash32String(data);
    }
};

struct Mutf8Equal {
    bool operator()(const uint8_t *mutf81, const uint8_t *mutf82) const
    {
        return IsEqual(mutf81, mutf82);
    }
};

struct Mutf8Less {
    bool operator()(const uint8_t *mutf81, const uint8_t *mutf82) const
    {
        return CompareMUtf8ToMUtf8(mutf81, mutf82) < 0;
    }
};

inline std::pair<uint16_t, uint16_t> SplitUtf16Pair(uint32_t pair)
{
    constexpr size_t P1_MASK = 0xffff;
    constexpr size_t P2_SHIFT = 16;
    return {pair >> P2_SHIFT, pair & P1_MASK};
}

}  // namespace ark::utf

#endif  // PANDA_LIBPANDABASE_UTILS_UTF_H_
