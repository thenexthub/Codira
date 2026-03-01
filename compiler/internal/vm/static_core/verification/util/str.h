/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_VERIFIER_UTIL_STR_HPP_
#define PANDA_VERIFIER_UTIL_STR_HPP_

#include "lazy.h"
#include "include/mem/panda_string.h"

#include <type_traits>

namespace ark::verifier {

template <typename StrT, typename Gen>
StrT Join(Gen gen, StrT delim = {", "})
{
    return FoldLeft(gen, StrT {""}, [needDelim = false, &delim](StrT accum, StrT str) mutable {
        if (needDelim) {
            accum += delim;
        }
        needDelim = true;
        return accum + str;
    });
}

template <typename Int, typename = std::enable_if_t<std::is_integral_v<Int>>>
// NOLINTNEXTLINE(readability-magic-numbers)
PandaString NumToStr(Int val, Int base = 10, size_t width = 0)
{
    PandaString result {};
    bool neg = false;
    if (val < 0) {
        neg = true;
        val = -val;
    }
    do {
        char c = static_cast<char>(val % base);
        constexpr char LETTER_DIGIT_START = static_cast<char>(10);
        if (c >= LETTER_DIGIT_START) {
            c += 'a' - LETTER_DIGIT_START;
        } else {
            c += '0';
        }
        result.insert(0, 1, c);
        val = val / base;
    } while (val);
    if (width > 0) {
        if (neg) {
            width -= 1;
        }
        if (result.length() < width) {
            result.insert(0, width - result.length(), '0');
        }
    }
    if (neg) {
        result.insert(0, "-");
    }
    return result;
}

template <typename Offset>
PandaString OffsetToHexStr(Offset offset)
{
    constexpr Offset BASE = 16U;
    // leave space for - if needed
    constexpr size_t WIDTH = sizeof(Offset) + (std::is_signed_v<Offset> ? 1 : 0);
    return NumToStr(offset, BASE, WIDTH);
}
}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_UTIL_STR_HPP_
