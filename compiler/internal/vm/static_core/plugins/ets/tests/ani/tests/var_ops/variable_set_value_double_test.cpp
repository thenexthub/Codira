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
#include <climits>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)
namespace ark::ets::ani::testing {

class VariableSetValueDoubleTest : public AniTest {
public:
    void SetUp() override
    {
        AniTest::SetUp();
        ASSERT_EQ(env_->FindNamespace("variable_set_value_double_test.anyns", &ns_), ANI_OK);
        ASSERT_NE(ns_, nullptr);
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ani_namespace ns_ {};
};

TEST_F(VariableSetValueDoubleTest, set_double_value_normal_1)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aDouble", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_double value = 2.0F;
    ASSERT_EQ(env_->Variable_SetValue_Double(variable, value), ANI_OK);
    ani_double result = 0.0F;
    ASSERT_EQ(env_->Variable_GetValue_Double(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(VariableSetValueDoubleTest, invalid_env)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aDouble", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_double value = 2.0F;
    ASSERT_EQ(env_->c_api->Variable_SetValue_Double(nullptr, variable, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueDoubleTest, set_double_value_invalid_env)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aDouble", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_double value = 2.0F;
    ASSERT_EQ(env_->c_api->Variable_SetValue_Double(nullptr, variable, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueDoubleTest, invalid_variable_type)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aBool", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_double value = 2.0F;
    ASSERT_EQ(env_->Variable_SetValue_Double(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueDoubleTest, invalid_args_variable)
{
    const ani_double value = 2.0F;
    ASSERT_EQ(env_->Variable_SetValue_Double(nullptr, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueDoubleTest, composite_case_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("variable_set_value_double_test.anyns.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "add", ":d", &method), ANI_OK);

    ani_double sum = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, method, &sum), ANI_OK);
    ASSERT_EQ(sum, 4.0F);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "sum", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_double getValue = 0.0F;
    ASSERT_EQ(env_->Variable_GetValue_Double(variable, &getValue), ANI_OK);
    ASSERT_EQ(getValue, sum);

    const ani_double value = 3.14F;
    ASSERT_EQ(env_->Variable_SetValue_Double(variable, value), ANI_OK);
    ani_double result = 0.0F;
    ASSERT_EQ(env_->Variable_GetValue_Double(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(VariableSetValueDoubleTest, composite_case_2)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aDouble", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_double values[] = {3.14F, 0, -9.42F};
    ani_double result = 0.0F;
    for (ani_double value : values) {
        ASSERT_EQ(env_->Variable_SetValue_Double(variable, value), ANI_OK);
        ASSERT_EQ(env_->Variable_GetValue_Double(variable, &result), ANI_OK);
        ASSERT_EQ(result, value);
    }
}

TEST_F(VariableSetValueDoubleTest, composite_case_3)
{
    ani_namespace result {};
    ASSERT_EQ(env_->FindNamespace("variable_set_value_double_test.anyns.second", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aDouble", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_double getValue1 = 0.0F;
    const ani_double val1 = 3.14F;
    ASSERT_EQ(env_->Variable_SetValue_Double(variable1, val1), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Double(variable1, &getValue1), ANI_OK);
    ASSERT_EQ(getValue1, val1);

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(result, "aValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ani_double getValue2 = 0.0F;
    const ani_double val2 = 6.28F;
    ASSERT_EQ(env_->Variable_SetValue_Double(variable2, val2), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Double(variable2, &getValue2), ANI_OK);
    ASSERT_EQ(getValue2, val2);
}

TEST_F(VariableSetValueDoubleTest, composite_case_4)
{
    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aDouble", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_double getValue1 = 0.0F;
    const ani_double val1 = 3.14F;
    ASSERT_EQ(env_->Variable_SetValue_Double(variable1, val1), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Double(variable1, &getValue1), ANI_OK);
    ASSERT_EQ(getValue1, val1);

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "testValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ani_double getValue2 = 0.0F;
    const ani_double val2 = 6.28F;
    ASSERT_EQ(env_->Variable_SetValue_Double(variable2, val2), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Double(variable2, &getValue2), ANI_OK);
    ASSERT_EQ(getValue2, val2);
}

TEST_F(VariableSetValueDoubleTest, check_initialization)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aDouble", &variable), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("variable_set_value_double_test.anyns"));
    const ani_double x = 12.07F;
    ASSERT_EQ(env_->Variable_SetValue_Double(variable, x), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("variable_set_value_double_test.anyns"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)