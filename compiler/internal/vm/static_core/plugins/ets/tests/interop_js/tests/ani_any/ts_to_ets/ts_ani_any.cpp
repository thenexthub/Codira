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
#include <ani.h>
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::interop::js::testing {
class NativeAniAnyTsToEtsTest : public EtsInteropTest {
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
    static void CheckANIStr(const ani_string &str, const std::string &expected)
    {
        ani_env *env {};
        ASSERT_EQ(GetAniEnv(&env), true);
        const ani_size utfBufferSize = 50U;
        std::array<char, utfBufferSize> utfBuffer = {0U};
        ani_size resultSize;
        const ani_size offset = 0;
        ASSERT_EQ(
            env->String_GetUTF8SubString(str, offset, expected.size(), utfBuffer.data(), utfBuffer.size(), &resultSize),
            ANI_OK);
        ASSERT_STREQ(utfBuffer.data(), expected.c_str());
    }
    static void CheckANIUTF16Str(const ani_string &str, std::u16string_view expected)
    {
        ani_env *env {};
        ASSERT_EQ(GetAniEnv(&env), true);
        const ani_size utfBufferSize = 50U;
        std::array<uint16_t, utfBufferSize> utfBuffer = {0U};
        ani_size resultSize;
        const ani_size offset = 0;
        ASSERT_EQ(env->String_GetUTF16SubString(str, offset, expected.size(), utfBuffer.data(), utfBuffer.size(),
                                                &resultSize),
                  ANI_OK);
        ASSERT_EQ(std::u16string_view(reinterpret_cast<const char16_t *>(utfBuffer.data()), expected.size()), expected);
    }
    static ani_boolean NativeCheckAnyGetProperty(ani_env *env, ani_ref anyRef)
    {
        std::string propName = "_name";
        std::string expectedName = "Leechy";
        ani_ref nameRef {};
        if (env->Any_GetProperty(anyRef, propName.c_str(), &nameRef) != ANI_OK) {
            return ANI_FALSE;
        }
        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(nameRef), expectedName);

        std::string propCountry = "_原產地";
        std::u16string_view expectedCountry = u"中國";
        ani_ref countryRef {};
        if (env->Any_GetProperty(anyRef, propCountry.c_str(), &countryRef) != ANI_OK) {
            return ANI_FALSE;
        }
        NativeAniAnyTsToEtsTest::CheckANIUTF16Str(static_cast<ani_string>(countryRef), expectedCountry);

        return ANI_TRUE;
    }

    static ani_boolean NativeCheckAnyGetPropertyByIndex(ani_env *env, ani_ref anyRef)
    {
        ani_ref firstTagRef {};
        if (env->Any_GetByIndex(anyRef, 0U, &firstTagRef) != ANI_OK) {
            return ANI_FALSE;
        }
        ani_ref secondTagRef {};
        if (env->Any_GetByIndex(anyRef, 1U, &secondTagRef) != ANI_OK) {
            return ANI_FALSE;
        }
        ani_ref thirdTagRef {};
        if (env->Any_GetByIndex(anyRef, 2U, &thirdTagRef) != ANI_OK) {
            return ANI_FALSE;
        }
        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(firstTagRef), "dev");
        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(secondTagRef), "ts");
        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(thirdTagRef), "ani");
        return ANI_TRUE;
    }

    static ani_boolean NativeCheckAnyGetPropertyByValue([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_ref anyRef)
    {
        std::string propName = "_name";
        std::string expectedName = "Leechy";
        ani_ref nameRef {};
        ani_string propNameStr {};
        if (env->String_NewUTF8(propName.c_str(), propName.size(), &propNameStr) != ANI_OK) {
            return ANI_FALSE;
        }
        if (env->Any_GetByValue(anyRef, static_cast<ani_ref>(propNameStr), &nameRef) != ANI_OK) {
            return ANI_FALSE;
        }
        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(nameRef), expectedName);

        return ANI_TRUE;
    }

    static ani_boolean NativeCheckAnySetProperty(ani_env *env, ani_ref anyRef)
    {
        const std::string newName = "newName";
        ani_string nameStr {};
        if (env->String_NewUTF8(newName.c_str(), newName.length(), &nameStr) != ANI_OK) {
            return ANI_FALSE;
        }

        if (env->Any_SetProperty(anyRef, "name", static_cast<ani_ref>(nameStr)) != ANI_OK) {
            return ANI_FALSE;
        }

        ani_ref valueRef {};
        if (env->Any_GetProperty(anyRef, "name", &valueRef) != ANI_OK) {
            return ANI_FALSE;
        }
        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(valueRef), newName);
        return ANI_TRUE;
    }

    static ani_boolean NativeCheckAnySetByIndex(ani_env *env, ani_ref anyRef)
    {
        std::string updated = "updated";
        ani_string strRef {};
        if (env->String_NewUTF8(updated.c_str(), updated.length(), &strRef) != ANI_OK) {
            return ANI_FALSE;
        }

        if (env->Any_SetByIndex(anyRef, 1U, static_cast<ani_ref>(strRef)) != ANI_OK) {
            return ANI_FALSE;
        }

        ani_ref valueRef {};
        if (env->Any_GetByIndex(anyRef, 1U, &valueRef) != ANI_OK) {
            return ANI_FALSE;
        }

        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(valueRef), updated);
        return ANI_TRUE;
    }

    static ani_boolean NativeCheckAnySetByValue(ani_env *env, ani_ref anyRef)
    {
        std::string keyStr = "_name";
        std::string newValue = "ArkTS";

        ani_string key {};
        ani_string val {};

        if (env->String_NewUTF8(keyStr.c_str(), keyStr.length(), &key) != ANI_OK) {
            return ANI_FALSE;
        }
        if (env->String_NewUTF8(newValue.c_str(), newValue.length(), &val) != ANI_OK) {
            return ANI_FALSE;
        }

        if (env->Any_SetByValue(anyRef, static_cast<ani_ref>(key), static_cast<ani_ref>(val)) != ANI_OK) {
            return ANI_FALSE;
        }

        ani_ref result {};
        if (env->Any_GetProperty(anyRef, "_name", &result) != ANI_OK) {
            return ANI_FALSE;
        }

        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(result), newValue);
        return ANI_TRUE;
    }

    static ani_boolean NativeCheckAnyCall(ani_env *env, ani_ref funcRef, ani_ref arg0Ref)
    {
        std::array<ani_ref, 1U> args = {arg0Ref};
        ani_ref result {};

        if (env->Any_Call(funcRef, 1U, args.data(), &result) != ANI_OK) {
            return ANI_FALSE;
        }

        std::string expected = "Hello, ";
        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(result), expected);
        return ANI_TRUE;
    }

    static ani_boolean NativeCheckAnyCallMethod(ani_env *env, ani_ref anyRef)
    {
        const char *methodName = "greet";

        std::string param = "Hello";
        ani_string strRef {};
        if (env->String_NewUTF8(param.c_str(), param.size(), &strRef) != ANI_OK) {
            return ANI_FALSE;
        }

        std::array<ani_ref, 1U> args = {static_cast<ani_ref>(strRef)};
        ani_ref result {};

        if (env->Any_CallMethod(anyRef, methodName, 1U, args.data(), &result) != ANI_OK) {
            return ANI_FALSE;
        }

        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(result), "Hello, I am Leechy");
        return ANI_TRUE;
    }

    static ani_boolean NativeCheckAnyNew(ani_env *env, ani_ref ctorRef)
    {
        std::string param = "Leechy";
        ani_string nameStr {};
        if (env->String_NewUTF8(param.c_str(), param.size(), &nameStr) != ANI_OK) {
            return ANI_FALSE;
        }

        std::array<ani_ref, 1U> args = {static_cast<ani_ref>(nameStr)};
        ani_ref result {};
        if (env->Any_New(ctorRef, 1U, args.data(), &result) != ANI_OK) {
            return ANI_FALSE;
        }

        ani_ref namePropRef {};
        if (env->Any_GetProperty(result, "name", &namePropRef) != ANI_OK) {
            return ANI_FALSE;
        }

        NativeAniAnyTsToEtsTest::CheckANIStr(static_cast<ani_string>(namePropRef), "Leechy");
        return ANI_TRUE;
    }

    static ani_boolean NativeCheckAnyInstanceOf(ani_env *env, ani_ref objRef, ani_ref typeRef)
    {
        ani_boolean isInstance = ANI_FALSE;
        if (env->Any_InstanceOf(objRef, typeRef, &isInstance) != ANI_OK) {
            return ANI_FALSE;
        }

        return isInstance == ANI_TRUE ? ANI_TRUE : ANI_FALSE;
    }

    bool RegisterETSChecker(ani_env *env)
    {
        ani_ref classRef = GetClassRefObject(env, "ets_any.ETSGLOBAL");
        if (classRef == nullptr) {
            return false;
        }

        std::array methods = {
            ani_native_function {"nativeCheckAnyGetProperty", nullptr,
                                 reinterpret_cast<void *>(NativeCheckAnyGetProperty)},
            ani_native_function {"nativeCheckAnyGetPropertyByIndex", nullptr,
                                 reinterpret_cast<void *>(NativeCheckAnyGetPropertyByIndex)},
            ani_native_function {"nativeCheckAnyGetPropertyByValue", nullptr,
                                 reinterpret_cast<void *>(NativeCheckAnyGetPropertyByValue)},
            ani_native_function {"nativeCheckAnySetProperty", nullptr,
                                 reinterpret_cast<void *>(NativeCheckAnySetProperty)},
            ani_native_function {"nativeCheckAnySetByIndex", nullptr,
                                 reinterpret_cast<void *>(NativeCheckAnySetByIndex)},
            ani_native_function {"nativeCheckAnySetByValue", nullptr,
                                 reinterpret_cast<void *>(NativeCheckAnySetByValue)},
            ani_native_function {"nativeCheckAnyCall", nullptr, reinterpret_cast<void *>(NativeCheckAnyCall)},
            ani_native_function {"nativeCheckAnyCallMethod", nullptr,
                                 reinterpret_cast<void *>(NativeCheckAnyCallMethod)},
            ani_native_function {"nativeCheckAnyNew", nullptr, reinterpret_cast<void *>(NativeCheckAnyNew)},
            ani_native_function {"nativeCheckAnyInstanceOf", nullptr,
                                 reinterpret_cast<void *>(NativeCheckAnyInstanceOf)},
        };
        return env->Module_BindNativeFunctions(static_cast<ani_module>(classRef), methods.data(), methods.size()) ==
               ANI_OK;
    }
};

TEST_F(NativeAniAnyTsToEtsTest, check_js_to_ets_ani_any)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    ASSERT_TRUE(RegisterETSChecker(aniEnv));
    ASSERT_TRUE(RunJsTestSuite("ts_ani_any.ts"));
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
}  // namespace ark::ets::interop::js::testing