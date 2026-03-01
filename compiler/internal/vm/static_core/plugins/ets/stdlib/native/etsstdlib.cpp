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

#include <ani.h>
#include <iostream>

#include "plugins/ets/stdlib/native/core/Intl.h"

namespace ark::ets::stdlib {

// EtsNapiOnLoad needs to implement issue #18135
/* And provide symbol after fix #18135: extern "C"*/
// NOLINTNEXTLINE(readability-identifier-naming)
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    // Initializing components
    if (intl::InitCoreIntl(env) != ANI_OK) {
        return ANI_ERROR;
    }

    *result = ANI_VERSION_1;
    return ANI_OK;
}

}  // namespace ark::ets::stdlib
