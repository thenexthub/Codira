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

class CallObjectMethodBooleanByNameTest : public AniTest {
public:
    static constexpr ani_int VAL1 = 5;
    static constexpr ani_int VAL2 = 6;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.A", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(
            env_->Class_FindStaticMethod(cls, "new_A", ":C{object_call_method_by_name_boolean_test.A}", &newMethod),
            ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);
        *objectResult = static_cast<ani_object>(ref);
    }

    void TestFuncVCorrectSignature(ani_object obj, ani_boolean *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Boolean_V(obj, "method", "C{std.core.String}:z", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_object obj, ani_boolean *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Boolean_V(obj, "method", "C{std/core/String}:z", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }
};

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean_a)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(object, "booleanByNameMethod", "ii:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean_v)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "booleanByNameMethod", "ii:z", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean_v_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "booleanByNameMethod", "ii:x", &res, VAL1, VAL2),
              ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "unknown_function", "ii:z", &res, VAL1, VAL2),
              ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_boolean)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(
        env_->c_api->Object_CallMethodByName_Boolean(env_, object, "booleanByNameMethod", "ii:z", &res, VAL1, VAL2),
        ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_v_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, nullptr, "ii:z", &res, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_v_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "booleanByNameMethod", "ii:z", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_v_invalid_object)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(nullptr, "booleanByNameMethod", "ii:z", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, call_method_boolean_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(nullptr, "booleanByNameMethod", "ii:z", &res, nullptr),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "ii:z", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanMethod", "ii:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    const ani_int value = 5;
    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "i:z", &res, value), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_value args[1U];
    args[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanMethod", "i:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanCompareMethod", "ii:z", &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanCompareMethod", "ii:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "ii:z", &res, VAL1, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanMethod", "ii:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 5;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ani_value args[1U];
    args[0U].i = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "protectedMethod", "i:z", &res, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "protectedMethod", "i:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "privateMethod", "i:z", &res, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "privateMethod", "i:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "callProtected", "i:z", &res, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "callProtected", "i:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL1), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "nestedMethod", "i:z", &res, VAL2), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ani_value args[1U];
    args[0U].i = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "nestedMethod", "i:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ani_value argsA[1];
    argsA[0].i = VAL1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "recursiveMethod", "i:z", &res, VAL1), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "recursiveMethod", "i:z", &res, argsA), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    const ani_int value1 = -5;
    ani_value argsB[1];
    argsB[0].i = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "recursiveMethod", "i:z", &res, value1), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "recursiveMethod", "i:z", &res, argsB), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj, VAL2), ANI_OK);

    ani_boolean res = ANI_FALSE;
    ani_char cValue1 = 'A';
    ani_double dValue1 = 1.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "calculateSum", nullptr, &res, VAL1, cValue1, dValue1),
              ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_char cValue2 = 'B';
    ani_value args[3U];
    args[0U].i = VAL1;
    args[1U].c = cValue2;
    args[2U].d = dValue1;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "calculateSum", nullptr, &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    ani_double dValue2 = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "calculateSum", nullptr, &res, VAL1, cValue2, dValue2),
              ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 15;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    const ani_int value1 = 5;
    const ani_int value2 = 6;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "ii:z", &res, value1, value2), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    const ani_int value3 = 7;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "ii:z", &res, value1, value3), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    const ani_int value4 = 3;
    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "booleanMethod", "ii:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);

    const ani_int value5 = 5;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "ii:z", &res, value1, value5), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    const ani_int value6 = 12;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "booleanMethod", "ii:z", &res, value1, value6), ANI_OK);
    ASSERT_EQ(res, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 10;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_boolean res = ANI_FALSE;
    const ani_int value = 10;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(obj, "jf", "i:z", &res, value), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);

    ani_value args[1U];
    args[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "jf", "i:z", &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_011)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;

    ani_boolean res = ANI_FALSE;
    ASSERT_EQ(
        env_->c_api->Object_CallMethodByName_Boolean(nullptr, object, "booleanByNameMethod", "ii:z", &res, VAL1, VAL2),
        ANI_INVALID_ARGS);
    ASSERT_EQ(
        env_->c_api->Object_CallMethodByName_Boolean_A(nullptr, object, "booleanByNameMethod", "ii:z", &res, args),
        ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(nullptr, "booleanByNameMethod", "ii:z", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(nullptr, "booleanByNameMethod", "ii:z", &res, args),
              ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, nullptr, "ii:z", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(object, nullptr, "ii:z", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "booleanByNameMethod", nullptr, &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(object, "booleanByNameMethod", nullptr, &res, args), ANI_OK);

    ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, "booleanByNameMethod", "ii:z", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(object, "booleanByNameMethod", "ii:z", nullptr, args),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_012)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;

    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    ani_boolean res = ANI_FALSE;
    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Boolean(object, methodName.data(), "ii:z", &res, VAL1, VAL2),
                  ANI_NOT_FOUND);
        ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(object, methodName.data(), "ii:z", &res, args),
                  ANI_NOT_FOUND);
    }
}

TEST_F(CallObjectMethodBooleanByNameTest, object_call_method_by_name_boolean_013)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;

    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    ani_boolean res = ANI_FALSE;
    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(
            env_->Object_CallMethodByName_Boolean(object, "booleanByNameMethod", methodName.data(), &res, VAL1, VAL2),
            ANI_INVALID_DESCRIPTOR);
        ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(object, "booleanByNameMethod", methodName.data(), &res, args),
                  ANI_INVALID_DESCRIPTOR);
    }
}

TEST_F(CallObjectMethodBooleanByNameTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_boolean_test.CheckWrongSignature", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_boolean res;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Boolean(env_, obj, "method", "C{std.core.String}:z", &res, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Boolean(env_, obj, "method", "C{std/core/String}:z", &res, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "method", "C{std.core.String}:z", &res, &arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Boolean_A(obj, "method", "C{std/core/String}:z", &res, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(obj, &res, str);
    TestFuncVWrongSignature(obj, &res, str);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)