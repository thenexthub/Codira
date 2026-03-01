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

class FunctionCallVoidTest : public AniTest {
public:
    static constexpr ani_int INT_VAL1 = 5;
    static constexpr ani_int INT_VAL2 = 6;

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult1, ani_function *fnResult2)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_void_test.ops", &ns), ANI_OK);
        ASSERT_NE(ns, nullptr);

        ani_function fn1 {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "sum", "ii:", &fn1), ANI_OK);
        ASSERT_NE(fn1, nullptr);

        ani_function fn2 {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "getCount", ":i", &fn2), ANI_OK);
        ASSERT_NE(fn2, nullptr);

        *nsResult = ns;
        *fnResult1 = fn1;
        *fnResult2 = fn2;
    }
};

TEST_F(FunctionCallVoidTest, function_call_void)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ani_int sum = 0;
    ASSERT_EQ(env_->c_api->Function_Call_Void(env_, fn1, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn2, &sum, nullptr), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallVoidTest, function_call_void_v)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ani_int sum = 0;
    ASSERT_EQ(env_->Function_Call_Void(fn1, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn2, &sum, nullptr), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallVoidTest, function_call_void_a)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;

    ani_int sum = 0;
    ASSERT_EQ(env_->c_api->Function_Call_Void_A(env_, fn1, args), ANI_OK);
    ASSERT_EQ(env_->c_api->Function_Call_Int(env_, fn2, &sum, nullptr), ANI_OK);
    ASSERT_EQ(sum, args[0U].i + args[1U].i);
}

TEST_F(FunctionCallVoidTest, function_call_void_invalid_function)
{
    ASSERT_EQ(env_->Function_Call_Void(nullptr, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallVoidTest, function_call_void_a_invalid_function)
{
    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;

    ASSERT_EQ(env_->Function_Call_Void_A(nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallVoidTest, function_call_void_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ASSERT_EQ(env_->Function_Call_Void_A(fn1, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallVoidTest, function_call_void_001)
{
    ani_namespace nA {};
    ani_function fA {};
    ani_function fGetInt {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_void_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "voidFunctionA", "ii:", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "getIntValue", ":i", &fGetInt), ANI_OK);
    ASSERT_NE(fGetInt, nullptr);

    ani_int sum = 0;
    ASSERT_EQ(env_->Function_Call_Void(fA, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Void_A(fA, args), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallVoidTest, function_call_void_002)
{
    ani_namespace nA {};
    ani_namespace nB {};
    ani_function fB {};
    ani_function fGetIntB {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_void_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_void_test.A.B", &nB), ANI_OK);
    ASSERT_NE(nB, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nB, "voidFunctionB", "ii:", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nB, "getIntValueB", ":i", &fGetIntB), ANI_OK);
    ASSERT_NE(fGetIntB, nullptr);

    ani_int sum = 0;
    ASSERT_EQ(env_->Function_Call_Void(fB, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetIntB, &sum), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Void_A(fB, args), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetIntB, &sum), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallVoidTest, function_call_void_003)
{
    ani_namespace nA {};
    ani_function fA {};
    ani_function fGetChar {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_void_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "voidFunctionA", "c:", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "getCharValue", ":c", &fGetChar), ANI_OK);
    ASSERT_NE(fGetChar, nullptr);

    ani_char value = 'a';
    const ani_char value1 = 'A';
    ASSERT_EQ(env_->Function_Call_Void(fA, value1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Char(fGetChar, &value), ANI_OK);
    ASSERT_EQ(value, value1);

    ani_value args[1U];
    args[0U].c = value1;
    ASSERT_EQ(env_->Function_Call_Void_A(fA, args), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Char(fGetChar, &value), ANI_OK);
    ASSERT_EQ(value, value1);
}

TEST_F(FunctionCallVoidTest, function_call_void_004)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_void_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fA {};
    ani_function fGetInt {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "ii:", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);
    ASSERT_EQ(env_->Module_FindFunction(module, "getIntExport", ":i", &fGetInt), ANI_OK);
    ASSERT_NE(fGetInt, nullptr);

    ani_int sum = 0;
    ASSERT_EQ(env_->Function_Call_Void(fA, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Void_A(fA, args), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallVoidTest, function_call_void_005)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_void_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fB {};
    ani_function fGetChar {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "c:", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);
    ASSERT_EQ(env_->Module_FindFunction(module, "getCharExport", ":c", &fGetChar), ANI_OK);
    ASSERT_NE(fGetChar, nullptr);

    ani_char value = 'a';
    const ani_char value1 = 'A';
    ASSERT_EQ(env_->Function_Call_Void(fB, value1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Char(fGetChar, &value), ANI_OK);
    ASSERT_EQ(value, value1);

    ani_value argsC[1U];
    argsC[0U].c = value1;
    ASSERT_EQ(env_->Function_Call_Void_A(fB, argsC), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Char(fGetChar, &value), ANI_OK);
    ASSERT_EQ(value, value1);
}

TEST_F(FunctionCallVoidTest, function_call_void_006)
{
    ani_namespace nA {};
    ani_function fA {};
    ani_function fGetInt {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_void_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "voidFunctionA", "ii:", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "getIntValue", ":i", &fGetInt), ANI_OK);
    ASSERT_NE(fGetInt, nullptr);

    ani_int sum = 0;
    ASSERT_EQ(env_->Function_Call_Void(fA, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Void_A(fA, args), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);
}

TEST_F(FunctionCallVoidTest, function_call_void_007)
{
    ani_namespace nA {};
    ani_function fA {};
    ani_function fGetInt {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_void_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "recursiveFunction", "i:", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "getIntValue", ":i", &fGetInt), ANI_OK);
    ASSERT_NE(fGetInt, nullptr);

    ani_int sum = 0;
    const ani_int value = 5;
    const ani_int value1 = 15;
    ASSERT_EQ(env_->Function_Call_Void(fA, value), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, value1);

    ani_value args[1U];
    args[0U].i = INT_VAL1;
    ASSERT_EQ(env_->Function_Call_Void_A(fA, args), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, value1 + value1);
}

TEST_F(FunctionCallVoidTest, function_call_void_008)
{
    ani_namespace nA {};
    ani_function fA {};
    ani_function fGetInt {};
    ani_function fGetChar {};
    ani_function fGetDouble {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_void_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "calculateSum", "icd:", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "getIntValue", ":i", &fGetInt), ANI_OK);
    ASSERT_NE(fGetInt, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "getCharValue", ":c", &fGetChar), ANI_OK);
    ASSERT_NE(fGetInt, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "getDoubleValue", ":d", &fGetDouble), ANI_OK);
    ASSERT_NE(fGetInt, nullptr);

    ani_int iResult = 0;
    ani_char cResult = 'a';
    ani_double dResult = 0.0;
    const ani_int iValue = 5;
    const ani_char cValue = 'H';
    const ani_double dValue = 3.0;
    ASSERT_EQ(env_->Function_Call_Void(fA, iValue, cValue, dValue), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &iResult), ANI_OK);
    ASSERT_EQ(iResult, iValue);
    ASSERT_EQ(env_->Function_Call_Char(fGetChar, &cResult), ANI_OK);
    ASSERT_EQ(cResult, cValue);
    ASSERT_EQ(env_->Function_Call_Double(fGetDouble, &dResult), ANI_OK);
    ASSERT_EQ(dResult, dValue);

    ani_value args[3U];
    args[0U].i = iValue;
    args[1U].c = cValue;
    args[2U].d = dValue;
    ASSERT_EQ(env_->Function_Call_Void_A(fA, args), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &iResult), ANI_OK);
    ASSERT_EQ(iResult, iValue);
    ASSERT_EQ(env_->Function_Call_Char(fGetChar, &cResult), ANI_OK);
    ASSERT_EQ(cResult, cValue);
    ASSERT_EQ(env_->Function_Call_Double(fGetDouble, &dResult), ANI_OK);
    ASSERT_EQ(dResult, dValue);
}

TEST_F(FunctionCallVoidTest, function_call_void_009)
{
    ani_namespace nA {};
    ani_function fA {};
    ani_function fGetInt {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_void_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "voidFunctionA", "ii:", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "getIntValue", ":i", &fGetInt), ANI_OK);
    ASSERT_NE(fGetInt, nullptr);

    ani_int sum = 0;
    ASSERT_EQ(env_->Function_Call_Void(fA, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, INT_VAL1 + INT_VAL2);

    const ani_int value1 = 5;
    ASSERT_EQ(env_->Function_Call_Void(fA, value1, INT_VAL2), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, value1 + INT_VAL2);

    const ani_int value2 = 2;
    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Function_Call_Void_A(fA, args), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);

    const ani_int value3 = 4;
    ASSERT_EQ(env_->Function_Call_Void(fA, value2, value3), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, value2 + value3);

    const ani_int value4 = 6;
    ASSERT_EQ(env_->Function_Call_Void(fA, value3, value4), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Int(fGetInt, &sum), ANI_OK);
    ASSERT_EQ(sum, value3 + value4);
}

TEST_F(FunctionCallVoidTest, function_call_void_010)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;

    ASSERT_EQ(env_->c_api->Function_Call_Void(nullptr, fn1, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Function_Call_Void_A(nullptr, fn1, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Void(nullptr, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Void_A(nullptr, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Void(fn1, nullptr), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void_A(fn1, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallVoidTest, check_initialization_void)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_void_test.ops"));
    ASSERT_EQ(env_->Function_Call_Void(fn1, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_void_test.ops"));
}

TEST_F(FunctionCallVoidTest, check_initialization_void_a)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    GetMethod(&ns, &fn1, &fn2);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_void_test.ops"));
    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Void_A(fn1, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_void_test.ops"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)