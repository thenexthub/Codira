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

class ClassCallStaticMethodByNameShortTest : public AniTest {
public:
    static constexpr ani_short VAL1 = 5;
    static constexpr ani_short VAL2 = 6;
    void GetClassData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_short *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_V(cls, name, "ss:s", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVCorrectSignature(ani_class cls, ani_short *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_V(cls, "method", "C{std.core.String}:s", value, args),
                  ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_class cls, ani_short *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_V(cls, "method", "C{std/core/String}:s", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }

    void TestCombineScene(const char *className, const char *methodName, ani_short expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ani_short value = 0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, methodName, "ss:s", &value, VAL1, VAL2), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].s = VAL1;
        args[1U].s = VAL2;
        ani_short valueA = 0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, methodName, "ss:s", &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "sum", nullptr, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "sum", nullptr, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "sum", nullptr, &value, args), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_class)
{
    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, nullptr, "sum", nullptr, &value, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, nullptr, nullptr, &value, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "aa", nullptr, &value, VAL1, VAL2),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "", nullptr, &value, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "\n", nullptr, &value, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "sum", nullptr, nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_null_class)
{
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(nullptr, "sum", nullptr, &value, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, nullptr, nullptr, &value, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "aa", nullptr, &value, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_v_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "sum", nullptr, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_class)
{
    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(nullptr, "sum", nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, nullptr, nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "aa", nullptr, &value, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "", nullptr, &value, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "\n", nullptr, &value, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_A_null_args)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "sum", nullptr, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.na.A", &cls), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "ss:s", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "funcA", "ss:s", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_short valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.nb.nc.A", &cls), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "ss:s", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "funcA", "ss:s", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_short valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.na.A", &cls), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "ss:s", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "funcA", "ss:s", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_short valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);

    ani_int valueI;
    const ani_int value1 = 4;
    const ani_int value2 = 7;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &valueI, value1, value2), ANI_OK);
    ASSERT_EQ(valueI, value2 - value1);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.nd.B", &cls), ANI_OK);

    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "ss:s", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "funcA", "ss:s", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);

    ani_short valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_null_env)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_short value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(nullptr, cls, "or", nullptr, &value, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short_A(nullptr, cls, "or", nullptr, &value, args),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_5)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.A", &clsA), ANI_OK);
    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.B", &clsB), ANI_OK);

    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(clsA, "funcA", "ss:s", &valueA, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ani_short valueB = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(clsB, "funcB", "ss:s", &valueB, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueB, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueAA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(clsA, "funcA", "ss:s", &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, VAL1 + VAL2);
    ani_short valueBA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(clsB, "funcB", "ss:s", &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_6)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.A", &cls), ANI_OK);
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "ss:s", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "funcA", "ss:s", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_int value2 = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value2, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value2, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_7)
{
    TestCombineScene("class_call_static_method_by_name_short_test.A", "funcB", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_8)
{
    TestCombineScene("class_call_static_method_by_name_short_test.C", "funcA", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_9)
{
    TestCombineScene("class_call_static_method_by_name_short_test.D", "funcA", VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_10)
{
    TestCombineScene("class_call_static_method_by_name_short_test.E", "funcA", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_11)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.F", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "increment", nullptr, VAL1, VAL2), ANI_OK);
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "getCount", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "getCount", nullptr, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameShortTest, call_static_method_by_name_short_combine_scenes_12)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.G", &cls), ANI_OK);
    ani_short value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "publicMethod", "ss:s", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "callPrivateMethod", "ss:s", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ani_short valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "publicMethod", "ss:s", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "callPrivateMethod", "ss:s", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameShortTest, check_initialization_short)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_short_test.G"));
    ani_short value {};

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "publicMethodx", "ss:s", &value, VAL1, VAL2),
              ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_short_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "publicMethod", "ss:s", &value, VAL1, VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_short_test.G"));
}

TEST_F(ClassCallStaticMethodByNameShortTest, check_initialization_short_a)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_short_test.G"));
    ani_short value {};
    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "publicMethodx", "ss:s", &value, args), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_short_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "publicMethod", "ss:s", &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_short_test.G"));
}

TEST_F(ClassCallStaticMethodByNameShortTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_short_test.CheckWrongSignature", &cls), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_short value {};
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "method", "C{std.core.String}:s", &value, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Short(env_, cls, "method", "C{std/core/String}:s", &value, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "method", "C{std.core.String}:s", &value, &arg), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short_A(cls, "method", "C{std/core/String}:s", &value, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(cls, &value, str);
    TestFuncVWrongSignature(cls, &value, str);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
