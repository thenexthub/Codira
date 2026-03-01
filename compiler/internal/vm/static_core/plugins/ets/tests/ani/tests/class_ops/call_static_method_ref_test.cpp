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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class CallStaticMethodTest : public AniTest {
public:
    static constexpr ani_int VAL1 = 5U;
    static constexpr ani_int VAL2 = 6U;
    static constexpr ani_double VAL3 = 4.5;
    static constexpr ani_double VAL4 = 7.5;
    static constexpr size_t ARG_COUNT = 2U;
    void GetMethodDataButton(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_ref_test.Phone", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "get_button_names", nullptr, &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void GetMethodDataString(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_ref_test.Phone", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "get_num_string", "ii:C{std.core.String}", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void CheckRefUp(ani_ref ref)
    {
        auto string = reinterpret_cast<ani_string>(ref);
        ani_size result = 0U;
        ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
        ASSERT_EQ(result, 2U);

        ani_size substrOffset = 0U;
        ani_size substrSize = result;
        const uint32_t bufferSize = 5U;
        char utfBuffer[bufferSize] = {};
        result = 0U;
        auto status =
            env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_STREQ(utfBuffer, "up");
    }

    void CheckRefNum(ani_ref ref)
    {
        auto string = reinterpret_cast<ani_string>(ref);
        ani_size result = 0U;
        ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
        ani_size substrOffset = 0U;
        ani_size substrSize = result;
        const uint32_t bufferSize = 10U;
        char utfBuffer[bufferSize] = {};
        result = 0U;
        auto status =
            env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
        ASSERT_EQ(status, ANI_OK);
        ASSERT_STREQ(utfBuffer, "INT5");
    }

    void TestCombineScene(const char *className, ani_int val1, ani_int val2)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_method method {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:C{std.core.String}", &method), ANI_OK);

        ani_ref value = nullptr;
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, &value, val1, val2), ANI_OK);
        CheckRefNum(value);

        ani_value args[2U];
        args[0U].i = val1;
        args[1U].i = val2;
        ani_ref valueA = nullptr;
        ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, &valueA, args), ANI_OK);
        CheckRefNum(valueA);
    }
};

TEST_F(CallStaticMethodTest, call_static_method_ref_0)
{
    ani_class cls {};
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);
}

TEST_F(CallStaticMethodTest, call_static_method_ref)
{
    ani_class cls {};
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref(env_, cls, method, &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ASSERT_EQ(result, 2U);

    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 5U;
    char utfBuffer[bufferSize] = {};
    result = 0U;
    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "up");
}

TEST_F(CallStaticMethodTest, call_static_method_ref_v)
{
    ani_class cls {};
    ani_static_method method = nullptr;
    GetMethodDataString(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, &ref, VAL1, VAL2), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {};
    result = 0U;
    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "INT5");
}

TEST_F(CallStaticMethodTest, call_static_method_ref_a)
{
    ani_class cls {};
    ani_static_method method = nullptr;
    GetMethodDataString(&cls, &method);

    ani_ref ref = nullptr;
    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, &ref, args), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto string = reinterpret_cast<ani_string>(ref);
    ani_size result = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(string, &result), ANI_OK);
    ani_size substrOffset = 0U;
    ani_size substrSize = result;
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {};
    result = 0U;
    auto status =
        env_->String_GetUTF8SubString(string, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &result);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "INT5");
}

TEST_F(CallStaticMethodTest, call_static_method_ref_invalid_cls)
{
    ani_class cls {};
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref(env_, nullptr, method, &ref), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_invalid_method)
{
    ani_class cls {};
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref(env_, cls, nullptr, &ref), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_invalid_result)
{
    ani_class cls {};
    ani_static_method method = nullptr;
    GetMethodDataButton(&cls, &method);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref(env_, cls, method, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(ref, nullptr);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_v_invalid_cls)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(nullptr, method, &ref, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_v_invalid_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, nullptr, &ref, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_v_invalid_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_a_invalid_cls)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(nullptr, method, &ref, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_a_invalid_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, nullptr, &ref, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_a_invalid_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_a_invalid_args)
{
    ani_class cls;
    ani_static_method method;
    GetMethodDataString(&cls, &method);

    ani_ref ref;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(nullptr, method, &ref, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_combine_scenes_1)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("call_static_method_ref_test.A", &clsA), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "funcA", "ii:C{std.core.String}", &methodA), ANI_OK);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("call_static_method_ref_test.B", &clsB), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "funcB", "ii:C{std.core.String}", &methodB), ANI_OK);

    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(clsA, methodA, &valueA, VAL1, VAL2), ANI_OK);
    CheckRefNum(valueA);

    ani_ref valueB = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(clsB, methodB, &valueB, VAL1, VAL2), ANI_OK);
    CheckRefUp(valueB);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_ref valueAA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(clsA, methodA, &valueAA, args), ANI_OK);
    CheckRefNum(valueAA);

    ani_ref valueBA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(clsB, methodB, &valueBA, args), ANI_OK);
    CheckRefUp(valueBA);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_ref_test.A", &cls), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:C{std.core.String}", &methodA), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "dd:d", &methodB), ANI_OK);

    ani_ref value = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, methodA, &value, VAL1, VAL2), ANI_OK);
    CheckRefNum(value);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, methodA, &valueA, args), ANI_OK);
    CheckRefNum(valueA);

    ani_double value2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, methodB, &value2, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value2, VAL3 + VAL4);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_ref_test.A", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcB", "ii:C{std.core.String}", &method), ANI_OK);

    ani_ref value = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, &value, VAL1, VAL2), ANI_OK);
    CheckRefNum(value);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, &valueA, args), ANI_OK);
    CheckRefNum(valueA);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_null_env)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodDataString(&cls, &method);

    ani_ref value = nullptr;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref(nullptr, cls, method, &value, VAL1, VAL2), ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Ref_A(nullptr, cls, method, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_combine_scenes_4)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_ref_test.D", &cls), ANI_OK);
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:C{std.core.String}", &method), ANI_OK);
    ani_ref value = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, &value, VAL1, VAL2), ANI_OK);
    CheckRefUp(value);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, &valueA, args), ANI_OK);
    CheckRefUp(valueA);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_combine_scenes_5)
{
    TestCombineScene("call_static_method_ref_test.C", VAL1, VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_combine_scenes_6)
{
    TestCombineScene("call_static_method_ref_test.E", VAL1, VAL2);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_combine_scenes_7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_ref_test.F", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "increment", nullptr, &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &method2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, method1, VAL1, VAL2), ANI_OK);
    ani_ref value = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method2, &value), ANI_OK);
    CheckRefNum(value);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method2, &valueA, args), ANI_OK);
    CheckRefNum(valueA);
}

TEST_F(CallStaticMethodTest, call_static_method_ref_combine_scenes_8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_ref_test.G", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "publicMethod", "ii:C{std.core.String}", &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "callPrivateMethod", "ii:C{std.core.String}", &method2), ANI_OK);
    ani_ref value = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method1, &value, VAL1, VAL2), ANI_OK);
    CheckRefNum(value);
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method2, &value, VAL1, VAL2), ANI_OK);
    CheckRefUp(value);

    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_ref valueA = nullptr;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method1, &valueA, args), ANI_OK);
    CheckRefNum(valueA);
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method2, &valueA, args), ANI_OK);
    CheckRefUp(valueA);
}

TEST_F(CallStaticMethodTest, check_initialization_ref)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodDataButton(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_ref_test.Phone"));
    ani_ref value {};
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref(cls, method, &value, VAL1, VAL2), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_ref_test.Phone"));
}

TEST_F(CallStaticMethodTest, check_initialization_ref_a)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodDataButton(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_ref_test.Phone"));
    ani_ref value {};
    ani_value args[2U];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_ref_test.Phone"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
