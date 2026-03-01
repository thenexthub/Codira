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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)
namespace ark::ets::ani::testing {

class VariableSetValueBooleanTest : public AniTest {
public:
    void SetUp() override
    {
        AniTest::SetUp();
        ASSERT_EQ(env_->FindNamespace("variable_set_value_boolean_test.anyns", &ns_), ANI_OK);
        ASSERT_NE(ns_, nullptr);
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ani_namespace ns_ {};
};

TEST_F(VariableSetValueBooleanTest, set_boolean_value_normal_1)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aBool", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable, value), ANI_OK);
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(VariableSetValueBooleanTest, invalid_env)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aBool", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->c_api->Variable_SetValue_Boolean(nullptr, variable, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueBooleanTest, invalid_variable_type)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aByte", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueBooleanTest, invalid_args_variable)
{
    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(nullptr, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueBooleanTest, other_type_value)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aBool", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_int value = 10U;
    ASSERT_EQ(env_->c_api->Variable_SetValue_Boolean(env_, variable, value), ANI_OK);
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable, &result), ANI_OK);
}

TEST_F(VariableSetValueBooleanTest, composite_case_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("variable_set_value_boolean_test.anyns.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "add", ":z", &method), ANI_OK);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, &result), ANI_OK);
    ASSERT_TRUE(result);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "result", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_boolean getValue1 = ANI_FALSE;
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable, &getValue1), ANI_OK);
    ASSERT_EQ(getValue1, result);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable, value), ANI_OK);
    ani_boolean getValue2 = ANI_FALSE;
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable, &getValue2), ANI_OK);
    ASSERT_EQ(getValue2, value);
}

TEST_F(VariableSetValueBooleanTest, composite_case_2)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aBool", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_boolean values[] = {ANI_TRUE, ANI_FALSE, ANI_TRUE};
    ani_boolean result = ANI_FALSE;
    for (ani_boolean value : values) {
        ASSERT_EQ(env_->Variable_SetValue_Boolean(variable, value), ANI_OK);
        ASSERT_EQ(env_->Variable_GetValue_Boolean(variable, &result), ANI_OK);
        ASSERT_EQ(result, value);
    }
}

TEST_F(VariableSetValueBooleanTest, composite_case_3)
{
    ani_namespace result {};
    ASSERT_EQ(env_->FindNamespace("variable_set_value_boolean_test.anyns.second", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aBool", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_boolean getValue1 = ANI_FALSE;
    ani_boolean val1 = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable1, val1), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable1, &getValue1), ANI_OK);
    ASSERT_EQ(getValue1, val1);

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(result, "aValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ani_boolean getValue2 = ANI_FALSE;
    ani_boolean val2 = ANI_FALSE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable2, val2), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable2, &getValue2), ANI_OK);
    ASSERT_EQ(getValue2, val2);
}

TEST_F(VariableSetValueBooleanTest, composite_case_4)
{
    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aBool", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_boolean getValue1 = ANI_FALSE;
    ani_boolean val1 = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable1, val1), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable1, &getValue1), ANI_OK);
    ASSERT_EQ(getValue1, val1);

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "testValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ani_boolean getValue2 = ANI_FALSE;
    ani_boolean val2 = ANI_FALSE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable2, val2), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Boolean(variable2, &getValue2), ANI_OK);
    ASSERT_EQ(getValue2, val2);
}

TEST_F(VariableSetValueBooleanTest, check_initialization)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aBool", &variable), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("variable_set_value_boolean_test.anyns"));
    const ani_boolean x = ANI_TRUE;
    ASSERT_EQ(env_->Variable_SetValue_Boolean(variable, x), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("variable_set_value_boolean_test.anyns"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)