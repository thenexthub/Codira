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
    static constexpr ani_long VAL1 = 123;
    static constexpr ani_long VAL2 = 456;
    static constexpr ani_int VAL3 = 6U;
    static constexpr size_t ARG_COUNT = 2U;
    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_long_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sum", "ll:l", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void TestCombineScene(const char *className, ani_long val1, ani_long val2, ani_long expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_method method {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ll:l", &method), ANI_OK);

        ani_long value = 0L;
        ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method, &value, val1, val2), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].l = val1;
        args[1U].l = val2;
        ani_long valueA = 0L;
        ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method, &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(CallStaticMethodTest, call_static_method_long)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum = 0L;
    const ani_long v1 = VAL1;
    const ani_long v2 = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Long(env_, cls, method, &sum, v1, v2), ANI_OK);
    ASSERT_EQ(sum, v1 + v2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_v)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum = 0L;
    const ani_long v1 = VAL1;
    const ani_long v2 = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method, &sum, v1, v2), ANI_OK);
    ASSERT_EQ(sum, v1 + v2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(nullptr, method, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(nullptr, method, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_A_null_args)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long sum = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_combine_scenes_1)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("call_static_method_long_test.A", &clsA), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "funcA", "ll:l", &methodA), ANI_OK);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("call_static_method_long_test.B", &clsB), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "funcB", "ll:l", &methodB), ANI_OK);

    ani_long valueA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(clsA, methodA, &valueA, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_long valueB = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(clsB, methodB, &valueB, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueB, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long valueAA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(clsA, methodA, &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, VAL1 + VAL2);

    ani_long valueBA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(clsB, methodB, &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_long_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_long_test.A", &cls), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ll:l", &methodA), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:i", &methodB), ANI_OK);

    ani_long value = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, methodA, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long valueA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, methodA, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_int value2 = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, methodB, &value2, VAL3, VAL3), ANI_OK);
    ASSERT_EQ(value2, VAL3 + VAL3);
}

TEST_F(CallStaticMethodTest, call_static_method_long_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_long_test.A", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcB", "ll:l", &method), ANI_OK);

    ani_long value = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long valueA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_null_env)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_long value = 0L;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Long(nullptr, cls, method, &value, VAL1, VAL2), ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Long_A(nullptr, cls, method, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_long_combine_scenes_4)
{
    TestCombineScene("call_static_method_long_test.C", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_combine_scenes_5)
{
    TestCombineScene("call_static_method_long_test.D", VAL1, VAL2, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_long_combine_scenes_6)
{
    TestCombineScene("call_static_method_long_test.E", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_combine_scenes_7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_long_test.F", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "increment", nullptr, &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &method2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, method1, VAL1, VAL2), ANI_OK);
    ani_long value = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method2, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long valueA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_long_combine_scenes_8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_long_test.G", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "publicMethod", "ll:l", &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "callPrivateMethod", "ll:l", &method2), ANI_OK);
    ani_long value = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method1, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method2, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long valueA = 0L;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method1, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, check_initialization_long)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_long_test.Operations"));
    ani_long value {};
    ASSERT_EQ(env_->Class_CallStaticMethod_Long(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_long_test.Operations"));
}

TEST_F(CallStaticMethodTest, check_initialization_long_a)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_long_test.Operations"));
    ani_long value {};
    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Long_A(cls, method, &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_long_test.Operations"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
