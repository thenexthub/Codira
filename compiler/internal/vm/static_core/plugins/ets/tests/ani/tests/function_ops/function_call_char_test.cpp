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

class FunctionCallCharTest : public AniTest {
public:
    static constexpr ani_char CHAR_VAL1 = 'C';
    static constexpr ani_char CHAR_VAL2 = 'A';
    static constexpr ani_char CHAR_VAL3 = 'B';

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_char_test.ops", &ns), ANI_OK);
        ASSERT_NE(ns, nullptr);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sub", "cc:c", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallCharTest, function_call_char)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_char sub = CHAR_VAL3;
    ASSERT_EQ(env_->c_api->Function_Call_Char(env_, fn, &sub, CHAR_VAL1, CHAR_VAL2), ANI_OK);
    ASSERT_EQ(sub, CHAR_VAL1 - CHAR_VAL2);
}

TEST_F(FunctionCallCharTest, function_call_char_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ani_char sub = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char(fn, &sub, CHAR_VAL1, CHAR_VAL2), ANI_OK);
    ASSERT_EQ(sub, CHAR_VAL1 - CHAR_VAL2);
}

TEST_F(FunctionCallCharTest, function_call_char_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].c = CHAR_VAL1;
    args[1U].c = CHAR_VAL2;

    ani_char sub = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char_A(fn, &sub, args), ANI_OK);
    ASSERT_EQ(sub, args[0U].b - args[1U].b);
}

TEST_F(FunctionCallCharTest, function_call_char_invalid_function)
{
    ani_char sub = CHAR_VAL3;
    ASSERT_EQ(env_->c_api->Function_Call_Char(env_, nullptr, &sub, CHAR_VAL1, CHAR_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->c_api->Function_Call_Char(env_, fn, nullptr, CHAR_VAL1, CHAR_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_v_invalid_function)
{
    ani_char sub = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char(nullptr, &sub, CHAR_VAL1, CHAR_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->Function_Call_Char(fn, nullptr, CHAR_VAL1, CHAR_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_a_invalid_function)
{
    ani_value args[2U];
    args[0U].c = CHAR_VAL2;
    args[1U].c = CHAR_VAL1;

    ani_char sub = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char_A(nullptr, &sub, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].c = CHAR_VAL2;
    args[1U].c = CHAR_VAL1;

    ASSERT_EQ(env_->Function_Call_Char_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_char sub = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char_A(fn, &sub, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, function_call_char_001)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_char_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "charFunctionA", "cc:c", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_char value = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, CHAR_VAL1, CHAR_VAL2), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);

    ani_value args[2U];
    args[0U].c = CHAR_VAL1;
    args[1U].c = CHAR_VAL2;
    ASSERT_EQ(env_->Function_Call_Char_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);
}

TEST_F(FunctionCallCharTest, function_call_char_002)
{
    ani_namespace nA {};
    ani_namespace nB {};
    ani_function fB {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_char_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_char_test.A.B", &nB), ANI_OK);
    ASSERT_NE(nB, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nB, "charFunctionB", "cc:c", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_char value = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char(fB, &value, CHAR_VAL1, CHAR_VAL2), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);

    ani_value args[2U];
    args[0U].c = CHAR_VAL1;
    args[1U].c = CHAR_VAL2;
    ASSERT_EQ(env_->Function_Call_Char_A(fB, &value, args), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);
}

TEST_F(FunctionCallCharTest, function_call_char_003)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_char_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "charFunctionA", "ccc:c", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_char value = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, CHAR_VAL1, CHAR_VAL2, CHAR_VAL3), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);

    ani_value args[3U];
    args[0U].c = CHAR_VAL1;
    args[1U].c = CHAR_VAL2;
    args[2U].c = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);
}

TEST_F(FunctionCallCharTest, function_call_char_004)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_char_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fA {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "cc:c", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_char value = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, CHAR_VAL1, CHAR_VAL2), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);

    ani_value args[2U];
    args[0U].c = CHAR_VAL1;
    args[1U].c = CHAR_VAL2;
    ASSERT_EQ(env_->Function_Call_Char_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);
}

TEST_F(FunctionCallCharTest, function_call_char_005)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_char_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_char value = CHAR_VAL3;
    ani_function fB {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "ccc:c", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_value argsB[3U];
    argsB[0U].c = CHAR_VAL1;
    argsB[1U].c = CHAR_VAL2;
    argsB[2U].c = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char(fB, &value, CHAR_VAL1, CHAR_VAL2, CHAR_VAL3), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL3);
    ASSERT_EQ(env_->Function_Call_Char_A(fB, &value, argsB), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL3);
}

TEST_F(FunctionCallCharTest, function_call_char_006)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_char_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "nestedFunction", "cc:c", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_char value = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, CHAR_VAL1, CHAR_VAL2), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);

    ani_value args[2U];
    args[0U].c = CHAR_VAL1;
    args[1U].c = CHAR_VAL2;
    ASSERT_EQ(env_->Function_Call_Char_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);
}

TEST_F(FunctionCallCharTest, function_call_char_007)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_char_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "recursiveFunction", "i:c", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_char value = CHAR_VAL3;
    const ani_int value1 = 5;
    const ani_char result = 'Z';
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, value1), ANI_OK);
    ASSERT_EQ(value, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Function_Call_Char_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, result);
}

TEST_F(FunctionCallCharTest, function_call_char_008)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_char_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "calculateSum", "ccdi:c", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_char value = CHAR_VAL3;
    const ani_double dValue = 1.0;
    const ani_int iValue = 1;
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, CHAR_VAL1, CHAR_VAL2, dValue, iValue), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL2);

    const ani_double dValue2 = 2.0;
    ani_value args[4U];
    args[0U].c = CHAR_VAL1;
    args[1U].c = CHAR_VAL2;
    args[2U].d = dValue2;
    args[3U].i = iValue;
    ASSERT_EQ(env_->Function_Call_Char_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);

    const ani_int iValue1 = 2;
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, CHAR_VAL1, CHAR_VAL2, dValue2, iValue1), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL2);
}

TEST_F(FunctionCallCharTest, function_call_char_009)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_char_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "charFunctionA", "cc:c", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_char value = CHAR_VAL3;
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, CHAR_VAL1, CHAR_VAL2), ANI_OK);
    ASSERT_EQ(value, CHAR_VAL1);

    const ani_char value1 = 'D';
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, value1, CHAR_VAL2), ANI_OK);
    ASSERT_EQ(value, value1);

    const ani_char value2 = 'A';
    ani_value args[2U];
    args[0U].c = value1;
    args[1U].c = value2;
    ASSERT_EQ(env_->Function_Call_Char_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, value1);

    const ani_char value3 = 'H';
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, value2, value3), ANI_OK);
    ASSERT_EQ(value, value3);

    const ani_char value4 = 'Z';
    ASSERT_EQ(env_->Function_Call_Char(fA, &value, value3, value4), ANI_OK);
    ASSERT_EQ(value, value4);
}

TEST_F(FunctionCallCharTest, function_call_char_010)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].c = CHAR_VAL1;
    args[1U].c = CHAR_VAL2;

    ani_char result = 0;
    ASSERT_EQ(env_->c_api->Function_Call_Char(nullptr, fn, &result, CHAR_VAL1, CHAR_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Function_Call_Char_A(nullptr, fn, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Char(nullptr, &result, CHAR_VAL1, CHAR_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Char_A(nullptr, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Char(fn, nullptr, CHAR_VAL1, CHAR_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Char_A(fn, nullptr, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Char(fn, &result, nullptr), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Char_A(fn, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallCharTest, check_initialization_char)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_char_test.ops"));
    ani_char result {};
    ASSERT_EQ(env_->Function_Call_Char(fn, &result, CHAR_VAL1, CHAR_VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_char_test.ops"));
}

TEST_F(FunctionCallCharTest, check_initialization_char_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_char_test.ops"));
    ani_char result {};
    ani_value args[2U];
    args[0U].c = CHAR_VAL1;
    args[1U].c = CHAR_VAL2;
    ASSERT_EQ(env_->Function_Call_Char_A(fn, &result, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_char_test.ops"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)