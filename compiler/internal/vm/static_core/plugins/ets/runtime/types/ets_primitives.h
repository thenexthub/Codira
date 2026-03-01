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
#ifndef PANDA_RUNTIME_ETS_TYPES_ETS_PRIMITIVES_H
#define PANDA_RUNTIME_ETS_TYPES_ETS_PRIMITIVES_H

#include <cstdint>
#include <type_traits>
#include "libarkbase/macros.h"

namespace ark::ets {
// Primitive types association got from runtime/class_linker.cpp:InitializeFields()
using EtsVoid = void;
using EtsBoolean = uint8_t;
using EtsByte = int8_t;
using EtsChar = uint16_t;
using EtsShort = int16_t;
using EtsUint = uint32_t;
using EtsInt = int32_t;
using EtsUlong = uint64_t;
using EtsLong = int64_t;
using EtsFloat = float;
using EtsDouble = double;

constexpr EtsBoolean ToEtsBoolean(bool b)
{
    return static_cast<EtsBoolean>(b);
}

constexpr bool FromEtsBoolean(EtsBoolean b)
{
    return b != 0U;
}

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_TYPES_ETS_PRIMITIVES_H
