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
    static constexpr ani_int INT_VAL1 = 5;
    static constexpr ani_int INT_VAL2 = 6;
    static constexpr ani_int INT_VAL3 = 7;

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_int_test.ops", &ns), ANI_OK);
        ASSERT_NE(ns, nullptr);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "ii:i", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallTest, function_call_int)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_int value = 0;
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn, &value, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallTest, function_call_int_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int_A(fn, &value, args), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallTest, function_call_int_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int(fn, &value, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallTest, function_call_int_invalid_function)
{
    ani_int value = 0;
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, nullptr, &value, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn, nullptr, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_a_invalid_function)
{
    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int_A(nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Int_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int_A(fn, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_v_invalid_function)
{
    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int(nullptr, &value, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);
    ASSERT_EQ(env_->Function_Call_Int(fn, nullptr, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, function_call_int_001)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_int_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "intFunctionA", "ii:i", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Int_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallTest, function_call_int_002)
{
    ani_namespace nA {};
    ani_namespace nB {};
    ani_function fB {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_int_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_int_test.A.B", &nB), ANI_OK);
    ASSERT_NE(nB, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nB, "intFunctionB", "ii:i", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int(fB, &value, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Int_A(fB, &value, args), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallTest, function_call_int_003)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_int_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "intFunctionA", "iii:i", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, INT_VAL1, INT_VAL2, INT_VAL3), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2 + INT_VAL3);

    ani_value args[3U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    args[2U].i = INT_VAL3;
    ASSERT_EQ(env_->Function_Call_Int_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2 + INT_VAL3);
}

TEST_F(FunctionCallTest, function_call_int_004)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_int_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fA {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "ii:i", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Int_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallTest, function_call_int_005)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_int_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_int value = 0;
    ani_function fB {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "iii:i", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_value argsB[3U];
    argsB[0U].i = INT_VAL1;
    argsB[1U].i = INT_VAL2;
    argsB[2U].i = INT_VAL3;
    ASSERT_EQ(env_->Function_Call_Int(fB, &value, INT_VAL1, INT_VAL2, INT_VAL3), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2 + INT_VAL3);
    ASSERT_EQ(env_->Function_Call_Int_A(fB, &value, argsB), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2 + INT_VAL3);
}

TEST_F(FunctionCallTest, function_call_int_006)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_int_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "nestedFunction", "ii:i", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Int_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallTest, function_call_int_007)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_int_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "recursiveFunction", "i:i", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_int value = 0;
    const ani_int value1 = 5;
    const ani_int result = 15;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, value1), ANI_OK);
    ASSERT_EQ(value, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Function_Call_Int_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, result);
}

TEST_F(FunctionCallTest, function_call_int_008)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_int_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "calculateSum", "iicd:i", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_int value = 0;
    const ani_char cValue = 'A';
    const ani_double dValue = 1.0;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, INT_VAL1, INT_VAL2, cValue, dValue), ANI_OK);
    ASSERT_EQ(value, INT_VAL2 - INT_VAL1);

    const ani_char cValue1 = 'B';
    ani_value args[4U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    args[2U].c = cValue1;
    args[3U].d = dValue;
    ASSERT_EQ(env_->Function_Call_Int_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, INT_VAL1);

    const ani_double dValue1 = 2.0;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, INT_VAL1, INT_VAL2, cValue1, dValue1), ANI_OK);
    ASSERT_EQ(value, INT_VAL2 + INT_VAL1);
}

TEST_F(FunctionCallTest, function_call_int_009)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_int_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "intFunctionA", "ii:i", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_int value = 0;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(value, INT_VAL1 + INT_VAL2);

    const ani_int value1 = 5;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, value1, INT_VAL2), ANI_OK);
    ASSERT_EQ(value, value1 + INT_VAL2);

    const ani_int value2 = 2;
    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Function_Call_Int_A(fA, &value, args), ANI_OK);
    ASSERT_EQ(value, value1 + value2);

    const ani_int value3 = 4;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, value2, value3), ANI_OK);
    ASSERT_EQ(value, value2 + value3);

    const ani_int value4 = 6;
    ASSERT_EQ(env_->Function_Call_Int(fA, &value, value3, value4), ANI_OK);
    ASSERT_EQ(value, value3 + value4);
}

TEST_F(FunctionCallTest, function_call_int_010)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;

    ani_int result = 0;
    ASSERT_EQ(env_->c_api->Function_Call_Int(nullptr, fn, &result, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Function_Call_Int_A(nullptr, fn, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Int(nullptr, &result, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Int_A(nullptr, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Int(fn, nullptr, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Int_A(fn, nullptr, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Int(fn, &result, nullptr), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int_A(fn, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallTest, check_initialization_int)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_int_test.ops"));
    ani_int result {};
    ASSERT_EQ(env_->Function_Call_Int(fn, &result, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_int_test.ops"));
}

TEST_F(FunctionCallTest, check_initialization_int_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_int_test.ops"));
    ani_int result {};
    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Int_A(fn, &result, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_int_test.ops"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)