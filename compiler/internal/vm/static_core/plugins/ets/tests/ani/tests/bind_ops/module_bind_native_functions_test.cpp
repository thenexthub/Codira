/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"
#include <array>
#include <iostream>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)
namespace ark::ets::ani::testing {

class ModuleBindNativeFunctionsTest : public AniTest {};

static ani_int Sum([[maybe_unused]] ani_env *env, ani_int a, ani_int b)
{
    return a + b;
}

static ani_int Foo([[maybe_unused]] ani_env *env, ani_int a, ani_int b)
{
    return a + b;
}

static ani_double Abs([[maybe_unused]] ani_env *env, ani_double a)
{
    return a;
}

static ani_int SumA([[maybe_unused]] ani_env *env, ani_int a, ani_int b, ani_int c)
{
    return a + b + c;
}

static ani_int FooA([[maybe_unused]] ani_env *env, ani_int a, ani_int b, ani_int c)
{
    return a + b + c;
}

static ani_string ConcatA(ani_env *env, ani_string s0, ani_string s1, ani_string s2)
{
    std::string str0 {};
    AniTest::GetStdString(env, s0, str0);
    std::string str1 {};
    AniTest::GetStdString(env, s1, str1);
    std::string str2 {};
    AniTest::GetStdString(env, s2, str2);

    std::string newStr = str0 + str1 + str2;

    ani_string result {};
    ani_status status = env->String_NewUTF8(newStr.c_str(), newStr.length(), &result);
    if (status != ANI_OK) {
        std::cerr << "Cannot create new utf8 string" << std::endl;
        std::abort();
    }
    return result;
}

static ani_string Concat(ani_env *env, ani_string s0, ani_string s1)
{
    std::string str0 {};
    AniTest::GetStdString(env, s0, str0);
    std::string str1 {};
    AniTest::GetStdString(env, s1, str1);

    std::string newStr = str0 + str1;

    ani_string result {};
    ani_status status = env->String_NewUTF8(newStr.c_str(), newStr.length(), &result);
    if (status != ANI_OK) {
        std::cerr << "Cannot create new utf8 string" << std::endl;
        std::abort();
    }
    return result;
}

static const char *CONCAT_SIGNATURE = "C{std.core.String}C{std.core.String}:C{std.core.String}";
static const char *CONCAT_SIGNATURE_A = "C{std.core.String}C{std.core.String}C{std.core.String}:C{std.core.String}";
static const char *MODULE_NAME = "@defModule.module_bind_native_functions_test";
static const ani_native_function NATIVE_FUNC_SUM = {"sum", "ii:i", reinterpret_cast<void *>(Sum)};
static const ani_native_function NATIVE_FUNC_SUM_A = {"sum", "iii:i", reinterpret_cast<void *>(SumA)};
static const ani_native_function NATIVE_FUNC_CONCAT = {"concat", CONCAT_SIGNATURE, reinterpret_cast<void *>(Concat)};
static const ani_native_function NATIVE_FUNC_CONCAT_A = {"concat", CONCAT_SIGNATURE_A,
                                                         reinterpret_cast<void *>(ConcatA)};

TEST_F(ModuleBindNativeFunctionsTest, bind_native_functions)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    std::array functions = {
        NATIVE_FUNC_SUM,
        NATIVE_FUNC_CONCAT,
    };
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkSum"), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkConcat"), ANI_TRUE);
}

TEST_F(ModuleBindNativeFunctionsTest, already_binded_function)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    std::array functions = {
        NATIVE_FUNC_SUM,
        NATIVE_FUNC_CONCAT,
    };
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_OK);
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_ALREADY_BINDED);

    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkSum"), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkConcat"), ANI_TRUE);
}

TEST_F(ModuleBindNativeFunctionsTest, invalid_arg_module)
{
    std::array functions = {
        NATIVE_FUNC_SUM,
        NATIVE_FUNC_CONCAT,
    };
    ASSERT_EQ(env_->Module_BindNativeFunctions(nullptr, functions.data(), functions.size()), ANI_INVALID_ARGS);
}

TEST_F(ModuleBindNativeFunctionsTest, invalid_arg_functions)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ASSERT_EQ(env_->Module_BindNativeFunctions(module, nullptr, 1), ANI_INVALID_ARGS);
}

TEST_F(ModuleBindNativeFunctionsTest, function_not_found)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    std::array functions = {
        ani_native_function {"sum", "ii:i", reinterpret_cast<void *>(Sum)},
        ani_native_function {"foo1", "ii:i", reinterpret_cast<void *>(Foo)},
        ani_native_function {"foo2", "iii:i", reinterpret_cast<void *>(FooA)},
        ani_native_function {"concatX", CONCAT_SIGNATURE, reinterpret_cast<void *>(Concat)},
    };
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_NOT_FOUND);
}

// NOLINTNEXTLINE(readability-named-parameter)
static ani_int NativeMethodsFooNative(ani_env *, ani_class)
{
    const ani_int answer = 42U;
    return answer;
}

// NOLINTNEXTLINE(readability-named-parameter)
static ani_long NativeMethodsLongFooNative(ani_env *, ani_class)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    return static_cast<ani_long>(84L);
}

TEST_F(ModuleBindNativeFunctionsTest, class_bindNativeMethods_combine_scenes_001)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".test001A.TestA001");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo", ":l", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_ALREADY_BINDED);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_method intFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", nullptr, &intFooMethod), ANI_OK);
    ASSERT_NE(intFooMethod, nullptr);

    ani_int fooResult = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int(object, intFooMethod, &fooResult), ANI_OK);
    ASSERT_EQ(fooResult, 42U);

    ani_method longFooMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "long_foo", nullptr, &longFooMethod), ANI_OK);
    ASSERT_NE(longFooMethod, nullptr);

    ani_long longFooResult = 0L;
    ASSERT_EQ(env_->Object_CallMethod_Long(object, longFooMethod, &longFooResult), ANI_OK);
    ASSERT_EQ(longFooResult, 84L);
}

TEST_F(ModuleBindNativeFunctionsTest, class_bindNativeMethods_combine_scenes_001_not_found)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".test001A.TestA001");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo11", ":i", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ani_size nrMethods = 2U;
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), nrMethods), ANI_NOT_FOUND);
}

TEST_F(ModuleBindNativeFunctionsTest, class_bindNativeMethods_combine_scenes_001_already_binded)
{
    ani_class cls {};
    const std::string clsName = std::string(MODULE_NAME).append(".test001A.TestA001");
    ASSERT_EQ(env_->FindClass(clsName.c_str(), &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
        ani_native_function {"long_foo", ":l", reinterpret_cast<void *>(NativeMethodsLongFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_ALREADY_BINDED);
}

TEST_F(ModuleBindNativeFunctionsTest, module_bind_native_functions_001)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    std::array functions = {NATIVE_FUNC_SUM, NATIVE_FUNC_SUM_A, NATIVE_FUNC_CONCAT_A, NATIVE_FUNC_CONCAT};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkSum"), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkSumA"), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkConcatA"), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkConcat"), ANI_TRUE);
}

TEST_F(ModuleBindNativeFunctionsTest, module_bind_native_functions_002)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    std::array functions = {NATIVE_FUNC_SUM, NATIVE_FUNC_SUM_A, NATIVE_FUNC_CONCAT_A, NATIVE_FUNC_CONCAT};
    ASSERT_EQ(env_->c_api->Module_BindNativeFunctions(nullptr, module, functions.data(), functions.size()),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_BindNativeFunctions(nullptr, functions.data(), functions.size()), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, nullptr, functions.size()), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), 0), ANI_OK);
}

TEST_F(ModuleBindNativeFunctionsTest, module_bind_native_functions_003)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &NATIVE_FUNC_SUM, 1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkSum"), ANI_TRUE);

    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &NATIVE_FUNC_SUM_A, 1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkSumA"), ANI_TRUE);

    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &NATIVE_FUNC_CONCAT_A, 1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkConcatA"), ANI_TRUE);

    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &NATIVE_FUNC_CONCAT, 1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>(MODULE_NAME, "checkConcat"), ANI_TRUE);
}

TEST_F(ModuleBindNativeFunctionsTest, module_bind_native_functions_004)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    std::array functions = {
        ani_native_function {"undefined", "ii:i", reinterpret_cast<void *>(Sum)},
    };
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_NOT_FOUND);
}

TEST_F(ModuleBindNativeFunctionsTest, bind_intrinsic)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("std.math", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    std::array functions = {
        ani_native_function {"abs", "d:d", reinterpret_cast<void *>(Abs)},
    };

    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_ALREADY_BINDED);
}

TEST_F(ModuleBindNativeFunctionsTest, bind_wrong_signature)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule(MODULE_NAME, &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    std::array functions = {
        ani_native_function {"checkSignature", "C{std.core.String}:", reinterpret_cast<void *>(Abs)},
    };

    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_OK);

    functions = {
        ani_native_function {"checkSignature", "C{std/core/String}:", reinterpret_cast<void *>(Abs)},
    };
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_INVALID_DESCRIPTOR);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)
