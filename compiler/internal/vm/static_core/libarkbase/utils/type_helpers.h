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

#ifndef PANDA_LIBPANDABASE_UTILS_TYPE_HELPERS_H_
#define PANDA_LIBPANDABASE_UTILS_TYPE_HELPERS_H_

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ark::helpers {

template <class T>
constexpr auto ToSigned(T v)
{
    using SignedType = std::make_signed_t<T>;
    return static_cast<SignedType>(v);
}

template <class T>
constexpr auto ToUnsigned(T v)
{
    using UnsignedType = std::make_unsigned_t<T>;
    return static_cast<UnsignedType>(v);
}

// CC-OFFNXT(G.FMT.10) project code style
template <typename T, std::enable_if_t<std::is_enum_v<T>> * = nullptr>
constexpr auto ToUnderlying(T value)
{
    return static_cast<std::underlying_type_t<T>>(value);
}

constexpr size_t UnsignedDifference(size_t x, size_t y)
{
    return x > y ? x - y : 0;
}

constexpr uint64_t UnsignedDifferenceUint64(uint64_t x, uint64_t y)
{
    return x > y ? x - y : 0;
}

template <typename T>
struct TypeIdentity {
    // NOLINTNEXTLINE(readability-identifier-naming)
    using type = T;
};

template <typename T>
struct RemoveAllPointers {
    // NOLINTNEXTLINE(readability-identifier-naming)
    using type = typename std::conditional_t<std::is_pointer_v<T>, RemoveAllPointers<std::remove_pointer_t<T>>,
                                             TypeIdentity<T>>::type;
};
}  // namespace ark::helpers

#ifdef __SIZEOF_INT128__
__extension__ using Int128 = __int128;
#else
using Int128 = struct Int128Type {
    constexpr Int128Type() = default;
    constexpr explicit Int128Type(std::int64_t v) : lo(v) {}
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::int64_t hi {0};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::int64_t lo {0};
    bool operator==(std::int64_t v) const
    {
        return (hi == 0) && (lo == v);
    }
};
static_assert(sizeof(Int128) == sizeof(std::int64_t) * 2U);
#endif

#endif  // PANDA_LIBPANDABASE_UTILS_STRING_HELPERS_H_
