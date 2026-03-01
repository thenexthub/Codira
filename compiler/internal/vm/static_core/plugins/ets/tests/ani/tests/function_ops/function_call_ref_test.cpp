/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class FunctionCallRefTest : public AniTest {
public:
    static constexpr ani_int INT_VAL1 = 1;
    static constexpr ani_int INT_VAL2 = 2;
    static constexpr ani_int INT_VAL3 = 3;

    void GetMethod(ani_namespace *nsResult, ani_function *fnResult)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_ref_test.ops", &ns), ANI_OK);
        ASSERT_NE(ns, nullptr);

        ani_function fn {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "concat", "ii:C{std.core.String}", &fn), ANI_OK);
        ASSERT_NE(fn, nullptr);

        *nsResult = ns;
        *fnResult = fn;
    }
};

TEST_F(FunctionCallRefTest, function_call_ref)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->c_api->Function_Call_Ref(env_, fn, &ref, INT_VAL1, INT_VAL2), ANI_OK);

    std::string result {};
    GetStdString(static_cast<ani_string>(ref), result);
    ASSERT_STREQ(result.c_str(), "12");
}

TEST_F(FunctionCallRefTest, function_call_ref_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].i = INT_VAL2;
    args[1U].i = INT_VAL1;
    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref_A(fn, &ref, args), ANI_OK);

    std::string result {};
    GetStdString(static_cast<ani_string>(ref), result);
    ASSERT_STREQ(result.c_str(), "21");
}

TEST_F(FunctionCallRefTest, function_call_ref_v)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref(fn, &ref, INT_VAL1, INT_VAL2), ANI_OK);

    std::string result {};
    GetStdString(static_cast<ani_string>(ref), result);
    ASSERT_STREQ(result.c_str(), "12");
}

TEST_F(FunctionCallRefTest, function_call_ref_invalid_function)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->c_api->Function_Call_Ref(env_, nullptr, &ref, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_EQ(env_->c_api->Function_Call_Ref(env_, fn, nullptr, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_a_invalid_function)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].i = INT_VAL2;
    args[1U].i = INT_VAL1;
    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref_A(nullptr, &ref, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_a_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].i = INT_VAL2;
    args[1U].i = INT_VAL1;
    ASSERT_EQ(env_->Function_Call_Ref_A(fn, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_a_invalid_args)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref_A(fn, &ref, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_v_invalid_function)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_ref ref {};
    ASSERT_EQ(env_->Function_Call_Ref(nullptr, &ref, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_v_invalid_result)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_EQ(env_->Function_Call_Ref(fn, nullptr, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, function_call_ref_001)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_ref_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "refFunctionA", "ii:C{std.core.String}", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_ref value {};
    std::string result {};
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, INT_VAL1, INT_VAL2), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Ref_A(fA, &value, args), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");
}

TEST_F(FunctionCallRefTest, function_call_ref_002)
{
    ani_namespace nA {};
    ani_namespace nB {};
    ani_function fB {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_ref_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_ref_test.A.B", &nB), ANI_OK);
    ASSERT_NE(nB, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nB, "refFunctionB", "ii:C{std.core.String}", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_ref value {};
    std::string result {};
    ASSERT_EQ(env_->Function_Call_Ref(fB, &value, INT_VAL1, INT_VAL2), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Less");

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Ref_A(fB, &value, args), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Less");
}

TEST_F(FunctionCallRefTest, function_call_ref_003)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_ref_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "refFunctionA", "iii:C{std.core.String}", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_ref value {};
    std::string result {};
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, INT_VAL1, INT_VAL2, INT_VAL3), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Less");

    ani_value args[3U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    args[2U].i = INT_VAL3;
    ASSERT_EQ(env_->Function_Call_Ref_A(fA, &value, args), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Less");
}

TEST_F(FunctionCallRefTest, function_call_ref_004)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_ref_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_function fA {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "ii:C{std.core.String}", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_ref value {};
    std::string result {};
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, INT_VAL1, INT_VAL2), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Ref_A(fA, &value, args), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");
}

TEST_F(FunctionCallRefTest, function_call_ref_005)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@functionModule.function_call_ref_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_ref value {};
    std::string result {};
    ani_function fB {};
    ASSERT_EQ(env_->Module_FindFunction(module, "moduleFunction", "iii:C{std.core.String}", &fB), ANI_OK);
    ASSERT_NE(fB, nullptr);

    ani_value argsB[3U];
    argsB[0U].i = INT_VAL1;
    argsB[1U].i = INT_VAL2;
    argsB[2U].i = INT_VAL3;
    ASSERT_EQ(env_->Function_Call_Ref(fB, &value, INT_VAL1, INT_VAL2, INT_VAL3), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Less");
    ASSERT_EQ(env_->Function_Call_Ref_A(fB, &value, argsB), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Less");
}

TEST_F(FunctionCallRefTest, function_call_ref_006)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_ref_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "nestedFunction", "ii:C{std.core.String}", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_ref value {};
    std::string result {};
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, INT_VAL1, INT_VAL2), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Ref_A(fA, &value, args), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");
}

TEST_F(FunctionCallRefTest, function_call_ref_007)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_ref_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "recursiveFunction", "i:C{std.core.String}", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_ref value {};
    std::string result {};
    const ani_int value1 = 5;
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, value1), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Greater");

    ani_value args[1U];
    args[0U].i = INT_VAL1;
    ASSERT_EQ(env_->Function_Call_Ref_A(fA, &value, args), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Greater");
}

TEST_F(FunctionCallRefTest, function_call_ref_008)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_ref_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "calculateSum", "iidi:C{std.core.String}", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_ref value {};
    std::string result {};
    const ani_double dValue = 1.0;
    const ani_int iValue = 1;
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, INT_VAL1, INT_VAL2, dValue, iValue), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "CharEquality");

    const ani_double dValue2 = 2.0;
    ani_value args[4U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    args[2U].d = dValue2;
    args[3U].i = iValue;
    ASSERT_EQ(env_->Function_Call_Ref_A(fA, &value, args), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "NumEquality");

    const ani_int iValue1 = 2;
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, INT_VAL1, INT_VAL2, dValue2, iValue1), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");
}

TEST_F(FunctionCallRefTest, function_call_ref_009)
{
    ani_namespace nA {};
    ani_function fA {};
    ASSERT_EQ(env_->FindNamespace("@functionModule.function_call_ref_test.A", &nA), ANI_OK);
    ASSERT_NE(nA, nullptr);
    ASSERT_EQ(env_->Namespace_FindFunction(nA, "refFunctionA", "ii:C{std.core.String}", &fA), ANI_OK);
    ASSERT_NE(fA, nullptr);

    ani_ref value {};
    std::string result {};
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, INT_VAL1, INT_VAL2), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");

    const ani_int value1 = 5;
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, value1, INT_VAL2), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");

    const ani_int value2 = 2;
    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Function_Call_Ref_A(fA, &value, args), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");

    const ani_int value3 = 2;
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, value2, value3), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Equality");

    const ani_int value4 = 6;
    ASSERT_EQ(env_->Function_Call_Ref(fA, &value, value3, value4), ANI_OK);
    GetStdString(static_cast<ani_string>(value), result);
    ASSERT_STREQ(result.c_str(), "Inequality");
}

TEST_F(FunctionCallRefTest, function_call_ref_010)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;

    ani_ref result = nullptr;
    ASSERT_EQ(env_->c_api->Function_Call_Ref(nullptr, fn, &result, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Function_Call_Ref_A(nullptr, fn, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Ref(nullptr, &result, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Ref_A(nullptr, &result, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Ref(fn, nullptr, INT_VAL1, INT_VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Function_Call_Ref_A(fn, nullptr, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Function_Call_Ref(fn, &result, nullptr), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Ref_A(fn, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FunctionCallRefTest, check_initialization_ref)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_ref_test.ops"));
    ani_ref result {};
    ASSERT_EQ(env_->Function_Call_Ref(fn, &result, INT_VAL1, INT_VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_ref_test.ops"));
}

TEST_F(FunctionCallRefTest, check_initialization_ref_a)
{
    ani_namespace ns {};
    ani_function fn {};
    GetMethod(&ns, &fn);

    ASSERT_FALSE(IsRuntimeClassInitialized("@functionModule.function_call_ref_test.ops"));
    ani_ref result {};
    ani_value args[2U];
    args[0U].i = INT_VAL1;
    args[1U].i = INT_VAL2;
    ASSERT_EQ(env_->Function_Call_Ref_A(fn, &result, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("@functionModule.function_call_ref_test.ops"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
