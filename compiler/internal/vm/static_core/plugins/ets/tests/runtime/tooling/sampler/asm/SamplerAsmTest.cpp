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

#include "SamplerAsmTest.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"

#include <iostream>
#include <array>

#ifdef __cplusplus
extern "C" {
#endif

// This function have 8 arguments in order to check the case
// when some arguments are put not into registers but on stack.
// CC-OFFNXT(G.FUN.01) test logic
int SumEightElements(int, int, int, int, int, int, int, int);

// NOLINTBEGIN(readability-magic-numbers, readability-named-parameter, cppcoreguidelines-pro-type-vararg,
// hicpp-signed-bitwice)

ANI_EXPORT ani_int NativeSumEightElements([[maybe_unused]] ani_env *)
{
    constexpr int MULT = 8;
    constexpr int OFF = 28;
    constexpr int ITERS = 10000000;
    for (size_t j = 0; j < 200U; j++) {
        for (int i = 0; i < ITERS; ++i) {
            int val = SumEightElements(i, i + 1U, i + 2U, i + 3U, i + 4U, i + 5U, i + 6U, i + 7U);
            if (val != MULT * i + OFF) {
                std::cerr << val << std::endl;
                return 1;
            }
        }
    }

    return 0;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static constexpr std::string_view MODULE_NAME = "SamplerAsmTest";
    ani_module md;
    if (ANI_OK != env->FindModule(MODULE_NAME.data(), &md)) {
        std::string msg = "Cannot find \"" + std::string(MODULE_NAME) + "\" module!";
        ark::ets::stdlib::ThrowNewError(env, "std.core.RuntimeException", msg.c_str(), "C{std.core.String}:");
        return ANI_ERROR;
    }

    const auto functions = std::array {
        ani_native_function {"NativeSumEightElements", ":i", reinterpret_cast<void *>(NativeSumEightElements)}};

    if (ANI_OK != env->Module_BindNativeFunctions(md, functions.data(), functions.size())) {
        std::string msg = "Cannot bind native functions to '" + std::string(MODULE_NAME) + "'";
        ark::ets::stdlib::ThrowNewError(env, "std.core.RuntimeException", msg.c_str(), "C{std.core.String}:");
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}

// NOLINTEND(readability-magic-numbers, readability-named-parameter, cppcoreguidelines-pro-type-vararg,
// hicpp-signed-bitwice)

#ifdef __cplusplus
}  // extern "C"
#endif
