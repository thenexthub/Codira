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

#ifndef PANDA_VERIFIER_UTIL_MISC_HPP_
#define PANDA_VERIFIER_UTIL_MISC_HPP_

#include "libarkbase/utils/hash.h"

#include <functional>
#include <cstddef>
#include <tuple>

namespace ark::verifier {

template <typename T>
size_t StdHash(const T &x)
{
    return std::hash<T> {}(x);
}

}  // namespace ark::verifier

namespace std {

template <typename T1, typename T2>
struct hash<std::pair<T1, T2>> {
    size_t operator()(const std::pair<T1, T2> &pair) const
    {
        return ark::MergeHashes(ark::verifier::StdHash(pair.first),
                                ark::verifier::StdHash(pair.second));  // NOLINT
    }
};

template <typename... T>
struct hash<std::tuple<T...>> {
    template <size_t N>
    size_t Helper(size_t tmpHash, const std::tuple<T...> &tuple) const
    {
        if constexpr (N < sizeof...(T)) {
            size_t tmp_hash1 = ark::MergeHashes(tmpHash, ark::verifier::StdHash(std::get<N>(tuple)));  // NOLINT
            return Helper<N + 1>(tmp_hash1, tuple);
        }

        return tmpHash;
    }

    size_t operator()(const std::tuple<T...> &tuple) const
    {
        return Helper<1>(ark::verifier::StdHash(std::get<0>(tuple)), tuple);
    }
};

}  // namespace std

#endif  // !PANDA_VERIFIER_UTIL_MISC_HPP_
