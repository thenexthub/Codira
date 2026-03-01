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

class ClassCallStaticMethodByNameTest : public AniTest {
public:
    static constexpr ani_int VAL1 = 5;
    static constexpr ani_int VAL2 = 6;
    void GetClassData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_byte *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_V(cls, name, "bb:b", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVCorrectSignature(ani_class cls, ani_byte *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_V(cls, "method", "C{std.core.String}:b", value, args),
                  ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_class cls, ani_byte *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_V(cls, "method", "C{std/core/String}:b", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }

    void TestCombineScene(const char *className, const char *methodName, const char *signature, ani_byte expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);

        ani_byte value = 0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, methodName, signature, &value, VAL1, VAL2), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].b = VAL1;
        args[1U].b = VAL2;
        ani_byte valueA = 0;
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, methodName, signature, &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_byte value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Byte(env_, cls, "sum", nullptr, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_v)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "sum", nullptr, &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_A)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "sum", nullptr, &value, args), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_null_class)
{
    ani_byte value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Byte(env_, nullptr, "sum", nullptr, &value, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_byte value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Byte(env_, cls, nullptr, nullptr, &value, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_byte value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Byte(env_, cls, "aa", nullptr, &value, VAL1, VAL2),
              ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "", nullptr, &value, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "\n", nullptr, &value, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Byte(env_, cls, "sum", nullptr, nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_v_null_class)
{
    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(nullptr, "sum", nullptr, &value, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_v_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, nullptr, nullptr, &value, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_v_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "aa", nullptr, &value, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_v_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "sum", nullptr, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_A_null_class)
{
    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(nullptr, "sum", nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_A_null_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, nullptr, nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_A_error_name)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "aa", nullptr, &value, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "", nullptr, &value, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "\n", nullptr, &value, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_A_null_result)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_A_null_args)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "sum", nullptr, &value, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.na.A", &cls), ANI_OK);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "funcA", "bb:b", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "funcA", "bb:b", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_byte valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.nb.nc.A", &cls), ANI_OK);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "funcA", "bb:b", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "funcA", "bb:b", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_byte valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.na.A", &cls), ANI_OK);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "funcA", "bb:b", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "funcA", "bb:b", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_byte valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);

    ani_int value2 = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "ii:i", &value2, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value2, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.nd.B", &cls), ANI_OK);

    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "funcA", "bb:b", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "funcA", "bb:b", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);

    ani_byte valueV = 0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_null_env)
{
    ani_class cls {};
    GetClassData(&cls);

    ani_byte value = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Byte(nullptr, cls, "or", nullptr, &value, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Byte_A(nullptr, cls, "or", nullptr, &value, args),
              ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_5)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.A", &clsA), ANI_OK);
    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.B", &clsB), ANI_OK);

    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(clsA, "funcA", "bb:b", &valueA, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ani_byte valueB = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(clsB, "funcB", "bb:b", &valueB, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(valueB, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueAA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(clsA, "funcA", "bb:b", &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, VAL1 + VAL2);
    ani_byte valueBA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(clsB, "funcB", "bb:b", &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_6)
{
    TestCombineScene("class_call_static_method_by_name_byte_test.A", "funcA", "bb:b", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_7)
{
    TestCombineScene("class_call_static_method_by_name_byte_test.A", "funcB", "bb:b", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_8)
{
    TestCombineScene("class_call_static_method_by_name_byte_test.C", "funcA", "bb:b", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_9)
{
    TestCombineScene("class_call_static_method_by_name_byte_test.D", "funcA", "bb:b", VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_10)
{
    TestCombineScene("class_call_static_method_by_name_byte_test.E", "funcA", "bb:b", VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_11)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.F", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "increment", nullptr, VAL1, VAL2), ANI_OK);
    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "getCount", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "getCount", nullptr, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameTest, call_static_method_by_name_byte_combine_scenes_12)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.G", &cls), ANI_OK);
    ani_byte value = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "publicMethod", "bb:b", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "callPrivateMethod", "bb:b", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;
    ani_byte valueA = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "publicMethod", "bb:b", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "callPrivateMethod", "bb:b", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);
}

TEST_F(ClassCallStaticMethodByNameTest, check_initialization_byte)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_byte_test.G"));
    ani_byte value {};

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "publicMethodx", "bb:b", &value, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_byte_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "publicMethod", "bb:b", &value, VAL1, VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_byte_test.G"));
}

TEST_F(ClassCallStaticMethodByNameTest, check_initialization_byte_a)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.G", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_byte_test.G"));
    ani_byte value {};
    ani_value args[2U];
    args[0U].b = VAL1;
    args[1U].b = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte(cls, "publicMethodx", "bb:b", &value, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_FALSE(IsRuntimeClassInitialized("class_call_static_method_by_name_byte_test.G"));

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "publicMethod", "bb:b", &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("class_call_static_method_by_name_byte_test.G"));
}

TEST_F(ClassCallStaticMethodByNameTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("class_call_static_method_by_name_byte_test.CheckWrongSignature", &cls), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_byte value {};
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Byte(env_, cls, "method", "C{std.core.String}:b", &value, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Byte(env_, cls, "method", "C{std/core/String}:b", &value, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "method", "C{std.core.String}:b", &value, &arg), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Byte_A(cls, "method", "C{std/core/String}:b", &value, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(cls, &value, str);
    TestFuncVWrongSignature(cls, &value, str);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
