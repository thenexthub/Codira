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
    static constexpr size_t ARG_COUNT = 2U;
    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_short_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sum", "ss:s", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void TestCombineScene(const char *className, ani_short val1, ani_short val2, ani_short expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_method method {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ss:s", &method), ANI_OK);

        ani_short value = 0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, method, &value, val1, val2), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].s = val1;
        args[1U].s = val2;
        ani_short valueA = 0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, method, &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(CallStaticMethodTest, call_static_method_short)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_short sum = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Short(env_, cls, method, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_short_v)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_short sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, method, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_short_A)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_short_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_short sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(nullptr, method, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_short_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_short sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_short_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, method, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_short_A_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(nullptr, method, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_short_A_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_short_A_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_short_A_null_args)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_short sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, method, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_short_combine_scenes_1)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("call_static_method_short_test.A", &clsA), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "funcA", "ss:s", &methodA), ANI_OK);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("call_static_method_short_test.B", &clsB), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "funcB", "ss:s", &methodB), ANI_OK);

    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(clsA, methodA, &valueA, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_short valueB = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(clsB, methodB, &valueB, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueB, 6U - 5U);

    ani_value args[ARG_COUNT];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueAA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(clsA, methodA, &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, VAL1 + VAL2);

    ani_short valueBA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(clsB, methodB, &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, 6U - 5U);
}

TEST_F(CallStaticMethodTest, call_static_method_short_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_short_test.A", &cls), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ss:s", &methodA), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:i", &methodB), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, methodA, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, methodA, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_int value2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, methodB, &value2, VAL2, VAL2), ANI_OK);
    ASSERT_EQ(value2, VAL2 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_short_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_short_test.A", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcB", "ss:s", &method), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, method, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_short_null_env)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Short(nullptr, cls, method, &value, VAL1, VAL2), ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Short_A(nullptr, cls, method, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_short_combine_scenes_4)
{
    TestCombineScene("call_static_method_short_test.C", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_short_combine_scenes_5)
{
    TestCombineScene("call_static_method_short_test.D", VAL1, VAL2, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_short_combine_scenes_6)
{
    TestCombineScene("call_static_method_short_test.E", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_short_combine_scenes_7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_short_test.F", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "increment", nullptr, &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &method2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, method1, VAL1, VAL2), ANI_OK);
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, method2, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_short_combine_scenes_8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_short_test.G", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "publicMethod", "ss:s", &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "callPrivateMethod", "ss:s", &method2), ANI_OK);
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, method1, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, method2, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, method1, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, check_initialization_short)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_short_test.Operations"));
    ani_short value {};
    ASSERT_EQ(env_->Class_CallStaticMethod_Short(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_short_test.Operations"));
}

TEST_F(CallStaticMethodTest, check_initialization_short_a)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_short_test.Operations"));
    ani_short value {};
    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Short_A(cls, method, &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_short_test.Operations"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
