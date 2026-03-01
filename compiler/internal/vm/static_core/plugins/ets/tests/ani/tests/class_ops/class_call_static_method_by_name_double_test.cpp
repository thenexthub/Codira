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

class ClassCallStaticMethodByNameDoubleTest : public AniTest {
public:
    static constexpr ani_double VAL1 = 1.5;
    static constexpr ani_double VAL2 = 2.5;
    static constexpr ani_int VAL3 = 5;
    static constexpr ani_int VAL4 = 6;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_double *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_V(cls, name, "dd:d", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVCorrectSignature(ani_class cls, ani_double *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_V(cls, "method", "C{std.core.String}:d", value, args),
                  ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_class cls, ani_double *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_V(cls, "method", "C{std/core/String}:d", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }

    void TestCombineScene(const char *className, const char *methodName, ani_double expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ani_double value = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, methodName, "dd:d", &value, VAL1, VAL2), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].d = VAL1;
        args[1U].d = VAL2;
        ani_double valueA = 0.0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, methodName, "dd:d", &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Double(env_, cls, "sum", nullptr, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_v)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "sum", nullptr, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "sum", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_null_class)
{
    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(nullptr, "sum", nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_double_by_name_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, nullptr, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "sum_not_exist", nullptr, &sum, VAL1, VAL2),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "", nullptr, &sum, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "\n", nullptr, &sum, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "sum", nullptr, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A_null_class)
{
    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(nullptr, "sum", nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, nullptr, nullptr, &sum, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "sum_not_exist", nullptr, &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "", nullptr, &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "\n", nullptr, &sum, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A_null_args)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "sum", nullptr, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.na.A", &cls), ANI_OK);

    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "dd:d", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "funcA", "dd:d", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_double valueV = 0.0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.nb.nc.A", &cls), ANI_OK);

    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "dd:d", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "funcA", "dd:d", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_double valueV = 0.0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.na.A", &cls), ANI_OK);

    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "dd:d", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "funcA", "dd:d", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_double valueV = 0.0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);

    ani_int value2 = 0;
    const ani_int value3 = 4;
    const ani_int value4 = 7;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value2, value3, value4), ANI_OK);
    ASSERT_EQ(value2, value4 - value3);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.nd.B", &cls), ANI_OK);
    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "dd:d", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "funcA", "dd:d", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);

    ani_double valueV = 0.0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_null_env)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_double value = 0.0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Double(nullptr, cls, "or", nullptr, &value, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Double_A(nullptr, cls, "or", nullptr, &value, args),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_5)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.A", &clsA), ANI_OK);
    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.B", &clsB), ANI_OK);

    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(clsA, "funcA", "dd:d", &valueA, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ani_double valueB = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(clsB, "funcB", "dd:d", &valueB, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueB, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueAA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(clsA, "funcA", "dd:d", &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, VAL1 + VAL2);
    ani_double valueBA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(clsB, "funcB", "dd:d", &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_6)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.A", &cls), ANI_OK);
    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "dd:d", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "funcA", "dd:d", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_int value2 = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value2, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value2, VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_7)
{
    TestCombineScene("class_call_static_method_by_name_double_test.A", "funcB", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_8)
{
    TestCombineScene("class_call_static_method_by_name_double_test.C", "funcA", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_9)
{
    TestCombineScene("class_call_static_method_by_name_double_test.D", "funcA", VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_10)
{
    TestCombineScene("class_call_static_method_by_name_double_test.E", "funcA", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_11)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.F", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "increment", nullptr, VAL1, VAL2), ANI_OK);
    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "getCount", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "getCount", nullptr, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_12)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.G", &cls), ANI_OK);
    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "publicMethod", "dd:d", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "callPrivateMethod", "dd:d", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "publicMethod", "dd:d", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "callPrivateMethod", "dd:d", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, check_initialization_double)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_double_test.G"));
    ani_double value {};

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "publicMethodx", "dd:d", &value, VAL1, VAL2),
              ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_double_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "publicMethod", "dd:d", &value, VAL1, VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_double_test.G"));
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, check_initialization_double_a)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_double_test.G"));
    ani_double value {};
    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "publicMethodx", "dd:d", &value, args), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_double_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "publicMethod", "dd:d", &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_double_test.G"));
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_double_test.CheckWrongSignature", &cls), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_double value {};
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethodByName_Double(env_, cls, "method", "C{std.core.String}:d", &value, str),
        ANI_OK);
    ASSERT_EQ(
        env_->c_api->Class_CallStaticMethodByName_Double(env_, cls, "method", "C{std/core/String}:d", &value, str),
        ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "method", "C{std.core.String}:d", &value, &arg), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "method", "C{std/core/String}:d", &value, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(cls, &value, str);
    TestFuncVWrongSignature(cls, &value, str);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
