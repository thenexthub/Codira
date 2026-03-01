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

class ClassCallStaticMethodByNameIntTest : public AniTest {
public:
    static constexpr ani_double VAL1 = 1.5;
    static constexpr ani_double VAL2 = 2.5;
    static constexpr ani_int VAL3 = 5;
    static constexpr ani_int VAL4 = 6;
    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_int *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_V(cls, name, "ii:i", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVCorrectSignature(ani_class cls, ani_int *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_V(cls, "method", "C{std.core.String}:i", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_class cls, ani_int *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_V(cls, "method", "C{std/core/String}:i", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }

    void TestCombineScene(const char *className, const char *methodName, ani_int expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ani_int value = 0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, methodName, "ii:i", &value, VAL3, VAL4), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].i = VAL3;
        args[1U].i = VAL4;
        ani_int valueA = 0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, methodName, "ii:i", &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_int sum = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, cls, "sum", nullptr, &sum, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(sum, VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_v)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "sum", nullptr, &sum, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(sum, VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_A)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;

    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "sum", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_invalid_cls)
{
    ani_int sum = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, nullptr, "sum", nullptr, &sum, VAL3, VAL4),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_invalid_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_int sum = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, cls, nullptr, nullptr, &sum, VAL3, VAL4),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "", nullptr, &sum, VAL3, VAL4), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "\n", nullptr, &sum, VAL3, VAL4), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_invalid_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, cls, "sum", nullptr, nullptr, VAL3, VAL4),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_v_invalid_cls)
{
    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(nullptr, "sum", nullptr, &sum, VAL3, VAL4), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_v_invalid_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, nullptr, nullptr, &sum, VAL3, VAL4), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "sum_not_exist", nullptr, &sum, VAL3, VAL4), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_v_invalid_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "sum", nullptr, nullptr, VAL3, VAL4), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_a_invalid_cls)
{
    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(nullptr, "sum", nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_a_invalid_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, nullptr, nullptr, &sum, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "sum_not_exist", nullptr, &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "", nullptr, &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "\n", nullptr, &sum, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_a_invalid_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_a_invalid_args)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "sum", nullptr, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.na.A", &cls), ANI_OK);

    ani_int value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value, VAL3 + VAL4);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "funcA", "ii:i", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL3 + VAL4);

    ani_int valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL3, VAL4);
    ASSERT_EQ(valueV, VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.nb.nc.A", &cls), ANI_OK);

    ani_int value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value, VAL3 + VAL4);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "funcA", "ii:i", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL3 + VAL4);

    ani_int valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL3, VAL4);
    ASSERT_EQ(valueV, VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.na.A", &cls), ANI_OK);
    ani_int value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value, VAL3 + VAL4);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "funcA", "ii:i", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL3 + VAL4);

    ani_int valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL3, VAL4);
    ASSERT_EQ(valueV, VAL3 + VAL4);

    ani_double value2 = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "dd:d", &value2, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value2, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.nd.B", &cls), ANI_OK);

    ani_int value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value, VAL4 - VAL3);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "funcA", "ii:i", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL4 - VAL3);

    ani_int valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL3, VAL4);
    ASSERT_EQ(valueV, VAL4 - VAL3);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_null_env)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_int value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(nullptr, cls, "or", nullptr, &value, VAL3, VAL4),
              ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int_A(nullptr, cls, "or", nullptr, &value, args),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_5)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.A", &clsA), ANI_OK);
    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.B", &clsB), ANI_OK);

    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(clsA, "funcA", "ii:i", &valueA, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(valueA, VAL3 + VAL4);
    ani_int valueB = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(clsB, "funcB", "ii:i", &valueB, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(valueB, VAL4 - VAL3);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int valueAA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(clsA, "funcA", "ii:i", &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, VAL3 + VAL4);
    ani_int valueBA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(clsB, "funcB", "ii:i", &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, VAL4 - VAL3);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_6)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.A", &cls), ANI_OK);
    ani_int value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value, VAL3 + VAL4);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "funcA", "ii:i", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL3 + VAL4);

    ani_double value2 = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "dd:d", &value2, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value2, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_7)
{
    TestCombineScene("class_call_static_method_by_name_int_test.A", "funcB", VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_8)
{
    TestCombineScene("class_call_static_method_by_name_int_test.C", "funcA", VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_9)
{
    TestCombineScene("class_call_static_method_by_name_int_test.D", "funcA", VAL4 - VAL3);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_10)
{
    TestCombineScene("class_call_static_method_by_name_int_test.E", "funcA", VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_11)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.F", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "increment", nullptr, VAL3, VAL4), ANI_OK);
    ani_int value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "getCount", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL3 + VAL4);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "getCount", nullptr, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL3 + VAL4);
}

TEST_F(ClassCallStaticMethodByNameIntTest, call_static_method_by_name_int_combine_scenes_12)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.G", &cls), ANI_OK);
    ani_int value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "publicMethod", "ii:i", &value, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value, VAL3 + VAL4);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "callPrivateMethod", "ii:i", &value, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value, VAL4 - VAL3);

    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;
    ani_int valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "publicMethod", "ii:i", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL3 + VAL4);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "callPrivateMethod", "ii:i", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL4 - VAL3);
}

TEST_F(ClassCallStaticMethodByNameIntTest, check_initialization_int)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_int_test.G"));
    ani_int value {};

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "publicMethodx", "ii:i", &value, VAL3, VAL4), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_int_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "publicMethod", "ii:i", &value, VAL3, VAL4), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_int_test.G"));
}

TEST_F(ClassCallStaticMethodByNameIntTest, check_initialization_int_a)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_int_test.G"));
    ani_int value {};
    ani_value args[2U];
    args[0U].i = VAL3;
    args[1U].i = VAL4;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "publicMethodx", "ii:i", &value, args), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_int_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "publicMethod", "ii:i", &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_int_test.G"));
}

TEST_F(ClassCallStaticMethodByNameIntTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_int_test.CheckWrongSignature", &cls), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_int value {};
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, cls, "method", "C{std.core.String}:i", &value, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Int(env_, cls, "method", "C{std/core/String}:i", &value, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "method", "C{std.core.String}:i", &value, &arg), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int_A(cls, "method", "C{std/core/String}:i", &value, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(cls, &value, str);
    TestFuncVWrongSignature(cls, &value, str);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
