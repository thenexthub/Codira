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
#ifndef RUNTIME_INCLUDE_TAIHE_UNIT_HPP_
#define RUNTIME_INCLUDE_TAIHE_UNIT_HPP_
// NOLINTBEGIN

#include <taihe/unit.abi.h>
#include <taihe/common.hpp>

namespace taihe {
struct unit {
    unit() = default;
    unit(unit const &) = default;
    unit &operator=(unit const &) = default;
    unit(unit &&) = default;
    unit &operator=(unit &&) = default;
    ~unit() = default;

private:
    // This dummy field is required to ensure that the struct is not empty, which
    // is necessary for C compatibility and ABI layout.
    char dummy = 0;
};

inline bool operator==(unit, unit)
{
    return true;
}

template <>
struct as_param<unit> {
    using type = unit;
};

template <>
struct as_abi<unit> {
    using type = TUnit;
};
}  // namespace taihe

template <>
struct std::hash<taihe::unit> {
    std::size_t operator()(taihe::unit) const noexcept
    {
        return 0;  // unit has no state, so we can return a constant hash value
    }
};
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_UNIT_HPP_