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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ModuleFindFunctionTest : public AniTest {
public:
    void CheckFunIntValue(ani_function fn, int value, ...)  // NOLINT(cppcoreguidelines-pro-type-vararg)
    {
        ani_int result = 0;
        va_list args;  // NOLINT(cppcoreguidelines-pro-type-vararg)
        va_start(args, value);
        ASSERT_EQ(env_->c_api->Function_Call_Int_V(env_, fn, &result, args), ANI_OK);
        va_end(args);
        ASSERT_EQ(result, value);
    }

    void CheckFunStringValue(ani_function fn, const char *value)
    {
        ani_ref ref {};
        ASSERT_EQ(env_->c_api->Function_Call_Ref(env_, fn, &ref), ANI_OK);
        std::string result {};
        GetStdString(static_cast<ani_string>(ref), result);
        ASSERT_STREQ(result.c_str(), value);
    }
};

TEST_F(ModuleFindFunctionTest, find_int_function)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialIntValue", ":i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    CheckFunIntValue(fn, 0);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValue", "C{std.core.String}:i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    const std::string example {"example"};
    ani_string string = nullptr;
    auto status = env_->String_NewUTF8(example.c_str(), example.size(), &string);
    ASSERT_EQ(status, ANI_OK);
    const int stringReturnValue = 1;                  // 1 function getIntValue(string): int, return 1
    CheckFunIntValue(fn, stringReturnValue, string);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValue", "i:i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    const int intValue = 0;             // 0 function getIntValue(int): int, return 0
    CheckFunIntValue(fn, intValue, 0);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValue", ":i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    const int value = 2;          // 2 function getIntValue(): int, return 2
    CheckFunIntValue(fn, value);  // NOLINT(cppcoreguidelines-pro-type-vararg)
}

TEST_F(ModuleFindFunctionTest, find_ref_function)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialStringValue", ":C{std.core.String}", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    CheckFunStringValue(fn, "a");
}

TEST_F(ModuleFindFunctionTest, invalid_arg_moduleName)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialBool", ":C{std.core.String}", &fn), ANI_NOT_FOUND);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_signature)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialStringValue", nullptr, &fn), ANI_OK);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_module)
{
    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(nullptr, "getInitialStringValue", ":C{std.core.String}", &fn),
              ANI_INVALID_ARGS);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_name)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "", ":C{std.core.String}", &fn), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindFunction(module, nullptr, ":C{std.core.String}", &fn), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_name_in_namespace)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValueOps", ":i", &fn), ANI_NOT_FOUND);
}

TEST_F(ModuleFindFunctionTest, invalid_arg_result)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialStringValue", ":C{std.core.String}", nullptr),
              ANI_INVALID_ARGS);
}

TEST_F(ModuleFindFunctionTest, invalid_env)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_function fn {};
    ASSERT_EQ(env_->c_api->Module_FindFunction(nullptr, module, "getInitialStringValue", ":C{std.core.String}", &fn),
              ANI_INVALID_ARGS);
}

TEST_F(ModuleFindFunctionTest, duplicate_no_signature)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "overloaded", nullptr, &fn), ANI_AMBIGUOUS);
}

TEST_F(ModuleFindFunctionTest, find_function)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ani_function fn1 {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialIntValue", ":i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    fn1 = fn;
    ASSERT_EQ(env_->Module_FindFunction(module, "getInitialIntValuexxxx", ":i", &fn), ANI_NOT_FOUND);
    ASSERT_EQ(fn, fn1);
}

TEST_F(ModuleFindFunctionTest, many_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_function fn {};
    char end = 'J';
    const int32_t loopCount = 3;
    for (int i = 0; i < loopCount; i++) {
        std::string str = "getIntValue";
        str += static_cast<char>(random() % (end - 'A') + 'A');
        ASSERT_EQ(env_->Module_FindFunction(module, str.c_str(), "i:", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);
    }
}

TEST_F(ModuleFindFunctionTest, find_func_all_Type)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValue", "i:i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
    fn = nullptr;

    ani_class kclass {};
    ASSERT_EQ(env_->FindClass("@abcModule.module_find_function_test.TestA", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);

    ani_static_method staticMethod {};
    ASSERT_EQ(env_->Class_FindStaticMethod(kclass, "addIntValue", "ii:i", &staticMethod), ANI_OK);
    ASSERT_NE(staticMethod, nullptr);
    fn = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "async_f", "i:C{std.core.Promise}", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(ModuleFindFunctionTest, find_func_B_in_namespace_A)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValueOps", ":i", &fn), ANI_NOT_FOUND);

    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("@abcModule.module_find_function_test.ops", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(ns, "getIntValueOps", ":i", &fn), ANI_OK);
    ASSERT_NE(fn, nullptr);
}

TEST_F(ModuleFindFunctionTest, check_initialization)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_function_test", &module), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_function_test", false));
    ani_function fn {};
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntValue", "i:i", &fn), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_function_test", false));
}

}  // namespace ark::ets::ani::testing
