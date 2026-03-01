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

#ifndef COMMON_INTERFACES_OBJECTS_UTILS_UTF_H
#define COMMON_INTERFACES_OBJECTS_UTILS_UTF_H

#include <cstddef>
#include <cstdint>

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
namespace common {
// NOLINTNEXTLINE(readability-identifier-naming, modernize-avoid-c-arrays)
static constexpr unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
class UtfUtils {
public:
    static constexpr uint8_t UTF8_1B_MAX = 0x7f;
    static constexpr uint16_t UTF8_2B_MAX = 0x7ff;
    static constexpr uint8_t UTF8_2B_FIRST = 0xc0;
    static constexpr uint8_t UTF8_2B_SECOND = 0x80;
    static constexpr uint16_t UTF8_3B_MAX = 0xffff;
    enum UtfLength : uint8_t { ONE = 1, TWO = 2, THREE = 3, FOUR = 4 };
    enum UtfOffset : uint8_t { SIX = 6, TEN = 10, TWELVE = 12, EIGHTEEN = 18 };
    static constexpr uint16_t SURROGATE_MASK = 0xF800;
    static constexpr uint16_t DECODE_LEAD_LOW = 0xD800;
    static constexpr uint16_t DECODE_LEAD_HIGH = 0xDBFF;
    static constexpr uint32_t UTF8_OFFSET = 6;
    static constexpr uint32_t UTF16_OFFSET = 10;
    static constexpr uint16_t DECODE_TRAIL_LOW = 0xDC00;
    static constexpr uint16_t DECODE_TRAIL_HIGH = 0xDFFF;
    static constexpr uint32_t DECODE_SECOND_FACTOR = 0x10000;
    static constexpr uint8_t BYTE_MASK = 0xbf;
    static constexpr uint8_t BYTE_MARK = 0x80;
    static constexpr size_t HI_SURROGATE_MIN = 0xD800;
    static constexpr size_t HI_SURROGATE_MAX = 0xDBFF;
    static constexpr size_t LO_SURROGATE_MIN = 0xDC00;
    static constexpr size_t LO_SURROGATE_MAX = 0xDFFF;
    static constexpr size_t LOW_3BITS = 0x7;
    static constexpr size_t LOW_4BITS = 0xF;
    static constexpr size_t LOW_5BITS = 0x1F;
    static constexpr size_t LOW_6BITS = 0x3F;
    static constexpr size_t OFFSET_18POS = 18;
    static constexpr size_t OFFSET_12POS = 12;
    static constexpr size_t OFFSET_10POS = 10;
    static constexpr size_t OFFSET_6POS = 6;
    static constexpr size_t SURROGATE_RAIR_START = 0x10000;
    static constexpr size_t CONST_2 = 2;
    static constexpr size_t CONST_3 = 3;
    static constexpr size_t CONST_4 = 4;
    static constexpr size_t H_SURROGATE_START = 0xD800;
    static constexpr size_t L_SURROGATE_START = 0xDC00;
    static constexpr uint16_t UTF16_REPLACEMENT_CHARACTER = 0xFFFD;
    static constexpr uint8_t LATIN1_LIMIT = 0xFF;
    static size_t DebuggerConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len,
                                                   size_t utf8Len, size_t start, bool modify = true,
                                                   bool isWriteBuffer = false);

    static size_t FixUtf8Len(const uint8_t *utf8, size_t utf8Len)
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
            // The third to last char claim there are more than 3 bytes next to it, it's invalid,
            // so drop the last three.
            trimSize = CONST_3;
        }
        return utf8Len - trimSize;
    }

    static size_t Utf8ToUtf16Size(const uint8_t *utf8, size_t utf8Len)
    {
        size_t safeUtf8Len = FixUtf8Len(utf8, utf8Len);
        size_t inPos = 0;
        size_t res = 0;
        while (inPos < safeUtf8Len) {
            uint8_t src = utf8[inPos];
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            switch (src & 0xF0) {
                case 0xF0: {
                    const uint8_t c2 = utf8[++inPos];
                    const uint8_t c3 = utf8[++inPos];
                    const uint8_t c4 = utf8[++inPos];
                    uint32_t codePoint = ((src & LOW_3BITS) << OFFSET_18POS) | ((c2 & LOW_6BITS) << OFFSET_12POS) |
                                         ((c3 & LOW_6BITS) << OFFSET_6POS) | (c4 & LOW_6BITS);
                    if (codePoint >= SURROGATE_RAIR_START) {
                        res += CONST_2;
                    } else {
                        res++;
                    }
                    inPos++;
                    break;
                }
                case 0xE0: {
                    inPos += CONST_3;
                    res++;
                    break;
                }
                case 0xD0:
                case 0xC0: {
                    inPos += CONST_2;
                    res++;
                    break;
                }
                default:
                    do {
                        inPos++;
                        res++;
                    } while (inPos < safeUtf8Len && utf8[inPos] < 0x80);
                    break;
            }
        }
        // The remain chars should be treated as single byte char.
        res += utf8Len - inPos;
        return res;
    }

    static size_t Utf16ToUtf8Size(const uint16_t *utf16, uint32_t length, bool modify = true,
                                  bool isGetBufferSize = false, bool cesu8 = false)
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

    // CC-OFFNXT(huge_depth, huge_method, G.FUN.01-CPP) solid logic
    static size_t ConvertRegionUtf8ToUtf16(const uint8_t *utf8In, uint16_t *utf16Out, size_t utf8Len, size_t utf16Len)
    {
        size_t safeUtf8Len = FixUtf8Len(utf8In, utf8Len);
        size_t inPos = 0;
        size_t outPos = 0;
        while (inPos < safeUtf8Len && outPos < utf16Len) {
            uint8_t src = utf8In[inPos];
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            switch (src & 0xF0) {
                case 0xF0: {
                    const uint8_t c2 = utf8In[++inPos];
                    const uint8_t c3 = utf8In[++inPos];
                    const uint8_t c4 = utf8In[++inPos];
                    uint32_t codePoint = ((src & LOW_3BITS) << OFFSET_18POS) | ((c2 & LOW_6BITS) << OFFSET_12POS) |
                                         ((c3 & LOW_6BITS) << OFFSET_6POS) | (c4 & LOW_6BITS);
                    if (codePoint >= SURROGATE_RAIR_START) {
                        DCHECK_CC(utf16Len >= 1);
                        // CC-OFFNXT(G.FUN.01-CPP) solid logic
                        if (outPos >= utf16Len - 1) {
                            return outPos;
                        }
                        codePoint -= SURROGATE_RAIR_START;
                        utf16Out[outPos++] = static_cast<uint16_t>((codePoint >> OFFSET_10POS) | H_SURROGATE_START);
                        // NOLINTNEXTLINE(hicpp-signed-bitwise)
                        utf16Out[outPos++] = static_cast<uint16_t>((codePoint & 0x3FF) | L_SURROGATE_START);
                    } else {
                        utf16Out[outPos++] = static_cast<uint16_t>(codePoint);
                    }
                    inPos++;
                    break;
                }
                case 0xE0: {
                    const uint8_t c2 = utf8In[++inPos];
                    const uint8_t c3 = utf8In[++inPos];
                    utf16Out[outPos++] = static_cast<uint16_t>(((src & LOW_4BITS) << OFFSET_12POS) |
                                                               ((c2 & LOW_6BITS) << OFFSET_6POS) | (c3 & LOW_6BITS));
                    inPos++;
                    break;
                }
                case 0xD0:
                case 0xC0: {
                    const uint8_t c2 = utf8In[++inPos];
                    utf16Out[outPos++] = static_cast<uint16_t>(((src & LOW_5BITS) << OFFSET_6POS) | (c2 & LOW_6BITS));
                    inPos++;
                    break;
                }
                default:
                    do {
                        utf16Out[outPos++] = static_cast<uint16_t>(utf8In[inPos++]);
                    } while (inPos < safeUtf8Len && outPos < utf16Len && utf8In[inPos] < 0x80);
                    break;
            }
        }
        // The remain chars should be treated as single byte char.
        while (inPos < utf8Len && outPos < utf16Len) {
            utf16Out[outPos++] = static_cast<uint16_t>(utf8In[inPos++]);
        }
        return outPos;
    }

    static size_t ConvertRegionUtf16ToLatin1(const uint16_t *utf16In, uint8_t *latin1Out, size_t utf16Len,
                                             size_t latin1Len);

    static size_t ConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len, size_t utf8Len,
                                           size_t start, bool modify = true, bool isWriteBuffer = false,
                                           bool cesu8 = false)
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
    // Methods for decode utf16 to unicode
    static uint32_t DecodeUTF16(uint16_t const *utf16, size_t len, size_t *index, bool cesu8 = false)
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
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        return ((high - DECODE_LEAD_LOW) << UTF16_OFFSET) + (low - DECODE_TRAIL_LOW) + DECODE_SECOND_FACTOR;
    }
    static size_t UTF8Length(uint32_t codepoint)
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
    static size_t EncodeUTF8(uint32_t codepoint, uint8_t *utf8, size_t index, size_t size)
    {
        for (size_t j = size - 1; j > 0; j--) {
            uint8_t cont = ((codepoint | BYTE_MARK) & BYTE_MASK);
            utf8[index + j] = cont;
            codepoint >>= UTF8_OFFSET;
        }
        utf8[index] = codepoint | firstByteMark[size];
        return size;
    }
    static bool IsUTF16HighSurrogate(uint16_t ch)
    {
        return DECODE_LEAD_LOW <= ch && ch <= DECODE_LEAD_HIGH;
    }
    static bool IsUTF16LowSurrogate(uint16_t ch)
    {
        return DECODE_TRAIL_LOW <= ch && ch <= DECODE_TRAIL_HIGH;
    }

private:
    static uint32_t HandleAndDecodeInvalidUTF16(uint16_t const *utf16, size_t len, size_t *index);
};
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_UTILS_UTF_H
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
