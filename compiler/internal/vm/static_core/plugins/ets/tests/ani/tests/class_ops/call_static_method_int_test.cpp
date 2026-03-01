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
    static constexpr ani_int VAL1 = 5U;
    static constexpr ani_int VAL2 = 6U;
    static constexpr ani_double VAL3 = 1.5;
    static constexpr ani_double VAL4 = 2.5;
    static constexpr size_t ARG_COUNT = 2U;
    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_int_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sum", "ii:i", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void TestCombineScene(const char *className, ani_int val1, ani_int val2, ani_int expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_method method {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:i", &method), ANI_OK);

        ani_int value = 0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method, &value, val1, val2), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].i = val1;
        args[1U].i = val2;
        ani_int valueA = 0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(CallStaticMethodTest, call_static_method_int)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_int sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Int(env_, cls, method, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_int_v)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_int_A)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_int_invalid_cls)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_int sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Int(env_, nullptr, method, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_invalid_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_int sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Int(env_, cls, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_invalid_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Int(env_, cls, method, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_v_invalid_cls)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(nullptr, method, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_v_invalid_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_v_invalid_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_a_invalid_cls)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(nullptr, method, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_a_invalid_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_a_invalid_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_a_invalid_args)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(nullptr, method, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_combine_scenes_1)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("call_static_method_int_test.A", &clsA), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "funcA", "ii:i", &methodA), ANI_OK);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("call_static_method_int_test.B", &clsB), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "funcB", "ii:i", &methodB), ANI_OK);

    ani_int valueA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(clsA, methodA, &valueA, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_int valueB;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(clsB, methodB, &valueB, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueB, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int valueAA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(clsA, methodA, &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, VAL1 + VAL2);

    ani_int valueBA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(clsB, methodB, &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_int_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_int_test.A", &cls), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:i", &methodA), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "dd:d", &methodB), ANI_OK);

    ani_int value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, methodA, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int valueA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, methodA, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_double value2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, methodB, &value2, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value2, VAL3 + VAL4);
}

TEST_F(CallStaticMethodTest, call_static_method_int_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_int_test.A", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcB", "ii:i", &method), ANI_OK);

    ani_int value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int valueA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_int_null_env)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ani_int value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Int(nullptr, cls, method, &value, VAL3, VAL4), ANI_INVALID_ARGS);
    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Int_A(nullptr, cls, method, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_int_combine_scenes_4)
{
    TestCombineScene("call_static_method_int_test.C", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_int_combine_scenes_5)
{
    TestCombineScene("call_static_method_int_test.D", VAL1, VAL2, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_int_combine_scenes_6)
{
    TestCombineScene("call_static_method_int_test.E", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_int_combine_scenes_7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_int_test.F", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "increment", nullptr, &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &method2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, method1, VAL1, VAL2), ANI_OK);
    ani_int value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method2, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_int_combine_scenes_8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_int_test.G", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "publicMethod", "ii:i", &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "callPrivateMethod", "ii:i", &method2), ANI_OK);
    ani_int value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method1, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method2, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method1, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, check_initialization_int)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_int_test.Operations"));
    ani_int value {};
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_int_test.Operations"));
}

TEST_F(CallStaticMethodTest, check_initialization_int_a)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_int_test.Operations"));
    ani_int value {};
    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_int_test.Operations"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
