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

#include "ani/ani.h"
#include <iostream>
#include <array>

#ifdef __cplusplus
extern "C" {
#endif

// NOLINTBEGIN(readability-magic-numbers, readability-named-parameter, cppcoreguidelines-pro-type-vararg,
// hicpp-signed-bitwice)

static ani_int NativeSlowFunction([[maybe_unused]] ani_env * /*unused*/, ani_int iterations)
{
    ani_int res = 0;

    for (ani_int i = 0; i < iterations; i++) {
        ++res;
    }

    for (size_t k = 0; k < 4U; ++k) {
        for (ani_int i = 0; i < iterations; i++) {
            res = res ^ i;
        }
    }
    // res = 20'000'000-200'000'000
    return res;
}

static ani_int NativeFastFunction([[maybe_unused]] ani_env * /*unused*/)
{
    ani_int res = 1;
    return res;
}

static ani_int NativeANISlowFunction(ani_env *env, ani_int iterations)
{
    ani_int res = 0;

    ani_module module;
    static const char *moduleName = "SamplerAniTest";
    auto status = env->FindModule(moduleName, &module);
    if (status != ANI_OK) {
        std::cerr << "Not found '" << moduleName << "'" << std::endl;
        return ANI_ERROR;
    }

    ani_function fn;
    status = env->Module_FindFunction(module, "SlowETSFunction", "i:i", &fn);
    if (status != ANI_OK) {
        std::cerr << "Function 'SlowETSFunction' not found" << std::endl;
        return ANI_ERROR;
    }

    status = env->Function_Call_Int(fn, &res, iterations);
    if (status != ANI_OK) {
        std::cerr << "Failed function call: 'Function_Call_Int'" << std::endl;
        return ANI_ERROR;
    }
    return res;
}

static ani_int NativeANIFastFunction(ani_env *env, ani_int iterations)
{
    ani_module module;
    static const char *moduleName = "SamplerAniTest";
    auto status = env->FindModule(moduleName, &module);
    if (status != ANI_OK) {
        std::cerr << "Not found '" << moduleName << "'" << std::endl;
        return ANI_ERROR;
    }

    ani_function fn;
    status = env->Module_FindFunction(module, "FastETSFunction", ":i", &fn);
    if (status != ANI_OK) {
        std::cerr << "Function 'FastETSFunction' not found" << std::endl;
        return ANI_ERROR;
    }

    ani_int res = 0;
    ani_int local = 0;
    for (int i = 0; i < iterations; ++i) {
        if (ANI_OK != env->Function_Call_Int(fn, &local)) {
            std::cerr << "Failed function call: 'Function_Call_Int'" << std::endl;
            return ANI_ERROR;
        }
        res += local;
    }
    return res;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    ani_module module;
    static const char *moduleName = "SamplerAniTest";
    auto status = env->FindModule(moduleName, &module);
    if (status != ANI_OK) {
        std::cerr << "Not found '" << moduleName << "'" << std::endl;
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"NativeSlowFunction", "i:i", reinterpret_cast<void *>(NativeSlowFunction)},
        ani_native_function {"NativeFastFunction", ":i", reinterpret_cast<void *>(NativeFastFunction)},
        ani_native_function {"NativeANIFastFunction", "i:i", reinterpret_cast<void *>(NativeANIFastFunction)},
        ani_native_function {"NativeANISlowFunction", "i:i", reinterpret_cast<void *>(NativeANISlowFunction)},
    };

    if (ANI_OK != env->Module_BindNativeFunctions(module, methods.data(), methods.size())) {
        std::cerr << "Cannot bind native methods to '" << moduleName << "'" << std::endl;
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}

// NOLINTEND(readability-magic-numbers, readability-named-parameter, cppcoreguidelines-pro-type-vararg,
// hicpp-signed-bitwice)

#ifdef __cplusplus
}
#endif
