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
    static constexpr size_t ARG_COUNT = 2U;
    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("call_static_method_bool_test.Operations", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "or", "zz:z", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }

    void TestCombineScene(const char *className, ani_boolean expectedValue)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass(className, &cls), ANI_OK);
        ani_static_method method {};
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "zz:z", &method), ANI_OK);

        ani_boolean value = expectedValue == ANI_TRUE ? ANI_FALSE : ANI_TRUE;
        ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, &value, ANI_TRUE, ANI_FALSE), ANI_OK);
        ASSERT_EQ(value, expectedValue);

        ani_value args[2U];
        args[0U].z = ANI_TRUE;
        args[1U].z = ANI_FALSE;
        ani_boolean valueA = expectedValue == ANI_TRUE ? ANI_FALSE : ANI_TRUE;
        ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, &valueA, args), ANI_OK);
        ASSERT_EQ(valueA, expectedValue);
    }
};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define CHECK_CALLED_STATIC_METHOD(cls, method, expectedValue)                                                   \
    do {                                                                                                         \
        ani_boolean value {};                                                                                    \
        ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, &value), ANI_OK);                            \
        ani_int methodChecker {};                                                                                \
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "getCheckerValue", ":i", &methodChecker), ANI_OK); \
        ASSERT_EQ(methodChecker, expectedValue);                                                                 \
    } while (0)
// NOLINTEND(cppcoreguidelines-macro-usage)

TEST_F(CallStaticMethodTest, call_static_method_bool)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_v)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(nullptr, method, &result, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, nullptr, &result, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, nullptr, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A_null_class)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(nullptr, method, &result, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A_null_method)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, nullptr, &result, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A_null_result)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[ARG_COUNT];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A_null_args)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, &result, nullptr), ANI_INVALID_ARGS);
}

// NOTE: disabled due to #22193
TEST_F(CallStaticMethodTest, DISABLED_call_static_method_bool_overflow)
{
    ani_class cls {};
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, &result, 10U, 20U), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_combine_scenes_1)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("call_static_method_bool_test.A", &clsA), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "funcA", "zz:z", &methodA), ANI_OK);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("call_static_method_bool_test.B", &clsB), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "funcB", "zz:z", &methodB), ANI_OK);

    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(clsA, methodA, &valueA, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);

    ani_boolean valueB = ANI_TRUE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(clsB, methodB, &valueB, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(valueB, ANI_FALSE);

    ani_value args[ARG_COUNT];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueAA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(clsA, methodA, &valueAA, args), ANI_OK);
    ASSERT_EQ(valueAA, ANI_TRUE);

    ani_boolean valueBA = ANI_TRUE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(clsB, methodB, &valueBA, args), ANI_OK);
    ASSERT_EQ(valueBA, ANI_FALSE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_bool_test.A", &cls), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "zz:z", &methodA), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "ii:i", &methodB), ANI_OK);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, methodA, &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[ARG_COUNT];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, methodA, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);

    ani_int value2 = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, methodB, &value2, 5U, 6U), ANI_OK);
    ASSERT_EQ(value2, 5U + 6U);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_bool_test.A", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcB", "zz:z", &method), ANI_OK);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);

    ani_value args[ARG_COUNT];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_null_env)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean(nullptr, cls, method, &value, ANI_TRUE, ANI_FALSE),
              ANI_INVALID_ARGS);
    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean_A(nullptr, cls, method, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_combine_scenes_4)
{
    TestCombineScene("call_static_method_bool_test.C", ANI_TRUE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_combine_scenes_5)
{
    TestCombineScene("call_static_method_bool_test.D", ANI_FALSE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_combine_scenes_6)
{
    TestCombineScene("call_static_method_bool_test.E", ANI_TRUE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_combine_scenes_7)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_bool_test.F", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "increment", nullptr, &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &method2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, method1, ANI_TRUE, ANI_FALSE), ANI_OK);
    ani_boolean value = ANI_TRUE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method2, &value), ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_TRUE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_FALSE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_combine_scenes_8)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("call_static_method_bool_test.G", &cls), ANI_OK);
    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "publicMethod", "zz:z", &method1), ANI_OK);
    ani_static_method method2 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "callPrivateMethod", "zz:z", &method2), ANI_OK);
    ani_boolean value = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method1, &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method2, &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_FALSE);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean valueA = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method1, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_TRUE);
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method2, &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, ANI_FALSE);
}

TEST_F(CallStaticMethodTest, check_initialization_bool)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_bool_test.Operations"));
    ani_boolean value {};
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_bool_test.Operations"));
}

TEST_F(CallStaticMethodTest, check_initialization_bool_a)
{
    ani_class cls {};
    ani_static_method method {};
    GetMethodData(&cls, &method);

    ASSERT_FALSE(IsRuntimeClassInitialized("call_static_method_bool_test.Operations"));
    ani_boolean value {};
    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, &value, args), ANI_OK);
    ASSERT_TRUE(IsRuntimeClassInitialized("call_static_method_bool_test.Operations"));
}

TEST_F(CallStaticMethodTest, check_hierarchy)
{
    ani_class clsParent {};
    ASSERT_EQ(env_->FindClass("call_static_method_bool_test.Parent", &clsParent), ANI_OK);
    ani_class clsChild {};
    ASSERT_EQ(env_->FindClass("call_static_method_bool_test.Child", &clsChild), ANI_OK);

    ani_static_method parentMethodInParent {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsParent, "parentStaticMethod", ":z", &parentMethodInParent), ANI_OK);
    ani_static_method parentMethodInChild {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsChild, "parentStaticMethod", ":z", &parentMethodInChild), ANI_OK);
    ani_static_method childMethodInParent {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsParent, "childStaticMethod", ":z", &childMethodInParent), ANI_NOT_FOUND);
    ani_static_method childMethodInChild {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsChild, "childStaticMethod", ":z", &childMethodInChild), ANI_OK);
    ani_static_method shadowingMethodInParent {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsParent, "shadowingStaticMethod", ":z", &shadowingMethodInParent), ANI_OK);
    ani_static_method shadowingMethodInChild {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsChild, "shadowingStaticMethod", ":z", &shadowingMethodInChild), ANI_OK);

    const ani_int parentMethodInParentWasCalled = 1;
    const ani_int childMethodInChildWasCalled = 2;
    const ani_int overridedMethodInChildWasCalled = 3;
    const ani_int overridedMethodInParentWasCalled = 4;

    // |--------------------------------------------------------------------------------------------------------------|
    // |  ani_class  |              ani_static_method              |  ani_status  |              value                |
    // |-------------|---------------------------------------------|--------------|-----------------------------------|
    // |   Parent    |   parentStaticMethod() from Parent class    |    ANI_OK    |   parentMethodInParentWasCalled   |
    // |   Parent    |   parentStaticMethod() from Child class     |    ANI_OK    |   parentMethodInParentWasCalled   |
    // |   Parent    |    childStaticMethod() from Child class     |      UB      |               --                  |
    // |   Parent    |  shadowingStaticMethod() from Child class   |      UB      |               --                  |
    // |   Parent    |  shadowingStaticMethod() from Parent class  |    ANI_OK    |  overridedMethodInParentWasCalled |
    // |   Child     |    parentStaticMethod() from Parent class   |    ANI_OK    |   parentMethodInParentWasCalled   |
    // |   Child     |    parentStaticMethod() from Child class    |    ANI_OK    |   parentMethodInParentWasCalled   |
    // |   Child     |    childStaticMethod() from Child class     |    ANI_OK    |    childMethodInChildWasCalled    |
    // |   Child     |  shadowingStaticMethod() from Child class   |    ANI_OK    |  overridedMethodInChildWasCalled  |
    // |   Child     |  shadowingStaticMethod() from Parent class  |    ANI_OK    |  overridedMethodInParentWasCalled |
    // |-------------|---------------------------------------------|--------------|-----------------------------------|

    CHECK_CALLED_STATIC_METHOD(clsParent, parentMethodInParent, parentMethodInParentWasCalled);
    CHECK_CALLED_STATIC_METHOD(clsParent, parentMethodInChild, parentMethodInParentWasCalled);
    CHECK_CALLED_STATIC_METHOD(clsParent, shadowingMethodInParent, overridedMethodInParentWasCalled);

    CHECK_CALLED_STATIC_METHOD(clsChild, parentMethodInParent, parentMethodInParentWasCalled);
    CHECK_CALLED_STATIC_METHOD(clsChild, parentMethodInChild, parentMethodInParentWasCalled);
    CHECK_CALLED_STATIC_METHOD(clsChild, childMethodInChild, childMethodInChildWasCalled);
    CHECK_CALLED_STATIC_METHOD(clsChild, shadowingMethodInChild, overridedMethodInChildWasCalled);
    CHECK_CALLED_STATIC_METHOD(clsChild, shadowingMethodInParent, overridedMethodInParentWasCalled);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
