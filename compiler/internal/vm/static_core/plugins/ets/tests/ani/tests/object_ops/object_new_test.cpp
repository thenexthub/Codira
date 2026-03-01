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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, readability-magic-numbers, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ObjectNewTest : public AniTest {
public:
    static constexpr ani_int VAL1 = 200;
    enum class ParameterScence : uint8_t {
        INVALID_ARG0 = 0x0,
        INVALID_ARG1 = 0x1,
        INVALID_ARG2 = 0x2,
        INVALID_ARG3 = 0x3,
        NORMAL = 0x4,
    };

    void GetTestData(ani_class *clsResult, ani_method *ctorResult, ani_string *modelResult, ani_int *weightResult)
    {
        const char m[] = "Pure P60";
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("object_new_test.MobilePhone", &cls), ANI_OK);

        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.String}i:", &ctor), ANI_OK);

        ani_string model {};
        ASSERT_EQ(env_->String_NewUTF8(m, strlen(m), &model), ANI_OK);

        *clsResult = cls;
        *ctorResult = ctor;
        *modelResult = model;
        *weightResult = VAL1;
    }

    ani_status TestFuncV(ObjectNewTest::ParameterScence scene, ani_class cls, ani_method method, ani_object *result,
                         ...)
    {
        va_list args {};
        va_start(args, result);
        ani_status aniResult = ANI_ERROR;
        switch (scene) {
            case ObjectNewTest::ParameterScence::INVALID_ARG0: {
                aniResult = env_->c_api->Object_New_V(nullptr, cls, method, result, args);
                break;
            }
            case ObjectNewTest::ParameterScence::INVALID_ARG1: {
                aniResult = env_->c_api->Object_New_V(env_, nullptr, method, result, args);
                break;
            }
            case ObjectNewTest::ParameterScence::INVALID_ARG2: {
                aniResult = env_->c_api->Object_New_V(env_, cls, nullptr, result, args);
                break;
            }
            case ObjectNewTest::ParameterScence::INVALID_ARG3: {
                aniResult = env_->c_api->Object_New_V(env_, cls, method, nullptr, args);
                break;
            }
            case ObjectNewTest::ParameterScence::NORMAL:
            default: {
                aniResult = env_->c_api->Object_New_V(env_, cls, method, result, args);
                break;
            }
        }
        va_end(args);
        return aniResult;
    }

    void GetTestString(ani_string *tag)
    {
        const char model[] = "Pure P60";
        ASSERT_EQ(env_->String_NewUTF8(model, strlen(model), tag), ANI_OK);
        ASSERT_NE(*tag, nullptr);
    }

    void TestObjectNewV(ani_env *env, ani_class cls, ani_method method, ani_string tag, ani_ref animalRef)
    {
        ani_object object {};
        ASSERT_EQ(TestFuncV(ParameterScence::NORMAL, cls, method, &object, VAL1, tag, animalRef), ANI_OK);

        ani_method checkMethod {};
        ASSERT_EQ(env->Class_FindMethod(cls, "checkValue", nullptr, &checkMethod), ANI_OK);
        ASSERT_NE(checkMethod, nullptr);

        ani_boolean result = ANI_FALSE;
        ASSERT_EQ(env->Object_CallMethod_Boolean(object, checkMethod, &result), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);
    }
    static constexpr int32_t LOOP_COUNT = 3;
};

TEST_F(ObjectNewTest, object_new)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls, ctor, &phone, model, weight), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkModel", phone, model), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkWeight", phone, weight), ANI_TRUE);
}

TEST_F(ObjectNewTest, object_new_a)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;

    ani_object phone {};
    ASSERT_EQ(env_->Object_New_A(cls, ctor, &phone, args), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkModel", phone, model), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkWeight", phone, weight), ANI_TRUE);
}

TEST_F(ObjectNewTest, object_new_v)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &phone, model, weight), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkModel", phone, model), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkWeight", phone, weight), ANI_TRUE);
}

TEST_F(ObjectNewTest, object_new_invalid_args0)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone {};
    ASSERT_EQ(env_->c_api->Object_New(nullptr, cls, ctor, &phone, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_invalid_args1)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone {};
    ASSERT_EQ(env_->c_api->Object_New(env_, nullptr, ctor, &phone, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_invalid_args2)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls, nullptr, &phone, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_invalid_args3)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ASSERT_EQ(env_->c_api->Object_New(env_, cls, ctor, nullptr, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_a_invalid_args0)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;

    ani_object phone {};
    ASSERT_EQ(env_->c_api->Object_New_A(nullptr, cls, ctor, &phone, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_a_invalid_args1)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;

    ani_object phone {};
    ASSERT_EQ(env_->Object_New_A(nullptr, ctor, &phone, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_a_invalid_args2)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;

    ani_object phone {};
    ASSERT_EQ(env_->Object_New_A(cls, nullptr, &phone, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_a_invalid_args3)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;

    ASSERT_EQ(env_->Object_New_A(cls, ctor, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_a_invalid_args4)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone {};
    ASSERT_EQ(env_->Object_New_A(cls, ctor, &phone, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_v_invalid_args1)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone {};
    ASSERT_EQ(env_->Object_New(nullptr, ctor, &phone, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_v_invalid_args2)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone {};
    ASSERT_EQ(env_->Object_New(cls, nullptr, &phone, model, weight), ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_v_invalid_args3)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ASSERT_EQ(env_->Object_New(cls, ctor, nullptr, model, weight), ANI_INVALID_ARGS);
}

// NODE: Enable when #22280 is resolved
TEST_F(ObjectNewTest, DISABLED_object_new_string)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

    ani_string str {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls, ctor, reinterpret_cast<ani_object *>(&str)), ANI_OK);

    // NODE: Check the resulting string
}

TEST_F(ObjectNewTest, no_argument_constructor)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_new_test.A", &cls), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ASSERT_NE(ctor, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls, ctor, &object), ANI_OK);

    ani_value args[2];  // NOLINT(modernize-avoid-c-arrays)
    ani_int arg1 = 2U;
    ani_int arg2 = 3U;
    args[0].i = arg1;
    args[1].i = arg2;

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "boolean_method", "ii:z", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_boolean res = ANI_TRUE;
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, false);
}

TEST_F(ObjectNewTest, unmatch_argument)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ani_object phone {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &phone, model, model), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkModel", phone, model), ANI_TRUE);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkWeight", phone, weight), ANI_FALSE);
}

TEST_F(ObjectNewTest, throw_error_constructor)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_new_test.B", &cls), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ASSERT_NE(ctor, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_PENDING_ERROR);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->ExistUnhandledError(&result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ObjectNewTest, object_new_loop)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_object phone {};
        ASSERT_EQ(env_->Object_New(cls, ctor, &phone, model, weight), ANI_OK);

        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkModel", phone, model), ANI_TRUE);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkWeight", phone, weight), ANI_TRUE);
    }
}

TEST_F(ObjectNewTest, object_new_a_combination_objcet)
{
    const ani_int value = VAL1;
    ani_string tag {};
    GetTestString(&tag);

    auto animalRef = CallEtsFunction<ani_ref>("object_new_test", "newAnimalObject");
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_new_test.Test", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "iC{std.core.String}C{object_new_test.Animal}:", &ctor), ANI_OK);
    ASSERT_NE(ctor, nullptr);

    ani_value args[3U];
    args[0U].i = value;
    args[1U].r = tag;
    args[2U].r = animalRef;

    ani_object testObject {};
    ASSERT_EQ(env_->Object_New_A(cls, ctor, &testObject, args), ANI_OK);
    ASSERT_NE(testObject, nullptr);

    ani_method checkMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "checkValue", nullptr, &checkMethod), ANI_OK);
    ASSERT_NE(checkMethod, nullptr);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(testObject, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    const ani_int value1 = 100;
    args[0U].i = value1;
    ani_method newTestMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "newTestObject",
                                     "iC{std.core.String}C{object_new_test.Animal}:C{object_new_test.Test}",
                                     &newTestMethod),
              ANI_OK);
    ASSERT_NE(newTestMethod, nullptr);

    const ani_int value2 = 50;
    args[0U].i = value2;
    ani_object testObject1 {};
    ASSERT_EQ(env_->Object_New_A(cls, newTestMethod, &testObject1, args), ANI_OK);
    ASSERT_NE(testObject, nullptr);

    ASSERT_EQ(env_->Object_CallMethod_Boolean(testObject1, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(ObjectNewTest, object_new_a_loop)
{
    for (uint16_t i = 0; i < LOOP_COUNT; ++i) {
        ani_class cls {};
        ani_method ctor {};
        ani_string model {};
        ani_int weight = 0;
        GetTestData(&cls, &ctor, &model, &weight);

        ani_value args[2U];
        args[0U].r = model;
        args[1U].i = weight;

        ani_object phone {};
        ASSERT_EQ(env_->Object_New_A(cls, ctor, &phone, args), ANI_OK);

        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkModel", phone, model), ANI_TRUE);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_new_test", "checkWeight", phone, weight), ANI_TRUE);
    }
}

TEST_F(ObjectNewTest, object_new_a_multiple_parameters_method)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_new_test.Mixture", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    const char *name =
        "C{std.core.Null}C{std.core.Array}C{std.core.Array}ifdzlsC{std.core.String}C{object_new_test.Animal}:C{"
        "object_new_test.Mixture}";
    ani_method newMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "newMixtureObject", name, &newMethod), ANI_OK);
    ASSERT_NE(newMethod, nullptr);

    ani_method checkMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "checkFun", nullptr, &checkMethod), ANI_OK);
    ASSERT_NE(checkMethod, nullptr);

    auto ref = CallEtsFunction<ani_ref>("object_new_test", "getNull");
    const auto intArray = static_cast<ani_fixedarray_int>(CallEtsFunction<ani_ref>("object_new_test", "getIntArray"));
    const auto byteArray =
        static_cast<ani_fixedarray_byte>(CallEtsFunction<ani_ref>("object_new_test", "getByteArray"));

    ani_string tag {};
    GetTestString(&tag);
    auto animalRef = CallEtsFunction<ani_ref>("object_new_test", "newAnimalObject");
    ani_value args[11U];
    args[0U].r = ref;
    args[1U].r = intArray;
    args[2U].r = byteArray;
    args[3U].i = 42;  // 42 is intrinsic value
    args[4U].f = 18.0F;
    args[5U].d = 18.0F;
    args[6U].z = ANI_FALSE;
    args[7U].l = 1000000;  // 1000000 is intrinsic value
    args[8U].s = 2U;
    args[9U].r = tag;
    args[10U].r = animalRef;

    ani_object object {};
    ASSERT_EQ(env_->Object_New_A(cls, newMethod, &object, args), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, checkMethod, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ObjectNewTest, object_new_v_invalid_args)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_new_test.Test", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method newTestMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "newTestObject",
                                     "iC{std.core.String}C{object_new_test.Animal}:C{object_new_test.Test}",
                                     &newTestMethod),
              ANI_OK);
    ASSERT_NE(newTestMethod, nullptr);

    ani_string tag {};
    GetTestString(&tag);
    auto animalRef = CallEtsFunction<ani_ref>("object_new_test", "newAnimalObject");
    ani_value arg0;
    ani_value arg1;
    ani_value arg2;
    arg0.i = VAL1;
    arg1.r = tag;
    arg2.r = animalRef;

    ani_object object {};
    ASSERT_EQ(TestFuncV(ObjectNewTest::ParameterScence::INVALID_ARG0, cls, newTestMethod, &object, arg0, arg1, arg2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(TestFuncV(ObjectNewTest::ParameterScence::INVALID_ARG1, cls, newTestMethod, &object, arg0, arg1, arg2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(TestFuncV(ObjectNewTest::ParameterScence::INVALID_ARG2, cls, newTestMethod, &object, arg0, arg1, arg2),
              ANI_INVALID_ARGS);
    ASSERT_EQ(TestFuncV(ObjectNewTest::ParameterScence::INVALID_ARG3, cls, newTestMethod, &object, arg0, arg1, arg2),
              ANI_INVALID_ARGS);
}

TEST_F(ObjectNewTest, object_new_v_ctor)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_new_test.Test", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "iC{std.core.String}C{object_new_test.Animal}:", &ctor), ANI_OK);
    ASSERT_NE(ctor, nullptr);

    ani_string tag {};
    GetTestString(&tag);
    auto animalRef = CallEtsFunction<ani_ref>("object_new_test", "newAnimalObject");

    TestObjectNewV(env_, cls, ctor, tag, animalRef);
}

TEST_F(ObjectNewTest, object_new_v_normal_method_loop)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("object_new_test.Test", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method newTestMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "newTestObject",
                                     "iC{std.core.String}C{object_new_test.Animal}:C{object_new_test.Test}",
                                     &newTestMethod),
              ANI_OK);
    ASSERT_NE(newTestMethod, nullptr);

    ani_string tag {};
    GetTestString(&tag);
    auto animalRef = CallEtsFunction<ani_ref>("object_new_test", "newAnimalObject");

    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        TestObjectNewV(env_, cls, newTestMethod, tag, animalRef);
    }
}

TEST_F(ObjectNewTest, check_initialization)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ASSERT_FALSE(IsRuntimeClassInitialized("object_new_test.MobilePhone"));
    ani_object phone {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &phone, model, weight), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("object_new_test.MobilePhone"));
}

TEST_F(ObjectNewTest, check_initialization_a)
{
    ani_class cls {};
    ani_method ctor {};
    ani_string model {};
    ani_int weight = 0;
    GetTestData(&cls, &ctor, &model, &weight);

    ASSERT_FALSE(IsRuntimeClassInitialized("object_new_test.MobilePhone"));
    ani_value args[2U];
    args[0U].r = model;
    args[1U].i = weight;
    ani_object phone {};
    ASSERT_EQ(env_->Object_New_A(cls, ctor, &phone, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("object_new_test.MobilePhone"));
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers, modernize-avoid-c-arrays)
