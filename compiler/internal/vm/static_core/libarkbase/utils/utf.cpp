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

#include "utf.h"

#include <cstddef>
#include <cstring>

#include <limits>
#include <tuple>
#include <utility>

#include "securec.h"

// NOLINTNEXTLINE(hicpp-signed-bitwise)
static constexpr uint32_t U16_SURROGATE_OFFSET = (0xd800 << 10UL) + 0xdc00 - 0x10000;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define U16_GET_SUPPLEMENTARY(lead, trail) \
    ((static_cast<uint32_t>(lead) << 10UL) + static_cast<uint32_t>(trail) - U16_SURROGATE_OFFSET)

namespace ark::utf {

/*
 * MUtf-8
 *
 * U+0000 => C0 80
 *
 * N  Bits for     First        Last        Byte 1      Byte 2      Byte 3      Byte 4      Byte 5      Byte 6
 *    code point   code point   code point
 * 1  7            U+0000       U+007F      0xxxxxxx
 * 2  11           U+0080       U+07FF      110xxxxx    10xxxxxx
 * 3  16           U+0800       U+FFFF      1110xxxx    10xxxxxx    10xxxxxx
 * 6  21           U+10000      U+10FFFF    11101101    1010xxxx    10xxxxxx    11101101    1011xxxx    10xxxxxx
 * for U+10000 -- U+10FFFF encodes the following (value - 0x10000)
 */

/*
 * Convert mutf8 sequence to utf16 pair and return pair: [utf16 code point, mutf8 size].
 * In case of invalid sequence return first byte of it.
 */
std::pair<uint32_t, size_t> ConvertMUtf8ToUtf16Pair(const uint8_t *data, size_t maxBytes)
{
    // NOTE(d.kovalneko): make the function safe
    Span<const uint8_t> sp(data, maxBytes);
    uint8_t d0 = sp[0];
    if ((d0 & MASK1) == 0) {
        return {d0, 1};
    }

    if (maxBytes < CONST_2 || sp[1] == 0) {
        return {d0, 1};
    }
    uint8_t d1 = sp[1];
    if ((d0 & MASK2) == 0) {
        return {((d0 & MASK_5BIT) << DATA_WIDTH) | (d1 & MASK_6BIT), 2};
    }

    if (maxBytes < CONST_3 || sp[CONST_2] == 0) {
        return {d0, 1};
    }
    uint8_t d2 = sp[CONST_2];
    if ((d0 & MASK3) == 0) {
        return {((d0 & MASK_4BIT) << (DATA_WIDTH * CONST_2)) | ((d1 & MASK_6BIT) << DATA_WIDTH) | (d2 & MASK_6BIT),
                CONST_3};
    }

    if (maxBytes < CONST_4 || sp[CONST_3] == 0) {
        return {d0, 1};
    }
    uint8_t d3 = sp[CONST_3];
    uint32_t codePoint = ((d0 & MASK_4BIT) << (DATA_WIDTH * CONST_3)) | ((d1 & MASK_6BIT) << (DATA_WIDTH * CONST_2)) |
                         ((d2 & MASK_6BIT) << DATA_WIDTH) | (d3 & MASK_6BIT);

    uint32_t pair = 0;
    pair |= ((codePoint >> (PAIR_ELEMENT_WIDTH - DATA_WIDTH)) + U16_LEAD) & MASK_16BIT;
    pair <<= PAIR_ELEMENT_WIDTH;
    pair |= (codePoint & MASK_10BIT) + U16_TAIL;

    return {pair, CONST_4};
}

static constexpr uint32_t CombineTwoU16(uint16_t d0, uint16_t d1)
{
    uint32_t codePoint = d0 - DECODE_LEAD_LOW;
    codePoint <<= (PAIR_ELEMENT_WIDTH - DATA_WIDTH);
    codePoint |= d1 - DECODE_TRAIL_LOW;  // NOLINT(hicpp-signed-bitwise
    codePoint += DECODE_SECOND_FACTOR;
    return codePoint;
}

bool IsMUtf8OnlySingleBytes(const uint8_t *mutf8In)
{
    while (*mutf8In != '\0') {    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (*mutf8In >= MASK1) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return false;
        }
        mutf8In += 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return true;
}

size_t ConvertRegionUtf16ToMUtf8(const uint16_t *utf16In, uint8_t *mutf8Out, size_t utf16Len, size_t mutf8Len,
                                 size_t start)
{
    return ConvertRegionUtf16ToUtf8(utf16In, mutf8Out, utf16Len, mutf8Len, start, true);
}

void ConvertMUtf8ToUtf16(const uint8_t *mutf8In, size_t mutf8Len, uint16_t *utf16Out)
{
    size_t inPos = 0;
    while (inPos < mutf8Len) {
        auto [pair, nbytes] = ConvertMUtf8ToUtf16Pair(mutf8In, mutf8Len - inPos);
        auto [p_hi, p_lo] = SplitUtf16Pair(pair);

        if (p_hi != 0) {
            *utf16Out++ = p_hi;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        *utf16Out++ = p_lo;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        mutf8In += nbytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        inPos += nbytes;
    }
}

size_t ConvertRegionMUtf8ToUtf16(const uint8_t *mutf8In, uint16_t *utf16Out, size_t mutf8Len, size_t utf16Len,
                                 size_t start)
{
    size_t inPos = 0;
    size_t outPos = 0;
    while (inPos < mutf8Len) {
        auto [pair, nbytes] = ConvertMUtf8ToUtf16Pair(mutf8In, mutf8Len - inPos);
        auto [p_hi, p_lo] = SplitUtf16Pair(pair);

        mutf8In += nbytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        inPos += nbytes;
        if (start > 0) {
            start -= nbytes;
            continue;
        }

        if (p_hi != 0) {
            if (outPos++ >= utf16Len - 1) {  // check for place for two uint16
                --outPos;
                break;
            }
            *utf16Out++ = p_hi;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        if (outPos++ >= utf16Len) {
            --outPos;
            break;
        }
        *utf16Out++ = p_lo;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return outPos;
}

int CompareMUtf8ToMUtf8(const uint8_t *mutf81, const uint8_t *mutf82)
{
    uint32_t c1;
    uint32_t c2;
    uint32_t n1;
    uint32_t n2;

    do {
        c1 = *mutf81;
        c2 = *mutf82;

        if (c1 == 0 && c2 == 0) {
            return 0;
        }

        if (c1 == 0 && c2 != 0) {
            return -1;
        }

        if (c1 != 0 && c2 == 0) {
            return 1;
        }

        std::tie(c1, n1) = ConvertMUtf8ToUtf16Pair(mutf81);
        std::tie(c2, n2) = ConvertMUtf8ToUtf16Pair(mutf82);

        mutf81 += n1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        mutf82 += n2;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    } while (c1 == c2);

    auto [c1p1, c1p2] = SplitUtf16Pair(c1);
    auto [c2p1, c2p2] = SplitUtf16Pair(c2);

    auto result = static_cast<int>(c1p1 - c2p1);
    if (result != 0) {
        return result;
    }

    return c1p2 - c2p2;
}

// compare plain utf8, which allows 0 inside a string
int CompareUtf8ToUtf8(const uint8_t *utf81, size_t utf81Length, const uint8_t *utf82, size_t utf82Length)
{
    uint32_t c1;
    uint32_t c2;
    uint32_t n1;
    uint32_t n2;

    uint32_t utf81Index = 0;
    uint32_t utf82Index = 0;

    do {
        if (utf81Index == utf81Length && utf82Index == utf82Length) {
            return 0;
        }

        if (utf81Index == utf81Length && utf82Index < utf82Length) {
            return -1;
        }

        if (utf81Index < utf81Length && utf82Index == utf82Length) {
            return 1;
        }

        c1 = *utf81;
        c2 = *utf82;

        std::tie(c1, n1) = ConvertMUtf8ToUtf16Pair(utf81);
        std::tie(c2, n2) = ConvertMUtf8ToUtf16Pair(utf82);

        utf81 += n1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf82 += n2;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf81Index += n1;
        utf82Index += n2;
    } while (c1 == c2);

    auto [c1p1, c1p2] = SplitUtf16Pair(c1);
    auto [c2p1, c2p2] = SplitUtf16Pair(c2);

    auto result = static_cast<int>(c1p1 - c2p1);
    if (result != 0) {
        return result;
    }

    return c1p2 - c2p2;
}

size_t Mutf8Size(const uint8_t *mutf8)
{
    return strlen(Mutf8AsCString(mutf8));
}

size_t MUtf8ToUtf16Size(const uint8_t *mutf8)
{
    // NOTE(d.kovalenko): make it faster
    size_t res = 0;
    while (*mutf8 != '\0') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto [pair, nbytes] = ConvertMUtf8ToUtf16Pair(mutf8);
        res += pair > MAX_U16 ? CONST_2 : 1;
        mutf8 += nbytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return res;
}

size_t MUtf8ToUtf16Size(const uint8_t *mutf8, size_t mutf8Len)
{
    size_t pos = 0;
    size_t res = 0;
    while (pos != mutf8Len) {
        auto [pair, nbytes] = ConvertMUtf8ToUtf16Pair(mutf8, mutf8Len - pos);
        if (nbytes == 0) {
            nbytes = 1;
        }
        res += pair > MAX_U16 ? CONST_2 : 1;
        mutf8 += nbytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pos += nbytes;
    }
    return res;
}

bool IsEqual(Span<const uint8_t> utf81, Span<const uint8_t> utf82)
{
    if (utf81.size() != utf82.size()) {
        return false;
    }

    return memcmp(utf81.data(), utf82.data(), utf81.size()) == 0;
}

bool IsEqual(const uint8_t *mutf81, const uint8_t *mutf82)
{
    return strcmp(Mutf8AsCString(mutf81), Mutf8AsCString(mutf82)) == 0;
}

bool IsValidModifiedUTF8(const uint8_t *elems)
{
    ASSERT(elems);

    while (*elems != '\0') {
        // NOLINTNEXTLINE(hicpp-signed-bitwise, readability-magic-numbers)
        switch (*elems & 0xf0) {
            case 0x00:
            case 0x10:  // NOLINT(readability-magic-numbers)
            case 0x20:  // NOLINT(readability-magic-numbers)
            case 0x30:  // NOLINT(readability-magic-numbers)
            case 0x40:  // NOLINT(readability-magic-numbers)
            case 0x50:  // NOLINT(readability-magic-numbers)
            case 0x60:  // NOLINT(readability-magic-numbers)
            case 0x70:  // NOLINT(readability-magic-numbers)
                // pattern 0xxx
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++elems;
                break;
            case 0x80:  // NOLINT(readability-magic-numbers)
            case 0x90:  // NOLINT(readability-magic-numbers)
            case 0xa0:  // NOLINT(readability-magic-numbers)
            case 0xb0:  // NOLINT(readability-magic-numbers)
                // pattern 10xx is illegal start
                return false;

            case 0xf0:  // NOLINT(readability-magic-numbers)
                // pattern 1111 0xxx starts four byte section
                if ((*elems & 0x08) != 0) {  // NOLINT(hicpp-signed-bitwise)
                    return false;
                }
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++elems;
                if ((*elems & 0xc0) != 0x80) {  // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
                    return false;
                }
                // no need break
                [[fallthrough]];

            case 0xe0:  // NOLINT(readability-magic-numbers)
                // pattern 1110
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++elems;
                if ((*elems & 0xc0) != 0x80) {  // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
                    return false;
                }
                // no need break
                [[fallthrough]];

            case 0xc0:  // NOLINT(readability-magic-numbers)
            case 0xd0:  // NOLINT(readability-magic-numbers)
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++elems;
                if ((*elems & 0xc0) != 0x80) {  // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
                    return false;
                }
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++elems;
                break;
            default:
                UNREACHABLE();
                break;
        }
    }
    return true;
}

uint32_t UTF16Decode(uint16_t lead, uint16_t trail)
{
    ASSERT((lead >= DECODE_LEAD_LOW && lead <= DECODE_LEAD_HIGH) &&
           (trail >= DECODE_TRAIL_LOW && trail <= DECODE_TRAIL_HIGH));
    uint32_t cp = (lead - DECODE_LEAD_LOW) * DECODE_FIRST_FACTOR + (trail - DECODE_TRAIL_LOW) + DECODE_SECOND_FACTOR;
    return cp;
}

bool IsValidUTF8(const std::vector<uint8_t> &data)
{
    uint32_t length = data.size();
    switch (length) {
        case UtfLength::ONE:
            if (data.at(0) >= BIT_MASK_1) {
                return false;
            }
            break;
        case UtfLength::TWO:
            if ((data.at(0) & BIT_MASK_3) != BIT_MASK_2) {
                return false;
            }
            break;
        case UtfLength::THREE:
            if ((data.at(0) & BIT_MASK_4) != BIT_MASK_3) {
                return false;
            }
            break;
        case UtfLength::FOUR:
            if ((data.at(0) & BIT_MASK_5) != BIT_MASK_4) {
                return false;
            }
            break;
        default:
            UNREACHABLE();
            break;
    }

    for (uint32_t i = 1; i < length; i++) {
        if ((data.at(i) & BIT_MASK_2) != BIT_MASK_1) {
            return false;
        }
    }
    return true;
}

Utf8Char ConvertUtf16ToUtf8(uint16_t d0, uint16_t d1, bool modify)
{
    // when first utf16 code is in 0xd800-0xdfff and second utf16 code is 0,
    // means that is a single code point, it needs to be represented by three UTF8 code.
    if (d1 == 0 && d0 >= DECODE_LEAD_LOW && d0 <= DECODE_TRAIL_HIGH) {
        auto ch0 = static_cast<uint8_t>(UTF8_3B_FIRST | static_cast<uint8_t>(d0 >> UtfOffset::TWELVE));
        auto ch1 = static_cast<uint8_t>(UTF8_3B_SECOND | (static_cast<uint8_t>(d0 >> UtfOffset::SIX) & MASK_6BIT));
        auto ch2 = static_cast<uint8_t>(UTF8_3B_THIRD | (d0 & MASK_6BIT));
        return {UtfLength::THREE, {ch0, ch1, ch2}};
    }

    if (d0 == 0) {
        if (modify) {
            // special case for \u0000 ==> C080 - 1100'0000 1000'0000
            return {UtfLength::TWO, {UTF8_2B_FIRST, UTF8_2B_SECOND}};
        }
        return {UtfLength::ONE, {0x00U}};
    }
    if (d0 <= UTF8_1B_MAX) {
        return {UtfLength::ONE, {static_cast<uint8_t>(d0)}};
    }
    if (d0 <= UTF8_2B_MAX) {
        auto ch0 = static_cast<uint8_t>(UTF8_2B_FIRST | static_cast<uint8_t>(d0 >> UtfOffset::SIX));
        auto ch1 = static_cast<uint8_t>(UTF8_2B_SECOND | (d0 & MASK_6BIT));
        return {UtfLength::TWO, {ch0, ch1}};
    }
    if (d0 < DECODE_LEAD_LOW || d0 > DECODE_LEAD_HIGH) {
        auto ch0 = static_cast<uint8_t>(UTF8_3B_FIRST | static_cast<uint8_t>(d0 >> UtfOffset::TWELVE));
        auto ch1 = static_cast<uint8_t>(UTF8_3B_SECOND | (static_cast<uint8_t>(d0 >> UtfOffset::SIX) & MASK_6BIT));
        auto ch2 = static_cast<uint8_t>(UTF8_3B_THIRD | (d0 & MASK_6BIT));
        return {UtfLength::THREE, {ch0, ch1, ch2}};
    }
    if (d1 < DECODE_TRAIL_LOW || d1 > DECODE_TRAIL_HIGH) {
        // Isolated high surrogate: malformed, replace with U+FFFD
        return {UtfLength::THREE, {0xEFU, 0xBFU, 0xBDU}};
    }

    uint32_t codePoint = CombineTwoU16(d0, d1);

    auto ch0 = static_cast<uint8_t>((codePoint >> UtfOffset::EIGHTEEN) | UTF8_4B_FIRST);
    auto ch1 = static_cast<uint8_t>(((codePoint >> UtfOffset::TWELVE) & MASK_6BIT) | MASK1);
    auto ch2 = static_cast<uint8_t>(((codePoint >> UtfOffset::SIX) & MASK_6BIT) | MASK1);
    auto ch3 = static_cast<uint8_t>((codePoint & MASK_6BIT) | MASK1);

    return {UtfLength::FOUR, {ch0, ch1, ch2, ch3}};
}

size_t Utf16ToUtf8Size(const uint16_t *utf16, uint32_t length, bool modify)
{
    size_t res = 1;  // zero byte
    // when utf16 data length is only 1 and code in 0xd800-0xdfff,
    // means that is a single code point, it needs to be represented by three UTF8 code.
    if (length == 1 && utf16[0] >= DECODE_LEAD_LOW &&  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf16[0] <= DECODE_TRAIL_HIGH) {               // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        res += UtfLength::THREE;
        return res;
    }

    for (uint32_t i = 0; i < length; ++i) {
        if (utf16[i] == 0) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (modify) {
                res += UtfLength::TWO;  // special case for U+0000 => C0 80
            } else {
                res += UtfLength::ONE;
            }
        } else if (utf16[i] <= UTF8_1B_MAX) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            res += 1;
        } else if (utf16[i] <= UTF8_2B_MAX) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            res += UtfLength::TWO;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else if (utf16[i] < DECODE_LEAD_LOW || utf16[i] > DECODE_LEAD_HIGH) {
            res += UtfLength::THREE;
        } else {
            if (i < length - 1 &&
                utf16[i + 1] >= DECODE_TRAIL_LOW &&   // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                utf16[i + 1] <= DECODE_TRAIL_HIGH) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                res += UtfLength::FOUR;
                ++i;
            } else {
                res += UtfLength::THREE;
            }
        }
    }
    return res;
}

size_t Utf16ToMUtf8Size(const uint16_t *mutf16, uint32_t length)
{
    return Utf16ToUtf8Size(mutf16, length, true);
}

// CC-OFFNXT(G.FUN.01) solid logic
size_t ConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len, size_t utf8Len,
                                size_t start, bool modify)
{
    size_t utf8Pos = 0;
    if (utf16In == nullptr || utf8Out == nullptr || utf8Len == 0) {
        return 0;
    }
    size_t end = start + utf16Len;
    for (size_t i = start; i < end; ++i) {
        uint16_t next16Code = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if ((i + 1) != end && IsAvailableNextUtf16Code(utf16In[i + 1])) {
            next16Code = utf16In[i + 1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Utf8Char ch = ConvertUtf16ToUtf8(utf16In[i], next16Code, modify);
        if (utf8Pos + ch.n > utf8Len) {
            break;
        }
        for (size_t c = 0; c < ch.n; ++c) {
            utf8Out[utf8Pos++] = ch.ch[c];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        if (ch.n == UtfLength::FOUR) {  // Two UTF-16 chars are used
            ++i;
        }
    }
    return utf8Pos;
}

std::pair<uint32_t, size_t> ConvertUtf8ToUtf16Pair(const uint8_t *data, bool combine)
{
    uint8_t d0 = data[0];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if ((d0 & MASK1) == 0) {
        return {d0, 1};
    }

    uint8_t d1 = data[1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if ((d0 & MASK2) == 0) {
        return {((d0 & MASK_5BIT) << DATA_WIDTH) | (d1 & MASK_6BIT), UtfLength::TWO};
    }

    uint8_t d2 = data[UtfLength::TWO];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if ((d0 & MASK3) == 0) {
        return {((d0 & MASK_4BIT) << UtfOffset::TWELVE) | ((d1 & MASK_6BIT) << DATA_WIDTH) | (d2 & MASK_6BIT),
                UtfLength::THREE};
    }

    uint8_t d3 = data[UtfLength::THREE];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint32_t codePoint = ((d0 & MASK_4BIT) << UtfOffset::EIGHTEEN) | ((d1 & MASK_6BIT) << UtfOffset::TWELVE) |
                         ((d2 & MASK_6BIT) << DATA_WIDTH) | (d3 & MASK_6BIT);

    uint32_t pair = 0;
    if (combine) {
        uint32_t lead = ((codePoint >> (PAIR_ELEMENT_WIDTH - DATA_WIDTH)) + U16_LEAD);
        uint32_t tail = ((codePoint & MASK_10BIT) + U16_TAIL) & MASK_16BIT;
        pair = U16_GET_SUPPLEMENTARY(lead, tail);  // NOLINT(hicpp-signed-bitwise)
    } else {
        pair |= ((codePoint >> (PAIR_ELEMENT_WIDTH - DATA_WIDTH)) + U16_LEAD) << PAIR_ELEMENT_WIDTH;
        pair |= ((codePoint & MASK_10BIT) + U16_TAIL) & MASK_16BIT;
    }

    return {pair, UtfLength::FOUR};
}

size_t Utf8ToUtf16Size(const uint8_t *utf8, size_t utf8Len)
{
    return MUtf8ToUtf16Size(utf8, utf8Len);
}

size_t ConvertRegionUtf8ToUtf16(const uint8_t *utf8In, uint16_t *utf16Out, size_t utf8Len, size_t utf16Len,
                                size_t start)
{
    return ConvertRegionMUtf8ToUtf16(utf8In, utf16Out, utf8Len, utf16Len, start);
}

bool IsUTF16SurrogatePair(const uint16_t lead)
{
    return lead >= DECODE_LEAD_LOW && lead <= DECODE_LEAD_HIGH;
}

/**
 * The table below is to translate integer numbers from [0..99] range to pairs of corresponding utf16 codes.
 * The pairs are packed into utf::BidigitsCode type.
 *
 * Example: 0  -> 0x00300030 ("00")
 *          1  -> 0x00310030 ("01")
 *          ...
 *          99 -> 0x00390039 ("99")
 */
using BidigitsCode = uint32_t;
static constexpr size_t BIDIGITS_CODE_TAB_SIZE = 100U;

static constexpr std::array<BidigitsCode, BIDIGITS_CODE_TAB_SIZE> BIDIGITS_CODE_TAB = {
    0x00300030, 0x00310030, 0x00320030, 0x00330030, 0x00340030, 0x00350030, 0x00360030, 0x00370030, 0x00380030,
    0x00390030, 0x00300031, 0x00310031, 0x00320031, 0x00330031, 0x00340031, 0x00350031, 0x00360031, 0x00370031,
    0x00380031, 0x00390031, 0x00300032, 0x00310032, 0x00320032, 0x00330032, 0x00340032, 0x00350032, 0x00360032,
    0x00370032, 0x00380032, 0x00390032, 0x00300033, 0x00310033, 0x00320033, 0x00330033, 0x00340033, 0x00350033,
    0x00360033, 0x00370033, 0x00380033, 0x00390033, 0x00300034, 0x00310034, 0x00320034, 0x00330034, 0x00340034,
    0x00350034, 0x00360034, 0x00370034, 0x00380034, 0x00390034, 0x00300035, 0x00310035, 0x00320035, 0x00330035,
    0x00340035, 0x00350035, 0x00360035, 0x00370035, 0x00380035, 0x00390035, 0x00300036, 0x00310036, 0x00320036,
    0x00330036, 0x00340036, 0x00350036, 0x00360036, 0x00370036, 0x00380036, 0x00390036, 0x00300037, 0x00310037,
    0x00320037, 0x00330037, 0x00340037, 0x00350037, 0x00360037, 0x00370037, 0x00380037, 0x00390037, 0x00300038,
    0x00310038, 0x00320038, 0x00330038, 0x00340038, 0x00350038, 0x00360038, 0x00370038, 0x00380038, 0x00390038,
    0x00300039, 0x00310039, 0x00320039, 0x00330039, 0x00340039, 0x00350039, 0x00360039, 0x00370039, 0x00380039,
    0x00390039};

void UInt64ToUtf16Array(uint64_t v, uint16_t *outUtf16Buf, uint32_t nDigits, bool negative)
{
    ASSERT(outUtf16Buf != nullptr && nDigits != 0);

    constexpr uint64_t POW10_1 = 10U;
    constexpr uint64_t POW10_2 = 100U;

    auto *bufEnd = outUtf16Buf + nDigits;
    constexpr uint64_t endShift = 2;  // uint32_t = 2 * uint16_t
    while (v >= POW10_2) {
        bufEnd -= endShift;
        if (memcpy_s(bufEnd, endShift * sizeof(uint16_t), &BIDIGITS_CODE_TAB[v % POW10_2], sizeof(uint32_t))) {
            UNREACHABLE();
        }
        v /= POW10_2;
    }
    if (v >= POW10_1) {
        bufEnd -= endShift;
        if (memcpy_s(bufEnd, endShift * sizeof(uint16_t), &BIDIGITS_CODE_TAB[v], sizeof(uint32_t))) {
            UNREACHABLE();
        }
    } else {
        *(outUtf16Buf + negative) = v + '0';
    }
    if (negative) {
        *outUtf16Buf = '-';
    }
}

static constexpr uint16_t C_SPACE = 0x0020;
static constexpr uint16_t C_0009 = 0x0009;
static constexpr uint16_t C_000D = 0x000D;
static constexpr uint16_t C_000E = 0x000E;
static constexpr uint16_t C_00A0 = 0x00A0;
static constexpr uint16_t C_1680 = 0x1680;
static constexpr uint16_t C_2000 = 0x2000;
static constexpr uint16_t C_200A = 0x200A;
static constexpr uint16_t C_2028 = 0x2028;
static constexpr uint16_t C_2029 = 0x2029;
static constexpr uint16_t C_202F = 0x202F;
static constexpr uint16_t C_205F = 0x205F;
static constexpr uint16_t C_3000 = 0x3000;
static constexpr uint16_t C_FEFF = 0xFEFF;

bool IsWhiteSpaceChar(uint16_t c)
{
    if (c == C_SPACE) {
        return true;
    }
    // [0x000E, 0x009F] -- common non-whitespace characters
    if (C_000E <= c && c < C_00A0) {
        return false;
    }
    // 0x0009 -- horizontal tab
    if (c < C_0009) {
        return false;
    }
    // 0x000A -- line feed or new line
    // 0x000B -- vertical tab
    // 0x000C -- formfeed
    // 0x000D -- carriage return
    if (c <= C_000D) {
        return true;
    }
    // 0x00A0 -- no-break space
    if (c == C_00A0) {
        return true;
    }
    // 0x1680 -- Ogham space mark
    if (c == C_1680) {
        return true;
    }
    // 0x2000 -- en quad
    if (c < C_2000) {
        return false;
    }
    // 0x2001 -- em quad
    // 0x2002 -- en space
    // 0x2003 -- em space
    // 0x2004 -- three-per-em space
    // 0x2005 -- four-per-em space
    // 0x2006 -- six-per-em space
    // 0x2007 -- figure space
    // 0x2008 -- punctuation space
    // 0x2009 -- thin space
    // 0x200A -- hair space
    if (c <= C_200A) {
        return true;
    }
    // 0x2028 -- line separator
    if (c == C_2028) {
        return true;
    }
    // 0x2029 -- paragraph separator
    if (c == C_2029) {
        return true;
    }
    // 0x202F -- narrow no-break space
    if (c == C_202F) {
        return true;
    }
    // 0x205F -- medium mathematical space
    if (c == C_205F) {
        return true;
    }
    // 0xFEFF -- byte order mark
    if (c == C_FEFF) {
        return true;
    }
    // 0x3000 -- ideographic space
    if (c == C_3000) {
        return true;
    }
    return false;
}

}  // namespace ark::utf
