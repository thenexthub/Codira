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
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"

namespace ark::ets::intrinsics {

EtsString *StdCoreFloatToString(float number, int radix)
{
    auto *cache = PandaEtsVM::GetCurrent()->GetFloatToStringCache();
    if (UNLIKELY(radix != helpers::DECIMAL || cache == nullptr)) {
        return helpers::FpToString(number, radix);
    }
    return cache->GetOrCache(EtsCoroutine::GetCurrent(), number);
}

extern "C" EtsBoolean StdCoreFloatIsNan(float v)
{
    return ToEtsBoolean(v != v);
}

extern "C" EtsBoolean StdCoreFloatIsFinite(float v)
{
    static const float POSITIVE_INFINITY = 1.0 / 0.0;
    static const float NEGATIVE_INFINITY = -1.0 / 0.0;

    return ToEtsBoolean(v == v && v != POSITIVE_INFINITY && v != NEGATIVE_INFINITY);
}

extern "C" EtsFloat StdCoreFloatBitCastFromInt(EtsInt i)
{
    return bit_cast<EtsFloat>(i);
}

extern "C" EtsInt StdCoreFloatBitCastToInt(EtsFloat f)
{
    return bit_cast<EtsInt>(f);
}

static inline bool IsInteger(float v)
{
    return std::isfinite(v) && (std::fabs(v - std::trunc(v)) <= std::numeric_limits<float>::epsilon());
}

extern "C" EtsBoolean StdCoreFloatIsInteger(float v)
{
    return ToEtsBoolean(IsInteger(v));
}

/*
 * In ETS Float.isSafeInteger returns (Float.isInteger(v) && (abs(v) <= Float.MAX_SAFE_INTEGER)).
 * MAX_SAFE_INTEGER is a max integer value that can be used as a float without losing precision.
 */
extern "C" EtsBoolean StdCoreFloatIsSafeInteger(float v)
{
    return ToEtsBoolean(IsInteger(v) && (std::fabs(v) <= helpers::MaxSafeInteger<float>()));
}

EtsShort StdCoreFloatToShort(EtsFloat val)
{
    // CC-OFFNXT(G.NAM.03) false positive
    int intVal = CastFloatToInt<EtsFloat, EtsInt>(val);
    return static_cast<int16_t>(intVal);
}

EtsByte StdCoreFloatToByte(EtsFloat val)
{
    // CC-OFFNXT(G.NAM.03) false positive
    int intVal = CastFloatToInt<EtsFloat, EtsInt>(val);
    return static_cast<int8_t>(intVal);
}

EtsInt StdCoreFloatToInt(EtsFloat val)
{
    return CastFloatToInt<EtsFloat, EtsInt>(val);
}

EtsLong StdCoreFloatToLong(EtsFloat val)
{
    return CastFloatToInt<EtsFloat, EtsLong>(val);
}

EtsDouble StdCoreFloatToDouble(EtsFloat val)
{
    return static_cast<double>(val);
}

}  // namespace ark::ets::intrinsics
