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

#if __cplusplus < 201402L
#error "C++14 or higher is required"
#endif

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

/**
 * NOLINTBEGIN(readability-identifier-naming)
 */

namespace sqldb {
namespace traits {

/**
 * From C++17.
 */
template <typename...>
using void_t = void;

/**
 * From C++17.
 */
template <bool B>
using bool_constant = std::integral_constant<bool, B>;

/**
 * From C++20.
 */
template <typename T>
struct remove_cvref {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

/**
 * From C++20.
 */
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

/**
 * Similar to std::is_scalar from C++11, but discards pointers.
 */
template <typename T>
struct is_scalar : public bool_constant<std::is_enum<T>::value || std::is_arithmetic<T>::value> {};

template <typename, typename = void>
struct is_callable : public std::false_type {};

template <typename T>
struct is_callable<T, void_t<decltype(&std::decay_t<T>::operator())>> : public std::true_type {};

template <typename T>
struct is_callable<T, std::enable_if_t<std::is_function<T>::value>> : public std::true_type {};

template <typename Callable, bool IsClass = std::is_class<Callable>::value>
struct function : public function<decltype(&std::decay_t<Callable>::operator())> {};

template <typename ClassType, typename ReturnType, typename... Arguments>
struct function<ReturnType (ClassType::*)(Arguments...) const, false> : public function<ReturnType(Arguments...)> {};

template <typename ClassType, typename ReturnType, typename... Arguments>
struct function<ReturnType (ClassType::*)(Arguments...), false> : public function<ReturnType(Arguments...)> {};

template <typename ReturnType, typename... Arguments>
struct function<ReturnType (*)(Arguments...), false> : public function<ReturnType(Arguments...)> {};

template <typename ReturnType, typename... Arguments>
struct function<ReturnType (&)(Arguments...), false> : public function<ReturnType(Arguments...)> {};

#if __cplusplus > 201402L

template <typename ClassType, typename ReturnType, typename... Arguments>
struct function<ReturnType (ClassType::*)(Arguments...) const noexcept, false>
    : public function<ReturnType(Arguments...)> {};

template <typename ClassType, typename ReturnType, typename... Arguments>
struct function<ReturnType (ClassType::*)(Arguments...) noexcept, false> : public function<ReturnType(Arguments...)> {};

template <typename ReturnType, typename... Arguments>
struct function<ReturnType (*)(Arguments...) noexcept, false> : public function<ReturnType(Arguments...)> {};

template <typename ReturnType, typename... Arguments>
struct function<ReturnType (&)(Arguments...) noexcept, false> : public function<ReturnType(Arguments...)> {};

#endif // __cplusplus > 201402L

template <typename ReturnType, typename... Arguments>
struct function<ReturnType(Arguments...), false> {
    using return_type = ReturnType;

    static constexpr std::size_t arity = sizeof...(Arguments);

    template <std::size_t Index>
    using argument = std::tuple_element_t<Index, std::tuple<Arguments...>>;
};

} // namespace traits

template <std::size_t Index = 0,
    typename Tuple,
    typename Callable,
    std::size_t Size = std::tuple_size<std::decay_t<Tuple>>::value>
std::enable_if_t<(Index == Size)> tuple_for_each(const Tuple &, Callable &&)
{
}

template <std::size_t Index = 0,
    typename Tuple,
    typename Callable,
    std::size_t Size = std::tuple_size<std::decay_t<Tuple>>::value>
std::enable_if_t<(Index < Size)> tuple_for_each(Tuple &&Vs, Callable &&F)
{
    F(std::get<Index>(Vs));
    tuple_for_each<Index + 1>(std::forward<Tuple>(Vs), std::forward<Callable>(F));
}

} // namespace sqldb

/**
 * NOLINTEND(readability-identifier-naming)
 */
