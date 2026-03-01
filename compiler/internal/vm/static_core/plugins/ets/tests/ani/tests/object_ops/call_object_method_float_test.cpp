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

class CallObjectMethodFloatTest : public AniGtestObjectOps {
public:
    static constexpr ani_float INIT_VALUE = 0.0F;
    static constexpr ani_float VAL = 2.0F;
    static constexpr ani_float VAL1 = 3.0F;
    static constexpr ani_float VAL2 = 4.0F;
    static constexpr ani_float VAL3 = 5.0F;
    static constexpr ani_float VAL4 = 7.0F;
    static constexpr ani_float VAL5 = 103.0F;
    static constexpr ani_int LOOP_COUNT = 3U;
};

TEST_F(CallObjectMethodFloatTest, object_call_method_float_a)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethod", "ff:f", &object, &method);

    ani_value args[2U];
    ani_float arg1 = VAL;
    ani_float arg2 = VAL1;
    args[0U].f = arg1;
    args[1U].f = arg2;

    ani_float sum = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodFloatTest, object_call_method_float_v)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethod", "ff:f", &object, &method);

    ani_float sum = INIT_VALUE;
    ani_float arg1 = VAL;
    ani_float arg2 = VAL1;
    ASSERT_EQ(env_->Object_CallMethod_Float(object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodFloatTest, object_call_method_float)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethod", "ff:f", &object, &method);

    ani_float sum = INIT_VALUE;
    ani_float arg1 = VAL;
    ani_float arg2 = VAL1;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Float(env_, object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(CallObjectMethodFloatTest, object_call_method_float_v_invalid_env)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethod", "ff:f", &object, &method);

    ani_float sum = INIT_VALUE;
    ani_float arg1 = VAL;
    ani_float arg2 = VAL1;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Float(nullptr, object, method, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodFloatTest, call_method_float_v_invalid_method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethod", "ff:f", &object, &method);

    ani_float sum = INIT_VALUE;
    ani_float arg1 = VAL;
    ani_float arg2 = VAL1;
    ASSERT_EQ(env_->Object_CallMethod_Float(object, nullptr, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodFloatTest, call_method_float_v_invalid_result)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethod", "ff:f", &object, &method);

    ani_float arg1 = VAL;
    ani_float arg2 = VAL1;
    ASSERT_EQ(env_->Object_CallMethod_Float(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodFloatTest, call_method_float_v_invalid_object)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethod", "ff:f", &object, &method);

    ani_float sum = INIT_VALUE;
    ani_float arg1 = VAL;
    ani_float arg2 = VAL1;
    ASSERT_EQ(env_->Object_CallMethod_Float(nullptr, method, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodFloatTest, call_method_float_a_invalid_args)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethod", "ff:f", &object, &method);

    ani_float sum = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodFloatTest, call_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethodVoidParam", ":f", &object, &method);

    ani_float result = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Float(object, method, &result), ANI_OK);
    ASSERT_EQ(result, VAL3);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL3);
}

TEST_F(CallObjectMethodFloatTest, call_Multiple_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethodMultipleParam", "szfsf:f", &object, &method);

    ani_value args[5U] = {};
    ani_short arg1 = 2U;
    ani_boolean arg2 = ANI_TRUE;
    ani_float arg3 = VAL1;
    ani_short arg4 = 4U;
    ani_float arg5 = VAL3;
    args[0U].s = arg1;
    args[1U].z = arg2;
    args[2U].f = arg3;
    args[3U].s = arg4;
    args[4U].f = arg5;

    ani_float result = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Float(object, method, &result, arg1, arg2, arg3, arg4, arg5), ANI_OK);
    ASSERT_EQ(result, arg3 + arg5);

    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, arg3 + arg5);
}

TEST_F(CallObjectMethodFloatTest, call_Parent_Class_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.B", "floatMethodVoidParam", ":f", &object, &method);

    ani_float result = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Float(object, method, &result), ANI_OK);
    ASSERT_EQ(result, VAL3);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL3);
}

TEST_F(CallObjectMethodFloatTest, call_Parent_Class_Method)
{
    ani_class clsC {};
    ASSERT_EQ(env_->FindClass("call_object_method_float_test.C", &clsC), ANI_OK);
    ASSERT_NE(clsC, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsC, "func", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_class clsD {};
    ASSERT_EQ(env_->FindClass("call_object_method_float_test.D", &clsD), ANI_OK);
    ASSERT_NE(clsD, nullptr);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(clsD, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsD, ctor, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_float result = 0U;
    ani_value args[2U] = {};
    ani_float arg1 = VAL1;
    ani_float arg2 = VAL;
    args[0U].f = arg1;
    args[1U].f = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Float(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL3);

    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL3);
}

TEST_F(CallObjectMethodFloatTest, call_Sub_Class_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.E", "func", nullptr, &object, &method);

    ani_float result = 0U;
    ani_value args[2U] = {};
    ani_float arg1 = VAL3;
    ani_float arg2 = VAL1;
    args[0U].f = arg1;
    args[1U].f = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Float(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL);

    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL);
}

TEST_F(CallObjectMethodFloatTest, multiple_Call_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "floatMethod", "ff:f", &object, &method);

    ani_float result = 0U;
    ani_value args[2U] = {};
    ani_float arg1 = VAL1;
    ani_float arg2 = VAL2;
    args[0U].f = arg1;
    args[1U].f = arg2;

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Object_CallMethod_Float(object, method, &result, arg1, arg2), ANI_OK);
        ASSERT_EQ(result, VAL4);
        ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &result, args), ANI_OK);
        ASSERT_EQ(result, VAL4);
    }
}

TEST_F(CallObjectMethodFloatTest, call_Nested_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "nestedMethod", "ff:f", &object, &method);

    ani_float result = INIT_VALUE;
    ani_value args[2U] = {};
    ani_float arg1 = VAL1;
    ani_float arg2 = VAL2;
    args[0U].f = arg1;
    args[1U].f = arg2;

    ASSERT_EQ(env_->Object_CallMethod_Float(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL4);

    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL4);
}

TEST_F(CallObjectMethodFloatTest, call_Recursion_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_float_test.A", "recursionMethod", "f:f", &object, &method);

    ani_float result = INIT_VALUE;
    ani_value args[1U] = {};
    ani_float arg1 = VAL1;
    args[0U].f = arg1;
    ASSERT_EQ(env_->Object_CallMethod_Float(object, method, &result, arg1), ANI_OK);
    ASSERT_EQ(result, VAL5);

    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL5);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
