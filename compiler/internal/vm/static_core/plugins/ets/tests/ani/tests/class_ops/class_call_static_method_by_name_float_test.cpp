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

class ClassCallStaticMethodByNameFloatTest : public AniTest {
public:
    static constexpr ani_float FLOAT_VAL1 = 1.5F;
    static constexpr ani_float FLOAT_VAL2 = 2.5F;
    static constexpr ani_int VAL3 = 5;
    static constexpr ani_int VAL4 = 6;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_float *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_V(cls, name, "ff:f", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVCorrectSignature(ani_class cls, ani_float *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_V(cls, "method", "C{std.core.String}:f", value, args),
                  ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_class cls, ani_float *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_V(cls, "method", "C{std/core/String}:f", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }

    void TestCombineScene(const char *className, const char *methodName, ani_float expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ani_float value = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, methodName, "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2),
                  ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].f = FLOAT_VAL1;
        args[1U].f = FLOAT_VAL2;
        ani_float valueA = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, methodName, "ff:f", &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float sum {};
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Float(env_, cls, "sum", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_v)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "sum", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_class)
{
    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(nullptr, "sum", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, nullptr, nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "sum_not_exist", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "\n", nullptr, &sum, FLOAT_VAL1, FLOAT_VAL2),
              ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "sum", nullptr, nullptr, FLOAT_VAL1, FLOAT_VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_class)
{
    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(nullptr, "sum", nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, nullptr, nullptr, &sum, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum_not_exist", nullptr, &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "", nullptr, &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "\n", nullptr, &sum, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_A_null_args)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float sum {};
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "sum", nullptr, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.na.A", &cls), ANI_OK);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "ff:f", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueV = 0.0F;
    TestFuncV(cls, "funcA", &valueV, FLOAT_VAL1, FLOAT_VAL2);
    ASSERT_EQ(valueV, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.nb.nc.A", &cls), ANI_OK);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "ff:f", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueV = 0.0F;
    TestFuncV(cls, "funcA", &valueV, FLOAT_VAL1, FLOAT_VAL2);
    ASSERT_EQ(valueV, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.na.A", &cls), ANI_OK);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "ff:f", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueV = 0.0F;
    TestFuncV(cls, "funcA", &valueV, FLOAT_VAL1, FLOAT_VAL2);
    ASSERT_EQ(valueV, FLOAT_VAL1 + FLOAT_VAL2);

    ani_int value2 = 0;
    const ani_int value3 = 4;
    const ani_int value4 = 7;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value2, value3, value4), ANI_OK);
    ASSERT_EQ(value2, value4 - value3);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.nd.B", &cls), ANI_OK);

    ani_float value = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL2 - FLOAT_VAL1);

    ani_value args[ARG_COUNT];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0F;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "ff:f", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL2 - FLOAT_VAL1);

    ani_float valueV = 0.0F;
    TestFuncV(cls, "funcA", &valueV, FLOAT_VAL1, FLOAT_VAL2);
    ASSERT_EQ(valueV, FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_null_env)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_float value = 0.0;
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethodByName_Float(nullptr, cls, "or", nullptr, &value, FLOAT_VAL1, FLOAT_VAL2),
        ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Float_A(nullptr, cls, "or", nullptr, &value, args),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_5)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.A", &clsA), ANI_OK);
    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.B", &clsB), ANI_OK);

    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(clsA, "funcA", "ff:f", &valueA, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_float valueB = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(clsB, "funcB", "ff:f", &valueB, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(valueB, FLOAT_VAL2 - FLOAT_VAL1);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueAA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(clsA, "funcA", "ff:f", &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, FLOAT_VAL1 + FLOAT_VAL2);
    ani_float valueBA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(clsB, "funcB", "ff:f", &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_6)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.A", &cls), ANI_OK);
    ani_float value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "funcA", "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "funcA", "ff:f", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);

    ani_int value2 = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value2, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value2, VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_7)
{
    TestCombineScene("class_call_static_method_by_name_float_test.A", "funcB", FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_8)
{
    TestCombineScene("class_call_static_method_by_name_float_test.C", "funcA", FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_9)
{
    TestCombineScene("class_call_static_method_by_name_float_test.D", "funcA", FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_10)
{
    TestCombineScene("class_call_static_method_by_name_float_test.E", "funcA", FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_11)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.F", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "increment", nullptr, FLOAT_VAL1, FLOAT_VAL2), ANI_OK);
    ani_float value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "getCount", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "getCount", nullptr, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, call_static_method_by_name_float_combine_scenes_12)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.G", &cls), ANI_OK);
    ani_float value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "publicMethod", "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2),
              ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL1 + FLOAT_VAL2);
    ASSERT_EQ(
        env_->Class_CallStaticMethodByName_Float(cls, "callPrivateMethod", "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2),
        ANI_OK);
    ASSERT_EQ(value, FLOAT_VAL2 - FLOAT_VAL1);

    ani_value args[2U];
    args[0U].f = FLOAT_VAL1;
    args[1U].f = FLOAT_VAL2;
    ani_float valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "publicMethod", "ff:f", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL1 + FLOAT_VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "callPrivateMethod", "ff:f", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, FLOAT_VAL2 - FLOAT_VAL1);
}

TEST_F(ClassCallStaticMethodByNameFloatTest, check_initialization_float)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_float_test.G"));
    ani_float value {};

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "publicMethodx", "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2),
              ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_float_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float(cls, "publicMethod", "ff:f", &value, FLOAT_VAL1, FLOAT_VAL2),
              ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_float_test.G"));
}

TEST_F(ClassCallStaticMethodByNameFloatTest, check_initialization_float_a)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_float_test.G"));
    ani_float value {};
    ani_value args[2U];
    args[0U].d = FLOAT_VAL1;
    args[1U].d = FLOAT_VAL1;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "publicMethodx", "ff:f", &value, args), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_float_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "publicMethod", "ff:f", &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_float_test.G"));
}

TEST_F(ClassCallStaticMethodByNameFloatTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_float_test.CheckWrongSignature", &cls), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_float value {};
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Float(env_, cls, "method", "C{std.core.String}:f", &value, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Float(env_, cls, "method", "C{std/core/String}:f", &value, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "method", "C{std.core.String}:f", &value, &arg), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Float_A(cls, "method", "C{std/core/String}:f", &value, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(cls, &value, str);
    TestFuncVWrongSignature(cls, &value, str);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
