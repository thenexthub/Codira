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

class FunctionCallFloatTest : public AniTest {
public:
    static constexpr ani_float FLOAT_VAL1 = 1.5F;
    static constexpr ani_float FLOAT_VAL2 = 2.5F;
    static constexpr ani_float FLOAT_VAL3 = 3.5F;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_float_test.ops", &ns), ANI_OK);
        ASSERT_NE(ns, nullptr);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "ff:f", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallFloatTest, Function_Call_Float)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->c_api->Function_Call_Float(env_, fn, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float(fn, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_invalid_function)
{
    ani_float value = 0.0F;
    ASSERT_EQ(env_->c_api->Function_Call_Float(env_, nullptr, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->c_api->Function_Call_Float(env_, fn, nullptr, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_a_invalid_function)
{
    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float_A(nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->Function_Call_Float_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float_A(fn, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_v_invalid_function)
{
    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float(nullptr, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->Function_Call_Float(fn, nullptr, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, function_call_float_001)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_float_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "floatFunctionA", "ff:f", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->Function_Call_Float_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_002)
{
    ani_namespace nA {};
    ani_namespace nB {};
    ani_function fB {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_float_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_float_test.A.B", &nB), ANI_OK);
    ASSERT_NE(nB, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nB, "floatFunctionB", "ff:f", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float(fB, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->Function_Call_Float_A(fB, &value, args), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_003)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_float_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "floatFunctionA", "fff:f", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, FLOAT_VAL1, FLOAT_VAL2, FLOAT_VAL3), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2 + FLOAT_VAL3);

    ani_value args[3U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    args[2U].f = FLOAT_VAL3;
    ASSERT_EQ(env_->Function_Call_Float_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2 + FLOAT_VAL3);
}

TEST_F(FunctionCallFloatTest, function_call_float_004)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_float_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fA {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "ff:f", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_float value = 0.0;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->Function_Call_Float_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_005)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_float_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_float value = 0.0;
    ani_function fB {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "fff:f", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_value argsB[3U];
    argsB[0U].f = FLOAT_VAL1;
    argsB[1U].f = FLOAT_VAL2;
    argsB[2U].f = FLOAT_VAL3;
    ASSERT_EQ(env_->Function_Call_Float(fB, &value, FLOAT_VAL1, FLOAT_VAL2, FLOAT_VAL3), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2 + FLOAT_VAL3);
    ASSERT_EQ(env_->Function_Call_Float_A(fB, &value, argsB), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2 + FLOAT_VAL3);
}

TEST_F(FunctionCallFloatTest, function_call_float_006)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_float_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "nestedFunction", "ff:f", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->Function_Call_Float_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(FunctionCallFloatTest, function_call_float_007)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_float_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "recursiveFunction", "i:f", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_float value = 0.0F;
    const ani_int value1 = 5;
    const ani_float result = 15;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, value1), ANI_OK);
    ASSERT_EQ(value, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Function_Call_Float_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, result);
}

TEST_F(FunctionCallFloatTest, function_call_float_008)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_float_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "calculateSum", "ffci:f", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_float value = 0.0F;
    const ani_char cValue = 'A';
    const ani_int iValue = 1;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, FLOAT_VAL1, FLOAT_VAL2, cValue, iValue), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL2 - FLOAT_VAL1);

    const ani_char cValue1 = 'B';
    ani_value args[4U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    args[2U].c = cValue1;
    args[3U].i = iValue;
    ASSERT_EQ(env_->Function_Call_Float_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1);

    const ani_int iValue1 = 2;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, FLOAT_VAL1, FLOAT_VAL2, cValue1, iValue1), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL2 + FLOAT_VAL1);
}

TEST_F(FunctionCallFloatTest, function_call_float_009)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_float_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "floatFunctionA", "ff:f", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    const ani_float value1 = 5.0;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, value1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, value1 + FLOAT_VAL2);

    const ani_float value2 = 2.0;
    ani_value args[ARG_COUNT];
    args[0U].f = value1;
    args[1U].f = value2;
    ASSERT_EQ(env_->Function_Call_Float_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, value1 + value2);

    const ani_float value3 = 4.0;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, value2, value3), ANI_OK);
    ASSERT_EQ(value, value2 + value3);

    const ani_float value4 = 6.0;
    ASSERT_EQ(env_->Function_Call_Float(fA, &value, value3, value4), ANI_OK);
    ASSERT_EQ(value, value3 + value4);
}

TEST_F(FunctionCallFloatTest, function_call_float_010)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float result = 0.0F;
    ASSERT_EQ(env_->c_api->Function_Call_Float(nullptr, fn, &result, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Function_Call_Float_A(nullptr, fn, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Float(nullptr, &result, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Float_A(nullptr, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Float(fn, nullptr, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Float_A(fn, nullptr, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Float(fn, &result, nullptr), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Float_A(fn, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallFloatTest, check_initialization_float)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_float_test.ops"));
    ani_float result {};
    ASSERT_EQ(env_->Function_Call_Float(fn, &result, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_float_test.ops"));
}

TEST_F(FunctionCallFloatTest, check_initialization_float_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_float_test.ops"));
    ani_float result {};
    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->Function_Call_Float_A(fn, &result, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_float_test.ops"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
