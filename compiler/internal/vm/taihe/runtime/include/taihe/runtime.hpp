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
#ifndef RUNTIME_INCLUDE_TAIHE_RUNTIME_HPP_
#define RUNTIME_INCLUDE_TAIHE_RUNTIME_HPP_
// NOLINTBEGIN

#if __has_include(<ani.h>)
#include <ani.h>
#elif __has_include(<ani/ani.h>)
#include <ani/ani.h>
#else
#error "ani.h not found. Please ensure the Ani SDK is correctly installed."
#endif

#include <taihe/object.hpp>
#include <taihe/string.hpp>

namespace taihe {
// Error handling functions

void set_error(taihe::string_view msg);
void set_business_error(int32_t err_code, taihe::string_view msg);
void reset_error();
bool has_error();
}  // namespace taihe

namespace taihe {
// VM and Environment related functions

void set_vm(ani_vm *vm);
ani_vm *get_vm();

inline ani_env *get_env()
{
    ani_env *env = nullptr;
    get_vm()->GetEnv(ANI_VERSION_1, &env);
    return env;
}

class env_guard {
    ani_env *env = nullptr;
    bool is_temporary;

public:
    env_guard()
    {
        is_temporary = get_vm()->GetEnv(ANI_VERSION_1, &env) != ANI_OK;
        if (is_temporary) {
            get_vm()->AttachCurrentThread(nullptr, ANI_VERSION_1, &env);
        }
    }

    ~env_guard()
    {
        if (is_temporary) {
            get_vm()->DetachCurrentThread();
        }
    }

    env_guard(env_guard const &) = delete;
    env_guard &operator=(env_guard const &) = delete;
    env_guard(env_guard &&) = delete;
    env_guard &operator=(env_guard &&) = delete;

    ani_env *get_env()
    {
        return env;
    }
};
}  // namespace taihe
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_RUNTIME_HPP_