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

#ifndef COMPILER_OPTIMIZER_IR_SELECT_TRANSFORM_TYPE_H
#define COMPILER_OPTIMIZER_IR_SELECT_TRANSFORM_TYPE_H

#include <cstdint>
#include <type_traits>
#include "libarkbase/macros.h"

/*
 * Shared by the following modules:
 * (1) Select[Imm]Transform IR;
 * (2) Aarch64 encoder.
 */

namespace ark::compiler {
enum class SelectTransformType : uint8_t {
    INC,
    INV,
    NEG,
    LAST = NEG,
};

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr T Transform(SelectTransformType type, T value)
{
    switch (type) {
        case SelectTransformType::INC:
            return value + 1;
        case SelectTransformType::INV:
            return ~value;  // NOLINT(hicpp-signed-bitwise)
        case SelectTransformType::NEG:
            return -value;
        default:
            UNREACHABLE();
            return 0;
    }
}

constexpr const char *GetSelectTransformTypeString(SelectTransformType type)
{
    switch (type) {
        case SelectTransformType::INC:
            return "INC";
        case SelectTransformType::INV:
            return "INV";
        case SelectTransformType::NEG:
            return "NEG";
        default:
            return "<invalid>";
    }
}
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_IR_SELECT_TRANSFORM_TYPE_H
