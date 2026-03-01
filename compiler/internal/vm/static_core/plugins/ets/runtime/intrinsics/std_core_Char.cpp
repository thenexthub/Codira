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

#include "intrinsics.h"
#include "helpers/ets_intrinsics_helpers.h"
#include "types/ets_primitives.h"

namespace ark::ets::intrinsics {

const char LOWER_TO_UPPER_OFFSET = 'a' - 'A';

extern "C" EtsBoolean StdCoreCharIsUpperCase(EtsChar value)
{
    return ToEtsBoolean(value >= 'A' && value <= 'Z');
}
extern "C" EtsChar StdCoreCharToUpperCase(EtsChar value)
{
    return (value >= 'a' && value <= 'z') ? value - LOWER_TO_UPPER_OFFSET : value;
}
extern "C" EtsBoolean StdCoreCharIsLowerCase(EtsChar value)
{
    return ToEtsBoolean(value >= 'a' && value <= 'z');
}
extern "C" EtsChar StdCoreCharToLowerCase(EtsChar value)
{
    return (value >= 'A' && value <= 'Z') ? value + LOWER_TO_UPPER_OFFSET : value;
}
extern "C" EtsBoolean StdCoreCharIsWhiteSpace(EtsChar value)
{
    return ToEtsBoolean(utf::IsWhiteSpaceChar(value));
}

EtsByte StdCoreCharToByte(EtsChar val)
{
    return static_cast<int8_t>(val);
}

EtsShort StdCoreCharToShort(EtsChar val)
{
    return static_cast<int16_t>(val);
}

EtsInt StdCoreCharToInt(EtsChar val)
{
    return static_cast<int32_t>(val);
}

EtsLong StdCoreCharToLong(EtsChar val)
{
    return static_cast<int64_t>(val);
}

EtsFloat StdCoreCharToFloat(EtsChar val)
{
    return static_cast<float>(val);
}

EtsDouble StdCoreCharToDouble(EtsChar val)
{
    return static_cast<double>(val);
}

}  // namespace ark::ets::intrinsics
