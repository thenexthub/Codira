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
#include "Value.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if __cplusplus > 201402L
#include <optional>
#endif

namespace sqldb {
namespace traits {

template <typename, typename = void>
struct Result;

template <typename T>
void result(sqlite3_context *C, T &&V)
{
    Result<traits::remove_cvref_t<T>>::call(C, std::forward<T>(V));
}

template <typename T>
struct Result<T, std::enable_if_t<std::is_integral<T>::value>> {
    static void call(sqlite3_context *C, T V) { sqlite::result_int64(C, static_cast<std::int64_t>(V)); }
};

template <typename T>
struct Result<T, std::enable_if_t<std::is_enum<T>::value>> {
    static void call(sqlite3_context *C, T V)
    {
        using U = std::underlying_type_t<T>;
        Result<U>::call(C, static_cast<U>(V));
    }
};

template <typename T>
struct Result<T, std::enable_if_t<std::is_floating_point<T>::value>> {
    static void call(sqlite3_context *C, T V) { sqlite::result_double(C, static_cast<double>(V)); }
};

template <>
struct Result<std::string> {
    static void call(sqlite3_context *C, const std::string &V)
    {
        sqlite::result_text64(C, V.data(), V.size(), sqlite::Transient, sqlite::UTF8);
    }
};

template <>
struct Result<std::nullptr_t> {
    static void call(sqlite3_context *C, std::nullptr_t) { sqlite::result_null(C); }
};

template <typename T, std::size_t N>
struct Result<std::array<T, N>, std::enable_if_t<is_scalar<T>::value>> {
    static void call(sqlite3_context *C, const std::array<T, N> &V)
    {
        sqlite::result_blob64(C, V.data(), V.size() * sizeof(T), sqlite::Transient);
    }
};

template <typename T>
struct Result<std::vector<T>, std::enable_if_t<is_scalar<T>::value>> {
    static void call(sqlite3_context *C, const std::vector<T> &V)
    {
        sqlite::result_blob64(C, V.data(), V.size() * sizeof(T), sqlite::Transient);
    }
};

#if __cplusplus > 201402L

template <>
struct Result<std::string_view> {
    static void call(sqlite3_context *C, std::string_view V)
    {
        sqlite::result_text64(C, V.data(), V.size(), sqlite::Transient, sqlite::UTF8);
    }
};

template <>
struct Result<std::nullopt_t> {
    static void call(sqlite3_context *C, std::nullopt_t) { sqlite::result_null(C); }
};

template <typename T>
struct Result<std::optional<T>> {
    static void call(sqlite3_context *C, const std::optional<T> &V)
    {
        if (V.has_value()) {
            Result<T>::call(C, V.value());
        } else {
            Result<std::nullopt_t>::call(C, std::nullopt);
        }
    }
};

#endif // __cplusplus > 201402L

} // namespace traits

namespace impl {

template <typename Function>
struct Scalar {
    Function F;
};

template <typename Scalar>
using ScalarTraits = traits::function<decltype(Scalar::F)>;

template <typename Scalar>
using ScalarReturnType = typename ScalarTraits<Scalar>::return_type;

template <typename Scalar>
auto &scalar(sqlite3_context *C)
{
    return static_cast<Scalar *>(sqlite::user_data(C))->F;
}

template <typename Scalar, typename... Ts>
std::enable_if_t<(sizeof...(Ts) == ScalarTraits<Scalar>::arity && std::is_void<ScalarReturnType<Scalar>>::value)>
scalar(sqlite3_context *C, int, sqlite3_value **, Ts &&...Args)
{
    scalar<Scalar>(C)(std::forward<Ts>(Args)...);
}

template <typename Scalar, typename... Ts>
std::enable_if_t<(sizeof...(Ts) == ScalarTraits<Scalar>::arity && !std::is_void<ScalarReturnType<Scalar>>::value)>
scalar(sqlite3_context *C, int, sqlite3_value **, Ts &&...Args)
{
    traits::result(C, scalar<Scalar>(C)(std::forward<Ts>(Args)...));
}

template <typename Scalar, typename... Ts, std::size_t I = sizeof...(Ts)>
std::enable_if_t<(I < ScalarTraits<Scalar>::arity)> scalar(sqlite3_context *C, int N, sqlite3_value **Vs, Ts &&...Args)
{
    using T = typename ScalarTraits<Scalar>::template argument<I>;
    scalar<Scalar>(C, N, Vs, std::forward<Ts>(Args)..., traits::value<T>(Vs[I]));
}

template <typename Step, typename Final>
struct Aggregate {
    Step S;
    Final F;
};

template <typename Aggregate>
using AggregateStepTraits = traits::function<decltype(Aggregate::S)>;

template <typename Aggregate>
using AggregateFinalTraits = traits::function<decltype(Aggregate::F)>;

template <typename Aggregate>
using AggregateContext =
    std::remove_const_t<std::remove_reference_t<typename AggregateStepTraits<Aggregate>::template argument<0>>>;

template <typename Aggregate>
AggregateContext<Aggregate> *context(sqlite3_context *C)
{
    struct AggregateData {
        AggregateContext<Aggregate> Context;
        bool ContextConstructed;
    };
    if (auto *Data = static_cast<AggregateData *>(sqlite::aggregate_context(C, sizeof(AggregateData)))) {
        if (!std::exchange(Data->ContextConstructed, true)) {
            new (std::addressof(Data->Context)) AggregateContext<Aggregate>();
        }
        return std::addressof(Data->Context);
    }
    return nullptr;
}

template <typename Aggregate>
auto &aggregate(sqlite3_context *C)
{
    return *static_cast<Aggregate *>(sqlite::user_data(C));
}

template <typename Aggregate, typename... Ts>
std::enable_if_t<(sizeof...(Ts) == AggregateStepTraits<Aggregate>::arity)> step(
    sqlite3_context *C, int, sqlite3_value **, Ts &&...Args)
{
    aggregate<Aggregate>(C).S(std::forward<Ts>(Args)...);
}

template <typename Aggregate, typename... Ts, std::size_t I = sizeof...(Ts)>
std::enable_if_t<(I > 0 && I < AggregateStepTraits<Aggregate>::arity)> step(
    sqlite3_context *C, int N, sqlite3_value **Vs, Ts &&...Args)
{
    using T = typename AggregateStepTraits<Aggregate>::template argument<I>;
    step<Aggregate>(C, N, Vs, std::forward<Ts>(Args)..., traits::value<T>(Vs[I - 1]));
}

template <typename Aggregate>
void step(sqlite3_context *C, int N, sqlite3_value **Vs)
{
    if (auto *Context = context<Aggregate>(C)) {
        step<Aggregate>(C, N, Vs, *Context);
    }
}

template <typename Aggregate>
void final(sqlite3_context *C)
{
    if (auto *Context = context<Aggregate>(C)) {
        using ContextType = std::remove_reference_t<decltype(*Context)>;
        traits::result(C, aggregate<Aggregate>(C).F(*Context));
        Context->~ContextType();
    }
}

} // namespace impl
} // namespace sqldb
