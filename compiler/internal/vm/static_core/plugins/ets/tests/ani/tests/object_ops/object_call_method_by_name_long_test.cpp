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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class CallObjectMethodByNamelongTest : public AniTest {
public:
    static constexpr ani_long VAL0 = 1000000;
    static constexpr ani_long VAL1 = 5000;
    static constexpr ani_long VAL2 = 6000;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.A", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":C{object_call_method_by_name_long_test.A}", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *objectResult = static_cast<ani_object>(ref);
    }

    void TestFuncVCorrectSignature(ani_object obj, ani_long *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Long_V(obj, "method", "C{std.core.String}:l", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_object obj, ani_long *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Long_V(obj, "method", "C{std/core/String}:l", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }
};

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "ll:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "xxxxxxxxxx", "ll:l", &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "ll:i", &sum, args), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_invalid_object)
{
    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(nullptr, "longMethod", "ll:l", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, nullptr, "ll:l", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_a_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "ll:l", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, cobject_call_method_by_name_long_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "ll:l", &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", "ll:l", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", nullptr, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "XXXXXXX", "ll:l", &sum, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", "ll:i", &sum, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_invalid_object)
{
    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(nullptr, "longMethod", "ll:l", &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, nullptr, "ll:l", &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", "ll:l", nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "ll:l", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL0);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "ll:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "l:l", &sum, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL0);

    ani_value args[1U];
    args[0U].l = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "l:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long sum {};
    const ani_long value1 = 50000;
    const ani_long value2 = 80000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longAddMethod", "ll:l", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, VAL0 + value1 + value2);

    ani_value args[2U];
    args[0U].l = value1;
    args[1U].l = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longAddMethod", "ll:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0 + value1 + value2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long sum {};
    const ani_long value1 = 50000;
    const ani_long value2 = 80000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "ll:l", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, VAL0 - value1 - value2);

    ani_value args[2U];
    args[0U].l = value1;
    args[1U].l = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "ll:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0 - value1 - value2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_long num {};
    ani_value args[1U];
    args[0U].l = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "protectedMethod", "l:l", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "protectedMethod", "l:l", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "privateMethod", "l:l", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 - VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "privateMethod", "l:l", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 - VAL1);

    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "callProtected", "l:l", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "callProtected", "l:l", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_long sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "nestedMethod", "l:l", &sum, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL2 + VAL1);

    ani_value args[1U];
    args[0U].l = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "nestedMethod", "l:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL2 + VAL1);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_long sum {};
    const ani_int value1 = 5;
    const ani_long result = 120;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "recursiveMethod", "i:l", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "recursiveMethod", "i:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, result);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_long sum {};
    const ani_long value1 = 1000;
    const ani_char value2 = 'A';
    const ani_int value3 = 1;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "calculateSum", "lci:l", &sum, value1, value2, value3), ANI_OK);
    ASSERT_EQ(sum, VAL2 - value1);

    const ani_char value4 = 'B';
    ani_value args[3U];
    args[0U].l = value1;
    args[1U].c = value4;
    args[2U].i = value3;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "calculateSum", "lci:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value1);

    const ani_int value5 = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "calculateSum", "lci:l", &sum, value1, value4, value5), ANI_OK);
    ASSERT_EQ(sum, VAL2 + value1);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    const ani_long arg = 15000;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_long sum {};
    const ani_long value1 = 5000;
    const ani_long value2 = 6000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "ll:l", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_long value3 = 7000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "ll:l", &sum, value1, value3), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_long value4 = 3000;
    ani_value args[2U];
    args[0U].l = value1;
    args[1U].l = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "ll:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_long value5 = 10000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "ll:l", &sum, value1, value5), ANI_OK);
    ASSERT_EQ(sum, value1 + value5);

    const ani_long value6 = 12000;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "ll:l", &sum, value1, value6), ANI_OK);
    ASSERT_EQ(sum, value1 + value6);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "l:", &method), ANI_OK);

    ani_object obj {};
    ani_long arg = 1000;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_long sum {};
    const ani_long value = 200;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "jf", "l:l", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg + value);

    ani_value args[1U];
    args[0U].l = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "jf", "l:l", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_011)
{
    ani_object obj {};
    GetMethodData(&obj);

    ani_long sum = 0L;
    const ani_long value1 = std::numeric_limits<ani_long>::max();
    const ani_long value2 = 0L;
    ani_value args1[2U];
    args1[0U].l = value1;
    args1[1U].l = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "ll:l", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "ll:l", &sum, args1), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);

    const ani_long value3 = std::numeric_limits<ani_long>::min();
    ani_value args2[2U];
    args2[0U].l = value3;
    args2[1U].l = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Long(obj, "longMethod", "ll:l", &sum, value3, value2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "longMethod", "ll:l", &sum, args2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_012)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long res = 0;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Long(nullptr, object, "longMethod", "ll:l", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Long_A(nullptr, object, "longMethod", "ll:l", &res, args),
              ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(nullptr, "longMethod", "ll:l", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(nullptr, "longMethod", "ll:l", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, nullptr, "ll:l", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, nullptr, "ll:l", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", nullptr, &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", nullptr, &res, args), ANI_OK);

    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", "ll:l", nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", "ll:l", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_013)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long res = 0;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Long(object, methodName.data(), "ll:l", &res, VAL1, VAL2),
                  ANI_NOT_FOUND);
        ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, methodName.data(), "ll:l", &res, args), ANI_NOT_FOUND);
    }
}

TEST_F(CallObjectMethodByNamelongTest, object_call_method_by_name_long_014)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ani_long res = 0;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "longMethod", methodName.data(), &res, VAL1, VAL2),
                  ANI_INVALID_DESCRIPTOR);
        ASSERT_EQ(env_->Object_CallMethodByName_Long_A(object, "longMethod", methodName.data(), &res, args),
                  ANI_INVALID_DESCRIPTOR);
    }
}

TEST_F(CallObjectMethodByNamelongTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_long_test.CheckWrongSignature", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_long res;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Long(env_, obj, "method", "C{std.core.String}:l", &res, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Long(env_, obj, "method", "C{std/core/String}:l", &res, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "method", "C{std.core.String}:l", &res, &arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long_A(obj, "method", "C{std/core/String}:l", &res, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(obj, &res, str);
    TestFuncVWrongSignature(obj, &res, str);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)