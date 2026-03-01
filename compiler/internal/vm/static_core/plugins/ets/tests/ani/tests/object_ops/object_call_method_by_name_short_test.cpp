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

class CallObjectMethodShortByNameTest : public AniTest {
public:
    static constexpr ani_short VAL0 = 100;
    static constexpr ani_short VAL1 = 5;
    static constexpr ani_short VAL2 = 6;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.A", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":C{object_call_method_by_name_short_test.A}", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *objectResult = static_cast<ani_object>(ref);
    }

    void TestFuncVCorrectSignature(ani_object obj, ani_short *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Short_V(obj, "method", "C{std.core.String}:s", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_object obj, ani_short *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Short_V(obj, "method", "C{std/core/String}:s", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }
};

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_a)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(object, "shortByNameMethod", "ss:s", &res, args), ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "shortByNameMethod", "ss:s", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Short(env_, object, "shortByNameMethod", "ss:s", &res, VAL1, VAL2),
              ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "shortByNameMethod", "ss:x", &res, VAL1, VAL2),
              ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "unknown_function", "ss:s", &res, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, nullptr, "ss:s", &res, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "shortByNameMethod", "ss:s", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_v_invalid_object)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(nullptr, "shortByNameMethod", "ss:s", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_short_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_short res {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(nullptr, "shortByNameMethod", "ss:s", &res, nullptr),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "ss:s", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL0);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortMethod", "ss:s", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "s:s", &sum, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL0);

    ani_value args[1U];
    args[0U].s = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortMethod", "s:s", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortAddMethod", "ss:s", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL0 + VAL1 + VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortAddMethod", "ss:s", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0 + VAL1 + VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "ss:s", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL0 - VAL1 - VAL2);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortMethod", "ss:s", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL0 - VAL1 - VAL2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);

    ani_short num {};
    ani_value args[1U];
    args[0U].s = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "protectedMethod", "s:s", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "protectedMethod", "s:s", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);

    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "privateMethod", "s:s", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 - VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "privateMethod", "s:s", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 - VAL1);

    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL0), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "callProtected", "s:s", &num, VAL1), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "callProtected", "s:s", &num, args), ANI_OK);
    ASSERT_EQ(num, VAL0 + VAL1);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_short sum {};
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "nestedMethod", "s:s", &sum, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL2 + VAL1);

    ani_value args[1U];
    args[0U].s = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "nestedMethod", "s:s", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL2 + VAL1);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_short sum {};
    const ani_int value1 = 5;
    const ani_short result = 120;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "recursiveMethod", "i:s", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, result);

    ani_value args[1U];
    args[0U].i = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "recursiveMethod", "i:s", &sum, args), ANI_OK);
    ASSERT_EQ(sum, result);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_short sum {};
    const ani_short value1 = 1;
    const ani_char value2 = 'A';
    const ani_double value3 = 1.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "calculateSum", "scd:s", &sum, value1, value2, value3), ANI_OK);
    ASSERT_EQ(sum, VAL2 - value1);

    const ani_char value4 = 'B';
    ani_value args[3U];
    args[0U].s = value1;
    args[1U].c = value4;
    args[2U].d = value3;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "calculateSum", "scd:s", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value1);

    const ani_double value5 = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "calculateSum", "scd:s", &sum, value1, value4, value5), ANI_OK);
    ASSERT_EQ(sum, VAL2 + value1);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    const ani_short arg = 15;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_short sum {};
    const ani_short value1 = 5;
    const ani_short value2 = 6;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "ss:s", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_short value3 = 7;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "ss:s", &sum, value1, value3), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_short value4 = 3;
    ani_value args[2U];
    args[0U].s = value1;
    args[1U].s = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortMethod", "ss:s", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_short value5 = 10;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "ss:s", &sum, value1, value5), ANI_OK);
    ASSERT_EQ(sum, value1 + value5);

    const ani_short value6 = 12;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortMethod", "ss:s", &sum, value1, value6), ANI_OK);
    ASSERT_EQ(sum, value1 + value6);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "s:", &method), ANI_OK);

    ani_object obj {};
    const ani_short arg = 10;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_short sum {};
    const ani_short value = 2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "jf", "s:s", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg + value);

    ani_value args[1U];
    args[0U].s = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "jf", "s:s", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_011)
{
    ani_object obj {};
    GetMethodData(&obj);

    ani_short sum = 0;
    const ani_short value1 = std::numeric_limits<ani_short>::max();
    const ani_short value2 = 0;
    ani_value args1[2U];
    args1[0U].s = value1;
    args1[1U].s = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortByNameMethod", "ss:s", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortByNameMethod", "ss:s", &sum, args1), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);

    const ani_short value3 = std::numeric_limits<ani_short>::min();
    ani_value args2[2U];
    args2[0U].s = value3;
    args2[1U].s = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Short(obj, "shortByNameMethod", "ss:s", &sum, value3, value2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "shortByNameMethod", "ss:s", &sum, args2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_012)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short res {};
    ASSERT_EQ(
        env_->c_api->Object_CallMethodByName_Short(nullptr, object, "shortByNameMethod", "ss:s", &res, VAL1, VAL2),
        ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Short_A(nullptr, object, "shortByNameMethod", "ss:s", &res, args),
              ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Short(nullptr, "shortByNameMethod", "ss:s", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(nullptr, "shortByNameMethod", "ss:s", &res, args),
              ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, nullptr, "ss:s", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(object, nullptr, "ss:s", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "shortByNameMethod", nullptr, &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(object, "shortByNameMethod", nullptr, &res, args), ANI_OK);

    ASSERT_EQ(env_->Object_CallMethodByName_Short(object, "shortByNameMethod", "ss:s", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(object, "shortByNameMethod", "ss:s", nullptr, args),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_013)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short res {};
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Short(object, methodName.data(), "ss:s", &res, VAL1, VAL2),
                  ANI_NOT_FOUND);
        ASSERT_EQ(env_->Object_CallMethodByName_Short_A(object, methodName.data(), "ss:s", &res, args), ANI_NOT_FOUND);
    }
}

TEST_F(CallObjectMethodShortByNameTest, object_call_method_by_name_short_014)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].s = VAL1;
    args[1U].s = VAL2;

    ani_short res {};
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Short(object, methodName.data(), "", &res, VAL1, VAL2),
                  ANI_INVALID_DESCRIPTOR);
        ASSERT_EQ(env_->Object_CallMethodByName_Short_A(object, methodName.data(), "", &res, args),
                  ANI_INVALID_DESCRIPTOR);
    }
}

TEST_F(CallObjectMethodShortByNameTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_short_test.CheckWrongSignature", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_short res;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Short(env_, obj, "method", "C{std.core.String}:s", &res, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Short(env_, obj, "method", "C{std/core/String}:s", &res, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "method", "C{std.core.String}:s", &res, &arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Short_A(obj, "method", "C{std/core/String}:s", &res, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(obj, &res, str);
    TestFuncVWrongSignature(obj, &res, str);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)