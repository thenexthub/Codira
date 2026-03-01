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

#include "plugins/ets/runtime/ets_stdlib_cache.h"
#include "libarkbase/utils/logger.h"

namespace ark::ets {

static void GetGlobalRef(ani_env *env, const char *classDescriptor, ani_class *result)
{
    ani_class cls;
    ani_status status = env->FindClass(classDescriptor, &cls);
    LOG_IF(status != ANI_OK, FATAL, ANI) << "Cannot find class: name=" << classDescriptor << " status=" << status;
    status = env->GlobalReference_Create(cls, reinterpret_cast<ani_ref *>(result));
    LOG_IF(status != ANI_OK, FATAL, ANI) << "Cannot create global reference to class: name=" << classDescriptor
                                         << " status=" << status;
}

static void GetGlobalRef(ani_env *env, const char *moduleDescriptor, ani_module *result)
{
    ani_module m;
    ani_status status = env->FindModule(moduleDescriptor, &m);
    LOG_IF(status != ANI_OK, FATAL, ANI) << "Cannot find module: name=" << moduleDescriptor << " status=" << status;
    status = env->GlobalReference_Create(m, reinterpret_cast<ani_ref *>(result));
    LOG_IF(status != ANI_OK, FATAL, ANI) << "Cannot create global reference to module: name=" << moduleDescriptor
                                         << " status=" << status;
}

static void CacheMethod(ani_env *env, ani_class cls, const char *methodName, const char *signature, ani_method *result)
{
    [[maybe_unused]] auto status = env->Class_FindMethod(cls, methodName, signature, result);
    ASSERT(status == ANI_OK);
    if (*result == nullptr) {
        LOG(FATAL, ANI) << "Cannot find method: name =" << methodName;
    }
}

static void CacheVariable(ani_env *env, ani_module module, const char *varName, ani_variable *result)
{
    [[maybe_unused]] auto status = env->Module_FindVariable(module, varName, result);
    ASSERT(status == ANI_OK);
    if (*result == nullptr) {
        LOG(FATAL, ANI) << "Cannot find variable: name =" << varName;
    }
}

PandaUniquePtr<StdlibCache> CreateStdLibCache(ani_env *env)
{
    auto stdlibCache = MakePandaUnique<StdlibCache>();

    // Cache modules
    GetGlobalRef(env, "std.core", &stdlibCache->std_core);

    // Cache classes
    GetGlobalRef(env, "std.core.StringBuilder", &stdlibCache->std_core_String_Builder);
    GetGlobalRef(env, "std.core.Console", &stdlibCache->std_core_Console);
    GetGlobalRef(env, "std.core.Array", &stdlibCache->escompat_Array);

    // Cache methods
    CacheMethod(env, stdlibCache->std_core_Console, "error",
                "C{std.core.Array}:", &stdlibCache->std_core_Console_error);
    CacheMethod(env, stdlibCache->std_core_String_Builder, "<ctor>", ":",
                &stdlibCache->std_core_String_Builder_default_ctor);
    CacheMethod(env, stdlibCache->std_core_String_Builder, "toString", ":C{std.core.String}",
                &stdlibCache->std_core_String_Builder_toString);
    CacheMethod(env, stdlibCache->std_core_String_Builder, "append",
                "C{std.core.String}C{std.core.String}C{std.core.String}:C{std.core.StringBuilder}",
                &stdlibCache->std_core_String_Builder_append);
    CacheMethod(env, stdlibCache->escompat_Array, "pushOne", "C{std.core.Object}:i",
                &stdlibCache->escompat_Array_pushOne);

    // Cache variables
    CacheVariable(env, stdlibCache->std_core, "console", &stdlibCache->std_core_console);
    return stdlibCache;
}

}  // namespace ark::ets
