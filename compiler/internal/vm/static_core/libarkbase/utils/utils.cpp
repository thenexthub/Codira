/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "libarkbase/utils/utils.h"

namespace ark {

uint32_t CountDigits(uint64_t v)
{
    constexpr uint64_t POW10_1 = 10ULL;
    constexpr uint64_t POW10_2 = 100ULL;
    constexpr uint64_t POW10_3 = 1000ULL;
    constexpr uint64_t POW10_4 = 10000ULL;
    uint32_t count = 1U;
    while (v >= POW10_1) {
        if (v < POW10_2) {
            return count + 1U;
        }
        if (v < POW10_3) {
            return count + 2U;
        }
        if (v < POW10_4) {
            return count + 3U;
        }
        count += 4U;
        v /= POW10_4;
    }
    return count;
}

}  // namespace ark
