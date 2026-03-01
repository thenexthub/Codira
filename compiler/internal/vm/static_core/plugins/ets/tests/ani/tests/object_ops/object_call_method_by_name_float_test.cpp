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

class CallObjectMethodByNamefloatTest : public AniTest {
public:
    static constexpr ani_float VAL1 = 2.0F;
    static constexpr ani_float VAL2 = 3.0F;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.A", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":C{object_call_method_by_name_float_test.A}", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

        *objectResult = static_cast<ani_object>(ref);
    }

    void TestFuncVCorrectSignature(ani_object obj, ani_float *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Float_V(obj, "method", "C{std.core.String}:f", value, args), ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_object obj, ani_float *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Object_CallMethodByName_Float_V(obj, "method", "C{std/core/String}:f", value, args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }
};

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;
    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "floatMethod", "ff:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;
    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "floatMethod", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;
    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "xxxxxxxxx", "ff:f", &sum, args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "floatMethod", "ff:i", &sum, args), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_invalid_object)
{
    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;
    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(nullptr, "floatMethod", "ff:f", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;
    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, nullptr, "ff:f", &sum, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_a_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "floatMethod", "ff:f", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, cobject_call_method_by_name_float_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "floatMethod", "ff:f", &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "floatMethod", "ff:f", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "floatMethod", nullptr, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "xxxxxxxxxx", "ff:f", &sum, VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "floatMethod", "ff:i", &sum, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_invalid_object)
{
    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(nullptr, "floatMethod", "ff:f", &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, nullptr, "ff:f", &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_invalid_result)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "floatMethod", "ff:f", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 100.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float sum = 0.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatMethod", "ff:f", &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, arg);

    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "floatMethod", "ff:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 100.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float sum = 0.0F;
    const ani_float value = 5.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatMethod", "f:f", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg);

    ani_value args[1U];
    args[0U].f = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "floatMethod", "f:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 100.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float sum = 0.0F;
    const ani_float value1 = 5.0F;
    const ani_float value2 = 8.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatAddMethod", "ff:f", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg + value1 + value2);

    ani_value args[2U];
    args[0U].f = value1;
    args[1U].f = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "floatAddMethod", "ff:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value1 + value2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 100.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float sum = 0.0F;
    const ani_float value1 = 5.0F;
    const ani_float value2 = 8.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatMethod", "ff:f", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg - value1 - value2);

    ani_value args[2U];
    args[0U].f = value1;
    args[1U].f = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "floatMethod", "ff:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg - value1 - value2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 100.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float num = 0.0F;
    const ani_float value = 5.0F;
    ani_value args[1U];
    args[0U].f = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "protectedMethod", "f:f", &num, value), ANI_OK);
    ASSERT_EQ(num, arg + value);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "protectedMethod", "f:f", &num, args), ANI_OK);
    ASSERT_EQ(num, arg + value);

    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "privateMethod", "f:f", &num, value), ANI_OK);
    ASSERT_EQ(num, arg - value);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "privateMethod", "f:f", &num, args), ANI_OK);
    ASSERT_EQ(num, arg - value);

    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "callProtected", "f:f", &num, value), ANI_OK);
    ASSERT_EQ(num, arg + value);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "callProtected", "f:f", &num, args), ANI_OK);
    ASSERT_EQ(num, arg + value);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 6.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float sum = 0.0F;
    const ani_float value = 5.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "nestedMethod", "f:f", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg + value);

    ani_value args[1U];
    args[0U].f = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "nestedMethod", "f:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 6.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float sum = 0.0F;
    const ani_int value1 = 5;
    const ani_float result = 120.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "recursiveMethod", "i:f", &sum, value1), ANI_OK);
    ASSERT_EQ(sum, result);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "recursiveMethod", "i:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, result);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 60.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float sum = 0.0F;
    const ani_float value1 = 10.0F;
    const ani_char value2 = 'A';
    const ani_int value3 = 1;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "calculateSum", "fci:f", &sum, value1, value2, value3), ANI_OK);
    ASSERT_EQ(sum, arg - value1);

    const ani_char value4 = 'B';
    ani_value args[3U];
    args[0U].f = value1;
    args[1U].c = value4;
    args[2U].i = value3;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "calculateSum", "fci:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, value1);

    const ani_int value5 = 2U;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "calculateSum", "fci:f", &sum, value1, value4, value5), ANI_OK);
    ASSERT_EQ(sum, arg + value1);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 15.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float sum = 0.0F;
    const ani_float value1 = 5.0F;
    const ani_float value2 = 6.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatMethod", "ff:f", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_float value3 = 7.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatMethod", "ff:f", &sum, value1, value3), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_float value4 = 3.0F;
    ani_value args[2U];
    args[0U].f = value1;
    args[1U].f = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "floatMethod", "ff:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg);

    const ani_float value5 = 10.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatMethod", "ff:f", &sum, value1, value5), ANI_OK);
    ASSERT_EQ(sum, value1 + value5);

    const ani_float value6 = 12.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatMethod", "ff:f", &sum, value1, value6), ANI_OK);
    ASSERT_EQ(sum, value1 + value6);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "f:", &method), ANI_OK);

    ani_object obj {};
    const ani_float arg = 10.0F;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_float sum = 0.0F;
    const ani_float value = 2.0F;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "jf", "f:f", &sum, value), ANI_OK);
    ASSERT_EQ(sum, arg + value);

    ani_value args[1U];
    args[0U].f = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "jf", "f:f", &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg + value);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_011)
{
    ani_object obj {};
    GetMethodData(&obj);

    ani_float sum = 0.0F;
    const ani_float value1 = std::numeric_limits<ani_float>::max();
    const ani_float value2 = 0.0F;
    ani_value args1[2U];
    args1[0U].f = value1;
    args1[1U].f = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatMethod", "ff:f", &sum, value1, value2), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "floatMethod", "ff:f", &sum, args1), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);

    const ani_float value3 = std::numeric_limits<ani_float>::min();
    ani_value args2[2U];
    args2[0U].f = value3;
    args2[1U].f = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Float(obj, "floatMethod", "ff:f", &sum, value3, value2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "floatMethod", "ff:f", &sum, args2), ANI_OK);
    ASSERT_EQ(sum, value3 + value2);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_012)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;

    ani_float res = 0.0F;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Float(nullptr, object, "floatMethod", "ff:f", &res, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Float_A(nullptr, object, "floatMethod", "ff:f", &res, args),
              ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Float(nullptr, "floatMethod", "ff:f", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(nullptr, "floatMethod", "ff:f", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, nullptr, "ff:f", &res, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, nullptr, "ff:f", &res, args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "floatMethod", nullptr, &res, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "floatMethod", nullptr, &res, args), ANI_OK);

    ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "floatMethod", "ff:f", nullptr, VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "floatMethod", "ff:f", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_013)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;

    ani_float res = 0.0F;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Float(object, methodName.data(), "ff:f", &res, VAL1, VAL2),
                  ANI_NOT_FOUND);
        ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, methodName.data(), "ff:f", &res, args), ANI_NOT_FOUND);
    }
}

TEST_F(CallObjectMethodByNamefloatTest, object_call_method_by_name_float_014)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].f = VAL1;
    args[1U].f = VAL2;

    ani_float res = 0.0F;
    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Float(object, "floatMethod", methodName.data(), &res, VAL1, VAL2),
                  ANI_INVALID_DESCRIPTOR);
        ASSERT_EQ(env_->Object_CallMethodByName_Float_A(object, "floatMethod", methodName.data(), &res, args),
                  ANI_INVALID_DESCRIPTOR);
    }
}

TEST_F(CallObjectMethodByNamefloatTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_float_test.CheckWrongSignature", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ani_float res;
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Float(env_, obj, "method", "C{std.core.String}:f", &res, str),
              ANI_OK);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Float(env_, obj, "method", "C{std/core/String}:f", &res, str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "method", "C{std.core.String}:f", &res, &arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Float_A(obj, "method", "C{std/core/String}:f", &res, &arg),
              ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(obj, &res, str);
    TestFuncVWrongSignature(obj, &res, str);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)