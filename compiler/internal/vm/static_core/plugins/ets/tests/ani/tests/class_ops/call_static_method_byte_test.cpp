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
    static constexpr ani_byte VAL1 = 5U;
    static constexpr ani_byte VAL2 = 6U;
    static constexpr size_t ARG_COUNT = 2U;
    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_byte_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sum", "bb:b", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void TestCombineScene(const char *className, ani_byte val1, ani_byte val2, ani_byte expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_method method {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "bb:b", &method), ANI_OK);

        ani_byte value = 0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, method, &value, val1, val2), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].b = val1;
        args[1U].b = val2;
        ani_byte valueA = 0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, method, &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(CallStaticMethodTest, call_static_method_byte)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_byte sum = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Byte(env_, cls, method, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_v)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_byte sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, method, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_A)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;

    ani_byte sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_byte sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(nullptr, method, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_byte sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, method, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_A_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;

    ani_byte sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(nullptr, method, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_A_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;

    ani_byte sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_A_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_A_null_args)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_byte sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, method, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_combine_scenes_1)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("call_static_method_byte_test.A", &clsA), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "funcA", "bb:b", &methodA), ANI_OK);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("call_static_method_byte_test.B", &clsB), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "funcB", "bb:b", &methodB), ANI_OK);

    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(clsA, methodA, &valueA, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_byte valueB = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(clsB, methodB, &valueB, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueB, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueAA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(clsA, methodA, &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, VAL1 + VAL2);

    ani_byte valueBA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(clsB, methodB, &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_byte_test.A", &cls), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "bb:b", &methodA), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:i", &methodB), ANI_OK);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, methodA, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, methodA, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_int value2 = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, methodB, &value2, VAL2, VAL2), ANI_OK);
    ASSERT_EQ(value2, VAL2 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_byte_test.A", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcB", "bb:b", &method), ANI_OK);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, method, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_null_env)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ani_byte value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Byte(nullptr, cls, method, &value, VAL1, VAL2), ANI_INVALID_ARGS);
    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Byte_A(nullptr, cls, method, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_combine_scenes_4)
{
    TestCombineScene("call_static_method_byte_test.C", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_combine_scenes_5)
{
    TestCombineScene("call_static_method_byte_test.D", VAL1, VAL2, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_combine_scenes_6)
{
    TestCombineScene("call_static_method_byte_test.E", VAL1, VAL2, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_combine_scenes_7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_byte_test.F", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "increment", nullptr, &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &method2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, method1, VAL1, VAL2), ANI_OK);
    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, method2, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_byte_combine_scenes_8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_byte_test.G", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "publicMethod", "bb:b", &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "callPrivateMethod", "bb:b", &method2), ANI_OK);
    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, method1, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, method2, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, method1, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, check_initialization_byte)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_byte_test.Operations"));
    ani_byte value {};
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_byte_test.Operations"));
}

TEST_F(CallStaticMethodTest, check_initialization_byte_a)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_byte_test.Operations"));
    ani_byte value {};
    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Byte_A(cls, method, &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_byte_test.Operations"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
