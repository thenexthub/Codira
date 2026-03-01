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

class VariableSetValueIntTest : public AniTest {
public:
    void SetUp() override
    {
        AniTest::SetUp();
        ASSERT_EQ(env_->FindNamespace("variable_set_value_int_test.anyns", &ns_), ANI_OK);
        ASSERT_NE(ns_, nullptr);
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ani_namespace ns_ {};
};

TEST_F(VariableSetValueIntTest, set_int_value_normal_1)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "intValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);
    ani_int result = 0;
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &result), ANI_OK);
    ASSERT_NE(result, 3U);

    const ani_int value = 6U;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, value), ANI_OK);
    result = 0U;
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(VariableSetValueIntTest, invalid_env)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "intValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_int value = 6U;
    ASSERT_EQ(env_->c_api->Variable_SetValue_Int(nullptr, variable, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueIntTest, invalid_variable_type)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "floatValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_int value = 6U;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueIntTest, invalid_args_variable)
{
    const ani_int value = 6U;
    ASSERT_EQ(env_->Variable_SetValue_Int(nullptr, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueIntTest, other_type_value)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "intValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_long value = std::numeric_limits<int64_t>::max();
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, value), ANI_OK);
    ani_int result = 0;
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &result), ANI_OK);
}

TEST_F(VariableSetValueIntTest, composite_case_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("variable_set_value_int_test.anyns.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "add", ":i", &method), ANI_OK);

    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method, &sum), ANI_OK);
    ASSERT_EQ(sum, 3U);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "sum", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_int getValue = 0;
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &getValue), ANI_OK);
    ASSERT_EQ(getValue, sum);

    const ani_int value = 3U;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, value), ANI_OK);
    ani_int result = 0;
    ASSERT_EQ(env_->Variable_GetValue_Int(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(VariableSetValueIntTest, composite_case_2)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "intValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_int values[] = {3U, 0, -9};
    ani_int result = 0;
    for (ani_int value : values) {
        ASSERT_EQ(env_->Variable_SetValue_Int(variable, value), ANI_OK);
        ASSERT_EQ(env_->Variable_GetValue_Int(variable, &result), ANI_OK);
        ASSERT_EQ(result, value);
    }
}

TEST_F(VariableSetValueIntTest, composite_case_3)
{
    ani_namespace result {};
    ASSERT_EQ(env_->FindNamespace("variable_set_value_int_test.anyns.second", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "intValue", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_int getValue1 = 0;
    const ani_int val1 = 3U;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable1, val1), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Int(variable1, &getValue1), ANI_OK);
    ASSERT_EQ(getValue1, val1);

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(result, "aValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ani_int getValue2 = 0;
    const ani_int val2 = 6U;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable2, val2), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Int(variable2, &getValue2), ANI_OK);
    ASSERT_EQ(getValue2, val2);
}

TEST_F(VariableSetValueIntTest, composite_case_4)
{
    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "intValue", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_int getValue1 = 0;
    const ani_int val1 = 3U;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable1, val1), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Int(variable1, &getValue1), ANI_OK);
    ASSERT_EQ(getValue1, val1);

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "testValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ani_int getValue2 = 0;
    const ani_int val2 = 6U;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable2, val2), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Int(variable2, &getValue2), ANI_OK);
    ASSERT_EQ(getValue2, val2);
}

TEST_F(VariableSetValueIntTest, check_initialization)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "intValue", &variable), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("variable_set_value_int_test.anyns"));
    const ani_int x = 217U;
    ASSERT_EQ(env_->Variable_SetValue_Int(variable, x), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("variable_set_value_int_test.anyns"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)
