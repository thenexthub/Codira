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
    static constexpr ani_float FLOAT_VAL1 = 1.5F;
    static constexpr ani_float FLOAT_VAL2 = 2.5F;
    static constexpr ani_int VAL1 = 6U;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_float_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sum", "ff:f", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void TestCombineScene(const char *className, ani_float val1, ani_float val2, ani_float expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_method method {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ff:f", &method), ANI_OK);

        ani_float value = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, method, &value, val1, val2), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].f = val1;
        args[1U].f = val2;
        ani_float valueA = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, method, &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(CallStaticMethodTest, call_static_method_float)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_float sum = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Float(env_, cls, method, &sum, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_float_v)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_float sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, method, &sum, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_float_A)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_float_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_float sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(nullptr, method, &sum, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_float_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_float sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_float_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, method, nullptr, FLOAT_VAL1, FLOAT_VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_float_A_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(nullptr, method, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_float_A_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_float_A_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_float_A_null_args)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_float sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, method, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_float_combine_scenes_1)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("call_static_method_float_test.A", &clsA), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "funcA", "ff:f", &methodA), ANI_OK);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("call_static_method_float_test.B", &clsB), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "funcB", "ff:f", &methodB), ANI_OK);

    ani_float valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(clsA, methodA, &valueA, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueB = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(clsB, methodB, &valueB, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(valueB, FLOAT_VAL2 - FLOAT_VAL1);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueAA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(clsA, methodA, &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueBA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(clsB, methodB, &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_float_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_float_test.A", &cls), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ff:f", &methodA), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:i", &methodB), ANI_OK);

    ani_float value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, methodA, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, methodA, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_int value2 = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, methodB, &value2, VAL1, VAL1), ANI_OK);
    ASSERT_EQ(value2, VAL1 + VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_float_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_float_test.A", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcB", "ff:f", &method), ANI_OK);

    ani_float value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, method, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, method, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_float_null_env)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ani_float value = 0.0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Float(nullptr, cls, method, &value, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Float_A(nullptr, cls, method, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_float_combine_scenes_4)
{
    TestCombineScene("call_static_method_float_test.C", FLOAT_VAL1, FLOAT_VAL2, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_float_combine_scenes_5)
{
    TestCombineScene("call_static_method_float_test.D", FLOAT_VAL1, FLOAT_VAL2, FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_float_combine_scenes_6)
{
    TestCombineScene("call_static_method_float_test.E", FLOAT_VAL1, FLOAT_VAL2, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_float_combine_scenes_7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_float_test.F", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "increment", nullptr, &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &method2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, method1, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ani_float value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, method2, &value), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_float_combine_scenes_8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_float_test.G", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "publicMethod", "ff:f", &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "callPrivateMethod", "ff:f", &method2), ANI_OK);
    ani_float value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, method1, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, method2, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL2 - FLOAT_VAL1);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, method1, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(CallStaticMethodTest, check_initialization_float)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_float_test.Operations"));
    ani_float value {};
    ASSERT_EQ(env_->Class_CallStaticMethod_Float(cls, method, &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_float_test.Operations"));
}

TEST_F(CallStaticMethodTest, check_initialization_float_a)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_float_test.Operations"));
    ani_float value {};
    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Float_A(cls, method, &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_float_test.Operations"));
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
