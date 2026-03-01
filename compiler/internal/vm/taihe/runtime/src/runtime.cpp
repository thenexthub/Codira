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
#include <taihe/runtime.hpp>

#include <iostream>

namespace taihe {
ani_vm *global_vm = nullptr;

void set_vm(ani_vm *vm)
{
    global_vm = vm;
}

ani_vm *get_vm()
{
    return global_vm;
}

static ani_error create_ani_error(ani_env *env, taihe::string_view msg)
{
    ani_class errCls;
    char const *className = "escompat.Error";
    if (ANI_OK != env->FindClass(className, &errCls)) {
        std::cerr << "Not found '" << className << std::endl;
        return nullptr;
    }

    ani_method errCtor;
    if (ANI_OK != env->Class_FindMethod(errCls, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &errCtor)) {
        std::cerr << "Get Ctor Failed '" << className << "'" << std::endl;
        return nullptr;
    }

    ani_string result_string {};
    env->String_NewUTF8(msg.c_str(), msg.size(), &result_string);

    ani_ref undefined;
    env->GetUndefined(&undefined);

    ani_error errObj;
    if (ANI_OK != env->Object_New(errCls, errCtor, reinterpret_cast<ani_object *>(&errObj), result_string, undefined)) {
        std::cerr << "Create Object Failed '" << className << "'" << std::endl;
        return nullptr;
    }
    return errObj;
}

static ani_error create_ani_business_error(ani_env *env, int32_t code, taihe::string_view msg)
{
    ani_class errCls;
    char const *className = "@ohos.base.BusinessError";
    if (ANI_OK != env->FindClass(className, &errCls)) {
        std::cerr << "Not found '" << className << std::endl;
        return nullptr;
    }

    ani_method errCtor;
    if (ANI_OK != env->Class_FindMethod(errCls, "<ctor>", "iC{escompat.Error}:", &errCtor)) {
        std::cerr << "Get Ctor Failed '" << className << "'" << std::endl;
        return nullptr;
    }

    ani_error errObj = create_ani_error(env, msg);
    ani_int errCode = static_cast<ani_int>(code);

    ani_error businessErrObj;
    if (ANI_OK != env->Object_New(errCls, errCtor, reinterpret_cast<ani_object *>(&businessErrObj), errCode, errObj)) {
        std::cerr << "Create Object Failed '" << className << "'" << std::endl;
        return nullptr;
    }
    return businessErrObj;
}

void ani_set_error(ani_env *env, taihe::string_view msg)
{
    ani_error errObj = create_ani_error(env, msg);
    env->ThrowError(errObj);
}

void ani_set_business_error(ani_env *env, int32_t code, taihe::string_view msg)
{
    ani_error businessErrObj = create_ani_business_error(env, code, msg);
    env->ThrowError(businessErrObj);
}

void ani_reset_error(ani_env *env)
{
    env->ResetError();
}

bool ani_has_error(ani_env *env)
{
    ani_boolean res;
    env->ExistUnhandledError(&res);
    return res;
}

void set_error(taihe::string_view msg)
{
    env_guard guard;
    ani_env *env = guard.get_env();
    ani_set_error(env, msg);
}

void set_business_error(int32_t code, taihe::string_view msg)
{
    env_guard guard;
    ani_env *env = guard.get_env();
    ani_set_business_error(env, code, msg);
}

void reset_error()
{
    env_guard guard;
    ani_env *env = guard.get_env();
    ani_reset_error(env);
}

bool has_error()
{
    env_guard guard;
    ani_env *env = guard.get_env();
    return ani_has_error(env);
}
}  // namespace taihe