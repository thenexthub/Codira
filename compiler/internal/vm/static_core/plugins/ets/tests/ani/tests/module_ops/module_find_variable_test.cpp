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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class ModuleFindVariableTest : public AniTest {};

TEST_F(ModuleFindVariableTest, get_int_variable)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleX", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
    const ani_int y = 0;
    ani_int x = 1;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, y), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &x), ANI_OK);
    ASSERT_EQ(x, y);
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleS", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(ModuleFindVariableTest, get_ref_variable)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXS", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(ModuleFindVariableTest, get_invalid_variable_name)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "sss", &variable), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindVariable(module, "", &variable), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Module_FindVariable(module, nullptr, &variable), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindVariableTest, invalid_args_result)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXS", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindVariableTest, invalid_env)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_variable variable {};
    ASSERT_EQ(env_->c_api->Module_FindVariable(nullptr, module, "moduleXS", &variable), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindVariableTest, invalid_module)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(nullptr, "moduleXS", &variable), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindVariableTest, find_int_variable)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ani_variable variable1 {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleX", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
    variable1 = variable;
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleSSSSSS", &variable), ANI_NOT_FOUND);
    ASSERT_EQ(variable, variable1);
}

TEST_F(ModuleFindVariableTest, many_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_variable variable {};
    char end = 'J';
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        std::string str = "module";
        str += static_cast<char>(random() % (end - 'A') + 'A');
        ASSERT_EQ(env_->Module_FindVariable(module, str.c_str(), &variable), ANI_OK);
    }
}

TEST_F(ModuleFindVariableTest, find_variable_B_in_namespace_A)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleT", &variable), ANI_NOT_FOUND);

    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("@abcModule.module_find_variable_test.ops", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "moduleT", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
}

TEST_F(ModuleFindVariableTest, find_const_variable)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXB", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_int y = 0;
    ani_int x = 1;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, y), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &x), ANI_OK);
    ASSERT_EQ(x, y);
}

TEST_F(ModuleFindVariableTest, combine_test_boolean)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleK", &variable), ANI_OK);

    ani_boolean result = ANI_TRUE;
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable, &result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable, value), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(ModuleFindVariableTest, combine_test_char)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleL", &variable), ANI_OK);

    ani_char result = '\0';
    ASSERT_EQ(env_->Variable_GetValue_Char(variable, &result), ANI_OK);
    ASSERT_EQ(result, 'G');
    ani_char value = 'w';
    ASSERT_EQ(env_->Variable_SetValue_Char(variable, value), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Char(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(ModuleFindVariableTest, combine_test_byte)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleM", &variable), ANI_OK);

    ani_byte result = 0;
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable, &result), ANI_OK);
    ASSERT_EQ(result, 2U);
    ani_byte value = 8U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable, value), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(ModuleFindVariableTest, combine_test_short)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleN", &variable), ANI_OK);

    ani_short result = 0;
    ASSERT_EQ(env_->Variable_GetValue_Short(variable, &result), ANI_OK);
    ASSERT_EQ(result, 10U);
    ani_short value = 20U;
    ASSERT_EQ(env_->Variable_SetValue_Short(variable, value), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Short(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(ModuleFindVariableTest, combine_test_long)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleO", &variable), ANI_OK);

    ani_long result = 0;
    ASSERT_EQ(env_->Variable_GetValue_Long(variable, &result), ANI_OK);
    ASSERT_EQ(result, 100U);
    ani_long value = 200U;
    ASSERT_EQ(env_->Variable_SetValue_Long(variable, value), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Long(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(ModuleFindVariableTest, combine_test_float)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleP", &variable), ANI_OK);

    ani_float result = 0.0F;
    ASSERT_EQ(env_->Variable_GetValue_Float(variable, &result), ANI_OK);
    ASSERT_EQ(result, 3.14F);
    ani_float value = 6.28F;
    ASSERT_EQ(env_->Variable_SetValue_Float(variable, value), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Float(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(ModuleFindVariableTest, combine_test_double)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleQ", &variable), ANI_OK);

    ani_double result = 0.0;
    ani_double value = 6.28;  // 6.28 is the test value
    ASSERT_EQ(env_->Variable_GetValue_Double(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
    value = 3.14;  // 3.14 is the test value
    ASSERT_EQ(env_->Variable_SetValue_Double(variable, value), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Double(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(ModuleFindVariableTest, combine_test_int)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleX", &variable), ANI_OK);

    ani_int result = 0U;
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &result), ANI_OK);
    ASSERT_EQ(result, 3U);
    ani_int value = 6U;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, value), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(ModuleFindVariableTest, combine_test_ref)
{
    ani_module module;
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXS", &variable), ANI_OK);

    ani_ref result = nullptr;
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &result), ANI_OK);

    std::string str {};
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "abc");

    ani_string string = {};
    auto status = env_->String_NewUTF8("hello", 5U, &string);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, string), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Ref(variable, &result), ANI_OK);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "hello");
}

TEST_F(ModuleFindVariableTest, check_initialization)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_variable_test", &module), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_variable_test", false));
    ani_variable variable {};
    ASSERT_EQ(env_->Module_FindVariable(module, "moduleXS", &variable), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_variable_test", false));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
