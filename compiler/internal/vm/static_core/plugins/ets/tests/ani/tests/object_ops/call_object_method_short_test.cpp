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

class CallObjectMethodShortTest : public AniGtestObjectOps {
public:
    static constexpr ani_short INIT_VALUE = 0U;
    static constexpr ani_short VAL = 2U;
    static constexpr ani_short VAL1 = 3U;
    static constexpr ani_short VAL2 = 5U;
    static constexpr ani_short VAL3 = 6U;
    static constexpr ani_short VAL4 = 103U;
};

TEST_F(CallObjectMethodShortTest, object_call_method_short_a)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethod", "ss:s", &object, &method);

    ani_value args[2U];
    ani_short arg1 = VAL;
    ani_short arg2 = VAL1;
    args[0U].s = arg1;
    args[1U].s = arg2;

    ani_short sum = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg2);
}

TEST_F(CallObjectMethodShortTest, object_call_method_short_v)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethod", "ss:s", &object, &method);

    ani_short sum = INIT_VALUE;
    ani_short arg1 = VAL;
    ani_short arg2 = VAL1;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg2);
}

TEST_F(CallObjectMethodShortTest, object_call_method_short)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethod", "ss:s", &object, &method);

    ani_short sum = INIT_VALUE;
    ani_short arg1 = 4U;
    ani_short arg2 = VAL1;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Short(env_, object, method, &sum, arg1, arg2), ANI_OK);
    ASSERT_EQ(sum, arg1);
}

TEST_F(CallObjectMethodShortTest, call_method_short_v_invalid_env)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethod", "ss:s", &object, &method);

    ani_short sum = INIT_VALUE;
    ani_short arg1 = VAL2;
    ani_short arg2 = VAL3;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Short(nullptr, object, method, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortTest, call_method_short_v_invalid_method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethod", "ss:s", &object, &method);

    ani_short sum = INIT_VALUE;
    ani_short arg1 = VAL2;
    ani_short arg2 = VAL3;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, nullptr, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortTest, call_method_short_v_invalid_result)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethod", "ss:s", &object, &method);

    ani_short arg1 = VAL2;
    ani_short arg2 = VAL3;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortTest, call_method_short_v_invalid_object)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethod", "ss:s", &object, &method);

    ani_short sum = INIT_VALUE;
    ani_short arg1 = VAL2;
    ani_short arg2 = VAL3;
    ASSERT_EQ(env_->Object_CallMethod_Short(nullptr, method, &sum, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortTest, call_method_short_a_invalid_args)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethod", "ss:s", &object, &method);

    ani_short sum = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodShortTest, call_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethodVoidParam", ":s", &object, &method);

    ani_short result = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &result), ANI_OK);
    ASSERT_EQ(result, VAL2);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL2);
}

TEST_F(CallObjectMethodShortTest, call_Multiple_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethodMultipleParam", "izfss:s", &object, &method);

    ani_value args[5U] = {};
    ani_int arg1 = VAL;
    ani_boolean arg2 = ANI_TRUE;
    ani_float arg3 = 3.0F;
    ani_short arg4 = 4U;
    ani_short arg5 = VAL2;
    args[0U].i = arg1;
    args[1U].z = arg2;
    args[2U].f = arg3;
    args[3U].s = arg4;
    args[4U].s = arg5;

    ani_short result = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &result, arg1, arg2, arg3, arg4, arg5), ANI_OK);
    ASSERT_EQ(result, arg4 + arg5);

    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, arg4 + arg5);
}

TEST_F(CallObjectMethodShortTest, call_Parent_Class_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.B", "shortMethodVoidParam", ":s", &object, &method);

    ani_short result = INIT_VALUE;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &result), ANI_OK);
    ASSERT_EQ(result, VAL2);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL2);
}

TEST_F(CallObjectMethodShortTest, call_Parent_Class_Method)
{
    ani_class clsC {};
    ASSERT_EQ(env_->FindClass("call_object_method_short_test.C", &clsC), ANI_OK);
    ASSERT_NE(clsC, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsC, "func", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_class clsD {};
    ASSERT_EQ(env_->FindClass("call_object_method_short_test.D", &clsD), ANI_OK);
    ASSERT_NE(clsD, nullptr);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(clsD, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsD, ctor, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_short result = 0U;
    ani_value args[2U] = {};
    ani_short arg1 = VAL;
    ani_short arg2 = VAL1;
    args[0U].s = arg1;
    args[1U].s = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL2);

    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL2);
}

TEST_F(CallObjectMethodShortTest, call_Sub_Class_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.E", "func", nullptr, &object, &method);

    ani_short result = 0U;
    ani_value args[2U] = {};
    ani_short arg1 = VAL3;
    ani_short arg2 = VAL1;
    args[0U].s = arg1;
    args[1U].s = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL1);

    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL1);
}

TEST_F(CallObjectMethodShortTest, multiple_Call_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "shortMethod", "ss:s", &object, &method);

    ani_short result = INIT_VALUE;
    ani_value args[2U] = {};
    ani_short arg1 = VAL1;
    ani_short arg2 = 4U;
    args[0U].s = arg1;
    args[1U].s = arg2;

    for (ani_short i = 0; i < VAL1; i++) {
        ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &result, arg1, arg2), ANI_OK);
        ASSERT_EQ(result, 4U);
        ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &result, args), ANI_OK);
        ASSERT_EQ(result, 4U);
    }
}

TEST_F(CallObjectMethodShortTest, call_Nested_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "nestedMethod", "ss:s", &object, &method);

    ani_short result = INIT_VALUE;
    ani_value args[2U] = {};
    ani_short arg1 = VAL1;
    ani_short arg2 = VAL1;
    args[0U].i = arg1;
    args[1U].i = arg2;

    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, VAL3);

    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL3);
}

TEST_F(CallObjectMethodShortTest, call_Recursion_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_short_test.A", "recursionMethod", "s:s", &object, &method);

    ani_short result = INIT_VALUE;
    ani_value args[1U] = {};
    ani_short arg1 = VAL1;
    args[0U].i = arg1;
    ASSERT_EQ(env_->Object_CallMethod_Short(object, method, &result, arg1), ANI_OK);
    ASSERT_EQ(result, VAL4);

    ASSERT_EQ(env_->Object_CallMethod_Short_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, VAL4);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
