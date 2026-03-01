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
static hybridgref g_ref = nullptr;
static napi_value NativeGetRef(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result {};
    if (!hybridgref_get_napi_value(env, g_ref, &result)) {
        napi_get_undefined(env, &result);
    }
    return result;
}

static void NativeSaveHybridGref(ani_env *env, ani_object ref)
{
    ASSERT_TRUE(hybridgref_create_from_ani(env, static_cast<ani_ref>(ref), &g_ref));
}

class HybridGrefPrimitiveEtsToTsTest : public EtsInteropTest {
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

    static bool RegisterNativeGetRef(napi_env env)
    {
        napi_value global;
        if (napi_get_global(env, &global) != napi_ok) {
            return false;
        }
        napi_value fn;
        if (napi_create_function(env, "nativeGetRef", NAPI_AUTO_LENGTH, NativeGetRef, nullptr, &fn) != napi_ok) {
            return false;
        }

        if (napi_set_named_property(env, global, "nativeGetRef", fn) != napi_ok) {
            return false;
        }

        return true;
    }

    bool RegisterNativeSaveRef(ani_env *env)
    {
        ani_ref classRef = GetClassRefObject(env, "ets_functions.ETSGLOBAL");
        if (classRef == nullptr) {
            return false;
        }
        std::array methods = {
            ani_native_function {"saveHybridGref", nullptr, reinterpret_cast<void *>(NativeSaveHybridGref)},
        };
        ani_status status =
            env->Module_BindNativeFunctions(static_cast<ani_module>(classRef), methods.data(), methods.size());
        return status == ANI_OK;
    }
};

TEST_F(HybridGrefPrimitiveEtsToTsTest, check_ets_hybridgref)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    napi_env napiEnv = GetJsEnv();

    ASSERT_TRUE(RegisterNativeGetRef(napiEnv));
    ASSERT_TRUE(RegisterNativeSaveRef(aniEnv));
    ASSERT_TRUE(RunJsTestSuite("ets_hybridgref.ts"));
}

TEST_F(HybridGrefPrimitiveEtsToTsTest, hybridgref_create_from_ani_invalid_args)
{
    hybridgref ref = nullptr;
    constexpr uintptr_t DUMMY_NATIVE_POINTER = 0x12345678;
    ASSERT_FALSE(hybridgref_create_from_ani(nullptr, reinterpret_cast<ani_ref>(DUMMY_NATIVE_POINTER), &ref));

    ani_env *env = nullptr;
    ASSERT_TRUE(GetAniEnv(&env));
    ASSERT_FALSE(hybridgref_create_from_ani(env, reinterpret_cast<ani_ref>(DUMMY_NATIVE_POINTER), nullptr));
}

TEST_F(HybridGrefPrimitiveEtsToTsTest, hybridgref_get_esvalue_invalid_args)
{
    ani_env *env = nullptr;
    ASSERT_TRUE(GetAniEnv(&env));
    constexpr uintptr_t DUMMY_NATIVE_POINTER = 0x12345678;
    auto dummyRef = reinterpret_cast<hybridgref>(DUMMY_NATIVE_POINTER);

    ani_object result = nullptr;
    ASSERT_FALSE(hybridgref_get_esvalue(nullptr, dummyRef, &result));
    ASSERT_FALSE(hybridgref_get_esvalue(env, nullptr, &result));
    ASSERT_FALSE(hybridgref_get_esvalue(env, dummyRef, nullptr));
}

TEST_F(HybridGrefPrimitiveEtsToTsTest, hybridgref_delete_from_ani_invalid_args)
{
    ani_env *env = nullptr;
    ASSERT_TRUE(GetAniEnv(&env));
    constexpr uintptr_t DUMMY_NATIVE_POINTER = 0x12345678;
    auto dummyRef = reinterpret_cast<hybridgref>(DUMMY_NATIVE_POINTER);

    ASSERT_FALSE(hybridgref_delete_from_ani(nullptr, dummyRef));
    ASSERT_FALSE(hybridgref_delete_from_ani(env, nullptr));
}

}  // namespace ark::ets::interop::js::testing