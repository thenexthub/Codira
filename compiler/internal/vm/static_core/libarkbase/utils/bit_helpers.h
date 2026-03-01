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

#ifndef PANDA_LIBPANDABASE_UTILS_BIT_HELPERS_H_
#define PANDA_LIBPANDABASE_UTILS_BIT_HELPERS_H_

#include "libarkbase/macros.h"

#include <cstddef>
#include <cstdint>

#include <limits>
#include <type_traits>

namespace ark::helpers {

template <size_t WIDTH>
struct UnsignedTypeHelper {
    // NOLINTNEXTLINE(readability-identifier-naming)
    using type = std::conditional_t<
        WIDTH <= std::numeric_limits<uint8_t>::digits, uint8_t,
        std::conditional_t<
            WIDTH <= std::numeric_limits<uint16_t>::digits, uint16_t,
            std::conditional_t<WIDTH <= std::numeric_limits<uint32_t>::digits, uint32_t,
                               std::conditional_t<WIDTH <= std::numeric_limits<uint64_t>::digits, uint64_t, void>>>>;
};

template <size_t WIDTH>
using UnsignedTypeHelperT = typename UnsignedTypeHelper<WIDTH>::type;

template <size_t WIDTH, bool IS_SIGNED>
struct TypeHelper {
    // NOLINTNEXTLINE(readability-identifier-naming)
    using type =
        std::conditional_t<IS_SIGNED, std::make_signed_t<UnsignedTypeHelperT<WIDTH>>, UnsignedTypeHelperT<WIDTH>>;
};

template <size_t WIDTH, bool IS_SIGNED>
using TypeHelperT = typename TypeHelper<WIDTH, IS_SIGNED>::type;

}  // namespace ark::helpers

#endif  // PANDA_LIBPANDABASE_UTILS_BIT_HELPERS_H_
