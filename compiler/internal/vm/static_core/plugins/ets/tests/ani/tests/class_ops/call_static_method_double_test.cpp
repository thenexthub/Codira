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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class CallStaticMethodTest : public AniTest {
public:
    static constexpr ani_double VAL1 = 1.5;
    static constexpr ani_double VAL2 = 2.5;
    static constexpr ani_int VAL3 = 6U;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_double_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sum", "dd:d", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void TestCombineScene(const char *className, ani_double val1, ani_double val2, ani_double expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_method method {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "dd:d", &method), ANI_OK);

        ani_double value = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, method, &value, val1, val2), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].d = val1;
        args[1U].d = val2;
        ani_double valueA = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, method, &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(CallStaticMethodTest, call_static_method_double)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Double(env_, cls, method, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_double_v)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, method, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_double_A)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_double_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(nullptr, method, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_double_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_double_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, method, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_double_A_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(nullptr, method, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_double_A_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_double_A_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_double_A_null_args)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, method, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_double_combine_scenes_1)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("call_static_method_double_test.A", &clsA), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "funcA", "dd:d", &methodA), ANI_OK);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("call_static_method_double_test.B", &clsB), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "funcB", "dd:d", &methodB), ANI_OK);

    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(clsA, methodA, &valueA, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_double valueB = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(clsB, methodB, &valueB, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueB, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueAA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(clsA, methodA, &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, VAL1 + VAL2);

    ani_double valueBA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(clsB, methodB, &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_double_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_double_test.A", &cls), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "dd:d", &methodA), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:i", &methodB), ANI_OK);

    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, methodA, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, methodA, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_int value2 = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, methodB, &value2, VAL3, VAL3), ANI_OK);
    ASSERT_EQ(value2, VAL3 + VAL3);
}

TEST_F(CallStaticMethodTest, call_static_method_double_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_double_test.A", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcB", "dd:d", &method), ANI_OK);

    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, method, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_double_null_env)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_double value = 0.0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Double(nullptr, cls, method, &value, VAL1, VAL2), ANI_INVALID_ARGS);
    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Double_A(nullptr, cls, method, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_double_combine_scenes_4)
{
    TestCombineScene("call_static_method_double_test.C", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_double_combine_scenes_5)
{
    TestCombineScene("call_static_method_double_test.D", VAL1, VAL2, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_double_combine_scenes_6)
{
    TestCombineScene("call_static_method_double_test.E", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_double_combine_scenes_7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_double_test.F", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "increment", nullptr, &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &method2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, method1, VAL1, VAL2), ANI_OK);
    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, method2, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_double_combine_scenes_8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_double_test.G", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "publicMethod", "dd:d", &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "callPrivateMethod", "dd:d", &method2), ANI_OK);
    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, method1, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, method2, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, method1, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, check_initialization_double)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_double_test.Operations"));
    ani_double value {};
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_double_test.Operations"));
}

TEST_F(CallStaticMethodTest, check_initialization_double_a)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_double_test.Operations"));
    ani_double value {};
    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double_A(cls, method, &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_double_test.Operations"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
