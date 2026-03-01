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

#include "common_components/base/config.h"
#include "common_components/base/utf_helper.h"
#include "common_interfaces/objects/utils/span.h"
#include "common_components/log/log.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
static constexpr int32_t U16_SURROGATE_OFFSET = (0xd800 << 10UL) + 0xdc00 - 0x10000;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define U16_GET_SUPPLEMENTARY(lead, trail) \
    ((static_cast<int32_t>(lead) << 10UL) + static_cast<int32_t>(trail) - U16_SURROGATE_OFFSET)

namespace common::utf_helper {

uint32_t UTF16Decode(uint16_t lead, uint16_t trail)
{
    DCHECK_CC((lead >= DECODE_LEAD_LOW && lead <= DECODE_LEAD_HIGH) &&
           (trail >= DECODE_TRAIL_LOW && trail <= DECODE_TRAIL_HIGH));
    uint32_t cp = (lead - DECODE_LEAD_LOW) * DECODE_FIRST_FACTOR + (trail - DECODE_TRAIL_LOW) + DECODE_SECOND_FACTOR;
    return cp;
}

// Methods for decode utf16 to unicode
uint32_t DecodeUTF16(uint16_t const *utf16, size_t len, size_t *index, bool cesu8)
{
    uint16_t high = utf16[*index];
    if ((high & SURROGATE_MASK) != DECODE_LEAD_LOW || !IsUTF16HighSurrogate(high) || *index == len - 1) {
        return high;
    }
    uint16_t low = utf16[*index + 1];
    if (!IsUTF16LowSurrogate(low) || cesu8) {
        return high;
    }
    (*index)++;
    return ((high - DECODE_LEAD_LOW) << UTF16_OFFSET) + (low - DECODE_TRAIL_LOW) + DECODE_SECOND_FACTOR;
}

uint32_t HandleAndDecodeInvalidUTF16(uint16_t const *utf16, size_t len, size_t *index)
{
    uint16_t first = utf16[*index];
    // A valid surrogate pair should always start with a High Surrogate
    if (IsUTF16LowSurrogate(first)) {
        return UTF16_REPLACEMENT_CHARACTER;
    }
    if (IsUTF16HighSurrogate(first) || (first & SURROGATE_MASK) == DECODE_LEAD_LOW) {
        if (*index == len - 1) {
            // A High surrogate not paired with another surrogate
            return UTF16_REPLACEMENT_CHARACTER;
        }
        uint16_t second = utf16[*index + 1];
        if (!IsUTF16LowSurrogate(second)) {
            // A High surrogate not followed by a low surrogate
            return UTF16_REPLACEMENT_CHARACTER;
        }
        // A valid surrogate pair, decode normally
        (*index)++;
        return ((first - DECODE_LEAD_LOW) << UTF16_OFFSET) + (second - DECODE_TRAIL_LOW) + DECODE_SECOND_FACTOR;
    }
    // A unicode not fallen into the range of representing by surrogate pair, return as it is
    return first;
}

inline size_t UTF8Length(uint32_t codepoint)
{
    if (codepoint <= UTF8_1B_MAX) {
        return UtfLength::ONE;
    }
    if (codepoint <= UTF8_2B_MAX) {
        return UtfLength::TWO;
    }
    if (codepoint <= UTF8_3B_MAX) {
        return UtfLength::THREE;
    }
    return UtfLength::FOUR;
}

// Methods for encode unicode to unicode
size_t EncodeUTF8(uint32_t codepoint, uint8_t* utf8, size_t index, size_t size)
{
    for (size_t j = size - 1; j > 0; j--) {
        uint8_t cont = ((codepoint | byteMark) & byteMask);
        utf8[index + j] = cont;
        codepoint >>= UTF8_OFFSET;
    }
    utf8[index] = codepoint | firstByteMark[size];
    return size;
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
            if (data.at(0) < UTF8_2B_FIRST_MIN) {
                return false;
            }
            break;
        case UtfLength::THREE:
            if ((data.at(0) & BIT_MASK_4) != BIT_MASK_3) {
                return false;
            }
            if (data.at(0) == UTF8_3B_FIRST && data.at(1) < UTF8_3B_SECOND_MIN) {
                return false;
            }
            // U+D800~U+DFFF is reserved for UTF-16 surrogate pairs, corresponds to %ED%A0%80~%ED%BF%BF
            if (data.at(0) == UTF8_3B_RESERVED_FIRST && data.at(1) >= UTF8_3B_RESERVED_SECOND_MIN &&
                data.at(1) <= UTF8_3B_RESERVED_SECOND_MAX) {
                return false;
            }
            break;
        case UtfLength::FOUR:
            if ((data.at(0) & BIT_MASK_5) != BIT_MASK_4) {
                return false;
            }
            if (data.at(0) == UTF8_4B_FIRST && data.at(1) < UTF8_4B_SECOND_MIN) {
                return false;
            }
            // max four length binary: 11110(100) 10(001111) 10(111111) 10(111111), max data[0] is 0xF4, data[1] is 0x8F
            if (data.at(0) > UTF8_4B_FIRST_MAX ||
               (data.at(0) == UTF8_4B_FIRST_MAX && data.at(1) > UTF8_4B_SECOND_MAX)) {
                return false;
            }
            break;
        default: //LCOV_EXCL_BR_LINE
            LOG_COMMON(FATAL) << "this branch is unreachable";
            UNREACHABLE_CC();
            break;
    }

    for (uint32_t i = 1; i < length; i++) {
        if ((data.at(i) & BIT_MASK_2) != BIT_MASK_1) {
            return false;
        }
    }
    return true;
}

Utf8Char ConvertUtf16ToUtf8(uint16_t d0, uint16_t d1, bool modify, bool isWriteBuffer)
{
    // when first utf16 code is in 0xd800-0xdfff and second utf16 code is 0,
    // means that is a single code point, it needs to be represented by three UTF8 code.
    if (d1 == 0 && d0 >= HI_SURROGATE_MIN && d0 <= LO_SURROGATE_MAX) {
        auto ch0 = static_cast<uint8_t>(UTF8_3B_FIRST | static_cast<uint8_t>(d0 >> UtfOffset::TWELVE));
        auto ch1 = static_cast<uint8_t>(UTF8_3B_SECOND | (static_cast<uint8_t>(d0 >> UtfOffset::SIX) & MASK_6BIT));
        auto ch2 = static_cast<uint8_t>(UTF8_3B_THIRD | (d0 & MASK_6BIT));
        return {UtfLength::THREE, {ch0, ch1, ch2}};
    }

    if (d0 == 0) {
        if (isWriteBuffer) {
            return {1, {0x00U}};
        }
        if (modify) {
            // special case for \u0000 ==> C080 - 1100'0000 1000'0000
            return {UtfLength::TWO, {UTF8_2B_FIRST, UTF8_2B_SECOND}};
        }
        // For print string, just skip '\u0000'
        return {0, {0x00U}};
    }
    if (d0 <= UTF8_1B_MAX) {
        return {UtfLength::ONE, {static_cast<uint8_t>(d0)}};
    }
    if (d0 <= UTF8_2B_MAX) {
        auto ch0 = static_cast<uint8_t>(UTF8_2B_FIRST | static_cast<uint8_t>(d0 >> UtfOffset::SIX));
        auto ch1 = static_cast<uint8_t>(UTF8_2B_SECOND | (d0 & MASK_6BIT));
        return {UtfLength::TWO, {ch0, ch1}};
    }
    if (d0 < HI_SURROGATE_MIN || d0 > HI_SURROGATE_MAX) {
        auto ch0 = static_cast<uint8_t>(UTF8_3B_FIRST | static_cast<uint8_t>(d0 >> UtfOffset::TWELVE));
        auto ch1 = static_cast<uint8_t>(UTF8_3B_SECOND | (static_cast<uint8_t>(d0 >> UtfOffset::SIX) & MASK_6BIT));
        auto ch2 = static_cast<uint8_t>(UTF8_3B_THIRD | (d0 & MASK_6BIT));
        return {UtfLength::THREE, {ch0, ch1, ch2}};
    }
    if (d1 < LO_SURROGATE_MIN || d1 > LO_SURROGATE_MAX) {
        // Bad sequence
        LOG_COMMON(FATAL) << "this branch is unreachable";
        UNREACHABLE_CC();
    }

    uint32_t codePoint = CombineTwoU16(d0, d1);

    auto ch0 = static_cast<uint8_t>((codePoint >> UtfOffset::EIGHTEEN) | UTF8_4B_FIRST);
    auto ch1 = static_cast<uint8_t>(((codePoint >> UtfOffset::TWELVE) & MASK_6BIT) | MASK1);
    auto ch2 = static_cast<uint8_t>(((codePoint >> UtfOffset::SIX) & MASK_6BIT) | MASK1);
    auto ch3 = static_cast<uint8_t>((codePoint & MASK_6BIT) | MASK1);
    return {UtfLength::FOUR, {ch0, ch1, ch2, ch3}};
}

size_t Utf16ToUtf8Size(const uint16_t *utf16, uint32_t length, bool modify, bool isGetBufferSize, bool cesu8)
{
    size_t res = 1;  // zero byte
    // when utf16 data length is only 1 and code in 0xd800-0xdfff,
    // means that is a single code point, it needs to be represented by three UTF8 code.
    if (length == 1 && utf16[0] >= HI_SURROGATE_MIN &&  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf16[0] <= LO_SURROGATE_MAX) {                 // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        res += UtfLength::THREE;
        return res;
    }

    for (uint32_t i = 0; i < length; ++i) {
        if (utf16[i] == 0) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (isGetBufferSize) {
                res += UtfLength::ONE;
            } else if (modify) {
                res += UtfLength::TWO;  // special case for U+0000 => C0 80
            }
        } else if (utf16[i] <= UTF8_1B_MAX) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            res += 1;
        } else if (utf16[i] <= UTF8_2B_MAX) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            res += UtfLength::TWO;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else if (utf16[i] < HI_SURROGATE_MIN || utf16[i] > HI_SURROGATE_MAX) {
            res += UtfLength::THREE;
        } else {
            if (!cesu8 && i < length - 1 &&
                utf16[i + 1] >= LO_SURROGATE_MIN &&  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                utf16[i + 1] <= LO_SURROGATE_MAX) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                res += UtfLength::FOUR;
                ++i;
            } else {
                res += UtfLength::THREE;
            }
        }
    }
    return res;
}

#if ENABLE_NEXT_OPTIMIZATION && defined(USE_CMC_GC)
size_t ConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len, size_t utf8Len,
                                size_t start, bool modify, bool isWriteBuffer, bool cesu8)
{
    if (utf16In == nullptr || utf8Out == nullptr || utf8Len == 0) {
        return 0;
    }
    size_t utf8Pos = 0;
    size_t end = start + utf16Len;
    for (size_t i = start; i < end; ++i) {
        uint32_t codepoint = utf16In[i];
        if (codepoint == 0) {
            if (isWriteBuffer) {
                utf8Out[utf8Pos++] = UTF8_NUL;
                continue;
            }
            if (modify) {
                utf8Out[utf8Pos++] = UTF8_2B_FIRST;
                utf8Out[utf8Pos++] = UTF8_2B_SECOND;
            }
            continue;
        }
        if (codepoint >= DECODE_LEAD_LOW && codepoint <= DECODE_LEAD_HIGH && i + 1 < end) {
            uint32_t high = utf16In[i];
            uint32_t low = utf16In[i + 1];
            if (!cesu8) {
                if (low >= DECODE_TRAIL_LOW && low <= DECODE_TRAIL_HIGH) {
                    codepoint =
                    SURROGATE_RAIR_START + ((high - DECODE_LEAD_LOW) << UTF16_OFFSET) + (low - DECODE_TRAIL_LOW);
                    i++;
                }
            }
        }
        if (codepoint <= UTF8_1B_MAX) {
            if (UNLIKELY_CC(utf8Pos + UTF8_SINGLE_BYTE_LENGTH > utf8Len)) {
                break;
            }
            utf8Out[utf8Pos++] = static_cast<uint8_t>(codepoint);
        } else if (codepoint <= UTF8_2B_MAX) {
            if (UNLIKELY_CC(utf8Pos + UTF8_DOUBLE_BYTE_LENGTH > utf8Len)) {
                break;
            }
            utf8Out[utf8Pos++] = (BIT_MASK_2 | (codepoint >> OFFSET_6POS));
            utf8Out[utf8Pos++] = (byteMark | (codepoint & LOW_6BITS));
        } else if (codepoint <= UTF8_3B_MAX) {
            if (UNLIKELY_CC(utf8Pos + UTF8_TRIPLE_BYTE_LENGTH > utf8Len)) {
                break;
            }
            utf8Out[utf8Pos++] = (UTF8_3B_FIRST | (codepoint >> OFFSET_12POS));
            utf8Out[utf8Pos++] = (byteMark | ((codepoint >> OFFSET_6POS) & LOW_6BITS));
            utf8Out[utf8Pos++] = (byteMark | (codepoint & LOW_6BITS));
        } else {
            if (UNLIKELY_CC(utf8Pos + UTF8_QUAD_BYTE_LENGTH > utf8Len)) {
                break;
            }
            utf8Out[utf8Pos++] = (UTF8_4B_FIRST | (codepoint >> OFFSET_18POS));
            utf8Out[utf8Pos++] = (byteMark | ((codepoint >> OFFSET_12POS) & LOW_6BITS));
            utf8Out[utf8Pos++] = (byteMark | ((codepoint >> OFFSET_6POS) & LOW_6BITS));
            utf8Out[utf8Pos++] = (byteMark | (codepoint & LOW_6BITS));
        }
    }
    return utf8Pos;
}
#else
size_t ConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len, size_t utf8Len,
                                size_t start, bool modify, bool isWriteBuffer, bool cesu8)
{
    if (utf16In == nullptr || utf8Out == nullptr || utf8Len == 0) {
        return 0;
    }
    size_t utf8Pos = 0;
    size_t end = start + utf16Len;
    for (size_t i = start; i < end; ++i) {
        uint32_t codepoint = DecodeUTF16(utf16In, end, &i, cesu8);
        if (codepoint == 0) {
            if (isWriteBuffer) {
                utf8Out[utf8Pos++] = 0x00U;
                continue;
            }
            if (modify) {
                // special case for \u0000 ==> C080 - 1100'0000 1000'0000
                utf8Out[utf8Pos++] = UTF8_2B_FIRST;
                utf8Out[utf8Pos++] = UTF8_2B_SECOND;
            }
            continue;
        }
        size_t size = UTF8Length(codepoint);
        if (utf8Pos + size > utf8Len) {
            break;
        }
        utf8Pos += EncodeUTF8(codepoint, utf8Out, utf8Pos, size);
    }
    return utf8Pos;
}
#endif

size_t DebuggerConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len, size_t utf8Len,
                                        size_t start, bool modify, bool isWriteBuffer)
{
    if (utf16In == nullptr || utf8Out == nullptr || utf8Len == 0) {
        return 0;
    }
    size_t utf8Pos = 0;
    size_t end = start + utf16Len;
    for (size_t i = start; i < end; ++i) {
        uint32_t codepoint = HandleAndDecodeInvalidUTF16(utf16In, end, &i);
        if (codepoint == 0) {
            if (isWriteBuffer) {
                utf8Out[utf8Pos++] = 0x00U;
                continue;
            }
            if (modify) {
                // special case for \u0000 ==> C080 - 1100'0000 1000'0000
                utf8Out[utf8Pos++] = UTF8_2B_FIRST;
                utf8Out[utf8Pos++] = UTF8_2B_SECOND;
            }
            continue;
        }
        size_t size = UTF8Length(codepoint);
        if (utf8Pos + size > utf8Len) {
            break;
        }
        utf8Pos += EncodeUTF8(codepoint, utf8Out, utf8Pos, size);
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
        return {((d0 & MASK_4BIT) << UtfOffset::TWELVE) | ((d1 & MASK_6BIT) << DATA_WIDTH) |
                    (d2 & MASK_6BIT),
                UtfLength::THREE};
    }

    uint8_t d3 = data[UtfLength::THREE];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint32_t codePoint = ((d0 & MASK_4BIT) << UtfOffset::EIGHTEEN) | ((d1 & MASK_6BIT) << UtfOffset::TWELVE) |
                         ((d2 & MASK_6BIT) << DATA_WIDTH) | (d3 & MASK_6BIT);

    uint32_t pair = 0;
    if (combine) {
        uint32_t lead = ((codePoint >> (PAIR_ELEMENT_WIDTH - DATA_WIDTH)) + U16_LEAD);
        uint32_t tail = ((codePoint & MASK_10BIT) + U16_TAIL) & MASK_16BIT;
        pair = static_cast<uint32_t>(U16_GET_SUPPLEMENTARY(lead, tail));  // NOLINTNEXTLINE(hicpp-signed-bitwise)
    } else {
        pair |= ((codePoint >> (PAIR_ELEMENT_WIDTH - DATA_WIDTH)) + U16_LEAD) << PAIR_ELEMENT_WIDTH;
        pair |= ((codePoint & MASK_10BIT) + U16_TAIL) & MASK_16BIT;
    }

    return {pair, UtfLength::FOUR};
}

// drop the tail bytes if the remain length can't fill the length it represents.
static inline size_t FixUtf8Len(const uint8_t* utf8, size_t utf8Len)
{
    size_t trimSize = 0;
    if (utf8Len >= 1 && utf8[utf8Len - 1] >= 0xC0) {
        // The last one char claim there are more than 1 byte next to it, it's invalid, so drop the last one.
        trimSize = 1;
    }
    if (utf8Len >= CONST_2 && utf8[utf8Len - CONST_2] >= 0xE0) {
        // The second to last char claim there are more than 2 bytes next to it, it's invalid, so drop the last two.
        trimSize = CONST_2;
    }
    if (utf8Len >= CONST_3 && utf8[utf8Len - CONST_3] >= 0xF0) {
        // The third to last char claim there are more than 3 bytes next to it, it's invalid, so drop the last three.
        trimSize = CONST_3;
    }
    return utf8Len - trimSize;
}

size_t Utf8ToUtf16Size(const uint8_t *utf8, size_t utf8Len)
{
    size_t safeUtf8Len = FixUtf8Len(utf8, utf8Len);
    size_t in_pos = 0;
    size_t res = 0;
    while (in_pos < safeUtf8Len) {
        uint8_t src = utf8[in_pos];
        switch (src & 0xF0) {
            case 0xF0: {
                const uint8_t c2 = utf8[++in_pos];
                const uint8_t c3 = utf8[++in_pos];
                const uint8_t c4 = utf8[++in_pos];
                uint32_t codePoint = ((src & LOW_3BITS) << OFFSET_18POS) | ((c2 & LOW_6BITS) << OFFSET_12POS) |
                    ((c3 & LOW_6BITS) << OFFSET_6POS) | (c4 & LOW_6BITS);
                if (codePoint >= SURROGATE_RAIR_START) {
                    res += CONST_2;
                } else {
                    res++;
                }
                in_pos++;
                break;
            }
            case 0xE0: {
                in_pos += CONST_3;
                res++;
                break;
            }
            case 0xD0:
            case 0xC0: {
                in_pos += CONST_2;
                res++;
                break;
            }
            default:
                do {
                    in_pos++;
                    res++;
                } while (in_pos < safeUtf8Len && utf8[in_pos] < 0x80);
                break;
        }
    }
    // The remain chars should be treated as single byte char.
    res += utf8Len - in_pos;
    return res;
}

size_t ConvertRegionUtf8ToUtf16(const uint8_t *utf8In, uint16_t *utf16Out, size_t utf8Len, size_t utf16Len)
{
    size_t safeUtf8Len = FixUtf8Len(utf8In, utf8Len);
    size_t in_pos = 0;
    size_t out_pos = 0;
    while (in_pos < safeUtf8Len && out_pos < utf16Len) {
        uint8_t src = utf8In[in_pos];
        switch (src & 0xF0) {
            case 0xF0: {
                const uint8_t c2 = utf8In[++in_pos];
                const uint8_t c3 = utf8In[++in_pos];
                const uint8_t c4 = utf8In[++in_pos];
                uint32_t codePoint = ((src & LOW_3BITS) << OFFSET_18POS) | ((c2 & LOW_6BITS) << OFFSET_12POS) |
                    ((c3 & LOW_6BITS) << OFFSET_6POS) | (c4 & LOW_6BITS);
                if (codePoint >= SURROGATE_RAIR_START) {
                    DCHECK_CC(utf16Len >= 1);
                    if (out_pos >= utf16Len - 1) {
                        return out_pos;
                    }
                    codePoint -= SURROGATE_RAIR_START;
                    utf16Out[out_pos++] = static_cast<uint16_t>((codePoint >> OFFSET_10POS) | H_SURROGATE_START);
                    utf16Out[out_pos++] = static_cast<uint16_t>((codePoint & 0x3FF) | L_SURROGATE_START);
                } else {
                    utf16Out[out_pos++] = static_cast<uint16_t>(codePoint);
                }
                in_pos++;
                break;
            }
            case 0xE0: {
                const uint8_t c2 = utf8In[++in_pos];
                const uint8_t c3 = utf8In[++in_pos];
                utf16Out[out_pos++] = static_cast<uint16_t>(((src & LOW_4BITS) << OFFSET_12POS) |
                    ((c2 & LOW_6BITS) << OFFSET_6POS) | (c3 & LOW_6BITS));
                in_pos++;
                break;
            }
            case 0xD0:
            case 0xC0: {
                const uint8_t c2 = utf8In[++in_pos];
                utf16Out[out_pos++] = static_cast<uint16_t>(((src & LOW_5BITS) << OFFSET_6POS) | (c2 & LOW_6BITS));
                in_pos++;
                break;
            }
            default:
                do {
                    utf16Out[out_pos++] = static_cast<uint16_t>(utf8In[in_pos++]);
                } while (in_pos < safeUtf8Len && out_pos < utf16Len && utf8In[in_pos] < 0x80);
                break;
        }
    }
    // The remain chars should be treated as single byte char.
    while (in_pos < utf8Len && out_pos < utf16Len) {
        utf16Out[out_pos++] = static_cast<uint16_t>(utf8In[in_pos++]);
    }
    return out_pos;
}

size_t ConvertRegionUtf16ToLatin1(const uint16_t *utf16In, uint8_t *latin1Out, size_t utf16Len, size_t latin1Len)
{
    if (utf16In == nullptr || latin1Out == nullptr || latin1Len == 0) {
        return 0;
    }
    size_t latin1Pos = 0;
    size_t end = utf16Len;
    for (size_t i = 0; i < end; ++i) {
        if (latin1Pos == latin1Len) {
            break;
        }
        uint32_t codepoint = DecodeUTF16(utf16In, end, &i);
        uint8_t latin1Code = static_cast<uint8_t>(codepoint & latin1Limit);
        latin1Out[latin1Pos++] = latin1Code;
    }
    return latin1Pos;
}

std::pair<int32_t, size_t> ConvertUtf8ToUnicodeChar(const uint8_t *utf8, size_t maxLen)
{
    if (maxLen == 0) {
        return {INVALID_UTF8, 0};
    }
    common::Span<const uint8_t> sp(utf8, maxLen);
    // one byte
    uint8_t d0 = sp[0];
    if ((d0 & BIT_MASK_1) == 0) {
        return {d0, UtfLength::ONE};
    }
    if (maxLen < UtfLength::TWO) {
        return {INVALID_UTF8, 0};
    }
    // two bytes
    uint8_t d1 = sp[UtfLength::ONE];
    if ((d0 & BIT_MASK_3) == BIT_MASK_2) {
        if ((d1 & BIT_MASK_2) == BIT_MASK_1) {
            return {((d0 & MASK_5BIT) << DATA_WIDTH) | (d1 & MASK_6BIT), UtfLength::TWO};
        } else {
            return {INVALID_UTF8, 0};
        }
    }
    if (maxLen < UtfLength::THREE) {
        return {INVALID_UTF8, 0};
    }
    // three bytes
    uint8_t d2 = sp[UtfLength::TWO];
    if ((d0 & BIT_MASK_4) == BIT_MASK_3) {
        if (((d1 & BIT_MASK_2) == BIT_MASK_1) && ((d2 & BIT_MASK_2) == BIT_MASK_1)) {
            return {((d0 & MASK_4BIT) << UtfOffset::TWELVE) |
                ((d1 & MASK_6BIT) << DATA_WIDTH) | (d2 & MASK_6BIT), UtfLength::THREE};
        } else {
            return {INVALID_UTF8, 0};
        }
    }
    if (maxLen < UtfLength::FOUR) {
        return {INVALID_UTF8, 0};
    }
    // four bytes
    uint8_t d3 = sp[UtfLength::THREE];
    if ((d0 & BIT_MASK_5) == BIT_MASK_4) {
        if (((d1 & BIT_MASK_2) == BIT_MASK_1) &&
            ((d2 & BIT_MASK_2) == BIT_MASK_1) && ((d3 & BIT_MASK_2) == BIT_MASK_1)) {
            return {((d0 & MASK_4BIT) << UtfOffset::EIGHTEEN) | ((d1 & MASK_6BIT) << UtfOffset::TWELVE) |
                ((d2 & MASK_6BIT) << DATA_WIDTH) | (d3 & MASK_6BIT), UtfLength::FOUR};
        } else {
            return {INVALID_UTF8, 0};
        }
    }
    return {INVALID_UTF8, 0};
}
}  // namespace common::utf_helper
