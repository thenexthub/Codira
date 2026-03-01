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

#include <array>
#include <iostream>

#include "ani.h"

extern "C" {
namespace ark::ets::ani {

ani_long AllocMemImpl([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_int len)
{
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    auto addr = reinterpret_cast<ani_long>(malloc(len));
    return addr;
}

void FreeMemImpl([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_long addr)
{
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    free(reinterpret_cast<void *>(addr));
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static const char *moduleName = "unsafe_memory";
    ani_module md;
    if (ANI_OK != env->FindModule(moduleName, &md)) {
        std::cerr << "Cannot find \"" << moduleName << "\" module!\n";
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"allocMem", "i:l", reinterpret_cast<void *>(AllocMemImpl)},
        ani_native_function {"freeMem", "l:", reinterpret_cast<void *>(FreeMemImpl)},
    };

    if (ANI_OK != env->Module_BindNativeFunctions(md, methods.data(), methods.size())) {
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}

}  // namespace ark::ets::ani

}  // extern "C"
