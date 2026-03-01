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

class VariableSetValueByteTest : public AniTest {
public:
    void SetUp() override
    {
        AniTest::SetUp();
        ASSERT_EQ(env_->FindNamespace("variable_set_value_byte_test.anyns", &ns_), ANI_OK);
        ASSERT_NE(ns_, nullptr);
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ani_namespace ns_ {};
};

TEST_F(VariableSetValueByteTest, set_byte_value_normal_1)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aByte", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_byte value = 1U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable, value), ANI_OK);
    ani_byte result = 0;
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(VariableSetValueByteTest, invalid_env)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aByte", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_byte value = 1U;
    ASSERT_EQ(env_->c_api->Variable_SetValue_Byte(nullptr, variable, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueByteTest, invalid_variable_type)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aDouble", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_byte value = 1U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueByteTest, invalid_args_variable)
{
    const ani_byte value = 1U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(nullptr, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueByteTest, other_type_value)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aByte", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_short value = std::numeric_limits<ani_short>::max();
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable, value), ANI_OK);
    ani_byte result = 0;
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable, &result), ANI_OK);
}

TEST_F(VariableSetValueByteTest, composite_case_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("variable_set_value_byte_test.anyns.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "add", ":b", &method), ANI_OK);

    ani_byte sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, method, &sum), ANI_OK);
    ASSERT_EQ(sum, 6U);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "sum", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_byte getValue = 0;
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable, &getValue), ANI_OK);
    ASSERT_EQ(getValue, sum);

    const ani_byte value = 40U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable, value), ANI_OK);
    ani_byte result = 0;
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(VariableSetValueByteTest, composite_case_2)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aByte", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    const ani_byte values[] = {40U, 0, -10};
    ani_byte result = 0;
    for (ani_byte value : values) {
        ASSERT_EQ(env_->Variable_SetValue_Byte(variable, value), ANI_OK);
        ASSERT_EQ(env_->Variable_GetValue_Byte(variable, &result), ANI_OK);
        ASSERT_EQ(result, value);
    }
}

TEST_F(VariableSetValueByteTest, composite_case_3)
{
    ani_namespace result {};
    ASSERT_EQ(env_->FindNamespace("variable_set_value_byte_test.anyns.second", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aByte", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_byte getValue1 = 0;
    const ani_byte val1 = 30U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable1, val1), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable1, &getValue1), ANI_OK);
    ASSERT_EQ(getValue1, val1);

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(result, "aValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ani_byte getValue2 = 0;
    const ani_byte val2 = 60U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable2, val2), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable2, &getValue2), ANI_OK);
    ASSERT_EQ(getValue2, val2);
}

TEST_F(VariableSetValueByteTest, composite_case_4)
{
    ani_variable variable1 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aByte", &variable1), ANI_OK);
    ASSERT_NE(variable1, nullptr);

    ani_byte getValue1 = 0;
    const ani_byte val1 = 30U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable1, val1), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable1, &getValue1), ANI_OK);
    ASSERT_EQ(getValue1, val1);

    ani_variable variable2 {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "testValue", &variable2), ANI_OK);
    ASSERT_NE(variable2, nullptr);

    ani_byte getValue2 = 0;
    const ani_byte val2 = 60U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable2, val2), ANI_OK);
    ASSERT_EQ(env_->Variable_GetValue_Byte(variable2, &getValue2), ANI_OK);
    ASSERT_EQ(getValue2, val2);
}

TEST_F(VariableSetValueByteTest, check_initialization)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "aByte", &variable), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("variable_set_value_byte_test.anyns"));
    const ani_byte x = 24U;
    ASSERT_EQ(env_->Variable_SetValue_Byte(variable, x), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("variable_set_value_byte_test.anyns"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-identifier-naming)
