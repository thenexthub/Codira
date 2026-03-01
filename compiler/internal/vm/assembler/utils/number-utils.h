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

#ifndef ASSEMBLER_UTILS_NUMBER_UTILS_H
#define ASSEMBLER_UTILS_NUMBER_UTILS_H

namespace panda::pandasm {

constexpr size_t HEX_BASE = 16;

constexpr size_t DEC_BASE = 10;

constexpr size_t OCT_BASE = 8;

constexpr size_t BIN_BASE = 2;

constexpr size_t MAX_DWORD = 65536;

inline bool IsHexNumber(const std::string_view &token)
{
    for (auto i : token) {
        if (!((i >= '0' && i <= '9') || (i >= 'A' && i <= 'F') || (i >= 'a' && i <= 'f'))) {
            return false;
        }
    }
    return true;
}

inline bool IsBinaryNumber(const std::string_view &token)
{
    for (auto i : token) {
        if (!(i == '0' || i == '1')) {
            return false;
        }
    }
    return true;
}

inline bool IsOctalNumber(const std::string_view &token)
{
    for (auto i : token) {
        if (!(i >= '0' && i <= '7')) {
            return false;
        }
    }
    return true;
}

inline bool ValidateInteger(const std::string_view &p)
{
    constexpr size_t GENERAL_SHIFT = 2;

    std::string_view token = p;

    if (token.back() == '-' || token.back() == '+' || token.back() == 'x' || token == ".") {
        return false;
    }

    if (token[0] == '-' || token[0] == '+') {
        token.remove_prefix(1);
    }

    if (token[0] == '0' && token.size() > 1 && token.find('.') == std::string::npos) {
        if (token[1] == 'x') {
            token.remove_prefix(GENERAL_SHIFT);
            return IsHexNumber(token);
        }

        if (token[1] == 'b') {
            token.remove_prefix(GENERAL_SHIFT);
            return (!token.empty() && IsBinaryNumber(token));
        }

        if (token[1] >= '0' && token[1] <= '9' && token.find('e') == std::string::npos) {
            token.remove_prefix(1);
            return IsOctalNumber(token);
        }
    }

    for (auto i : token) {
        if (!(i >= '0' && i <= '9')) {
            return false;
        }
    }

    return true;
}

inline int64_t IntegerNumber(std::string_view p)
{
    constexpr size_t GENERAL_SHIFT = 2;

    // expects a valid number
    if (p.size() == 1) {
        return p[0] - '0';
    }

    size_t minus_shift = 0;
    if (p[0] == '-') {
        minus_shift++;
    }

    if (p[minus_shift + 1] == 'b') {
        p.remove_prefix(GENERAL_SHIFT + minus_shift);
        return std::strtoull(p.data(), nullptr, BIN_BASE) * (minus_shift == 0 ? 1 : -1);
    }

    if (p[minus_shift + 1] == 'x') {
        return std::strtoull(p.data(), nullptr, HEX_BASE);
    }

    if (p[minus_shift] == '0') {
        return std::strtoull(p.data(), nullptr, OCT_BASE);
    }

    return std::strtoull(p.data(), nullptr, DEC_BASE);
}

inline bool ValidateFloat(const std::string_view &p)
{
    std::string_view token = p;

    if (ValidateInteger(token)) {
        return true;
    }

    if (token[0] == '-' || token[0] == '+') {
        token.remove_prefix(1);
    }

    bool dot = false;
    bool exp = false;
    bool nowexp = false;

    for (auto i : token) {
        if (nowexp && (i == '-' || i == '+')) {
            nowexp = false;
            continue;
        }

        if (nowexp) {
            nowexp = false;
        }

        if (i == '.' && !exp && !dot) {
            dot = true;
        } else if (!exp && i == 'e') {
            nowexp = true;
            exp = true;
        } else if (!(i >= '0' && i <= '9')) {
            return false;
        }
    }

    return !nowexp;
}

inline double FloatNumber(std::string_view p, bool is_64bit)
{
    constexpr size_t GENERAL_SHIFT = 2;
    // expects a valid number
    if (p.size() > GENERAL_SHIFT && p.substr(0, GENERAL_SHIFT) == "0x") {  // hex literal
        char *end = nullptr;
        if (is_64bit) {
            return bit_cast<double>(strtoull(p.data(), &end, 0));
        } else {
            return bit_cast<float>(static_cast<uint32_t>(strtoull(p.data(), &end, 0)));
        }
    }
    return std::strtold(std::string(p.data(), p.length()).c_str(), nullptr);
}

inline size_t ToNumber(std::string_view p)
{
    size_t sum = 0;

    for (char i : p) {
        if (isdigit(i) != 0) {
            sum = sum * DEC_BASE + static_cast<size_t>(i - '0');
        } else {
            return MAX_DWORD;
        }
    }

    return sum;
}

}  // namespace panda::pandasm

#endif  // ASSEMBLER_UTILS_NUMBER_UTILS_H
