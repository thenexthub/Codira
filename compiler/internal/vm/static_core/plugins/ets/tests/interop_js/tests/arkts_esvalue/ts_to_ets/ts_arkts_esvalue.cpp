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
#include "plugins/ets/runtime/libani_helpers/interop_js/arkts_esvalue.h"
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
void NoopFinalize(napi_env env, void *finalizeData, void *finalizeHint)
{
    (void)env;
    (void)finalizeData;
    (void)finalizeHint;
}

static napi_value NativeWrapRef(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];  // NOLINT(modernize-avoid-c-arrays)
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return nullptr;
    }
    napi_valuetype valuetype;
    status = napi_typeof(env, argv[0], &valuetype);
    if (status != napi_ok) {
        return nullptr;
    }

    bool isObject = (valuetype == napi_object);

    if (!isObject) {
        return nullptr;
    }
    constexpr uintptr_t DUMMY_NATIVE_POINTER = 0x12345678;
    void *nativePtr = reinterpret_cast<void *>(DUMMY_NATIVE_POINTER);
    status = napi_wrap(env, argv[0], nativePtr, NoopFinalize, nullptr, nullptr);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static ani_long NativeGetWrappedPtr([[maybe_unused]] ani_env *env, ani_object esValue)
{
    void *nativePtr = nullptr;
    if (!arkts_esvalue_unwrap(env, esValue, &nativePtr) || nativePtr == nullptr) {
        return 0;
    }
    return reinterpret_cast<ani_long>(nativePtr);
}

static ani_object NativeGetRef(ani_env *env)
{
    ani_object result {};
    hybridgref_get_esvalue(env, g_jsToEtsRef, &result);
    return result;
}

class NativeArktsESvalueTsToEtsTest : public EtsInteropTest {
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

    static bool RegisterNativeWrapRef(napi_env env)
    {
        napi_value global;
        if (napi_get_global(env, &global) != napi_ok) {
            return false;
        }

        napi_value fn;
        if (napi_create_function(env, "nativeWrapRef", NAPI_AUTO_LENGTH, NativeWrapRef, nullptr, &fn) != napi_ok) {
            return false;
        }

        return napi_set_named_property(env, global, "nativeWrapRef", fn) == napi_ok;
    }

    bool RegisterETSGetter(ani_env *env)
    {
        ani_ref classRef = GetClassRefObject(env, "ets_functions.ETSGLOBAL");
        if (classRef == nullptr) {
            return false;
        }

        std::array methods = {
            ani_native_function {"nativeGetRef", nullptr, reinterpret_cast<void *>(NativeGetRef)},
            ani_native_function {"nativeGetWrappedPtr", nullptr, reinterpret_cast<void *>(NativeGetWrappedPtr)},
        };
        return env->Module_BindNativeFunctions(static_cast<ani_module>(classRef), methods.data(), methods.size()) ==
               ANI_OK;
    }
};

TEST_F(NativeArktsESvalueTsToEtsTest, check_js_to_ets_arkts_value)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    napi_env napiEnv = GetJsEnv();

    ASSERT_TRUE(RegisterNativeSaveRef(napiEnv));
    ASSERT_TRUE(RegisterNativeWrapRef(napiEnv));
    ASSERT_TRUE(RegisterETSGetter(aniEnv));
    ASSERT_TRUE(RunJsTestSuite("ts_arkts_esvalue.ts"));
}

TEST_F(NativeArktsESvalueTsToEtsTest, UnwrapESValue_InvalidArgs)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));

    ani_ref dummyObj = nullptr;
    void *resultPtr = nullptr;
    ASSERT_EQ(aniEnv->GetUndefined(&dummyObj), ANI_OK);
    ASSERT_FALSE(arkts_esvalue_unwrap(nullptr, static_cast<ani_object>(dummyObj), &resultPtr));
    ASSERT_FALSE(arkts_esvalue_unwrap(aniEnv, nullptr, &resultPtr));
    ASSERT_FALSE(arkts_esvalue_unwrap(aniEnv, static_cast<ani_object>(dummyObj), nullptr));
}
}  // namespace ark::ets::interop::js::testing