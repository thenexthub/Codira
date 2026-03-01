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
#include "mate.ani.hpp"
#include "mate.bar.ani.hpp"
#include "mate.foo.ani.hpp"
#include "nova.ani.hpp"
#include "pura.ani.hpp"
#include "pura.baz.ani.hpp"
#include "test.inner.ani.hpp"

#if __has_include(<ani.h>)
#include <ani.h>
#elif __has_include(<ani/ani.h>)
#include <ani/ani.h>
#else
#error "ani.h not found. Please ensure the Ani SDK is correctly installed."
#endif

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    if (ANI_OK != mate::foo::ANIRegister(env)) {
        std::cerr << "Error from mate::foo::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != mate::bar::ANIRegister(env)) {
        std::cerr << "Error from mate::bar::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != mate::ANIRegister(env)) {
        std::cerr << "Error from mate::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != pura::ANIRegister(env)) {
        std::cerr << "Error from pura::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != test::inner::ANIRegister(env)) {
        std::cerr << "Error from test::inner::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != pura::baz::ANIRegister(env)) {
        std::cerr << "Error from pura::baz::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    if (ANI_OK != nova::ANIRegister(env)) {
        std::cerr << "Error from nova::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}