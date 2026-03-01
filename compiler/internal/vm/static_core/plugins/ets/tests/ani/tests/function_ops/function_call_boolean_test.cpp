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

class FunctionCallBooleanTest : public AniTest {
public:
    static constexpr ani_int BOOLEAN_VAL1 = 2;
    static constexpr ani_int BOOLEAN_VAL2 = 1;
    static constexpr ani_int BOOLEAN_VAL3 = 2;

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_boolean_test.ops", &ns), ANI_OK);
        ASSERT_NE(ns, nullptr);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "or", "zz:z", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallBooleanTest, function_call_boolean)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Function_Call_Boolean(env_, fn, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean(fn, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fn, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_invalid_function)
{
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Function_Call_Boolean(env_, nullptr, &result, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_EQ(env_->c_api->Function_Call_Boolean(env_, fn, nullptr, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_a_invalid_function)
{
    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean_A(nullptr, &result, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ASSERT_EQ(env_->Function_Call_Boolean_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fn, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_001)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_boolean_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "booleanFunctionA", "ii:z", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, BOOLEAN_VAL1, BOOLEAN_VAL3), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[2U];
    args[0U].i = BOOLEAN_VAL1;
    args[1U].i = BOOLEAN_VAL3;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_002)
{
    ani_namespace nA {};
    ani_namespace nB {};
    ani_function fB {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_boolean_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_boolean_test.A.B", &nB), ANI_OK);
    ASSERT_NE(nB, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nB, "booleanFunctionB", "ii:z", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean(fB, &value, BOOLEAN_VAL1, BOOLEAN_VAL2), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[2U];
    args[0U].i = BOOLEAN_VAL1;
    args[1U].i = BOOLEAN_VAL2;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fB, &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_003)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_boolean_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "booleanFunctionA", "iii:z", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, BOOLEAN_VAL1, BOOLEAN_VAL2, BOOLEAN_VAL3), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[3U];
    args[0U].i = BOOLEAN_VAL1;
    args[1U].i = BOOLEAN_VAL2;
    args[2U].i = BOOLEAN_VAL3;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_004)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_boolean_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fA {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "i:z", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, BOOLEAN_VAL1), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[1U];
    args[0U].i = BOOLEAN_VAL1;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_005)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_boolean_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_boolean value = ANI_FALSE;
    ani_function fB {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "ii:z", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_value argsB[2U];
    argsB[0U].i = BOOLEAN_VAL1;
    argsB[1U].i = BOOLEAN_VAL2;
    ASSERT_EQ(env_->Function_Call_Boolean(fB, &value, BOOLEAN_VAL1, BOOLEAN_VAL2), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
    ASSERT_EQ(env_->Function_Call_Boolean_A(fB, &value, argsB), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_006)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_boolean_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "nestedFunction", "ii:z", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, BOOLEAN_VAL1, BOOLEAN_VAL2), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[2U];
    args[0U].i = BOOLEAN_VAL1;
    args[1U].i = BOOLEAN_VAL2;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_007)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_boolean_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "recursiveFunction", "i:z", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_boolean value = ANI_FALSE;
    const ani_int value1 = 5;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, value1), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_008)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_boolean_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "calculateBoolean", "iicd:z", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_boolean value = ANI_FALSE;
    const ani_char cValue = 'A';
    const ani_double dValue = 1.0;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, BOOLEAN_VAL1, BOOLEAN_VAL2, cValue, dValue), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    const ani_char cValue1 = 'B';
    ani_value args[4U];
    args[0U].i = BOOLEAN_VAL1;
    args[1U].i = BOOLEAN_VAL2;
    args[2U].c = cValue1;
    args[3U].d = dValue;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    const ani_double dValue1 = 2.0;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, BOOLEAN_VAL1, BOOLEAN_VAL2, cValue1, dValue1), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_009)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_boolean_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "booleanFunctionA", "ii:z", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, BOOLEAN_VAL1, BOOLEAN_VAL2), ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    const ani_int value1 = 5;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, value1, BOOLEAN_VAL2), ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    const ani_int value2 = 2;
    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    const ani_int value3 = 3;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, value2, value3), ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    const ani_int value4 = 3;
    ASSERT_EQ(env_->Function_Call_Boolean(fA, &value, value3, value4), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(FunctionCallBooleanTest, function_call_boolean_010)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Function_Call_Boolean(nullptr, fn, &result, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Function_Call_Boolean_A(nullptr, fn, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Boolean(nullptr, &result, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Boolean_A(nullptr, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Boolean(fn, nullptr, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Boolean_A(fn, nullptr, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Boolean(fn, &result, nullptr), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Boolean_A(fn, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallBooleanTest, check_initialization_bool)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_boolean_test.ops"));
    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean(fn, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_boolean_test.ops"));
}

TEST_F(FunctionCallBooleanTest, check_initialization_bool_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_boolean_test.ops"));
    ani_boolean result = ANI_FALSE;
    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ASSERT_EQ(env_->Function_Call_Boolean_A(fn, &result, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_boolean_test.ops"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
