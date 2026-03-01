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

#include "ani_gtest_object_ops.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class CallObjectMethodCharTest : public AniGtestObjectOps {
public:
    static constexpr ani_char VAL1 = 'a';
    static constexpr ani_char VAL2 = 'b';
    static constexpr ani_char VAL3 = 'A';
    static constexpr ani_char VAL4 = 'D';
    static constexpr ani_char VAL5 = 'F';
    static constexpr ani_char VAL6 = 'B';
    static constexpr ani_char VAL7 = 'i';
    static constexpr ani_int LOOP_COUNT = 3U;
};

TEST_F(CallObjectMethodCharTest, object_call_method_char_a)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethod", "cc:c", &object, &method);

    ani_value args[2U];
    ani_char arg1 = VAL1;
    ani_char arg2 = VAL2;
    args[0U].c = arg1;
    args[1U].c = arg2;

    ani_char result = '0';
    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, arg2);
}

TEST_F(CallObjectMethodCharTest, call_method_char_a_invalid_args)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethod", "cc:c", &object, &method);

    ani_char result = '0';
    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharTest, object_call_method_char_v)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethod", "cc:c", &object, &method);

    ani_char result = '0';
    ani_char arg1 = VAL1;
    ani_char arg2 = VAL2;
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, arg2);
}

TEST_F(CallObjectMethodCharTest, object_call_method_char)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethod", "cc:c", &object, &method);

    ani_char result = '0';
    ani_char arg1 = VAL1;
    ani_char arg2 = VAL2;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Char(env_, object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, arg2);
}

TEST_F(CallObjectMethodCharTest, call_method_char_v_invalid_env)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethod", "cc:c", &object, &method);

    ani_char result = '0';
    ani_char arg1 = VAL1;
    ani_char arg2 = VAL2;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Char(nullptr, object, method, &result, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharTest, call_method_char_v_invalid_object)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethod", "cc:c", &object, &method);

    ani_char result = '0';
    ani_char arg1 = VAL1;
    ani_char arg2 = VAL2;
    ASSERT_EQ(env_->Object_CallMethod_Char(nullptr, method, &result, arg1, arg2), ANI_INVALID_ARGS);
}
TEST_F(CallObjectMethodCharTest, call_method_char_v_invalid_method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethod", "cc:c", &object, &method);

    ani_char result = '0';
    ani_char arg1 = VAL1;
    ani_char arg2 = VAL2;
    ASSERT_EQ(env_->Object_CallMethod_Char(object, nullptr, &result, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharTest, call_method_char_v_invalid_result)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethod", "cc:c", &object, &method);

    ani_char arg1 = VAL1;
    ani_char arg2 = VAL2;
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodCharTest, call_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethodVoidParam", ":c", &object, &method);

    ani_char result = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result), ANI_OK);
    ASSERT_EQ(result, VAL3);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL3);
}

TEST_F(CallObjectMethodCharTest, call_Multiple_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethodMultipleParam", "bzffb:c", &object, &method);

    ani_value args[5U] = {};
    ani_byte arg1 = 2U;
    ani_boolean arg2 = ANI_TRUE;
    ani_float arg3 = 3.0F;
    ani_float arg4 = 4.0F;
    ani_byte arg5 = 5U;
    args[0U].b = arg1;
    args[1U].z = arg2;
    args[2U].f = arg3;
    args[3U].f = arg4;
    args[4U].b = arg5;

    ani_char result = '0';
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result, arg1, arg2, arg3, arg4, arg5), ANI_OK);
    ASSERT_EQ(result, VAL3);

    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL3);
}

TEST_F(CallObjectMethodCharTest, call_Parent_Class_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.B", "charMethodVoidParam", ":c", &object, &method);

    ani_char result = '0';
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result), ANI_OK);
    ASSERT_EQ(result, VAL3);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL3);
}

TEST_F(CallObjectMethodCharTest, call_Parent_Class_Method)
{
    ani_class clsC {};
    ASSERT_EQ(env_->FindClass("call_object_method_char_test.C", &clsC), ANI_OK);
    ASSERT_NE(clsC, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsC, "func", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_class clsD {};
    ASSERT_EQ(env_->FindClass("call_object_method_char_test.D", &clsD), ANI_OK);
    ASSERT_NE(clsD, nullptr);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(clsD, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsD, ctor, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_char result = '0';
    ani_value args[2U] = {};
    ani_char arg1 = VAL4;
    ani_byte arg2 = 2U;
    args[0U].c = arg1;
    args[1U].b = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL5);

    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL5);
}

TEST_F(CallObjectMethodCharTest, call_Sub_Class_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.E", "func", "cb:c", &object, &method);

    ani_char result = '0';
    ani_value args[2U] = {};
    ani_char arg1 = VAL4;
    ani_byte arg2 = 2U;
    args[0U].c = arg1;
    args[1U].b = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL6);

    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL6);
}

TEST_F(CallObjectMethodCharTest, multiple_Call_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "charMethod", "cc:c", &object, &method);

    ani_char result = '0';
    ani_value args[2U] = {};
    ani_char arg1 = 'X';
    ani_char arg2 = 'Y';
    args[0U].c = arg1;
    args[1U].c = arg2;

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result, arg1, arg2), ANI_OK);
        ASSERT_EQ(result, 'Y');
        ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
        ASSERT_EQ(result, 'Y');
    }
}

TEST_F(CallObjectMethodCharTest, call_Nested_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "nestedMethod", "ci:c", &object, &method);

    ani_char result = '0';
    ani_value args[2U] = {};
    ani_char arg1 = VAL3;
    ani_int arg2 = 3U;
    args[0U].c = arg1;
    args[1U].i = arg2;

    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL4);

    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL4);
}

TEST_F(CallObjectMethodCharTest, call_Recursion_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_char_test.A", "recursionMethod", "c:c", &object, &method);

    ani_char result = '0';
    ani_value args[1U] = {};
    ani_char arg1 = VAL3;
    args[0U].c = arg1;
    ASSERT_EQ(env_->Object_CallMethod_Char(object, method, &result, arg1), ANI_OK);
    ASSERT_EQ(result, VAL7);

    ASSERT_EQ(env_->Object_CallMethod_Char_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL7);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
