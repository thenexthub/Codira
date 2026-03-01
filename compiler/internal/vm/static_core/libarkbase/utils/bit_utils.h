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

#ifndef LIBPANDABASE_UTILS_BIT_UTILS_H_
#define LIBPANDABASE_UTILS_BIT_UTILS_H_

#include "libarkbase/globals.h"
#include "libarkbase/macros.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include <limits>
#include <type_traits>
#include <bitset>

#include <securec.h>
#include "libarkbase/utils/utils.h"

#define PANDA_BIT_UTILS_CTZ __builtin_ctz      // NOLINT(cppcoreguidelines-macro-usage)
#define PANDA_BIT_UTILS_CTZLL __builtin_ctzll  // NOLINT(cppcoreguidelines-macro-usage)

#define PANDA_BIT_UTILS_CLZ __builtin_clz      // NOLINT(cppcoreguidelines-macro-usage)
#define PANDA_BIT_UTILS_CLZLL __builtin_clzll  // NOLINT(cppcoreguidelines-macro-usage)

#define PANDA_BIT_UTILS_FFS __builtin_ffs      // NOLINT(cppcoreguidelines-macro-usage)
#define PANDA_BIT_UTILS_FFSLL __builtin_ffsll  // NOLINT(cppcoreguidelines-macro-usage)

#define PANDA_BIT_UTILS_POPCOUNT __builtin_popcount      // NOLINT(cppcoreguidelines-macro-usage)
#define PANDA_BIT_UTILS_POPCOUNTLL __builtin_popcountll  // NOLINT(cppcoreguidelines-macro-usage)

namespace ark {

template <typename T>
constexpr int Clz(T x)
{
    constexpr size_t RADIX = 2;
    static_assert(std::is_integral<T>::value, "T must be integral");
    static_assert(std::is_unsigned<T>::value, "T must be unsigned");
    static_assert(std::numeric_limits<T>::radix == RADIX, "Unexpected radix!");
    static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) <= sizeof(uint32_t), "Unsupported sizeof(T)");
    ASSERT(x != 0U);

    if (sizeof(T) == sizeof(uint64_t)) {
        return PANDA_BIT_UTILS_CLZLL(x);
    }
    return PANDA_BIT_UTILS_CLZ(x) - (std::numeric_limits<uint32_t>::digits - std::numeric_limits<T>::digits);
}

template <typename T>
constexpr int Ctz(T x)
{
    constexpr size_t RADIX = 2;
    static_assert(std::is_integral<T>::value, "T must be integral");
    static_assert(std::is_unsigned<T>::value, "T must be unsigned");
    static_assert(std::numeric_limits<T>::radix == RADIX, "Unexpected radix!");
    static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) <= sizeof(uint32_t), "Unsupported sizeof(T)");
    ASSERT(x != 0U);

    if (sizeof(T) == sizeof(uint64_t)) {
        return PANDA_BIT_UTILS_CTZLL(x);
    }
    return PANDA_BIT_UTILS_CTZ(x);
}

template <typename T>
constexpr int Popcount(T x)
{
    if constexpr (std::is_enum_v<T>) {
        using UnderlyingType = std::underlying_type_t<T>;
        return Popcount(static_cast<UnderlyingType>(x));
    } else {
        constexpr size_t RADIX = 2;
        static_assert(std::is_integral<T>::value, "T must be integral");
        static_assert(std::is_unsigned<T>::value, "T must be unsigned");
        static_assert(std::numeric_limits<T>::radix == RADIX, "Unexpected radix!");
        static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) <= sizeof(uint32_t), "Unsupported sizeof(T)");

        if (sizeof(T) == sizeof(uint64_t)) {
            return PANDA_BIT_UTILS_POPCOUNTLL(x);
        }
        return PANDA_BIT_UTILS_POPCOUNT(x);
    }
}

// How many bits (minimally) does it take to store the constant 'value'? i.e. 1 for 1, 2 for 2 and 3, 3 for 4 and 5 etc.
template <typename T>
constexpr size_t MinimumBitsToStore(T value)
{
    static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) <= sizeof(uint32_t), "Unsupported sizeof(T)");
    if constexpr (std::is_enum_v<T>) {  // NOLINT
        using UnderlyingType = std::make_unsigned_t<std::underlying_type_t<T>>;
        auto uvalue = static_cast<UnderlyingType>(value);
        if (uvalue == 0) {
            uvalue = 1;
        }
        return std::numeric_limits<UnderlyingType>::digits - Clz(static_cast<UnderlyingType>(uvalue));
    } else {  // NOLINT
        constexpr size_t RADIX = 2;
        static_assert(std::is_integral_v<T>, "T must be integral");
        static_assert(std::is_unsigned_v<T>, "T must be unsigned");
        static_assert(std::numeric_limits<T>::radix == RADIX, "Unexpected radix!");
        if (value == 0) {
            return 0;
        }
        return std::numeric_limits<T>::digits - Clz(value);
    }
}

template <typename T>
constexpr T BitFloor(T value)
{
    constexpr size_t RADIX = 2;
    static_assert(std::is_integral<T>::value, "T must be integral");
    static_assert(std::is_unsigned<T>::value, "T must be unsigned");
    static_assert(std::numeric_limits<T>::radix == RADIX, "Unexpected radix!");
    static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) <= sizeof(uint32_t), "Unsupported sizeof(T)");

    if (value != 0) {
        return T {1} << (MinimumBitsToStore(value) - 1);
    }
    return 0;
}

template <typename T>
constexpr T BitCeil(T value)
{
    constexpr size_t RADIX = 2;
    static_assert(std::is_integral<T>::value, "T must be integral");
    static_assert(std::is_unsigned<T>::value, "T must be unsigned");
    static_assert(std::numeric_limits<T>::radix == RADIX, "Unexpected radix!");
    static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) <= sizeof(uint32_t), "Unsupported sizeof(T)");

    if (value <= 1U) {
        return T {1};
    }
    if constexpr (std::is_same_v<T, decltype(+value)>) {
        return T {1} << MinimumBitsToStore(value - 1);  // Without integral promotion
    } else {
        // With integral promotion: UB is triggered if the bit-ceil value can not be represented by T, analogous to the
        // behavior of shifting left operator (x << y) when y exceeds bit size of decltype(+x).
        constexpr int OFFSET_FOR_UB = std::numeric_limits<unsigned>::digits - std::numeric_limits<T>::digits;
        return T {1U << (MinimumBitsToStore(value - 1U) + OFFSET_FOR_UB) >> OFFSET_FOR_UB};
    }
}

template <typename T>
constexpr int Ffs(T x)
{
    constexpr size_t RADIX = 2;
    static_assert(std::is_integral<T>::value, "T must be integral");
    static_assert(std::is_unsigned<T>::value, "T must be unsigned");
    static_assert(std::numeric_limits<T>::radix == RADIX, "Unexpected radix!");
    static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) <= sizeof(uint32_t), "Unsupported sizeof(T)");

    if (sizeof(T) == sizeof(uint64_t)) {
        return PANDA_BIT_UTILS_FFSLL(x);
    }
    return PANDA_BIT_UTILS_FFS(x);
}

template <size_t N, typename T>
constexpr bool IsAligned(T value)
{
    static_assert(std::is_integral<T>::value, "T must be integral");
    return value % N == 0;
}

template <typename T>
constexpr bool IsAligned(T value, size_t n)
{
    static_assert(std::is_integral<T>::value, "T must be integral");
    return value % n == 0;
}

template <typename T>
constexpr T RoundUp(T x, size_t n)
{
    static_assert(std::is_integral<T>::value, "T must be integral");
    return (static_cast<size_t>(x) + n - 1U) & (-n);
}

constexpr size_t BitsToBytesRoundUp(size_t numBits)
{
    return RoundUp(numBits, BITS_PER_BYTE) / BITS_PER_BYTE;
}

template <typename T>
constexpr T RoundDown(T x, size_t n)
{
    static_assert(std::is_integral<T>::value, "T must be integral");
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return x & static_cast<size_t>(-n);
}

template <typename T>
constexpr T SwapBits(T value, T mask, uint32_t offset)
{
    return ((value >> offset) & mask) | ((value & mask) << offset);
}

template <typename T>
inline uint8_t GetByteFrom(T value, uint64_t index)
{
    static_assert(std::is_unsigned<T>::value, "T must be unsigned");
    constexpr uint8_t OFFSET_BYTE = 3;
    constexpr uint8_t MASK = 0xffU;
    uint64_t shift = index << OFFSET_BYTE;
    return static_cast<uint8_t>((value >> shift) & MASK);
}

inline uint16_t ReverseBytes(uint16_t value)
{
    constexpr uint32_t OFFSET_0 = 8;
    return static_cast<uint16_t>(value << OFFSET_0) | static_cast<uint16_t>(value >> OFFSET_0);
}

inline uint32_t ReverseBytes(uint32_t value)
{
    constexpr uint32_t BYTES_MASK = 0xff00ffU;
    constexpr uint32_t OFFSET_0 = 8;
    constexpr uint32_t OFFSET_1 = 16;
    value = SwapBits(value, BYTES_MASK, OFFSET_0);
    return (value >> OFFSET_1) | (value << OFFSET_1);
}

inline uint64_t ReverseBytes(uint64_t value)
{
    constexpr uint64_t BYTES_MASK = 0xff00ff00ff00ffLU;
    constexpr uint64_t WORDS_MASK = 0xffff0000ffffLU;
    constexpr uint32_t OFFSET_0 = 8;
    constexpr uint32_t OFFSET_1 = 16;
    constexpr uint32_t OFFSET_2 = 32;
    value = SwapBits(value, BYTES_MASK, OFFSET_0);
    value = SwapBits(value, WORDS_MASK, OFFSET_1);
    return (value >> OFFSET_2) | (value << OFFSET_2);
}

template <typename T>
constexpr T BSWAP(T x)
{
    if (sizeof(T) == sizeof(uint16_t)) {
        return ReverseBytes(static_cast<uint16_t>(x));
    }
    if (sizeof(T) == sizeof(uint32_t)) {
        return ReverseBytes(static_cast<uint32_t>(x));
    }
    return ReverseBytes(static_cast<uint64_t>(x));
}

inline uint32_t ReverseBits(uint32_t value)
{
    constexpr uint32_t BITS_MASK = 0x55555555U;
    constexpr uint32_t TWO_BITS_MASK = 0x33333333U;
    constexpr uint32_t HALF_BYTES_MASK = 0x0f0f0f0fU;
    constexpr uint32_t OFFSET_0 = 1;
    constexpr uint32_t OFFSET_1 = 2;
    constexpr uint32_t OFFSET_2 = 4;
    value = SwapBits(value, BITS_MASK, OFFSET_0);
    value = SwapBits(value, TWO_BITS_MASK, OFFSET_1);
    value = SwapBits(value, HALF_BYTES_MASK, OFFSET_2);
    return ReverseBytes(value);
}

inline uint64_t ReverseBits(uint64_t value)
{
    constexpr uint64_t BITS_MASK = 0x5555555555555555LU;
    constexpr uint64_t TWO_BITS_MASK = 0x3333333333333333LU;
    constexpr uint64_t HALF_BYTES_MASK = 0x0f0f0f0f0f0f0f0fLU;
    constexpr uint32_t OFFSET_0 = 1;
    constexpr uint32_t OFFSET_1 = 2;
    constexpr uint32_t OFFSET_2 = 4;
    value = SwapBits(value, BITS_MASK, OFFSET_0);
    value = SwapBits(value, TWO_BITS_MASK, OFFSET_1);
    value = SwapBits(value, HALF_BYTES_MASK, OFFSET_2);
    return ReverseBytes(value);
}

inline uint32_t BitCount(int32_t value)
{
    constexpr size_t BIT_SIZE = sizeof(int32_t) * 8;
    return std::bitset<BIT_SIZE>(value).count();
}

inline uint32_t BitCount(uint32_t value)
{
    constexpr size_t BIT_SIZE = sizeof(uint32_t) * 8;
    return std::bitset<BIT_SIZE>(value).count();
}

inline uint32_t BitCount(int64_t value)
{
    constexpr size_t BIT_SIZE = sizeof(int64_t) * 8;
    return std::bitset<BIT_SIZE>(value).count();
}

template <typename T>
inline constexpr uint32_t BitNumbers()
{
    constexpr int BIT_NUMBER_OF_CHAR = 8;
    return sizeof(T) * BIT_NUMBER_OF_CHAR;
}

template <typename T>
inline constexpr T ExtractBits(T value, size_t offset, size_t count)
{
    static_assert(std::is_integral<T>::value, "T must be integral");
    static_assert(std::is_unsigned<T>::value, "T must be unsigned");
    ASSERT(sizeof(value) * ark::BITS_PER_BYTE >= offset + count);
    return (value >> offset) & ((1ULL << count) - 1);
}

template <typename T>
inline constexpr uint32_t Low32Bits(T value)
{
    return static_cast<uint32_t>(reinterpret_cast<uint64_t>(value));
}

template <typename T>
inline constexpr uint32_t High32Bits(T value)
{
    if constexpr (sizeof(T) < sizeof(uint64_t)) {  // NOLINT
        return 0;
    }
    return static_cast<uint32_t>(reinterpret_cast<uint64_t>(value) >> BITS_PER_UINT32);
}

template <class To, class From>
inline To bit_cast(const From &src) noexcept  // NOLINT(readability-identifier-naming)
{
    static_assert(sizeof(To) == sizeof(From), "size of the types must be equal");
    static_assert(std::is_trivially_copyable_v<To> && std::is_trivially_copyable_v<From>,
                  "source and destination types must be trivially copyable");
    static_assert(std::is_trivially_constructible_v<To>, "destination type must be default constructible");

    To dst {};
    MemcpyUnsafe(&dst, &src, sizeof(To));
    return dst;
}

template <class To, class From>
inline To down_cast(const From &src) noexcept  // NOLINT(readability-identifier-naming)
{
    static_assert(sizeof(To) <= sizeof(From), "size of the types must be lesser");
    To dst {};
    MemcpyUnsafe(&dst, &src, sizeof(To));
    return dst;
}

template <typename T>
inline constexpr uint32_t BitsNumInValue(const T v)
{
    return sizeof(v) * ark::BITS_PER_BYTE;
}

template <typename T>
inline constexpr uint32_t BitsNumInType()
{
    return sizeof(T) * ark::BITS_PER_BYTE;
}

template <typename From, typename To>
constexpr To CastFloatToInt(From value)
{
    static_assert(std::is_floating_point_v<From>);
    static_assert(std::is_integral_v<To>);
    To res;
    constexpr To MIN_INT = std::numeric_limits<To>::min();
    constexpr To MAX_INT = std::numeric_limits<To>::max();
    constexpr auto FLOAT_MIN_INT = static_cast<From>(MIN_INT);
    constexpr auto FLOAT_MAX_INT = static_cast<From>(MAX_INT);

    if (value > FLOAT_MIN_INT) {
        if (value < FLOAT_MAX_INT) {
            res = static_cast<To>(value);
        } else {
            res = MAX_INT;
        }
    } else if (std::isnan(value)) {
        res = 0;
    } else {
        res = MIN_INT;
    }
    return res;
}

}  // namespace ark

#endif  // LIBPANDABASE_UTILS_BIT_UTILS_H_
