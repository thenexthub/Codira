/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "cpu_features.h"

namespace panda::compiler {
#if defined PANDA_TARGET_MOBILE || defined PANDA_TARGET_OHOS || defined PANDA_TARGET_UNIX
#include <asm/hwcap.h>
#include <sys/auxv.h>

bool CpuFeaturesHasCrc32()
{
    auto hwcaps = getauxval(AT_HWCAP);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return (hwcaps & HWCAP_CRC32) != 0;
}
#elif PANDA_TARGET_WINDOWS
bool CpuFeaturesHasCrc32()
{
    return false;
}
#else
#error "Unsupported target"
#endif
}  // namespace panda::compiler
