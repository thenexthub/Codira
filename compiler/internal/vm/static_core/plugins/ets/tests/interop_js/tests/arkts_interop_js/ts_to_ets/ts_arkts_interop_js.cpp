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

#include <thread>

#include <ani.h>

#include "ets_interop_js_gtest.h"
#include "libani_helpers/interop_js/arkts_esvalue.h"
#include "libani_helpers/interop_js/arkts_interop_js_api.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::interop::js::testing {

class ArktsNapiScopeTest : public EtsInteropTest {
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

    static constexpr uintptr_t dummyPoiner = 0x1234;

    static bool CheckNativePointer(void *data)
    {
        return reinterpret_cast<uintptr_t>(data) == dummyPoiner;
    }

    static void DummyFinalizer([[maybe_unused]] napi_env env, [[maybe_unused]] void *data, [[maybe_unused]] void *hint)
    {
        ASSERT_TRUE(CheckNativePointer(data));
    }

    static ani_ref CreateJsObject(ani_env *env)
    {
        ani_ref undefinedRef {};
        env->GetUndefined(&undefinedRef);
        ani_ref anyRef {};
        {
            napi_env napiEnv {};
            if (!arkts_napi_scope_open(env, &napiEnv)) {
                return undefinedRef;
            }

            napi_value jsObj {};
            if (napi_create_object(napiEnv, &jsObj) != napi_ok) {
                return undefinedRef;
            }
            // Use N-API which don't have counterparts in other ArkTS interfaces
            auto status =
                napi_wrap(napiEnv, jsObj, reinterpret_cast<void *>(dummyPoiner), DummyFinalizer, nullptr, nullptr);
            if (status != napi_ok) {
                return undefinedRef;
            }

            napi_value nameValue {};
            if (napi_create_string_utf8(napiEnv, "jsProperty", NAPI_AUTO_LENGTH, &nameValue) != napi_ok) {
                return undefinedRef;
            }
            if (napi_set_named_property(napiEnv, jsObj, "jsProperty", nameValue) != napi_ok) {
                return undefinedRef;
            }
            if (!arkts_napi_scope_close_n(napiEnv, 1, &jsObj, &anyRef)) {
                return undefinedRef;
            }
        }
        return anyRef;
    }

    static ani_boolean CheckCreatedJsObject(ani_env *env, ani_object esValue)
    {
        void *data = nullptr;
        if (!arkts_esvalue_unwrap(env, esValue, &data)) {
            return ANI_FALSE;
        }
        return CheckNativePointer(data) ? ANI_TRUE : ANI_FALSE;
    }

    ani_status RegisterNativeFunctions(ani_env *env)
    {
        ani_ref classRef = GetClassRefObject(env, "ets_functions.ETSGLOBAL");
        if (classRef == nullptr) {
            return ANI_ERROR;
        }
        std::array methods = {
            ani_native_function {"createJsObject", nullptr, reinterpret_cast<void *>(CreateJsObject)},
            ani_native_function {"checkCreatedJsObject", nullptr, reinterpret_cast<void *>(CheckCreatedJsObject)},
        };
        return env->Module_BindNativeFunctions(static_cast<ani_module>(classRef), methods.data(), methods.size());
    }
};

TEST_F(ArktsNapiScopeTest, CreateAnyFromJSObject)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    ASSERT_EQ(RegisterNativeFunctions(aniEnv), ANI_OK);
    ASSERT_TRUE(RunJsTestSuite("ts_arkts_interop_js.ts"));
}

TEST_F(ArktsNapiScopeTest, CheckScopeOpenApi)
{
    EXPECT_FALSE(arkts_napi_scope_open(nullptr, nullptr));
    napi_env jsEnv {};
    EXPECT_FALSE(arkts_napi_scope_open(nullptr, &jsEnv));
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    EXPECT_FALSE(arkts_napi_scope_open(aniEnv, nullptr));

    ASSERT_TRUE(arkts_napi_scope_open(aniEnv, &jsEnv));
    ASSERT_TRUE(arkts_napi_scope_close_n(jsEnv, 0, nullptr, nullptr));

    // Check that scope can be opened once again
    ASSERT_TRUE(arkts_napi_scope_open(aniEnv, &jsEnv));
    ASSERT_TRUE(arkts_napi_scope_close_n(jsEnv, 0, nullptr, nullptr));
}

TEST_F(ArktsNapiScopeTest, CheckScopeOpenApiFromThreads)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));

    // Check that scope cannot be opened in unattached thread
    std::thread unattachedThread([aniEnv]() {
        napi_env jsEnv {};
        ASSERT_FALSE(arkts_napi_scope_open(aniEnv, &jsEnv));
    });
    unattachedThread.join();

    // Check that scope can be opened in a newly attached thread
    ani_vm *vm = nullptr;
    ani_size result = 0;
    ASSERT_EQ(ANI_GetCreatedVMs(&vm, 1, &result), ANI_OK);
    ASSERT_EQ(result, 1);
    std::thread attachedThread([vm]() {
        ani_option interopEnabled {"--interop=enable", nullptr};
        ani_options aniArgs {1, &interopEnabled};
        ani_env *env = nullptr;
        ASSERT_EQ(vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env), ANI_OK);
        napi_env jsEnv {};
        ASSERT_TRUE(arkts_napi_scope_open(env, &jsEnv));
        ASSERT_TRUE(arkts_napi_scope_close_n(jsEnv, 0, nullptr, nullptr));
        ASSERT_EQ(vm->DetachCurrentThread(), ANI_OK);
    });
    attachedThread.join();
}

static void CheckAnyRef(ani_env *env, ani_ref anyRef, std::string_view testString)
{
    ani_boolean isNullish = ANI_FALSE;
    ASSERT_EQ(env->Reference_IsNullishValue(anyRef, &isNullish), ANI_OK);
    ASSERT_EQ(isNullish, ANI_FALSE);
    // const esValue = ESValue.wrap(anyRef)
    ani_class esValueClass {};
    ASSERT_EQ(env->FindClass("std.interop.ESValue", &esValueClass), ANI_OK);
    ani_ref esValue {};
    ASSERT_EQ(env->Class_CallStaticMethodByName_Ref(esValueClass, "wrap", "C{std.core.Object}:C{std.interop.ESValue}",
                                                    &esValue, anyRef),
              ANI_OK);
    // const property = esValue.getPropertySafe(testString)
    ani_string propertyName {};
    ASSERT_EQ(env->String_NewUTF8(testString.data(), testString.size(), &propertyName), ANI_OK);
    ani_ref property {};
    ASSERT_EQ(env->Object_CallMethodByName_Ref(static_cast<ani_object>(esValue), "getPropertySafe",
                                               "C{std.core.String}:C{std.interop.ESValue}", &property, propertyName),
              ANI_OK);
    // const propertyAsString = property.toString()
    ani_ref propertyAsString {};
    ASSERT_EQ(env->Object_CallMethodByName_Ref(static_cast<ani_object>(property), "toString", ":C{std.core.String}",
                                               &propertyAsString),
              ANI_OK);
    // assertEQ(propertyAsString, testString)
    ani_size stringSize = 0;
    ASSERT_EQ(env->String_GetUTF8Size(static_cast<ani_string>(propertyAsString), &stringSize), ANI_OK);
    std::string buffer(stringSize + 1, 0);
    ani_size written = 0;
    ASSERT_EQ(env->String_GetUTF8(static_cast<ani_string>(propertyAsString), buffer.data(), buffer.size(), &written),
              ANI_OK);
    ASSERT_EQ(written, stringSize);
    buffer.resize(written);
    ASSERT_EQ(buffer, testString);
}

TEST_F(ArktsNapiScopeTest, CheckScopeCloseNApi)
{
    static constexpr std::string_view TEST_STRING1 = "jsProperty1";
    static constexpr std::string_view TEST_STRING2 = "jsProperty2";

    std::array<ani_ref, 2U> anyRefs {};
    EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, 0, nullptr, nullptr));
    EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, 1, nullptr, nullptr));
    EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, 0, nullptr, anyRefs.data()));
    EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, 1, nullptr, anyRefs.data()));

    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    {
        napi_env jsEnv {};
        ASSERT_TRUE(arkts_napi_scope_open(aniEnv, &jsEnv));

        std::array<napi_value, 2U> jsValues {};
        ASSERT_EQ(napi_create_object(jsEnv, &jsValues[0]), napi_ok);
        ASSERT_EQ(napi_create_object(jsEnv, &jsValues[1]), napi_ok);
        napi_value nameValue {};
        ASSERT_EQ(napi_create_string_utf8(jsEnv, TEST_STRING1.data(), NAPI_AUTO_LENGTH, &nameValue), napi_ok);
        ASSERT_EQ(napi_set_named_property(jsEnv, jsValues[0], TEST_STRING1.data(), nameValue), napi_ok);
        ASSERT_EQ(napi_create_string_utf8(jsEnv, TEST_STRING2.data(), NAPI_AUTO_LENGTH, &nameValue), napi_ok);
        ASSERT_EQ(napi_set_named_property(jsEnv, jsValues[1], TEST_STRING2.data(), nameValue), napi_ok);

        EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, 0, jsValues.data(), nullptr));
        EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, 0, nullptr, anyRefs.data()));
        EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, 0, jsValues.data(), anyRefs.data()));
        EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, jsValues.size(), jsValues.data(), nullptr));
        EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, jsValues.size(), nullptr, anyRefs.data()));
        EXPECT_FALSE(arkts_napi_scope_close_n(nullptr, jsValues.size(), jsValues.data(), anyRefs.data()));
        EXPECT_FALSE(arkts_napi_scope_close_n(jsEnv, jsValues.size(), jsValues.data(), nullptr));
        EXPECT_FALSE(arkts_napi_scope_close_n(jsEnv, jsValues.size(), nullptr, anyRefs.data()));

        ASSERT_TRUE(arkts_napi_scope_close_n(jsEnv, jsValues.size(), jsValues.data(), anyRefs.data()));
    }

    CheckAnyRef(aniEnv, anyRefs[0], TEST_STRING1);
    CheckAnyRef(aniEnv, anyRefs[1], TEST_STRING2);
}

}  // namespace ark::ets::interop::js::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
