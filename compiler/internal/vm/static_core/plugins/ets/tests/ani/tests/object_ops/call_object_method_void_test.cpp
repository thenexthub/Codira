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

class CallObjectMethodVoidTest : public AniGtestObjectOps {
public:
    static constexpr ani_int INIT_VALUE = 0U;
    static constexpr ani_int VAL = 2U;
    static constexpr ani_int VAL1 = 3U;
    static constexpr ani_int VAL2 = 5U;
    static constexpr ani_int VAL3 = 6U;
    static constexpr ani_int VAL4 = 10U;
    static constexpr ani_int VAL5 = 103U;
    static constexpr ani_int VAL6 = 7U;
    void GetValueMethod(ani_method *checkMethod)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_object_method_void_test.A", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ASSERT_EQ(env_->Class_FindMethod(cls, "getCount", ":i", checkMethod), ANI_OK);
        ASSERT_NE(checkMethod, nullptr);
    }
};

TEST_F(CallObjectMethodVoidTest, object_call_method_void_a)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);
    ani_method getMethod {};
    GetValueMethod(&getMethod);

    ani_value args[2];
    ani_int arg1 = VAL;
    ani_int arg2 = VAL1;
    args[0].i = arg1;
    args[1].i = arg2;
    ani_int sum = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, voidMethod, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, args[0].i + args[1].i);
}

TEST_F(CallObjectMethodVoidTest, object_call_method_void_v)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);
    ani_method getMethod {};
    GetValueMethod(&getMethod);

    ani_int a = 4;
    ani_int b = 5;
    ani_int sum = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Void(object, voidMethod, a, b), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallObjectMethodVoidTest, object_call_method_void)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);
    ani_method getMethod {};
    GetValueMethod(&getMethod);

    ani_int a = VAL3;
    ani_int b = VAL6;
    ani_int sum = INIT_VALUE;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Void(env_, object, voidMethod, a, b), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_v_invalid_env)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);

    ani_int a = VAL3;
    ani_int b = VAL6;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Void(nullptr, object, voidMethod, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_v_invalid_method)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);

    ani_int a = VAL3;
    ani_int b = VAL6;
    ASSERT_EQ(env_->Object_CallMethod_Void(object, nullptr, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_v_invalid_object)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);

    ani_int a = VAL3;
    ani_int b = VAL6;
    ASSERT_EQ(env_->Object_CallMethod_Void(nullptr, voidMethod, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_a_invalid_method)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);

    ani_value args[2];
    ani_int arg1 = VAL;
    ani_int arg2 = VAL1;
    args[0].i = arg1;
    args[1].i = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_a_invalid_object)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);

    ani_value args[2];
    ani_int arg1 = VAL;
    ani_int arg2 = VAL1;
    args[0].i = arg1;
    args[1].i = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Void_A(nullptr, voidMethod, args), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_a_invalid_args)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);

    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, voidMethod, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_method_void_invalid_env)
{
    ani_object object {};
    ani_method voidMethod {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &voidMethod);
    ani_int arg1 = 2;
    ani_int arg2 = 3;

    ASSERT_EQ(env_->c_api->Object_CallMethod_Void(nullptr, object, voidMethod, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodVoidTest, call_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethodVoidParam", ":", &object, &method);
    ani_method checkMethod {};
    GetValueMethod(&checkMethod);

    ani_int result {};
    ASSERT_EQ(env_->Object_CallMethod_Void(object, method), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL4);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, method, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL4);
}

TEST_F(CallObjectMethodVoidTest, call_Multiple_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethodMultipleParam", "izfif:", &object, &method);

    const ani_int arg1 = VAL;
    const ani_boolean arg2 = ANI_TRUE;
    const ani_float arg3 = 3.0F;
    const ani_int arg4 = 4U;
    const ani_float arg5 = 5.0F;
    ani_int result = 0;
    ani_method checkMethod {};
    GetValueMethod(&checkMethod);
    ASSERT_EQ(env_->Object_CallMethod_Void(object, method, arg1, arg2, arg3, arg4, arg5), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL3);
}

TEST_F(CallObjectMethodVoidTest, call_Parent_Class_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_void_test.B", "voidMethodVoidParam", ":", &object, &method);

    ani_int result = 0;
    ani_method checkMethod {};
    GetValueMethod(&checkMethod);
    ASSERT_EQ(env_->Object_CallMethod_Void(object, method), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL4);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, method, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL4);
}

TEST_F(CallObjectMethodVoidTest, call_Parent_Class_Method)
{
    ani_class clsC {};
    ASSERT_EQ(env_->FindClass("call_object_method_void_test.C", &clsC), ANI_OK);
    ASSERT_NE(clsC, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsC, "func", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_class clsD {};
    ASSERT_EQ(env_->FindClass("call_object_method_void_test.D", &clsD), ANI_OK);
    ASSERT_NE(clsD, nullptr);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(clsD, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsD, ctor, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U] = {};
    ani_int arg1 = VAL;
    ani_int arg2 = VAL1;
    args[0U].i = arg1;
    args[1U].i = arg2;

    ani_int result = 0;
    ani_method checkMethod {};
    ASSERT_EQ(env_->Class_FindMethod(clsD, "getCount", ":i", &checkMethod), ANI_OK);
    ASSERT_NE(checkMethod, nullptr);

    ASSERT_EQ(env_->Object_CallMethod_Void(object, method, arg1, arg2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL2);

    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, method, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL2);
}

TEST_F(CallObjectMethodVoidTest, call_Sub_Class_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_void_test.E", "func", nullptr, &object, &method);

    ani_int result = 0;
    ani_value args[2U] = {};
    ani_int arg1 = VAL3;
    ani_int arg2 = VAL1;
    args[0U].i = arg1;
    args[1U].i = arg2;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_object_method_void_test.E", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ani_method checkMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "getCount", ":i", &checkMethod), ANI_OK);
    ASSERT_NE(checkMethod, nullptr);

    ASSERT_EQ(env_->Object_CallMethod_Void(object, method, arg1, arg2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL1);

    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, method, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL1);
}

TEST_F(CallObjectMethodVoidTest, multiple_Call_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_void_test.A", "voidMethod", "ii:", &object, &method);

    ani_value args[2U] = {};
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL1;
    args[0U].i = arg1;
    args[1U].i = arg2;

    ani_int result = 0;
    ani_method checkMethod {};
    GetValueMethod(&checkMethod);

    for (ani_int i = 0; i < VAL1; i++) {
        ASSERT_EQ(env_->Object_CallMethod_Void(object, method, arg1, arg2), ANI_OK);
        ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
        ASSERT_EQ(result, VAL3);
        ASSERT_EQ(env_->Object_CallMethod_Void_A(object, method, args), ANI_OK);
        ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
        ASSERT_EQ(result, VAL3);
    }
}

TEST_F(CallObjectMethodVoidTest, call_Nested_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_void_test.A", "nestedMethod", "ii:", &object, &method);

    ani_value args[2U] = {};
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL1;
    args[0U].i = arg1;
    args[1U].i = arg2;

    ani_int result = 0;
    ani_method checkMethod {};
    GetValueMethod(&checkMethod);
    ASSERT_EQ(env_->Object_CallMethod_Void(object, method, arg1, arg2), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL3);

    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, method, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL3);
}

TEST_F(CallObjectMethodVoidTest, call_Recursion_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_void_test.A", "recursionMethod", "i:", &object, &method);

    ani_value args[1U] = {};
    ani_int arg1 = VAL1;
    args[0U].i = arg1;

    ani_int result = 0;
    ani_method checkMethod {};
    GetValueMethod(&checkMethod);
    ASSERT_EQ(env_->Object_CallMethod_Void(object, method, arg1), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL5);

    ASSERT_EQ(env_->Object_CallMethod_Void_A(object, method, args), ANI_OK);
    ASSERT_EQ(env_->Object_CallMethod_Int(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, VAL5);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
