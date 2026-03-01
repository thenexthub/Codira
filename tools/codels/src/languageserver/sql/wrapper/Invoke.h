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

#include <cstddef>
#include <type_traits>
#include <utility>

namespace sqldb {
namespace impl {

template <typename Callable,
    typename... Ts,
    std::size_t I = sizeof...(Ts),
    std::enable_if_t<(I == traits::function<Callable>::arity), int> = 0>
auto invoke(sqlite3_stmt *, Callable &&F, Ts &&...Vs)
{
    return std::forward<Callable>(F)(std::forward<Ts>(Vs)...);
}

template <typename Callable,
    typename... Ts,
    std::size_t I = sizeof...(Ts),
    std::enable_if_t<(I < traits::function<Callable>::arity), int> = 0>
auto invoke(sqlite3_stmt *S, Callable &&F, Ts &&...Vs)
{
    using T = typename traits::function<Callable>::template argument<I>;
    return invoke(S, std::forward<Callable>(F), std::forward<Ts>(Vs)..., traits::value<T>(sqlite::column_value(S, I)));
}

} // namespace impl

template <typename Callable, typename... Ts>
std::exception_ptr invoke(Callable &&F, Ts &&...Args) noexcept
{
#ifndef NO_EXCEPTIONS
    try {
#endif
        std::invoke(std::forward<Callable>(F), std::forward<Ts>(Args)...);
#ifndef NO_EXCEPTIONS
    } catch (...) {
        return std::current_exception();
    }
#endif
    return nullptr;
}

} // namespace sqldb
