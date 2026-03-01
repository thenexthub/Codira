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

#ifndef LIBPANDAFILE_HELPERS_H_
#define LIBPANDAFILE_HELPERS_H_

#include "libarkbase/macros.h"
#include "libarkbase/utils/bit_helpers.h"
#include "libarkbase/utils/leb128.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/span.h"

#include <cstdint>

#include <limits>
#include <optional>

namespace ark::panda_file::helpers {

constexpr size_t UINT_BYTE2_SHIFT = 8U;
constexpr size_t UINT_BYTE3_SHIFT = 16U;
constexpr size_t UINT_BYTE4_SHIFT = 24U;

template <size_t WIDTH>
inline auto Read(Span<const uint8_t> *sp)
{
    constexpr size_t BYTE_WIDTH = std::numeric_limits<uint8_t>::digits;
    constexpr size_t BITWIDTH = BYTE_WIDTH * WIDTH;
    using UnsignedType = ark::helpers::TypeHelperT<BITWIDTH, false>;

    UnsignedType result = 0;
    for (size_t i = 0; i < WIDTH; i++) {
        UnsignedType tmp = static_cast<UnsignedType>((*sp)[i]) << (i * BYTE_WIDTH);
        result |= tmp;
    }
    *sp = sp->SubSpan(WIDTH);
    return result;
}

template <>
inline auto Read<sizeof(uint16_t)>(Span<const uint8_t> *sp)
{
    auto *p = sp->data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint16_t result = *(p++);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-signed-bitwise)
    result |= static_cast<uint16_t>(*p) << UINT_BYTE2_SHIFT;
    *sp = sp->SubSpan(sizeof(uint16_t));
    return result;
}

template <>
inline auto Read<sizeof(uint32_t)>(Span<const uint8_t> *sp)
{
    auto *p = sp->data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint32_t result = *(p++);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result |= static_cast<uint32_t>(*(p++)) << UINT_BYTE2_SHIFT;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result |= static_cast<uint32_t>(*(p++)) << UINT_BYTE3_SHIFT;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result |= static_cast<uint32_t>(*p) << UINT_BYTE4_SHIFT;
    *sp = sp->SubSpan(sizeof(uint32_t));
    return result;
}

template <size_t WIDTH>
inline auto Read(Span<const uint8_t> sp)
{
    return Read<WIDTH>(&sp);
}

inline uint32_t ReadULeb128(Span<const uint8_t> *sp)
{
    uint32_t result;
    size_t n;
    [[maybe_unused]] bool isFull;
    std::tie(result, n, isFull) = leb128::DecodeUnsigned<uint32_t>(sp->data());
    if (!isFull) {
        LOG(FATAL, PANDAFILE) << "ULEB128 decode failed: input truncated or malformed";
    }
    *sp = sp->SubSpan(n);
    return result;
}

// CC-OFFNXT(G.FUD.06) solid logic
inline void SkipULeb128(Span<const uint8_t> *sp)
{
    if ((*sp)[0U] <= leb128::PAYLOAD_MASK) {
        *sp = sp->SubSpan(1U);
        return;
    }

    if ((*sp)[1U] <= leb128::PAYLOAD_MASK) {
        *sp = sp->SubSpan(2U);
        return;
    }

    if ((*sp)[2U] <= leb128::PAYLOAD_MASK) {
        *sp = sp->SubSpan(3U);
        return;
    }

    ASSERT((*sp)[3U] <= leb128::PAYLOAD_MASK);
    *sp = sp->SubSpan(4U);
}

inline int32_t ReadLeb128(Span<const uint8_t> *sp)
{
    int32_t result;
    size_t n;
    [[maybe_unused]] bool isFull;
    std::tie(result, n, isFull) = leb128::DecodeSigned<int32_t>(sp->data());
    ASSERT(isFull);
    *sp = sp->SubSpan(n);
    return result;
}

template <size_t ALIGNMENT>
inline const uint8_t *Align(const uint8_t *ptr)
{
    auto intptr = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t aligned = (intptr - 1U + ALIGNMENT) & -ALIGNMENT;
    return reinterpret_cast<const uint8_t *>(aligned);
}

template <size_t ALIGNMENT, class T>
inline T Align(T n)
{
    return (n - 1U + ALIGNMENT) & -ALIGNMENT;
}

template <class T, class E>
inline std::optional<T> GetOptionalTaggedValue(Span<const uint8_t> sp, E tag, Span<const uint8_t> *next)
{
    if (sp[0] == static_cast<uint8_t>(tag)) {
        sp = sp.SubSpan(1);
        T value = static_cast<T>(Read<sizeof(T)>(&sp));
        *next = sp;
        return value;
    }
    *next = sp;

    // NB! This is a workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
    // which fails Release builds for GCC 8 and 9.
    std::optional<T> novalue = {};
    return novalue;
}

template <class T, class E, class Callback>
inline void EnumerateTaggedValues(Span<const uint8_t> sp, E tag, Callback cb, Span<const uint8_t> *next)
{
    while (sp[0] == static_cast<uint8_t>(tag)) {
        sp = sp.SubSpan(1);
        T value(Read<sizeof(T)>(&sp));
        cb(value);
    }

    if (next == nullptr) {
        return;
    }

    *next = sp;
}

template <class T, class E, class Callback>
inline bool EnumerateTaggedValuesWithEarlyStop(Span<const uint8_t> sp, E tag, Callback cb)
{
    while (sp[0] == static_cast<uint8_t>(tag)) {
        sp = sp.SubSpan(1);
        T value(Read<sizeof(T)>(&sp));
        if (cb(value)) {
            return true;
        }
    }

    return false;
}

}  // namespace ark::panda_file::helpers

#endif  // LIBPANDAFILE_HELPERS_H_
