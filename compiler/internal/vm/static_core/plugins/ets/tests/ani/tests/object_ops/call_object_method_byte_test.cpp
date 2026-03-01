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

class CallObjectMethodByteTest : public AniGtestObjectOps {
public:
    static constexpr ani_int VAL1 = 2U;
    static constexpr ani_int VAL2 = 3U;
    static constexpr ani_int VAL3 = 4U;
    static constexpr ani_int VAL4 = 5U;
    static constexpr ani_int VAL5 = 6U;
    static constexpr ani_int VAL6 = 103U;
};

TEST_F(CallObjectMethodByteTest, object_call_method_byte_a)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethod", "bb:b", &object, &method);

    ani_value args[VAL1];
    ani_byte arg1 = VAL4;
    ani_byte arg2 = VAL2;
    args[0U].b = arg1;
    args[1U].b = arg2;

    ani_byte sum = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &sum, args), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_CallMethod_Byte_A(env_, object, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1);
}

TEST_F(CallObjectMethodByteTest, object_call_method_byte_v)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethod", "bb:b", &object, &method);

    ani_byte sum = 0U;
    ani_byte arg1 = VAL1;
    ani_byte arg2 = VAL2;
    ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg2);
}

TEST_F(CallObjectMethodByteTest, object_call_method_byte)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethod", "bb:b", &object, &method);

    ani_byte sum = 0U;
    ani_byte arg1 = VAL3;
    ani_byte arg2 = VAL2;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Byte(env_, object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1);
}

TEST_F(CallObjectMethodByteTest, call_method_byte_v_invalid_env)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethod", "bb:b", &object, &method);

    ani_byte sum = 0U;
    ani_byte arg1 = VAL4;
    ani_byte arg2 = VAL5;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Byte(nullptr, object, method, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteTest, call_method_byte_v_invalid_method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethod", "bb:b", &object, &method);

    ani_byte sum = 0U;
    ani_byte arg1 = VAL4;
    ani_byte arg2 = VAL5;
    ASSERT_EQ(env_->Object_CallMethod_Byte(object, nullptr, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteTest, call_method_byte_v_invalid_result)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethod", "bb:b", &object, &method);

    ani_byte arg1 = VAL4;
    ani_byte arg2 = VAL5;
    ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteTest, call_method_byte_v_invalid_object)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethod", "bb:b", &object, &method);

    ani_byte sum = 0U;
    ani_byte arg1 = VAL4;
    ani_byte arg2 = VAL5;
    ASSERT_EQ(env_->Object_CallMethod_Byte(nullptr, method, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteTest, call_method_byte_a_invalid_args)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethod", "bb:b", &object, &method);

    ani_byte sum = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodByteTest, call_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethodVoidParam", ":b", &object, &method);

    ani_byte result = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, &result), ANI_OK);
    ASSERT_EQ(result, VAL4);

    ani_value args[VAL1];
    ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL4);
}

TEST_F(CallObjectMethodByteTest, call_Multiple_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethodMultipleParam", "bzffb:b", &object, &method);

    ani_value args[5U] = {};
    ani_byte arg1 = VAL1;
    ani_boolean arg2 = ANI_TRUE;
    ani_float arg3 = 3.0F;
    ani_float arg4 = 4.0F;
    ani_byte arg5 = VAL4;
    args[0U].b = arg1;
    args[1U].z = arg2;
    args[2U].f = arg3;
    args[3U].f = arg4;
    args[4U].b = arg5;

    ani_byte result = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, &result, arg1, arg2, arg3, arg4, arg5), ANI_OK);
    ASSERT_EQ(result, arg5);

    ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, arg5);
}

TEST_F(CallObjectMethodByteTest, call_Parent_Class_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.B", "byteMethodVoidParam", ":b", &object, &method);

    ani_byte result = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, &result), ANI_OK);
    ASSERT_EQ(result, VAL4);

    ani_value args[VAL1];
    ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL4);
}

TEST_F(CallObjectMethodByteTest, call_Parent_Class_Method)
{
    ani_class clsC {};
    ASSERT_EQ(env_->FindClass("call_object_method_byte_test.C", &clsC), ANI_OK);
    ASSERT_NE(clsC, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsC, "func", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_class clsD {};
    ASSERT_EQ(env_->FindClass("call_object_method_byte_test.D", &clsD), ANI_OK);
    ASSERT_NE(clsD, nullptr);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(clsD, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsD, ctor, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_byte result = 0U;
    ani_value args[2U] = {};
    ani_byte arg1 = VAL3;
    ani_byte arg2 = VAL1;
    args[0U].b = arg1;
    args[1U].b = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL5);

    ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL5);
}

TEST_F(CallObjectMethodByteTest, call_Sub_Class_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.E", "func", nullptr, &object, &method);

    ani_byte result = 0U;
    ani_value args[2U] = {};
    ani_byte arg1 = VAL3;
    ani_byte arg2 = VAL1;
    args[0U].b = arg1;
    args[1U].b = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL1);

    ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL1);
}

TEST_F(CallObjectMethodByteTest, multiple_Call_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "byteMethod", "bb:b", &object, &method);

    ani_byte result = 0U;
    ani_value args[2U] = {};
    ani_byte arg1 = VAL3;
    ani_byte arg2 = VAL2;
    args[0U].b = arg1;
    args[1U].b = arg2;

    for (ani_int i = 0; i < VAL2; i++) {
        ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, &result, arg1, arg2), ANI_OK);
        ASSERT_EQ(result, VAL3);
        ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &result, args), ANI_OK);
        ASSERT_EQ(result, VAL3);
    }
}

TEST_F(CallObjectMethodByteTest, call_Nested_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "nestedMethod", "bb:b", &object, &method);

    ani_byte result = 0U;
    ani_value args[2U] = {};
    ani_byte arg1 = VAL2;
    ani_byte arg2 = VAL2;
    args[0U].b = arg1;
    args[1U].b = arg2;

    ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL5);

    ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL5);
}

TEST_F(CallObjectMethodByteTest, call_Recursion_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_byte_test.A", "recursionMethod", "b:b", &object, &method);

    ani_byte result = 0U;
    ani_value args[1U] = {};
    ani_byte arg1 = VAL2;
    args[0U].i = arg1;
    ASSERT_EQ(env_->Object_CallMethod_Byte(object, method, &result, arg1), ANI_OK);
    ASSERT_EQ(result, VAL6);

    ASSERT_EQ(env_->Object_CallMethod_Byte_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL6);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
