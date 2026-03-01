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

class CallObjectMethodByNameDoubleTest : public AniTest {
public:
    static constexpr ani_double VAL1 = 2.0;
    static constexpr ani_double VAL2 = 3.0;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.A", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(
            env_->Class_FindStaticMethod(cls, "new_A", ":C{object_call_method_by_name_double_test.A}", &newMethod),
            ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *objectResult = static_cast<ani_object>(ref);
    }

    void TestFuncVCorrectSignature(ani_object obj, ani_double *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Double_V(obj, "method", "C{std.core.String}:d", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_object obj, ani_double *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Double_V(obj, "method", "C{std/core/String}:d", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }
};

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double sum = 0.0;

    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "doubleMethod", "dd:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double sum = 0.0;

    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "doubleMethod", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "xxxxxxx", "dd:d", &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "doubleMethod", "dd:i", &sum, args), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_invalid_object)
{
    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(nullptr, "doubleMethod", "dd:d", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, nullptr, "dd:d", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_a_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "doubleMethod", "dd:d", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, cobject_call_method_by_name_double_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "doubleMethod", "dd:d", &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "doubleMethod", "dd:d", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "doubleMethod", nullptr, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "xxxxxxx", "dd:d", &sum, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "doubleMethod", "dd:i", &sum, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_invalid_object)
{
    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(nullptr, "doubleMethod", "dd:d", &sum, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, nullptr, "dd:d", &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "doubleMethod", "dd:d", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 100.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleMethod", "dd:d", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, arg);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "doubleMethod", "dd:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 100.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double sum = 0.0;
    const ani_double value = 5.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleMethod", "d:d", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg);

    ani_value args[1U];
    args[0U].d = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "doubleMethod", "d:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 100.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double sum = 0.0;
    const ani_double value1 = 5.0;
    const ani_double value2 = 8.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleAddMethod", "dd:d", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg + value1 + value2);

    ani_value args[2U];
    args[0U].d = value1;
    args[1U].d = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "doubleAddMethod", "dd:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value1 + value2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 100.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double sum = 0.0;
    const ani_double value1 = 5.0;
    const ani_double value2 = 8.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleMethod", "dd:d", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg - value1 - value2);

    ani_value args[2U];
    args[0U].d = value1;
    args[1U].d = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "doubleMethod", "dd:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg - value1 - value2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 100.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double num = 0.0;
    const ani_double value = 5.0;
    ani_value args[1U];
    args[0U].d = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "protectedMethod", "d:d", &num, value), ANI_OK);
    ASSERT_EQ(num, arg + value);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "protectedMethod", "d:d", &num, args), ANI_OK);
    ASSERT_EQ(num, arg + value);

    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "privateMethod", "d:d", &num, value), ANI_OK);
    ASSERT_EQ(num, arg - value);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "privateMethod", "d:d", &num, args), ANI_OK);
    ASSERT_EQ(num, arg - value);

    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "callProtected", "d:d", &num, value), ANI_OK);
    ASSERT_EQ(num, arg + value);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "callProtected", "d:d", &num, args), ANI_OK);
    ASSERT_EQ(num, arg + value);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 6.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double sum = 0.0;
    const ani_double value = 5.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "nestedMethod", "d:d", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg + value);

    ani_value args[1U];
    args[0U].d = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "nestedMethod", "d:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 6.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double sum = 0.0;
    const ani_int value1 = 5;
    const ani_double result = 120;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "recursiveMethod", "i:d", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "recursiveMethod", "i:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, result);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 6.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double sum = 0.0;
    const ani_double value1 = 1.0;
    const ani_char value2 = 'A';
    const ani_int value3 = 1;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "calculateSum", "dci:d", &sum, value1, value2, value3), ANI_OK);
    ASSERT_EQ(sum, arg - value1);

    const ani_char value4 = 'B';
    ani_value args[3U];
    args[0U].d = value1;
    args[1U].c = value4;
    args[2U].i = value3;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "calculateSum", "dci:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value1);

    const ani_int value5 = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "calculateSum", "dci:d", &sum, value1, value4, value5), ANI_OK);
    ASSERT_EQ(sum, arg + value1);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 15.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double sum = 0.0;
    const ani_double value1 = 5.0;
    const ani_double value2 = 6.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleMethod", "dd:d", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_double value3 = 7.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleMethod", "dd:d", &sum, value1, value3), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_double value4 = 3.0;
    ani_value args[2U];
    args[0U].d = value1;
    args[1U].d = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "doubleMethod", "dd:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_double value5 = 10.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleMethod", "dd:d", &sum, value1, value5), ANI_OK);
    ASSERT_EQ(sum, value1 + value5);

    const ani_double value6 = 12.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleMethod", "dd:d", &sum, value1, value6), ANI_OK);
    ASSERT_EQ(sum, value1 + value6);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "d:", &method), ANI_OK);

    ani_object obj {};
    const ani_double arg = 10.0;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_double sum = 0.0;
    const ani_double value = 2.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "jf", "d:d", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg + value);

    ani_value args[1U];
    args[0U].d = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "jf", "d:d", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_011)
{
    ani_object obj {};
    GetMethodData(&obj);

    ani_double sum = 0.0;
    const ani_double value1 = std::numeric_limits<ani_double>::max();
    const ani_double value2 = 0;
    ani_value args1[2U];
    args1[0U].d = value1;
    args1[1U].d = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleMethod", "dd:d", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "doubleMethod", "dd:d", &sum, args1), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);

    const ani_double value3 = std::numeric_limits<ani_double>::min();
    ani_value args2[2U];
    args2[0U].d = value3;
    args2[1U].d = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "doubleMethod", "dd:d", &sum, value3, value2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "doubleMethod", "dd:d", &sum, args2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_012)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double res = 0.0;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Double(nullptr, object, "doubleMethod", "dd:d", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Double_A(nullptr, object, "doubleMethod", "dd:d", &res, args),
              ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Double(nullptr, "doubleMethod", "dd:d", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(nullptr, "doubleMethod", "dd:d", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, nullptr, "dd:d", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, nullptr, "dd:d", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "doubleMethod", nullptr, &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "doubleMethod", nullptr, &res, args), ANI_OK);

    ASSERT_EQ(env_->Object_CallMethodByName_Double(object, "doubleMethod", "dd:d", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, "doubleMethod", "dd:d", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_013)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double res = 0.0;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Double(object, methodName.data(), "dd:d", &res, VAL1, VAL2),
                  ANI_NOT_FOUND);
        ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, methodName.data(), "dd:d", &res, args), ANI_NOT_FOUND);
    }
}

TEST_F(CallObjectMethodByNameDoubleTest, object_call_method_by_name_double_014)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double res = 0.0;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Double(object, methodName.data(), "", &res, VAL1, VAL2),
                  ANI_INVALID_DESCRIPTOR);
        ASSERT_EQ(env_->Object_CallMethodByName_Double_A(object, methodName.data(), "", &res, args),
                  ANI_INVALID_DESCRIPTOR);
    }
}

TEST_F(CallObjectMethodByNameDoubleTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_double_test.CheckWrongSignature", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_double res;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Double(env_, obj, "method", "C{std.core.String}:d", &res, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Double(env_, obj, "method", "C{std/core/String}:d", &res, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "method", "C{std.core.String}:d", &res, &arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Double_A(obj, "method", "C{std/core/String}:d", &res, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(obj, &res, str);
    TestFuncVWrongSignature(obj, &res, str);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)