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

#include "ani_textencoder.h"

namespace ani_helper {
constexpr uint8_t HIGER_4_BITS_MASK = 0xF0;
constexpr uint8_t FOUR_BYTES_STYLE = 0xF0;
constexpr uint8_t THREE_BYTES_STYLE = 0xE0;
constexpr uint8_t TWO_BYTES_STYLE1 = 0xD0;
constexpr uint8_t TWO_BYTES_STYLE2 = 0xC0;
constexpr uint8_t LOW_8_BITS_MASK = 0xFF;
constexpr uint8_t ONE_BYTE_MASK = 0x80;
constexpr uint32_t LOWER_10_BITS_MASK = 0x03FFU;
constexpr uint8_t LOWER_6_BITS_MASK = 0x3FU;
constexpr uint8_t LOWER_5_BITS_MASK = 0x1FU;
constexpr uint8_t LOWER_4_BITS_MASK = 0x0FU;
constexpr uint8_t LOWER_3_BITS_MASK = 0x07U;
constexpr uint32_t HIGH_AGENT_MASK = 0xD800U;
constexpr uint32_t LOW_AGENT_MASK = 0xDC00U;
constexpr uint32_t UTF8_VALID_BITS = 6;
constexpr uint32_t UTF16_SPECIAL_VALUE = 0x10000;
constexpr size_t ONE_MORE_BYTE_TO_CONSUME = 1;
constexpr size_t TWO_MORE_BYTES_TO_CONSUME = 2;
constexpr size_t THREE_MORE_BYTES_TO_CONSUME = 3;
constexpr size_t SHIFT_TEN_BITS = 10;

bool IsOneByte(uint8_t u8Char)
{
    return (u8Char & ONE_BYTE_MASK) == 0;
}

static bool Utf8ToUtf16LEToData(const unsigned char *data, std::u16string &u16Str, size_t resultLengthLimit,
                                size_t firstByteIndex, uint8_t c1)
{
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-signed-bitwise)
    uint8_t c2 = data[1 + firstByteIndex];  // The second byte
    uint8_t c3 = data[2 + firstByteIndex];  // The third byte
    uint8_t c4 = data[3 + firstByteIndex];  // The forth byte
    // Calculate the UNICODE code point value (3 bits lower for the first byte, 6 bits for the other)
    // CC-OFFNXT(G.FMT.02-CPP) project code style
    uint32_t codePoint = ((c1 & LOWER_3_BITS_MASK) << (3 * UTF8_VALID_BITS)) |
                         ((c2 & LOWER_6_BITS_MASK) << (2 * UTF8_VALID_BITS)) |
                         ((c3 & LOWER_6_BITS_MASK) << UTF8_VALID_BITS) | (c4 & LOWER_6_BITS_MASK);
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-signed-bitwise)
    // In UTF-16, U+10000 to U+10FFFF represent surrogate pairs with two 16-bit units
    if (codePoint >= UTF16_SPECIAL_VALUE) {
        if (u16Str.length() + 1 == resultLengthLimit) {
            return false;
        }
        codePoint -= UTF16_SPECIAL_VALUE;
        // 10 : a half of 20 , shift right 10 bits
        u16Str.push_back(static_cast<char16_t>((codePoint >> SHIFT_TEN_BITS) | HIGH_AGENT_MASK));
        u16Str.push_back(static_cast<char16_t>((codePoint & LOWER_10_BITS_MASK) | LOW_AGENT_MASK));
    } else {  // In UTF-16, U+0000 to U+D7FF and U+E000 to U+FFFF are Unicode code point values
        // assume it does not exist (if any, not encoded)
        u16Str.push_back(static_cast<char16_t>(codePoint));
    }
    return true;
}

bool Utf8ToUtf16LEByteCheck(const unsigned char *data, std::u16string &u16Str, size_t inputSizeBytes,
                            size_t resultLengthLimit, size_t *inputCharCount)
{
    for (size_t i = 0; i < inputSizeBytes;) {
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint8_t c1 = data[i];
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (IsOneByte(c1)) {
            i += 1;  // 1 : Proceeds 1 byte
            u16Str.push_back(static_cast<char16_t>(c1));
            continue;
        }
        switch (c1 & HIGER_4_BITS_MASK) {
            case FOUR_BYTES_STYLE: {
                if (i + THREE_MORE_BYTES_TO_CONSUME >= inputSizeBytes ||
                    !Utf8ToUtf16LEToData(data, u16Str, resultLengthLimit, i, c1)) {
                    return false;
                }
                i += THREE_MORE_BYTES_TO_CONSUME + 1;
                if (inputCharCount != nullptr) {
                    *inputCharCount += 2;  // 2 : A four-byte character is considered 2 characters
                }
                break;
            }
            case THREE_BYTES_STYLE: {
                if (i + TWO_MORE_BYTES_TO_CONSUME >= inputSizeBytes) {
                    return false;
                }
                // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                uint8_t c2 = data[i + 1];
                uint8_t c3 = data[i + 2];
                // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                i += TWO_MORE_BYTES_TO_CONSUME + 1;
                // CC-OFFNXT(G.FMT.02-CPP) project code style
                // NOLINTBEGIN(hicpp-signed-bitwise)
                uint32_t codePoint = ((c1 & LOWER_4_BITS_MASK) << (2 * UTF8_VALID_BITS)) |
                                     ((c2 & LOWER_6_BITS_MASK) << UTF8_VALID_BITS) | (c3 & LOWER_6_BITS_MASK);
                // NOLINTEND(hicpp-signed-bitwise)
                u16Str.push_back(static_cast<char16_t>(codePoint));
                break;
            }
            case TWO_BYTES_STYLE1:
            case TWO_BYTES_STYLE2: {
                if (i + ONE_MORE_BYTE_TO_CONSUME >= inputSizeBytes) {
                    return false;
                }
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                uint8_t c2 = data[i + 1];
                i += ONE_MORE_BYTE_TO_CONSUME + 1;
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                uint32_t codePoint = ((c1 & LOWER_5_BITS_MASK) << UTF8_VALID_BITS) | (c2 & LOWER_6_BITS_MASK);
                u16Str.push_back(static_cast<char16_t>(codePoint));
                break;
            }
            default: {
                return false;
            }
        }
    }
    return true;
}

std::u16string Utf8ToUtf16LE(std::string_view u8Str, bool *ok)
{
    std::u16string u16Str;
    u16Str.reserve(u8Str.size());
    const auto *data = reinterpret_cast<const unsigned char *>(u8Str.data());
    bool isOk = Utf8ToUtf16LEByteCheck(data, u16Str, u8Str.length(), 0, nullptr);
    if (ok != nullptr) {
        *ok = isOk;
    }
    return u16Str;
}

std::u16string Utf8ToUtf16LE(std::string_view u8Str, size_t resultLengthLimit, size_t *nInputCharsConsumed, bool *ok)
{
    std::u16string u16Str;
    u16Str.reserve(std::min(u8Str.length(), resultLengthLimit));
    const auto *data = reinterpret_cast<const unsigned char *>(u8Str.data());

    size_t inputCharCount = 0;
    size_t inputSizeBytes = u8Str.length();
    bool isOk = Utf8ToUtf16LEByteCheck(data, u16Str, inputSizeBytes, resultLengthLimit, &inputCharCount);
    if (nInputCharsConsumed != nullptr) {
        *nInputCharsConsumed = inputCharCount;
    }
    if (ok != nullptr) {
        *ok = isOk;
    }
    return u16Str;
}

std::u16string Utf16LEToBE(std::u16string_view wstr)
{
    std::u16string str16;
    str16.reserve(wstr.size());
    const char16_t *data = wstr.data();
    for (unsigned int i = 0; i < wstr.length(); i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        char16_t wc = data[i];
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        char16_t high = (wc >> 8) & LOW_8_BITS_MASK;  // 8 : offset value
        char16_t low = wc & LOW_8_BITS_MASK;
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        char16_t c16 = (low << 8) | high;  // 8 : offset value
        str16.push_back(c16);
    }
    return str16;
}

bool Utf8ByteCheck(std::string_view u8Str, size_t &inputCharCount, size_t resultSizeBytesLimit, size_t &i, bool &isOk)
{
    size_t len = u8Str.length();
    const auto *data = reinterpret_cast<const unsigned char *>(u8Str.data());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint8_t c1 = data[i];
    if (IsOneByte(c1)) {  // only 1 byte represents the UNICODE code point
        ++i;
        ++inputCharCount;
        return true;
    }
    bool continues = true;
    switch (c1 & HIGER_4_BITS_MASK) {
        case FOUR_BYTES_STYLE: {  // 4 byte characters, from 0x10000 to 0x10FFFF
            isOk = (i + THREE_MORE_BYTES_TO_CONSUME < len);
            continues = (i + THREE_MORE_BYTES_TO_CONSUME < resultSizeBytesLimit);
            if (isOk && continues) {
                inputCharCount += 2;  // 2 : 4-byte characters are considered 2 input characters.
                i += (THREE_MORE_BYTES_TO_CONSUME + 1);
            }
            break;
        }
        case THREE_BYTES_STYLE: {  // 3 byte characters, from 0x800 to 0xFFFF
            isOk = (i + TWO_MORE_BYTES_TO_CONSUME < len);
            continues = (i + TWO_MORE_BYTES_TO_CONSUME < resultSizeBytesLimit);
            if (isOk && continues) {
                ++inputCharCount;
                i += (TWO_MORE_BYTES_TO_CONSUME + 1);
            }
            break;
        }
        case TWO_BYTES_STYLE1:  // 2 byte characters, from 0x80 to 0x7FF
        case TWO_BYTES_STYLE2: {
            isOk = (i + ONE_MORE_BYTE_TO_CONSUME < len);
            continues = (i + ONE_MORE_BYTE_TO_CONSUME < resultSizeBytesLimit);
            if (isOk && continues) {
                ++inputCharCount;
                i += (ONE_MORE_BYTE_TO_CONSUME + 1);
            }
            break;
        }
        default: {
            isOk = false;
            break;
        }
    }
    return continues && isOk;
}

std::string_view Utf8GetPrefix(std::string_view u8Str, size_t resultSizeBytesLimit, size_t *nInputCharsConsumed,
                               bool *ok)
{
    size_t i = 0;
    size_t inputCharCount = 0;
    bool isOk = true;
    for (size_t len = u8Str.length(); i < len && i < resultSizeBytesLimit;) {
        if (!Utf8ByteCheck(u8Str, inputCharCount, resultSizeBytesLimit, i, isOk)) {
            break;
        }
    }
    if (nInputCharsConsumed != nullptr) {
        *nInputCharsConsumed = inputCharCount;
    }
    if (ok != nullptr) {
        *ok = isOk;
    }
    return u8Str.substr(0, i);
}
}  // namespace ani_helper
