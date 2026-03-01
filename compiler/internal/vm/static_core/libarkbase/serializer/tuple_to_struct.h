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

#ifndef PANDA_LIBPANDABASE_UTILS_SERIALIZER_TUPLE_TO_STRUCT_H_
#define PANDA_LIBPANDABASE_UTILS_SERIALIZER_TUPLE_TO_STRUCT_H_

#include <tuple>

namespace ark::serializer::internal {

template <typename Struct, size_t... IS, typename Tuple>
Struct TupleToStructImpl([[maybe_unused]] std::index_sequence<IS...> is, Tuple &&tup)
{
    return {std::get<IS>(std::forward<Tuple>(tup))...};
}

template <typename Struct, typename Tuple>
Struct TupleToStruct(Tuple &&tup)
{
    using T = std::remove_reference_t<Tuple>;
    auto sequence = std::make_index_sequence<std::tuple_size_v<T>> {};

    return TupleToStructImpl<Struct>(sequence, std::forward<Tuple>(tup));
}

}  // namespace ark::serializer::internal

#endif  // PANDA_LIBPANDABASE_UTILS_SERIALIZER_TUPLE_TO_STRUCT_H_
