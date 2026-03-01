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

// NOLINTBEGIN(readability-identifier-naming, misc-non-private-member-variables-in-classes)
namespace ark::ets::ani::testing {

class VariableGetValueLongTest : public AniTest {
public:
    void SetUp() override
    {
        AniTest::SetUp();
        ASSERT_EQ(env_->FindNamespace("variable_get_value_long_test.anyns", &ns_), ANI_OK);
        ASSERT_NE(ns_, nullptr);
    }

    ani_namespace ns_ {};
};

TEST_F(VariableGetValueLongTest, get_long_value_normal_1)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "x", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_long x = 0L;
    ASSERT_EQ(env_->Variable_GetValue_Long(variable, &x), ANI_OK);
    ASSERT_EQ(x, 3L);
}

TEST_F(VariableGetValueLongTest, invalid_env)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "x", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_long x = 0L;
    ASSERT_EQ(env_->c_api->Variable_GetValue_Long(nullptr, variable, &x), ANI_INVALID_ARGS);
}

TEST_F(VariableGetValueLongTest, invalid_variable_type)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "z", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_long x = 0L;
    ASSERT_EQ(env_->Variable_GetValue_Long(variable, &x), ANI_INVALID_TYPE);
}

TEST_F(VariableGetValueLongTest, invalid_args_variable)
{
    ani_long x = 0L;
    ASSERT_EQ(env_->Variable_GetValue_Long(nullptr, &x), ANI_INVALID_ARGS);
}

TEST_F(VariableGetValueLongTest, invalid_args_value)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "x", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ASSERT_EQ(env_->Variable_GetValue_Long(variable, nullptr), ANI_INVALID_ARGS);
}

TEST_F(VariableGetValueLongTest, check_initialization)
{
    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns_, "x", &variable), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("variable_get_value_long_test.anyns"));
    ani_long x;
    ASSERT_EQ(env_->Variable_GetValue_Long(variable, &x), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("variable_get_value_long_test.anyns"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(readability-identifier-naming, misc-non-private-member-variables-in-classes)
