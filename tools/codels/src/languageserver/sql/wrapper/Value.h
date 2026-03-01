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

#pragma once

#include "SQLiteAPI.h"
#include "Traits.h"

#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#if __cplusplus > 201402L
#include <optional>
#endif

namespace sqldb {
namespace traits {

template <typename, typename = void>
struct Value;

template <typename T>
decltype(auto) value(sqlite3_value *V)
{
    return Value<traits::remove_cvref_t<T>>::call(V);
}

template <>
struct Value<std::decay_t<decltype(std::ignore)>> {
    static auto call(sqlite3_value *) { return std::ignore; }
};

template <typename T>
struct Value<T, std::enable_if_t<std::is_integral<T>::value>> {
    static T call(sqlite3_value *V) { return static_cast<T>(sqlite::value_int64(V)); }
};

template <typename T>
struct Value<T, std::enable_if_t<std::is_enum<T>::value>> {
    static T call(sqlite3_value *V) { return static_cast<T>(sqlite::value_int64(V)); }
};

template <typename T>
struct Value<T, std::enable_if_t<std::is_floating_point<T>::value>> {
    static T call(sqlite3_value *V) { return static_cast<T>(sqlite::value_double(V)); }
};

template <>
struct Value<const char *> {
    static const char *call(sqlite3_value *V)
    {
        const unsigned char *T = sqlite::value_text(V);
        return T ? reinterpret_cast<const char *>(T) : "";
    }
};

template <typename T>
inline const T *data(sqlite3_value *V) noexcept
{
    return static_cast<const T *>(sqlite::value_blob(V));
}

template <typename T>
inline std::size_t count(sqlite3_value *V) noexcept
{
    return sqlite::value_bytes(V) / sizeof(T);
}

template <>
struct Value<std::string> {
    static std::string call(sqlite3_value *V) { return {data<char>(V), count<char>(V)}; }
};

template <typename T>
struct Value<std::vector<T>, std::enable_if_t<is_scalar<T>::value>> {
    static std::vector<T> call(sqlite3_value *V) { return {data<T>(V), data<T>(V) + count<T>(V)}; }
};

#if __cplusplus > 201402L

template <>
struct Value<std::string_view> {
    static std::string_view call(sqlite3_value *V) { return {data<char>(V), count<char>(V)}; }
};

template <typename T>
struct Value<std::optional<T>> {
    static std::optional<T> call(sqlite3_value *V)
    {
        if (sqlite::value_type(V) != sqlite::Null) {
            return Value<T>::call(V);
        }
        return std::nullopt;
    }
};

#endif // __cplusplus > 201402L

#if __cplusplus > 201703L

#if defined(__cpp_lib_char8_t)

template <>
struct Value<std::u8string> {
    static std::u8string call(sqlite3_value *V) { return {data<char8_t>(V), count<char8_t>(V)}; }
};

template <>
struct Value<std::u8string_view> {
    static std::u8string_view call(sqlite3_value *V) { return {data<char8_t>(V), count<char8_t>(V)}; }
};

#endif // defined(__cpp_lib_char8_t)

#endif // __cplusplus > 201703L

} // namespace traits
} // namespace sqldb
