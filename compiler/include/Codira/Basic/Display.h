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

/**
 * @file
 *
 * This file converts utf-8 string to the displayed column width of unicode as follows:
 *
 *    - The null character (U+0000) has a column width of 0.
 *
 *    - Other C0/C1 control characters and DEL will lead to a return value of -1.
 *
 *    - Non-spacing and enclosing combining characters (general category code Mn or Me in the Unicode database) have a
 *      column width of 0.
 *
 *    - SOFT HYPHEN (U+00AD) has a column width of 1.
 *
 *    - Other format characters (general category code Cf in the Unicode database) and ZERO WIDTH SPACE (U+200B)
 *      have a column width of 0.
 *
 *    - Hangul Jamo medial vowels and final consonants (U+1160-U+11FF) have a column width of 0.
 *
 *    - Spacing characters in the East Asian Wide (W) or East Asian Full-width (F) category as defined in Unicode
 *      Technical Report #11 have a column width of 2.
 *
 *    - All remaining characters (including all printable ISO 8859-1 and WGL4 characters, Unicode control characters,
 *      etc.) have a column width of 1.
 */

#ifndef CODIRA_BASIC_DISPLAY_H
#define CODIRA_BASIC_DISPLAY_H

#include <vector>
#include <bitset>
#include <unordered_map>
#include <cstdint>

namespace Codira {
static const size_t NORMAL_CODEPOINT_LEN = 4;
static const size_t HORIZONTAL_TAB_LEN = 4;
static const uint8_t ASCII_BASE = 127;

/// Characters that need escaped when print to console.
static std::unordered_map<uint8_t, std::string> escapePrintMap = {
    {'\b', "\\b"}, {'\t', "\\t"}, {'\n', "\\n"},
    {'\v', "\\v"}, {'\f', "\\f"}, {'\r', "\\r"}
};

/// Convert arithmetic value to hex string with length. All letters returned are in uppercase.
template<typename T> std::string ToHexString(T w, size_t len = sizeof(T) >> 1)
{
    static_assert(std::is_arithmetic<std::decay_t<T>>::value, "only support converting arithmetic value to hex");
    static const std::string digits("0123456789ABCDEF");
    std::string ret(len, '0');
    for (size_t i = 0, j = (len - 1) * NORMAL_CODEPOINT_LEN; i < len; ++i, j -= NORMAL_CODEPOINT_LEN) {
        ret[i] = digits[(w >> j) & 0x0f];
    }
    return ret;
}

inline std::string ToBinaryString(uint8_t num)
{
    const static int bStringLen = 8;
    return "0b" + std::bitset<bStringLen>(num).to_string();
}

std::basic_string<char32_t> UTF8ToChar32(const std::string& str);
std::string Char32ToUTF8(const char32_t& str);
std::string Char32ToUTF8(const std::basic_string<char32_t>& str);
/// Returns a string of spaces, with length at least enough to fill content[0..column-1] using unicode DisplayWidth.
std::string GetSpaceBeforeTarget(const std::string& content, int column);
/// Convert the input Unicode scalar point \ref ch into a string to be printed in diagnostic message.
std::string ConvertChar(const int32_t& ch);
std::string ConvertUnicode(const int32_t& str);
///@{
/// Get unicode display width, that is how many spaces it takes to render them in console.
/// Used by fmt.
size_t DisplayWidth(const std::basic_string<char32_t>& pwcs);
size_t DisplayWidth(const std::string& str) noexcept;
///@}
}

#endif // CODIRA_BASIC_DISPLAY_H
