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

#ifndef LIBPANDABASE_UTILS_LEB128_H
#define LIBPANDABASE_UTILS_LEB128_H

#include "bit_helpers.h"
#include "bit_utils.h"

#include <limits>
#include <tuple>

namespace panda::leb128 {

constexpr size_t PAYLOAD_WIDTH = 7;
constexpr size_t PAYLOAD_MASK = 0x7f;
constexpr size_t EXTENSION_BIT = 0x80;
constexpr size_t SIGN_BIT = 0x40;

template <class T>
inline std::tuple<T, size_t, bool> DecodeUnsigned(const uint8_t *data)
{
    static_assert(std::is_unsigned_v<T>, "T must be unsigned");

    T result = 0;

    constexpr size_t BITWIDTH = std::numeric_limits<T>::digits;
    constexpr size_t N = (BITWIDTH + PAYLOAD_WIDTH - 1) / PAYLOAD_WIDTH;

    for (size_t i = 0; i < N; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint8_t byte = data[i] & PAYLOAD_MASK;
        size_t shift = i * PAYLOAD_WIDTH;
        result |= static_cast<T>(byte) << shift;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if ((data[i] & EXTENSION_BIT) == 0) {
            bool is_full = MinimumBitsToStore(byte) <= (BITWIDTH - shift);
            return {result, i + 1, is_full};
        }
    }

    return {result, N, false};
}

#if defined(PANDA_TARGET_ARM64) && !(PANDA_TARGET_MACOS)

template <>
inline std::tuple<uint32_t, size_t, bool> DecodeUnsigned<uint32_t>(const uint8_t *data)
{
    if (LIKELY(!(data[0] & 0x80))) {
        return {data[0], 1, true};
    }

    uint64_t rev = 0;
    constexpr size_t LEB128_PAGE_SIZE = 4096;
    constexpr size_t LEB128_PREFETCH_SIZE = 8;
    constexpr size_t LEB128_BITS_PER_BYTE = 8;
    bool read_pass_end_of_page = ((uintptr_t)data & (LEB128_PAGE_SIZE-1)) > LEB128_PAGE_SIZE - LEB128_PREFETCH_SIZE;
    if (UNLIKELY(read_pass_end_of_page)) {
        int i = -1;
        do {
            i++;
            rev |= (uint64_t)(data[i]) << (LEB128_BITS_PER_BYTE * i);
        } while (data[i] & 0x80);
    } else {
        rev = ((uint64_t *)data)[0];
    }
    uint64_t lenbit = (~rev) & (0x8080808080);
    uint64_t len = (__builtin_ctz(lenbit) >> 3) + 1;
    uint64_t valid_rev = rev & (lenbit ^ (lenbit - 1));

    uint32_t result = (valid_rev & 0x7f) | ((valid_rev & 0x7f00) >> 1) | ((valid_rev & 0x7f0000) >> 2) |
        ((valid_rev & 0x7f000000) >> 3) | ((valid_rev & 0x0f00000000) >> 4);
    return {result, len, !(valid_rev & 0xf000000000)};
}

#else

template <>
inline std::tuple<uint32_t, size_t, bool> DecodeUnsigned<uint32_t>(const uint8_t *data)
{
    constexpr size_t LEB128_BYTE2_SHIFT = 7U;
    constexpr size_t LEB128_BYTE3_SHIFT = 14U;
    constexpr size_t LEB128_BYTE4_SHIFT = 21U;
    constexpr size_t LEB128_BYTE5_SHIFT = 28U;
    constexpr size_t LEB128_BYTE5_BIT_SIZE = 4;

    bool valid = true;
    const uint8_t *p = data;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint32_t result = *(p++);
    if (UNLIKELY(result > PAYLOAD_MASK)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint32_t byte = *(p++);
        result = (result & PAYLOAD_MASK) | ((byte & PAYLOAD_MASK) << LEB128_BYTE2_SHIFT);
        if (byte > PAYLOAD_MASK) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            byte = *(p++);
            result |= (byte & PAYLOAD_MASK) << LEB128_BYTE3_SHIFT;
            if (byte > PAYLOAD_MASK) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                byte = *(p++);
                result |= (byte & PAYLOAD_MASK) << LEB128_BYTE4_SHIFT;
                if (byte > PAYLOAD_MASK) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    byte = *(p++);
                    valid = byte < (1U << LEB128_BYTE5_BIT_SIZE);
                    result |= byte << LEB128_BYTE5_SHIFT;
                }
            }
        }
    }
    return {result, p - data, valid};
}

#endif

template <class T>
inline std::tuple<T, size_t, bool> DecodeSigned(const uint8_t *data)
{
    static_assert(std::is_signed_v<T>, "T must be signed");

    T result = 0;

    using unsigned_type = std::make_unsigned_t<T>;

    constexpr size_t BITWIDTH = std::numeric_limits<unsigned_type>::digits;
    constexpr size_t N = (BITWIDTH + PAYLOAD_WIDTH - 1) / PAYLOAD_WIDTH;

    for (size_t i = 0; i < N; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint8_t byte = data[i];
        size_t shift = i * PAYLOAD_WIDTH;
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        result |= static_cast<unsigned_type>(byte & PAYLOAD_MASK) << shift;

        if ((byte & EXTENSION_BIT) == 0) {
            shift = BITWIDTH - shift;
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            int8_t signed_extended = static_cast<int8_t>(byte << 1) >> 1;
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            uint8_t masked = (signed_extended ^ (signed_extended >> PAYLOAD_WIDTH)) | 1;
            bool is_full = MinimumBitsToStore(masked) <= shift;
            if (shift > PAYLOAD_WIDTH) {
                shift -= PAYLOAD_WIDTH;
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                result = static_cast<T>(result << shift) >> shift;
            }
            return {result, i + 1, is_full};
        }
    }

    return {result, N, false};
}

template <class T>
inline size_t EncodeUnsigned(T data, uint8_t *out)
{
    size_t i = 0;
    uint8_t byte = data & PAYLOAD_MASK;
    data >>= PAYLOAD_WIDTH;

    while (data != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        out[i++] = byte | EXTENSION_BIT;
        byte = data & PAYLOAD_MASK;
        data >>= PAYLOAD_WIDTH;
    }

    out[i++] = byte;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return i;
}

template <class T>
inline size_t EncodeSigned(T data, uint8_t *out)
{
    static_assert(std::is_signed_v<T>, "T must be signed");

    size_t i = 0;
    bool more = true;

    while (more) {
        auto byte = static_cast<uint8_t>(static_cast<size_t>(data) & PAYLOAD_MASK);
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        data >>= PAYLOAD_WIDTH;
        more = !((data == 0 && (byte & SIGN_BIT) == 0) || (data == -1 && (byte & SIGN_BIT) != 0));
        if (more) {
            byte |= EXTENSION_BIT;
        }
        out[i++] = byte;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    return i;
}

template <class T>
inline size_t UnsignedEncodingSize(T data)
{
    return (MinimumBitsToStore(data | 1U) + PAYLOAD_WIDTH - 1) / PAYLOAD_WIDTH;
}

template <class T>
inline size_t SignedEncodingSize(T data)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    data = data ^ (data >> (std::numeric_limits<T>::digits - 1));
    using unsigned_type = std::make_unsigned_t<T>;
    auto udata = static_cast<unsigned_type>(data);
    return MinimumBitsToStore(static_cast<unsigned_type>(udata | 1U)) / PAYLOAD_WIDTH + 1;
}

}  // namespace panda::leb128

#endif  // LIBPANDABASE_UTILS_LEB128_H
