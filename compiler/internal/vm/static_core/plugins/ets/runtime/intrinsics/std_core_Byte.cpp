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

EtsShort StdCoreByteToShort(EtsByte val)
{
    return static_cast<int16_t>(val);
}

EtsInt StdCoreByteToInt(EtsByte val)
{
    return static_cast<int32_t>(val);
}

EtsLong StdCoreByteToLong(EtsByte val)
{
    return static_cast<int64_t>(val);
}

EtsFloat StdCoreByteToFloat(EtsByte val)
{
    return static_cast<float>(val);
}

EtsDouble StdCoreByteToDouble(EtsByte val)
{
    return static_cast<double>(val);
}

EtsChar StdCoreByteToChar(EtsByte val)
{
    return static_cast<uint16_t>(val);
}

EtsByte StdCoreByteParseInt(EtsString *str, int32_t radix)
{
    return helpers::StringToInt8(str, radix);
}

}  // namespace ark::ets::intrinsics
