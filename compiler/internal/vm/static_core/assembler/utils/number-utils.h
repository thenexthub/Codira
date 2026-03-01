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

#ifndef PANDA_ASSEMBLER_UTILS_NUMBERS_UTILS_H
#define PANDA_ASSEMBLER_UTILS_NUMBERS_UTILS_H

#include "libarkbase/utils/bit_utils.h"

namespace ark::pandasm {

constexpr size_t HEX_BASE = 16;

constexpr size_t DEC_BASE = 10;

constexpr size_t OCT_BASE = 8;

constexpr size_t BIN_BASE = 2;

constexpr size_t MAX_DWORD = 65536;

inline bool ValidateXToken(std::string_view token, size_t shift)
{
    token.remove_prefix(shift);

    for (auto i : token) {
        if (!((i >= '0' && i <= '9') || (i >= 'A' && i <= 'F') || (i >= 'a' && i <= 'f'))) {
            return false;
        }
    }

    return true;
}

inline bool ValidateBToken(std::string_view token, size_t shift)
{
    token.remove_prefix(shift);
    if (token.empty()) {
        return false;
    }
    for (auto i : token) {
        if (!(i == '0' || i == '1')) {
            return false;
        }
    }

    return true;
}

inline bool ValidateZeroToTenToken(std::string_view token)
{
    token.remove_prefix(1);

    for (auto i : token) {
        if (!(i >= '0' && i <= '7')) {
            return false;
        }
    }

    return true;
}

bool ValidateInteger(std::string_view p);

int64_t IntegerNumber(std::string_view p);

bool ValidateFloat(std::string_view p);

inline double FloatNumber(std::string_view p, bool is64bit)
{
    constexpr size_t GENERAL_SHIFT = 2;
    // expects a valid number
    if (p.size() > GENERAL_SHIFT && p.substr(0, GENERAL_SHIFT) == "0x") {  // hex literal
        char *end = nullptr;
        if (is64bit) {
            return bit_cast<double>(strtoull(p.data(), &end, 0));
        }
        // CC-OFFNXT(G.FUU.01) false positive
        return bit_cast<float>(static_cast<uint32_t>(strtoull(p.data(), &end, 0)));
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

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_UTILS_NUMBERS_UTILS_H
