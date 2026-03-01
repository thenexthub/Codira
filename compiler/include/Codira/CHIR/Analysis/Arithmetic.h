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

#ifndef CODIRA_CHIR_ANALYSIS_ARITHMETIC_H
#define CODIRA_CHIR_ANALYSIS_ARITHMETIC_H

#include <cstdint>
#include <type_traits>
#include <limits>
#include <climits>

#include <codira/Utils/CheckUtils.h>

namespace Codira::CHIR {
/// Sign extend \p val to a new width \p width
int64_t SignExtend64(uint64_t val, unsigned srcWidth);

/// Sign extend \p val to a new width \p width
template <class T>
int64_t SignExtend64(T val)
{
    static_assert(std::is_unsigned_v<T>);
    return SignExtend64(val, sizeof(T) * CHAR_BIT);
}

/// value 64
constexpr unsigned B64 = 64u;

/// whether a value is power of 2.
template <typename T>
bool IsPowerOf2(T val)
{
    static_assert(std::is_unsigned_v<T>);
    return val && !(val & (val - 1));
}

/// Count leading zeroes
template <typename T>
size_t Clz(T val);

/// Count leading ones
template <typename T>
size_t Clo(T val);

/// Count trailing zeroes
template <typename T>
size_t Ctz(T val);

/// Count trailing ones
template <typename T>
size_t Cto(T val);

/// Count the number of set bits
template <typename T>
unsigned Popcnt(T val);

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#endif
namespace {
/// count leading zero of a value
template <typename T, size_t sizeOfT = sizeof(T)> struct LeadingZeroesCounter {
    static_assert(std::is_unsigned_v<T>);
#if __cplusplus >= 201907L
    static inline size_t Count(T val)
    {
        return static_cast<size_t>(std::countl_zero(val));
    }
#else
    static size_t Count(T val)
    {
        size_t zeroBits = 0;
        // bisection method
        for (T shift = std::numeric_limits<T>::digits >> 1; shift; shift >>= 1) {
            T tmp = val >> shift;
            if (tmp) {
                val = tmp;
            } else {
                zeroBits |= shift;
            }
        }
        return zeroBits;
    }
#endif
};

/// count leading zero of a value implement
#if __cplusplus < 201907L && (__GNUC__ >= 4 || defined(_MSC_VER))
// template specialization for 32-bit integers when gcc and msvc builtin available
template <typename T> struct LeadingZeroesCounter<T, 4U> {
    static size_t Count(T val)
    {
#if __has_builtin(__builtin_clz) || defined(__GNUC__)
        return __builtin_clz(val);
#elif define(_MSC_VER)
        unsigned long index;
        _BitScanReverse(&index, val);
        return index ^ (sizeof(T) * CHAR_BIT - 1u);
#endif
    }
};
#endif

/// count leading zero of a value implement
#if __cplusplus < 201907L && (!defined(_MSC_VER) || defined(_M_X64))
// template specialization for 64-bit integers when gcc and msvc builtin available
template <typename T> struct LeadingZeroesCounter<T, 8U> {
    static size_t Count(T val)
    {
#if __has_builtin(__builtin_clzll)
        return __builtin_clzll(val);
#elif define(_MSC_VER)
        unsigned long index;
        _BitScanReverse64(&index, val);
        return index ^ (sizeof(T) * CHAR_BIT - 1u);
#endif
    }
};
#endif
} // namespace

template <typename T> size_t Clz(T val)
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);
    if (!val) {
        return std::numeric_limits<T>::digits;
    }
    return LeadingZeroesCounter<T>::Count(val);
}

template <typename T> size_t Clo(T val)
{
    return Clz<T>(~val);
}

namespace {
/// count trailing zero of a value
template <typename T, size_t sizeOfT = sizeof(T)> struct TrailingZeroesCounter {
#if __cplusplus >= 201907L
    static inline size_t Count(T val)
    {
        return std::countr_zero(val);
    }
#else
    static size_t Count(T val)
    {
        if (val & 1) {
            return 0;
        }
        // bisection method
        size_t zeroBits = 0;
        T shift = std::numeric_limits<T>::digits >> 1;
        T mask = std::numeric_limits<T>::max() >> shift;
        while (shift) {
            if ((val & mask) == 0) {
                val >>= shift;
                zeroBits |= shift;
            }
            shift >>= 1;
            mask >>= shift;
        }
        return zeroBits;
    }
#endif
};

/// count trailing zero of a value implement
#if __cplusplus < 201907L && (__GNUC__ >= 4 || defined(_MSC_VER))
// template specialization for 32-bit integers when gcc and msvc builtin available
template <typename T> struct TrailingZeroesCounter<T, 4U> {
    static size_t Count(T val)
    {
#if __has_builtin(__builtin_ctz) || defined(__GNUC__)
        return __builtin_ctz(val);
#elif define(_MSC_VER)
        unsigned long index;
        _BitScanForward(&index, val);
        return index;
#endif
    }
};
#endif

/// count trailing zero of a value implement
#if __cplusplus < 201907L && (!defined(_MSC_VER) || defined(_M_X64))
// template specialization for 64-bit integers when gcc and msvc builtin available
template <typename T> struct TrailingZeroesCounter<T, 8U> {
    static size_t Count(T val)
    {
#if __has_builtin(__builtin_ctzll)
        return __builtin_ctzll(val);
#elif define(_MSC_VER)
        unsigned long index;
        _BitScanForward64(&index, val);
        return index;
#endif
    }
};
#endif
} // namespace

template <typename T> size_t Ctz(T val)
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);
    if (!val) {
        return std::numeric_limits<T>::digits;
    }
    return TrailingZeroesCounter<T>::Count(val);
}

template <typename T> size_t Cto(T val)
{
    return Ctz<T>(~val);
}

namespace {
/// template specialization for 64-bit integers
template <typename T, size_t sizeOfT = sizeof(T)> struct PopulationCounter {
    static unsigned Count(T val)
    {
        static_assert(sizeOfT <= 4);
#if __cplusplus >= 201907L
        return static_cast<unsigned>(std::popcount(val));
#elif __GNUC__ >= 4
        return __builtin_popcount(val);
#elif defined(_MSC_VER)
        return __popcnt(val);
#else
        uint32_t v = val;
        v = v - ((v >> 1) & 0x55555555);
        v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
        return (((v + (v >> 4)) & 0x0f0f0f0f) * 0x1010101) >> 24;
#endif
    }
};

/// template specialization for 64-bit integers
template <typename T> struct PopulationCounter<T, 8U> {
    static unsigned Count(T val)
    {
#if __cplusplus >= 201907L
        return static_cast<unsigned>(std::popcount(val));
#elif __GNUC__ >= 4
        return __builtin_popcountll(val);
#elif defined(_MSC_VER)
        return __popcnt64(val);
#else
        uint64_t v = val;
        v = v - ((v >> 1) & 0x5555555555555555ULL);
        v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
        v = (v + (v >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
        return static_cast<unsigned>(static_cast<uint64_t>(v * 0x0101010101010101ULL) >> 56);
#endif
    }
};
} // namespace

template <typename T> unsigned Popcnt(T val)
{
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);
    return PopulationCounter<T>::Count(val);
}
} // namespace Codira::CHIR
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif
