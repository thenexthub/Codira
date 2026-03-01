/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>
#include "intrinsics.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "types/ets_primitives.h"

namespace ark::ets::intrinsics {

EtsShort StdCoreLongToShort(EtsLong val)
{
    return static_cast<int16_t>(val);
}

EtsByte StdCoreLongToByte(EtsLong val)
{
    return static_cast<int8_t>(val);
}

EtsInt StdCoreLongToInt(EtsLong val)
{
    return static_cast<int32_t>(val);
}

EtsFloat StdCoreLongToFloat(EtsLong val)
{
    return static_cast<float>(val);
}

EtsDouble StdCoreLongToDouble(EtsLong val)
{
    return static_cast<double>(val);
}

EtsChar StdCoreLongToChar(EtsLong val)
{
    return static_cast<uint16_t>(val);
}

EtsLong StdCoreLongParseInt(EtsString *str, int32_t radix)
{
    return helpers::StringToInt64(str, radix);
}

}  // namespace ark::ets::intrinsics
