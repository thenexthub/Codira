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

class CallObjectMethodByNameVoidTest : public AniTest {
public:
    static constexpr ani_long VAL1 = 1000000;
    static constexpr ani_long VAL2 = 2000000;

    void GetMethodData(ani_object *objectResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.A", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method newMethod {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "new_A", ":C{object_call_method_by_name_void_test.A}", &newMethod),
                  ANI_OK);
        ani_ref ref {};
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);
        *objectResult = static_cast<ani_object>(ref);
    }

    void TestFuncVCorrectSignature(ani_object obj, ...)
    {
        va_list args {};
        va_start(args, obj);
        ASSERT_EQ(env_->Object_CallMethodByName_Void_V(obj, "method", "C{std.core.String}:", args), ANI_OK);
        va_end(args);
    }

    void TestFuncVWrongSignature(ani_object obj, ...)
    {
        va_list args {};
        va_start(args, obj);
        ASSERT_EQ(env_->Object_CallMethodByName_Void_V(obj, "method", "C{std/core/String}:", args),
                  ANI_INVALID_DESCRIPTOR);
        va_end(args);
    }
};

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long value {};

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, "voidMethod", "ll:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long value {};

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, "voidMethod", nullptr, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ani_long value {};

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, "xxxxxxx", "ll:", args), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", "ll:l", &value), ANI_NOT_FOUND);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_invalid_object)
{
    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(nullptr, "voidMethod", "ll:", args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, nullptr, "ll:", args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_a_invalid_args)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, "voidMethod", "ll:", nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_normal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long value {};
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "voidMethod", "ll:", VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_normal_1)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long value {};
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "voidMethod", nullptr, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", nullptr, &value), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_abnormal)
{
    ani_object object {};
    GetMethodData(&object);

    ani_long value {};
    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "xxxxxxxxx", "ll:", VAL1, VAL2), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Object_CallMethodByName_Long(object, "getValue", "ll:l", &value), ANI_NOT_FOUND);
}
TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_invalid_object)
{
    ASSERT_EQ(env_->Object_CallMethodByName_Void(nullptr, "voidMethod", "ll:", VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_invalid_method)
{
    ani_object object {};
    GetMethodData(&object);

    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, nullptr, "ll:", VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    const ani_int value1 = 5;
    const ani_int value2 = 6;
    ani_int res = 0;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "ii:", value1, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);

    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidMethod", "ii:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_char res = 'a';
    const ani_char value = 'D';
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "c:", value), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "getCharValue", ":c", &res), ANI_OK);
    ASSERT_EQ(res, value);

    ani_value args[1U];
    args[0U].c = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidMethod", "c:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "getCharValue", ":c", &res), ANI_OK);
    ASSERT_EQ(res, value);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    ani_int arg = 100;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value1 = 5;
    const ani_int value2 = 8;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidSunMethod", "ii:", value1, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);

    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidSunMethod", "ii:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 10;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value1 = 5;
    const ani_int value2 = 5;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "ii:", value1, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, arg + value1 + value2);

    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidMethod", "ii:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, arg + value1 + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 5;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value = 5;
    ani_value argsA[1U];
    argsA[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "protectedMethod", "i:", value), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value);
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "protectedMethod", "i:", argsA), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value);

    ani_double res1 = 0.0;
    const ani_double value1 = 5.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "privateMethod", "d:", value1), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "getDoubleValue", ":d", &res1), ANI_OK);
    ASSERT_EQ(res1, value1);
    ani_value argsB[1U];
    argsB[0U].d = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "privateMethod", "d:", argsB), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "getDoubleValue", ":d", &res1), ANI_OK);
    ASSERT_EQ(res1, value1);

    const ani_int value2 = 5;
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &method), ANI_OK);
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "callProtected", "ii:", value, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value + value2);
    ani_value argsC[2U];
    argsC[0U].i = value;
    argsC[1U].i = value2;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "callProtected", "ii:", argsC), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 6;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value = 10;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "nestedMethod", "i:", value), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, arg + value);

    ani_value args[1U];
    args[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "nestedMethod", "i:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, arg + value);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 6;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value1 = 5;
    const ani_int value2 = 15;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "recursiveMethod", "i:", value1), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value2);

    ani_value args[1U];
    args[0U].i = value1;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "recursiveMethod", "i:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value2 + value2);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_008)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 6;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res1 = 0;
    ani_char res2 = 'a';
    ani_double res3 = 0.0;
    const ani_int value1 = 1;
    const ani_char value2 = 'A';
    const ani_double value3 = 1.0;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "calculateSum", "icd:", value1, value2, value3), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res1), ANI_OK);
    ASSERT_EQ(res1, value1);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "getCharValue", ":c", &res2), ANI_OK);
    ASSERT_EQ(res2, value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "getDoubleValue", ":d", &res3), ANI_OK);
    ASSERT_EQ(res1, value3);

    ani_value args[3U];
    args[0U].i = value1;
    args[1U].c = value2;
    args[2U].c = value3;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "calculateSum", "icd:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res1), ANI_OK);
    ASSERT_EQ(res1, value1);
    ASSERT_EQ(env_->Object_CallMethodByName_Char(obj, "getCharValue", ":c", &res2), ANI_OK);
    ASSERT_EQ(res2, value2);
    ASSERT_EQ(env_->Object_CallMethodByName_Double(obj, "getDoubleValue", ":d", &res3), ANI_OK);
    ASSERT_EQ(res1, value3);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_009)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    ani_int arg = 15;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value1 = 5;
    const ani_int value2 = 6;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "ii:", value1, value2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value2);

    const ani_int value3 = 7;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "ii:", value1, value3), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value3);

    const ani_int value4 = 3;
    ani_value args[2U];
    args[0U].i = value1;
    args[1U].i = value4;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "voidMethod", "ii:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value4);

    const ani_int value5 = 5;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "ii:", value1, value5), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value5);

    const ani_int value6 = 12;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "voidMethod", "ii:", value1, value6), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, value1 + value6);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_010)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.C", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &method), ANI_OK);

    ani_object obj {};
    const ani_int arg = 10;
    ASSERT_EQ(env_->Object_New(cls, method, &obj, arg), ANI_OK);

    ani_int res = 0;
    const ani_int value = 10;
    ASSERT_EQ(env_->Object_CallMethodByName_Void(obj, "jf", "i:", value), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, arg + value);

    ani_value args[1U];
    args[0U].i = value;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "jf", "i:", args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getIntValue", ":i", &res), ANI_OK);
    ASSERT_EQ(res, arg + value);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_011)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Void(nullptr, object, "voidMethod", "ll:", VAL1, VAL2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Void_A(nullptr, object, "voidMethod", "ll:", args),
              ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Void(nullptr, "voidMethod", "ll:", VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(nullptr, "voidMethod", "ll:", args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, nullptr, "ll:", VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, nullptr, "ll:", args), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Object_CallMethodByName_Void(object, "voidMethod", nullptr, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, "voidMethod", nullptr, args), ANI_OK);
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_012)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Void(object, methodName.data(), "ll:", VAL1, VAL2), ANI_NOT_FOUND);
        ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, methodName.data(), "ll:", args), ANI_NOT_FOUND);
    }
}

TEST_F(CallObjectMethodByNameVoidTest, object_call_method_by_name_void_013)
{
    ani_object object {};
    GetMethodData(&object);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;

    const std::array<std::string_view, 4U> invalidMethodNames = {{"", "测试emoji🙂🙂", "\n\r\t", "\x01\x02\x03"}};

    for (const auto &methodName : invalidMethodNames) {
        ASSERT_EQ(env_->Object_CallMethodByName_Void(object, methodName.data(), "", VAL1, VAL2),
                  ANI_INVALID_DESCRIPTOR);
        ASSERT_EQ(env_->Object_CallMethodByName_Void_A(object, methodName.data(), "", args), ANI_INVALID_DESCRIPTOR);
    }
}

TEST_F(CallObjectMethodByNameVoidTest, check_wrong_signature)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_call_method_by_name_void_test.CheckWrongSignature", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &method), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, method, &obj), ANI_OK);

    std::string input = "hello";

    ani_string str;
    ASSERT_EQ(env_->String_NewUTF8(input.c_str(), input.size(), &str), ANI_OK);

    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Void(env_, obj, "method", "C{std.core.String}:", str), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_CallMethodByName_Void(env_, obj, "method", "C{std/core/String}:", str),
              ANI_INVALID_DESCRIPTOR);

    ani_value arg;
    arg.r = str;
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "method", "C{std.core.String}:", &arg), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethodByName_Void_A(obj, "method", "C{std/core/String}:", &arg), ANI_INVALID_DESCRIPTOR);

    TestFuncVCorrectSignature(obj, str);
    TestFuncVWrongSignature(obj, str);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)