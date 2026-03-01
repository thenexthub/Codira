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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/libani_helpers/interop_js/hybridgref_ani.h"
#include "plugins/ets/runtime/libani_helpers/interop_js/hybridgref_napi.h"
#include <ani.h>

namespace ark::ets::interop::js::testing {
static hybridgref g_jsToEtsRef = nullptr;

static napi_value NativeSaveRef(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];  // NOLINT(modernize-avoid-c-arrays)
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        return undefined;
    }

    if (g_jsToEtsRef != nullptr) {
        hybridgref_delete_from_napi(env, g_jsToEtsRef);
        g_jsToEtsRef = nullptr;
    }

    bool ok = hybridgref_create_from_napi(env, argv[0], &g_jsToEtsRef);
    napi_value result;
    napi_get_boolean(env, ok, &result);
    return result;
}

static ani_object NativeGetRef(ani_env *env)
{
    ani_object result {};
    hybridgref_get_esvalue(env, g_jsToEtsRef, &result);
    return result;
}

class NativeGrefTsToEtsTest : public EtsInteropTest {
public:
    static bool GetAniEnv(ani_env **env)
    {
        ani_vm *aniVm;
        ani_size res;

        auto status = ANI_GetCreatedVMs(&aniVm, 1U, &res);
        if (status != ANI_OK || res == 0) {
            return false;
        }

        status = aniVm->GetEnv(ANI_VERSION_1, env);
        return status == ANI_OK && *env != nullptr;
    }

    static bool RegisterNativeSaveRef(napi_env env)
    {
        napi_value global;
        if (napi_get_global(env, &global) != napi_ok) {
            return false;
        }

        napi_value fn;
        if (napi_create_function(env, "nativeSaveRef", NAPI_AUTO_LENGTH, NativeSaveRef, nullptr, &fn) != napi_ok) {
            return false;
        }

        return napi_set_named_property(env, global, "nativeSaveRef", fn) == napi_ok;
    }

    bool RegisterETSGetter(ani_env *env)
    {
        ani_ref classRef = GetClassRefObject(env, "ets_functions.ETSGLOBAL");
        if (classRef == nullptr) {
            return false;
        }

        std::array methods = {
            ani_native_function {"nativeGetRef", nullptr, reinterpret_cast<void *>(NativeGetRef)},
        };
        return env->Module_BindNativeFunctions(static_cast<ani_module>(classRef), methods.data(), methods.size()) ==
               ANI_OK;
    }
};

TEST_F(NativeGrefTsToEtsTest, check_js_to_ets_hybridgref)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    napi_env napiEnv = GetJsEnv();

    ASSERT_TRUE(RegisterNativeSaveRef(napiEnv));
    ASSERT_TRUE(RegisterETSGetter(aniEnv));
    ASSERT_TRUE(RunJsTestSuite("ts_hybridgref.ts"));
}

TEST_F(NativeGrefTsToEtsTest, hybridgref_create_from_napi_invalid_args)
{
    napi_env env = GetJsEnv();
    napi_value dummyValue {};
    hybridgref ref = nullptr;

    ASSERT_FALSE(hybridgref_create_from_napi(nullptr, dummyValue, &ref));
    ASSERT_FALSE(hybridgref_create_from_napi(env, dummyValue, nullptr));
    ASSERT_FALSE(hybridgref_create_from_napi(nullptr, nullptr, nullptr));
}

TEST_F(NativeGrefTsToEtsTest, hybridgref_delete_from_napi_invalid_args)
{
    napi_env env = GetJsEnv();
    constexpr uintptr_t DUMMY_NATIVE_POINTER = 0x12345678;
    auto dummyRef = reinterpret_cast<hybridgref>(DUMMY_NATIVE_POINTER);

    ASSERT_FALSE(hybridgref_delete_from_napi(nullptr, dummyRef));
    ASSERT_FALSE(hybridgref_delete_from_napi(env, nullptr));
}

TEST_F(NativeGrefTsToEtsTest, hybridgref_get_napi_value_invalid_args)
{
    napi_env env = GetJsEnv();
    napi_value result {};
    constexpr uintptr_t DUMMY_NATIVE_POINTER = 0x12345678;
    auto dummyRef = reinterpret_cast<hybridgref>(DUMMY_NATIVE_POINTER);

    ASSERT_FALSE(hybridgref_get_napi_value(nullptr, dummyRef, &result));
    ASSERT_FALSE(hybridgref_get_napi_value(env, dummyRef, nullptr));
    ASSERT_FALSE(hybridgref_get_napi_value(env, nullptr, &result));
}
}  // namespace ark::ets::interop::js::testing