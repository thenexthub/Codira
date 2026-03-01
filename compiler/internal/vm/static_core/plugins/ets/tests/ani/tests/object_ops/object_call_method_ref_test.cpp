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

class ObjectCallMethodRefTest : public AniGtestObjectOps {
public:
    static constexpr ani_int INIT_VALUE = 0;
    static constexpr ani_int VAL = 2;
    static constexpr ani_int VAL1 = 3;
    static constexpr ani_int VAL2 = 5;
    static constexpr ani_int VAL3 = 6;
    static constexpr ani_int VAL4 = 11;
};

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_a)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethod", nullptr, &object, &method);

    ani_value args[2U];
    ani_int arg1 = VAL;
    ani_int arg2 = VAL1;
    args[0U].i = arg1;
    args[1U].i = arg2;

    std::string str {};
    ani_ref result {};
    ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, args), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "nihao");
}

TEST_F(ObjectCallMethodRefTest, object_call_method_ref_v)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethod", nullptr, &object, &method);

    ani_int arg1 = VAL;
    ani_int arg2 = VAL1;
    std::string str {};
    ani_ref result {};
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "nihao");
}

TEST_F(ObjectCallMethodRefTest, call_method_ref_v_invalid_env)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethod", nullptr, &object, &method);

    ani_ref result {};
    ani_int arg1 = VAL2;
    ani_int arg2 = VAL3;
    ASSERT_EQ(env_->c_api->Object_CallMethod_Ref(nullptr, object, method, &result, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, call_method_ref_v_invalid_method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethod", nullptr, &object, &method);

    ani_ref result {};
    ani_int arg1 = VAL2;
    ani_int arg2 = VAL3;
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, nullptr, &result, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, call_method_ref_v_invalid_result)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethod", nullptr, &object, &method);

    ani_int arg1 = VAL2;
    ani_int arg2 = VAL3;
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, call_method_ref_v_invalid_object)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethod", nullptr, &object, &method);

    ani_ref result {};
    ani_int arg1 = VAL2;
    ani_int arg2 = VAL3;
    ASSERT_EQ(env_->Object_CallMethod_Ref(nullptr, method, &result, arg1, arg2), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, call_method_ref_a_invalid_args)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethod", nullptr, &object, &method);

    ani_ref result {};
    ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectCallMethodRefTest, call_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethodVoidParam", nullptr, &object, &method);

    std::string str {};
    ani_ref result {};
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "hello world");

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, args), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "hello world");
}

TEST_F(ObjectCallMethodRefTest, call_Multiple_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethodMultipleParam", nullptr, &object, &method);

    ani_value args[5U] = {};
    ani_int arg1 = VAL;
    ani_boolean arg2 = ANI_TRUE;
    ani_float arg3 = 3.0F;
    ani_short arg4 = 4U;
    ani_double arg5 = 5.0F;
    args[0U].i = arg1;
    args[1U].z = arg2;
    args[2U].f = arg3;
    args[3U].s = arg4;
    args[4U].d = arg5;

    std::string str {};
    ani_ref result {};
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, &result, arg1, arg2, arg3, arg4, arg5), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "china");

    ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, args), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "china");
}

TEST_F(ObjectCallMethodRefTest, call_Parent_Class_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethodVoidParam", nullptr, &object, &method);

    std::string str {};
    ani_ref result {};
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "hello world");

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, args), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "hello world");
}

TEST_F(ObjectCallMethodRefTest, call_Parent_Class_Method)
{
    ani_class clsC {};
    ASSERT_EQ(env_->FindClass("object_call_method_ref_test.C", &clsC), ANI_OK);
    ASSERT_NE(clsC, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsC, "func", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_class clsD {};
    ASSERT_EQ(env_->FindClass("object_call_method_ref_test.D", &clsD), ANI_OK);
    ASSERT_NE(clsD, nullptr);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(clsD, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsD, ctor, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U] = {};
    ani_int arg1 = VAL3;
    ani_int arg2 = VAL4;
    args[0U].i = arg1;
    args[1U].i = arg2;

    std::string str {};
    ani_ref result {};
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "nihao");

    ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, args), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "nihao");
}

TEST_F(ObjectCallMethodRefTest, call_Sub_Class_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.E", "func", nullptr, &object, &method);

    ani_value args[2U] = {};
    ani_int arg1 = VAL3;
    ani_int arg2 = VAL2;
    args[0U].i = arg1;
    args[1U].i = arg2;

    std::string str {};
    ani_ref result {};
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "hello world");

    ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, args), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "hello world");
}

TEST_F(ObjectCallMethodRefTest, multiple_Call_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "stringMethod", nullptr, &object, &method);

    std::string str {};
    ani_ref result {};
    ani_value args[2U] = {};
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL1;
    args[0U].i = arg1;
    args[1U].i = arg2;
    for (ani_int i = 0; i < VAL3; i++) {
        ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, &result, arg1, arg2), ANI_OK);
        ASSERT_NE(result, nullptr);
        GetStdString(static_cast<ani_string>(result), str);
        ASSERT_STREQ(str.c_str(), "nihao");

        ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, args), ANI_OK);
        ASSERT_NE(result, nullptr);
        GetStdString(static_cast<ani_string>(result), str);
        ASSERT_STREQ(str.c_str(), "nihao");
    }
}

TEST_F(ObjectCallMethodRefTest, call_Nested_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "nestedMethod", nullptr, &object, &method);

    std::string str {};
    ani_ref result {};
    ani_value args[2U] = {};
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "nihao");

    ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, args), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "nihao");
}

TEST_F(ObjectCallMethodRefTest, call_Recursion_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("object_call_method_ref_test.A", "recursionMethod", nullptr, &object, &method);

    std::string str {};
    ani_ref result {};
    ani_value args[1U] = {};
    ani_int arg1 = VAL1;
    args[0U].i = arg1;
    ASSERT_EQ(env_->Object_CallMethod_Ref(object, method, &result, arg1), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "hello");

    ASSERT_EQ(env_->Object_CallMethod_Ref_A(object, method, &result, args), ANI_OK);
    ASSERT_NE(result, nullptr);
    GetStdString(static_cast<ani_string>(result), str);
    ASSERT_STREQ(str.c_str(), "hello");
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)