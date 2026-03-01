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

class CallObjectMethodCharByNameTest : public AniTest {
public:
    static constexpr ani_char VAL = 'a';
    static constexpr ani_char VAL1 = 'C';
    static constexpr ani_char VAL2 = 'A';
    static constexpr ani_char VAL3 = 'H';

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.A", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":C{object_call_method_by_name_char_test.A}", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);
        *objectResult = static_cast<ani_object>(ref);
    }

    void TestFuncVCorrectSignature(ani_object obj, ani_char *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Char_V(obj, "method", "C{std.core.String}:c", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_object obj, ani_char *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Char_V(obj, "method", "C{std/core/String}:c", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }
};

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_a)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].c = VAL2;
    args[1U].c = VAL1;

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(object, "charByNameMethod", "cc:c", &res, args), ANI_OK);
    ASSERT_EQ(res, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "charByNameMethod", "cc:c", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Char(env_, object, "charByNameMethod", "cc:c", &res, VAL1, VAL2),
              ANI_OK);
    ASSERT_EQ(res, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "charByNameMethod", "cc:x", &res, VAL1, VAL2),
              ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "unknown_function", "cc:c", &res, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, nullptr, "cc:c", &res, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "charByNameMethod", "cc:c", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_v_invalid_object)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(nullptr, "charByNameMethod", "cc:c", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_char_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_char res = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(nullptr, "charByNameMethod", "cc:c", &res, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "cc:c", &sum, VAL2, VAL1), ANI_OK);
    ASSERT_EQ(sum, VAL1);

    ani_value args[2U];
    args[0U].c = VAL2;
    args[1U].c = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charMethod", "cc:c", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_char value1 = 'A';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "c:c", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, VAL3);

    const ani_char value2 = 'Z';
    ani_value args[1U];
    args[0U].c = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charMethod", "c:c", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value2);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charComparisonMethod", "cc:c", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1);

    ani_value args[2U];
    args[0U].c = VAL1;
    args[1U].c = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charComparisonMethod", "cc:c", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "cc:c", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL2);

    ani_value args[2U];
    args[0U].c = VAL1;
    args[1U].c = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charMethod", "cc:c", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL2);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char num;
    const ani_char value1 = 'A';
    const ani_char value2 = 'V';
    ani_value argsA[1U];
    argsA[0U].c = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "protectedMethod", "c:c", &num, value1), ANI_OK);
    ASSERT_EQ(num, VAL3);
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "protectedMethod", "c:c", &num, argsA), ANI_OK);
    ASSERT_EQ(num, VAL3);

    ani_value argsB[2U];
    argsB[0U].c = value1;
    argsB[1U].c = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "privateMethod", "cc:c", &num, value1, value2), ANI_OK);
    ASSERT_EQ(num, value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "privateMethod", "cc:c", &num, argsB), ANI_OK);
    ASSERT_EQ(num, value2);

    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "callProtected", "c:c", &num, value1), ANI_OK);
    ASSERT_EQ(num, VAL3);

    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "callProtected", "c:c", &num, argsA), ANI_OK);
    ASSERT_EQ(num, VAL3);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_char value = 'D';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "nestedMethod", "c:c", &sum, value), ANI_OK);
    ASSERT_EQ(sum, VAL3);

    ani_value args[1U];
    args[0U].c = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "nestedMethod", "c:c", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL3);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_int value1 = 5;
    const ani_char result1 = 'Z';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "recursiveMethod", "i:c", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, result1);

    const ani_int value2 = -1;
    const ani_char result2 = 'A';
    ani_value args[1U];
    args[0U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "recursiveMethod", "i:c", &sum, args), ANI_OK);
    ASSERT_EQ(sum, result2);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_double dValue1 = 1.0;
    const ani_int iValue1 = 1;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "calculateSum", "ccdi:c", &sum, VAL1, VAL2, dValue1, iValue1),
              ANI_OK);
    ASSERT_EQ(sum, VAL3);

    const ani_double dValue2 = 2.0;
    ani_value args[4U];
    args[0U].c = VAL1;
    args[1U].c = VAL2;
    args[2U].d = dValue2;
    args[3U].i = iValue1;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "calculateSum", "ccdi:c", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1);

    const ani_int iValue2 = 2;
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "calculateSum", "ccdi:c", &sum, VAL1, VAL2, dValue2, iValue2),
              ANI_OK);
    ASSERT_EQ(sum, VAL2);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_char value1 = 'D';
    const ani_char value2 = 'S';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "cc:c", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, value2);

    const ani_char value3 = 'D';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "cc:c", &sum, value1, value3), ANI_OK);
    ASSERT_EQ(sum, value3);

    const ani_char value4 = 'Z';
    ani_value args[2U];
    args[0U].c = value1;
    args[1U].c = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "charMethod", "cc:c", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value4);

    const ani_char value5 = 'A';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "cc:c", &sum, value1, value5), ANI_OK);
    ASSERT_EQ(sum, value1);

    const ani_char value6 = 'B';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "charMethod", "cc:c", &sum, value1, value6), ANI_OK);
    ASSERT_EQ(sum, value1);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "c:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL3), ANI_OK);

    ani_char sum = VAL;
    const ani_char value = 'C';
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "jf", "c:c", &sum, value), ANI_OK);
    ASSERT_EQ(sum, value);

    ani_value args[1U];
    args[0U].c = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "jf", "c:c", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_011)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].c = VAL1;
    args[1U].c = VAL2;

    ani_char res = VAL;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Char(nullptr, object, "charByNameMethod", "cc:c", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Char_A(nullptr, object, "charByNameMethod", "cc:c", &res, args),
              ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Char(nullptr, "charByNameMethod", "cc:c", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(nullptr, "charByNameMethod", "cc:c", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, nullptr, "cc:c", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(object, nullptr, "cc:c", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "charByNameMethod", nullptr, &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(object, "charByNameMethod", nullptr, &res, args), ANI_OK);

    ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "charByNameMethod", "cc:c", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(object, "charByNameMethod", "cc:c", nullptr, args),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_012)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].c = VAL1;
    args[1U].c = VAL2;

    ani_char res = VAL;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Char(object, methodName.data(), "cc:c", &res, VAL1, VAL2),
                  ANI_NOT_FOUND);
        ASSERT_EQ(env_->Object_CallMethodByName_Char_A(object, methodName.data(), "cc:c", &res, args), ANI_NOT_FOUND);
    }
}

TEST_F(CallObjectMethodCharByNameTest, object_call_method_by_name_char_013)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].c = VAL1;
    args[1U].c = VAL2;

    ani_char res = VAL;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Char(object, "charByNameMethod", methodName.data(), &res, VAL1, VAL2),
                  ANI_INVALID_DESCRIPTOR);
        ASSERT_EQ(env_->Object_CallMethodByName_Char_A(object, "charByNameMethod", methodName.data(), &res, args),
                  ANI_INVALID_DESCRIPTOR);
    }
}

TEST_F(CallObjectMethodCharByNameTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_char_test.CheckWrongSignature", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_char res;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Char(env_, obj, "method", "C{std.core.String}:c", &res, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Char(env_, obj, "method", "C{std/core/String}:c", &res, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "method", "C{std.core.String}:c", &res, &arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Char_A(obj, "method", "C{std/core/String}:c", &res, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(obj, &res, str);
    TestFuncVWrongSignature(obj, &res, str);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)