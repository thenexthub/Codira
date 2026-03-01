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

class FunctionCallTest : public AniTest {
public:
    static constexpr ani_long LONG_VAL1 = 100;
    static constexpr ani_long LONG_VAL2 = 200;
    static constexpr ani_long LONG_VAL3 = 300;

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_long_test.ops", &ns), ANI_OK);
        ASSERT_NE(ns, nullptr);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "ll:l", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallTest, function_call_long)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_long value {};
    ASSERT_EQ(env_->c_api->Function_Call_Long(env_, fn, &value, LONG_VAL1, LONG_VAL2), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);
}

TEST_F(FunctionCallTest, function_call_long_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);
}

TEST_F(FunctionCallTest, function_call_long_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long(fn, &value, LONG_VAL1, LONG_VAL2), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);
}

TEST_F(FunctionCallTest, function_call_long_invalid_function)
{
    ani_long value {};
    ASSERT_EQ(env_->c_api->Function_Call_Long(env_, nullptr, &value, LONG_VAL1, LONG_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->c_api->Function_Call_Long(env_, fn, nullptr, LONG_VAL1, LONG_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_long_a_invalid_function)
{
    ani_value args[2U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long_A(nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_long_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    ASSERT_EQ(env_->Function_Call_Long_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_long_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long_A(fn, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_long_v_invalid_function)
{
    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long(nullptr, &value, LONG_VAL1, LONG_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_long_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->Function_Call_Long(fn, nullptr, LONG_VAL1, LONG_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_long_001)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_long_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "longFunctionA", "ll:l", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, LONG_VAL1, LONG_VAL2), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);

    ani_value args[2U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    ASSERT_EQ(env_->Function_Call_Long_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);
}

TEST_F(FunctionCallTest, function_call_long_002)
{
    ani_namespace nA {};
    ani_namespace nB {};
    ani_function fB {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_long_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_long_test.A.B", &nB), ANI_OK);
    ASSERT_NE(nB, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nB, "longFunctionB", "ll:l", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long(fB, &value, LONG_VAL1, LONG_VAL2), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);

    ani_value args[2U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    ASSERT_EQ(env_->Function_Call_Long_A(fB, &value, args), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);
}

TEST_F(FunctionCallTest, function_call_long_003)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_long_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "longFunctionA", "lll:l", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, LONG_VAL1, LONG_VAL2, LONG_VAL3), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2 + LONG_VAL3);

    ani_value args[3U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    args[2U].l = LONG_VAL3;
    ASSERT_EQ(env_->Function_Call_Long_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2 + LONG_VAL3);
}

TEST_F(FunctionCallTest, function_call_long_004)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_long_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fA {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "ll:l", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, LONG_VAL1, LONG_VAL2), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);

    ani_value args[2U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    ASSERT_EQ(env_->Function_Call_Long_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);
}

TEST_F(FunctionCallTest, function_call_long_005)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_long_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_long value {};
    ani_function fB {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "lll:l", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_value argsB[3U];
    argsB[0U].l = LONG_VAL1;
    argsB[1U].l = LONG_VAL2;
    argsB[2U].l = LONG_VAL3;
    ASSERT_EQ(env_->Function_Call_Long(fB, &value, LONG_VAL1, LONG_VAL2, LONG_VAL3), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2 + LONG_VAL3);
    ASSERT_EQ(env_->Function_Call_Long_A(fB, &value, argsB), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2 + LONG_VAL3);
}

TEST_F(FunctionCallTest, function_call_long_006)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_long_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "nestedFunction", "ll:l", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, LONG_VAL1, LONG_VAL2), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);

    ani_value args[2U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    ASSERT_EQ(env_->Function_Call_Long_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);
}

TEST_F(FunctionCallTest, function_call_long_007)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_long_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "recursiveFunction", "i:l", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_long value {};
    const ani_int value1 = 5;
    const ani_long result = 15;
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, value1), ANI_OK);
    ASSERT_EQ(value, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Function_Call_Long_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, result);
}

TEST_F(FunctionCallTest, function_call_long_008)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_long_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "calculateSum", "llcd:l", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_long value {};
    const ani_char cValue = 'A';
    const ani_double dValue = 1.0;
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, LONG_VAL1, LONG_VAL2, cValue, dValue), ANI_OK);
    ASSERT_EQ(value, LONG_VAL2 - LONG_VAL1);

    const ani_char cValue1 = 'B';
    ani_value args[4U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    args[2U].c = cValue1;
    args[3U].d = dValue;
    ASSERT_EQ(env_->Function_Call_Long_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1);

    const ani_double dValue1 = 2.0;
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, LONG_VAL1, LONG_VAL2, cValue1, dValue1), ANI_OK);
    ASSERT_EQ(value, LONG_VAL2 + LONG_VAL1);
}

TEST_F(FunctionCallTest, function_call_long_009)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_long_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "longFunctionA", "ll:l", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_long value {};
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, LONG_VAL1, LONG_VAL2), ANI_OK);
    ASSERT_EQ(value, LONG_VAL1 + LONG_VAL2);

    const ani_long value1 = 500;
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, value1, LONG_VAL2), ANI_OK);
    ASSERT_EQ(value, value1 + LONG_VAL2);

    const ani_long value2 = 200;
    ani_value args[2U];
    args[0U].l = value1;
    args[1U].l = value2;
    ASSERT_EQ(env_->Function_Call_Long_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, value1 + value2);

    const ani_long value3 = 400;
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, value2, value3), ANI_OK);
    ASSERT_EQ(value, value2 + value3);

    const ani_long value4 = 600;
    ASSERT_EQ(env_->Function_Call_Long(fA, &value, value3, value4), ANI_OK);
    ASSERT_EQ(value, value3 + value4);
}

TEST_F(FunctionCallTest, function_call_long_010)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;

    ani_long result = 0;
    ASSERT_EQ(env_->c_api->Function_Call_Long(nullptr, fn, &result, LONG_VAL1, LONG_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Function_Call_Long_A(nullptr, fn, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Long(nullptr, &result, LONG_VAL1, LONG_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Long_A(nullptr, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Long(fn, nullptr, LONG_VAL1, LONG_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Long_A(fn, nullptr, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Long(fn, &result, nullptr), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Long_A(fn, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, check_initialization_long)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_long_test.ops"));
    ani_long result {};
    ASSERT_EQ(env_->Function_Call_Long(fn, &result, LONG_VAL1, LONG_VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_long_test.ops"));
}

TEST_F(FunctionCallTest, check_initialization_long_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_long_test.ops"));
    ani_long result {};
    ani_value args[2U];
    args[0U].l = LONG_VAL1;
    args[1U].l = LONG_VAL2;
    ASSERT_EQ(env_->Function_Call_Long_A(fn, &result, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_long_test.ops"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)