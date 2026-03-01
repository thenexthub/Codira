/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_TOOLING_INSPECTOR_EVALUATION_BASE64_H
#define PANDA_TOOLING_INSPECTOR_EVALUATION_BASE64_H

#include <array>
#include <optional>
#include <string>

#include "libarkbase/macros.h"
#include "libarkbase/utils/span.h"

namespace ark::tooling::inspector {
class Base64Decoder final {
public:
    Base64Decoder() = delete;

    static std::optional<size_t> DecodedSize(Span<const uint8_t> input)
    {
        if (input.empty() || input.size() % ENCODED_GROUP_BYTES != 0) {
            return {};
        }

        auto sz = input.size() / ENCODED_GROUP_BYTES * DECODED_GROUP_BYTES;
        auto last = input.end() - 1;
        for (size_t i = 0; i < MAX_PADDINGS && *last == PADDING_CHAR; ++i, --last) {
            --sz;
        }
        return sz;
    }

    static bool Decode(const std::string &encoded, std::string &decoded)
    {
        ASSERT(!encoded.empty());

        auto sz = encoded.size();
        Span encodingInput(reinterpret_cast<const uint8_t *>(encoded.c_str()), sz);
        auto bytecodeSize = Base64Decoder::DecodedSize(encodingInput);
        if (!bytecodeSize) {
            return false;
        }
        decoded.resize(*bytecodeSize);
        return Base64Decoder::Decode(reinterpret_cast<uint8_t *>(decoded.data()), encodingInput);
    }

    static bool Decode(uint8_t *output, Span<const uint8_t> input)
    {
        ASSERT(output);

        if (input.empty() || input.size() % ENCODED_GROUP_BYTES != 0) {
            return false;
        }

        std::array<uint8_t, ENCODED_GROUP_BYTES> decodingBuffer = {0};
        size_t baseIdx = 0;
        auto srcIter = input.begin();
        for (auto endIter = input.end(); srcIter != endIter && *srcIter != PADDING_CHAR; ++srcIter) {
            auto decoded = DecodeChar(*srcIter);
            if (decoded == INVALID_VALUE) {
                return false;
            }
            decodingBuffer[baseIdx++] = decoded;

            if (baseIdx == ENCODED_GROUP_BYTES) {
                DecodeSextetsGroup(decodingBuffer, Span(output, DECODED_GROUP_BYTES));
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                output += DECODED_GROUP_BYTES;
                baseIdx = 0;
            }
        }

        if (baseIdx != 0) {
            // Decode the remainder part.
            std::array<uint8_t, DECODED_GROUP_BYTES> decodedRemainder = {0};
            DecodeSextetsGroup(decodingBuffer, Span(decodedRemainder.data(), DECODED_GROUP_BYTES));
            for (size_t idx = 0, end = baseIdx - 1; idx < end; ++idx) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                *output++ = decodedRemainder[idx];
            }
        }

        // Only padding symbols could remain.
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto remainder = input.end() - srcIter;
        return (remainder == 0 || (remainder == 1 && *srcIter == PADDING_CHAR) ||
                (remainder == MAX_PADDINGS && *srcIter == PADDING_CHAR && *(srcIter + 1) == PADDING_CHAR));
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

private:
    static constexpr size_t DECODED_GROUP_BYTES = 3;
    static constexpr size_t ENCODED_GROUP_BYTES = 4;

private:
    static uint8_t DecodeChar(uint8_t encoded)
    {
        if (encoded >= DECODE_TABLE.size()) {
            return INVALID_VALUE;
        }
        return DECODE_TABLE[encoded];
    }

    /// @brief Converts 4 sextets into 3 output bytes.
    static void DecodeSextetsGroup(const std::array<uint8_t, ENCODED_GROUP_BYTES> &decodingBuffer, Span<uint8_t> output)
    {
        // 0b00110000
        static constexpr uint8_t FOURTH_TO_FIFTH_BITS = 0x30U;
        // 0b00111100
        static constexpr uint8_t SECOND_TO_FIFTH_BITS = 0x3CU;
        static constexpr uint8_t TWO_BITS_SHIFT = 2U;
        static constexpr uint8_t FOUR_BITS_SHIFT = 4U;
        static constexpr uint8_t SIX_BITS_SHIFT = 6U;

        ASSERT(output.size() == DECODED_GROUP_BYTES);
        // NOLINTBEGIN(hicpp-signed-bitwise)
        output[0UL] =
            (decodingBuffer[0UL] << TWO_BITS_SHIFT) | ((decodingBuffer[1UL] & FOURTH_TO_FIFTH_BITS) >> FOUR_BITS_SHIFT);
        output[1UL] =
            (decodingBuffer[1UL] << FOUR_BITS_SHIFT) | ((decodingBuffer[2UL] & SECOND_TO_FIFTH_BITS) >> TWO_BITS_SHIFT);
        output[2UL] = (decodingBuffer[2UL] << SIX_BITS_SHIFT) | decodingBuffer[3UL];
        // NOLINTEND(hicpp-signed-bitwise)
    }

private:
    static constexpr size_t MAX_PADDINGS = 2U;
    static constexpr uint8_t PADDING_CHAR = '=';
    static constexpr uint8_t INVALID_VALUE = 255U;
    static constexpr std::array<uint8_t, 123> DECODE_TABLE = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 62,  255, 255, 255, 63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255, 255, 255, 255,
        255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,
        19,  20,  21,  22,  23,  24,  25,  255, 255, 255, 255, 255, 255, 26,  27,  28,  29,  30,  31,  32,  33,
        34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51};
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_EVALUATION_BASE64_H
