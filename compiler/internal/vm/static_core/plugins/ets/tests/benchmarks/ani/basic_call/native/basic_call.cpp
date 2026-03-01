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

#include "plugins/ets/runtime/ani/ani.h"
#include <iostream>
#include <array>

extern "C" {
// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_s(ani_env *env, [[maybe_unused]] ani_class self, [[maybe_unused]] ani_long a,
                        [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_long basic_call_sc([[maybe_unused]] ani_long a, [[maybe_unused]] ani_int b, [[maybe_unused]] ani_long c)
{
    return c;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_sf(ani_env *env, [[maybe_unused]] ani_class, [[maybe_unused]] ani_long a,
                         [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_v(ani_env *env, [[maybe_unused]] ani_object, [[maybe_unused]] ani_long a,
                        [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_vf(ani_env *env, [[maybe_unused]] ani_object, [[maybe_unused]] ani_long a,
                         [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_v_final(ani_env *env, [[maybe_unused]] ani_object, [[maybe_unused]] ani_long a,
                              [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_vf_final(ani_env *env, [[maybe_unused]] ani_object, [[maybe_unused]] ani_long a,
                               [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_long basic_call_baseline(ani_env *env, [[maybe_unused]] ani_class)
{
    return 1;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cout << "Wrong version of ANI!\n";
        return ANI_ERROR;
    }
    ani_class cls;
    if (ANI_OK != env->FindClass("$bench_name.BasicCall", &cls)) {
        std::cout << "Class BasicCall not found!\n";
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"basic_call_v", "lC{std.core.String}C{std.core.Object}:C{std.core.String}",
                             reinterpret_cast<void *>(basic_call_v)},
        ani_native_function {"basic_call_vf", "lC{std.core.String}C{std.core.Object}:C{std.core.String}",
                             reinterpret_cast<void *>(basic_call_vf)},
        ani_native_function {"basic_call_v_final", "lC{std.core.String}C{std.core.Object}:C{std.core.String}",
                             reinterpret_cast<void *>(basic_call_v_final)},
        ani_native_function {"basic_call_vf_final", "lC{std.core.String}C{std.core.Object}:C{std.core.String}",
                             reinterpret_cast<void *>(basic_call_vf_final)},
    };
    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        std::cout << "Binding native methods failed!\n";
        return ANI_ERROR;
    }
    std::array staticMethods = {
        ani_native_function {"basic_call_s", "lC{std.core.String}C{std.core.Object}:C{std.core.String}",
                             reinterpret_cast<void *>(basic_call_s)},
        ani_native_function {"basic_call_sc", "lil:l", reinterpret_cast<void *>(basic_call_sc)},
        ani_native_function {"basic_call_sf", "lC{std.core.String}C{std.core.Object}:C{std.core.String}",
                             reinterpret_cast<void *>(basic_call_sf)},
        ani_native_function {"basic_call_baseline", ":l", reinterpret_cast<void *>(basic_call_baseline)},
    };
    if (ANI_OK != env->Class_BindStaticNativeMethods(cls, staticMethods.data(), staticMethods.size())) {
        std::cout << "Binding native methods failed!\n";
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}

// NOLINTEND(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)

}  // extern "C"
