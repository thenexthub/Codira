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

#include <tuple>
#include <type_traits>
#include <utility>

namespace sqldb {
namespace impl {

template <typename Callable, std::enable_if_t<traits::is_callable<Callable>::value, int> = 0>
void store(sqlite3_stmt *S, int I, Callable &&F)
{
    using T = typename traits::function<Callable>::template argument<0>;
    std::forward<Callable>(F)(traits::value<T>(sqlite::column_value(S, I)));
}

template <typename Variable, std::enable_if_t<!traits::is_callable<Variable>::value, int> = 0>
void store(sqlite3_stmt *S, int I, Variable &V)
{
    V = traits::value<Variable>(sqlite::column_value(S, I));
}

inline void store(sqlite3_stmt *, int, decltype(std::ignore)) {}

} // namespace impl
} // namespace sqldb
