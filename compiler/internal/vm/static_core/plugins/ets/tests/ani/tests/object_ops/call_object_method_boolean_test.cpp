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

/**
 * @brief Unit test class for testing boolean method calls on ani objects.
 *
 * Inherits from the AniTest base class and provides test cases to verify
 * correct functionality of calling boolean-returning methods with various
 * parameter scenarios.
 */
class CallObjectMethodBooleanTest : public AniGtestObjectOps {
public:
    static constexpr ani_int VAL1 = 5U;
    static constexpr ani_int VAL2 = 6U;
    static constexpr ani_int VAL3 = 2U;
    static constexpr ani_int VAL4 = 3U;
};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define CHECK_FIELD_AFTER_CALL_METHOD(obj, method, expectedValue)                                           \
    do {                                                                                                    \
        ani_boolean value {};                                                                               \
        ASSERT_EQ(env_->Object_CallMethod_Boolean(obj, method, &value), ANI_OK);                            \
        ani_int methodChecker {};                                                                           \
        ASSERT_EQ(env_->Object_CallMethodByName_Int(obj, "getCheckerValue", ":i", &methodChecker), ANI_OK); \
        ASSERT_EQ(methodChecker, expectedValue);                                                            \
    } while (0)
// NOLINTEND(cppcoreguidelines-macro-usage)

/**
 * @brief Test case for calling a boolean-returning method with an argument array.
 *
 * This test verifies the correct behavior of calling a method using an array
 * of integer arguments and checks the return value.
 */
TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean_a)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethod", "ii:z", &object, &method);

    ani_value args[2];  // NOLINT(modernize-avoid-c-arrays)
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    args[0].i = arg1;
    args[1].i = arg2;

    ani_boolean res = ANI_FALSE;
    // Call the method and verify the return value.
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

/**
 * @brief Test case for calling a boolean-returning method with variadic arguments.
 *
 * This test ensures that the method correctly handles variadic arguments and
 * produces the expected boolean result.
 */
TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean_v)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethod", "ii:z", &object, &method);

    ani_boolean res = ANI_FALSE;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Call the method using variadic arguments and verify the return value.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &res, arg1, arg2), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

/**
 * @brief Test case for calling a boolean-returning method with specific inputs.
 *
 * Verifies the functionality of calling a method using variadic arguments with
 * different inputs and checking the boolean return value.
 */
TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethod", "ii:z", &object, &method);

    ani_boolean res = ANI_FALSE;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Call the method and verify the return value.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->c_api->Object_CallMethod_Boolean(env_, object, method, &res, arg1, arg2), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanTest, object_call_method_boolean_v_invalid_env)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethod", "ii:z", &object, &method);

    ani_boolean res = ANI_FALSE;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Call the method using variadic arguments and verify the return value.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &res, arg1, arg2), ANI_OK);
    ASSERT_EQ(res, ANI_TRUE);
}

/**
 * @brief Test case for handling invalid method pointer during invocation.
 *
 * Ensures the method call returns the expected error when a nullptr is passed
 * as the method pointer.
 */
TEST_F(CallObjectMethodBooleanTest, call_method_boolean_v_invalid_method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethod", "ii:z", &object, &method);

    ani_boolean res = ANI_FALSE;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Attempt to call the method with a nullptr method pointer.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, nullptr, &res, arg1, arg2), ANI_INVALID_ARGS);
}

/**
 * @brief Test case for handling null result pointers during method invocation.
 *
 * Ensures the method call returns the expected error when the result pointer is null.
 */
TEST_F(CallObjectMethodBooleanTest, call_method_boolean_v_invalid_result)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethod", "ii:z", &object, &method);

    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Attempt to call the method with a nullptr result pointer.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, nullptr, arg1, arg2), ANI_INVALID_ARGS);
}

/**
 * @brief Test case for handling invalid object pointers during method invocation.
 *
 * Ensures the method call returns the expected error when a nullptr is passed
 * as the object pointer.
 */
TEST_F(CallObjectMethodBooleanTest, call_method_boolean_v_invalid_object)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethod", "ii:z", &object, &method);

    ani_boolean res;
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    // Attempt to call the method with a nullptr object pointer.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethod_Boolean(nullptr, method, &res, arg1, arg2), ANI_INVALID_ARGS);
}

/**
 * @brief Test case for handling invalid argument arrays during method invocation.
 *
 * Ensures the method call returns the expected error when the argument array is null.
 */
TEST_F(CallObjectMethodBooleanTest, call_method_boolean_a_invalid_args)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethod", "ii:z", &object, &method);

    ani_boolean res;
    // Attempt to call the method with a nullptr argument array.
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &res, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallObjectMethodBooleanTest, call_Void_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethodVoidParam", ":z", &object, &method);

    ani_boolean result = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanTest, call_Multiple_Param_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethodMultipleParam", "bffb:z", &object, &method);

    ani_value args[4U] = {};
    ani_byte arg1 = VAL3;
    ani_float arg2 = 2.0F;
    ani_float arg3 = 3.0F;
    ani_byte arg4 = 4U;
    args[0U].b = arg1;
    args[1U].f = arg2;
    args[2U].f = arg3;
    args[3U].b = arg4;

    ani_boolean result;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2, arg3, arg4), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanTest, call_Parent_Class_Void_Param_Method_1)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.B", "booleanMethodVoidParam", ":z", &object, &method);

    ani_boolean result = 0U;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ani_value args[2U];
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanTest, call_Parent_Class_Method)
{
    ani_class clsC {};
    ASSERT_EQ(env_->FindClass("call_object_method_boolean_test.C", &clsC), ANI_OK);
    ASSERT_NE(clsC, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsC, "func", "ii:z", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_class clsD {};
    ASSERT_EQ(env_->FindClass("call_object_method_boolean_test.D", &clsD), ANI_OK);
    ASSERT_NE(clsD, nullptr);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(clsD, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsD, ctor, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_boolean result = ANI_TRUE;
    ani_value args[2U] = {};
    ani_int arg1 = VAL3;
    ani_int arg2 = VAL4;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanTest, call_Sub_Class_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.E", "func", "ii:z", &object, &method);

    ani_boolean result = ANI_FALSE;
    ani_value args[2U] = {};
    ani_int arg1 = VAL1;
    ani_int arg2 = VAL2;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanTest, multiple_Call_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "booleanMethod", "ii:z", &object, &method);

    ani_boolean result = ANI_FALSE;
    ani_value args[2U] = {};
    ani_byte arg1 = 6U;
    ani_byte arg2 = 7U;
    args[0U].b = arg1;
    args[1U].b = arg2;

    for (ani_int i = 0; i < VAL4; i++) {
        ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);
        ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
        ASSERT_EQ(result, ANI_TRUE);
    }
}

TEST_F(CallObjectMethodBooleanTest, call_Nested_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "nestedMethod", nullptr, &object, &method);

    ani_boolean result = ANI_TRUE;
    ani_value args[2U] = {};
    ani_byte arg1 = VAL4;
    ani_byte arg2 = VAL4;
    args[0U].b = arg1;
    args[1U].b = arg2;

    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1, arg2), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}

TEST_F(CallObjectMethodBooleanTest, call_Recursion_Method)
{
    ani_object object {};
    ani_method method {};
    GetMethodAndObject("call_object_method_boolean_test.A", "recursionMethod", "i:z", &object, &method);

    ani_boolean result = ANI_FALSE;
    ani_value args[1U] = {};
    ani_byte arg1 = VAL4;
    args[0U].i = arg1;
    ASSERT_EQ(env_->Object_CallMethod_Boolean(object, method, &result, arg1), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallObjectMethodBooleanTest, check_hierarchy)
{
    ani_class clsParent {};
    ASSERT_EQ(env_->FindClass("call_object_method_boolean_test.Parent", &clsParent), ANI_OK);
    ani_method parentCtor {};
    ASSERT_EQ(env_->Class_FindMethod(clsParent, "<ctor>", ":", &parentCtor), ANI_OK);
    ani_object parentObj {};
    ASSERT_EQ(env_->Object_New(clsParent, parentCtor, &parentObj), ANI_OK);

    ani_class clsChild {};
    ASSERT_EQ(env_->FindClass("call_object_method_boolean_test.Child", &clsChild), ANI_OK);
    ani_method childCtor {};
    ASSERT_EQ(env_->Class_FindMethod(clsChild, "<ctor>", ":", &childCtor), ANI_OK);
    ani_object childObj {};
    ASSERT_EQ(env_->Object_New(clsChild, childCtor, &childObj), ANI_OK);

    ani_method parentMethodInParent {};
    ASSERT_EQ(env_->Class_FindMethod(clsParent, "parentMethod", ":z", &parentMethodInParent), ANI_OK);
    ani_method parentMethodInChild {};
    ASSERT_EQ(env_->Class_FindMethod(clsChild, "parentMethod", ":z", &parentMethodInChild), ANI_OK);
    ani_method childMethodInParent {};
    ASSERT_EQ(env_->Class_FindMethod(clsParent, "childMethod", ":z", &childMethodInParent), ANI_NOT_FOUND);
    ani_method childMethodInChild {};
    ASSERT_EQ(env_->Class_FindMethod(clsChild, "childMethod", ":z", &childMethodInChild), ANI_OK);
    ani_method overridedMethodInParent {};
    ASSERT_EQ(env_->Class_FindMethod(clsParent, "overridedMethod", ":z", &overridedMethodInParent), ANI_OK);
    ani_method overridedMethodInChild {};
    ASSERT_EQ(env_->Class_FindMethod(clsChild, "overridedMethod", ":z", &overridedMethodInChild), ANI_OK);

    const ani_int parentMethodInParentWasCalled = 1;
    const ani_int childMethodInChildWasCalled = 2;
    const ani_int overridedMethodInChildWasCalled = 3;
    const ani_int overridedMethodInParentWasCalled = 4;

    // |----------------------------------------------------------------------------------------------------------|
    // |  ani_class  |           ani_static_method           |   ani_status  |               value                |
    // |-------------|---------------------------------------|---------------|------------------------------------|
    // |   Parent    |   parentMethod() from Parent class    |    ANI_OK     |    parentMethodInParentWasCalled   |
    // |   Parent    |   parentMethod() from Child class     |    ANI_OK     |    parentMethodInParentWasCalled   |
    // |   Parent    |    childMethod() from Child class     |      UB       |                --                  |
    // |   Parent    |  overridedMethod() from Child class   |    ANI_OK     |  overridedMethodInParentWasCalled  |
    // |   Parent    |  overridedMethod() from Parent class  |    ANI_OK     |  overridedMethodInParentWasCalled  |
    // |   Child     |    parentMethod() from Parent class   |    ANI_OK     |    parentMethodInParentWasCalled   |
    // |   Child     |    parentMethod() from Child class    |    ANI_OK     |    parentMethodInParentWasCalled   |
    // |   Child     |    childMethod() from Child class     |    ANI_OK     |     childMethodInChildWasCalled    |
    // |   Child     |  overridedMethod() from Child class   |    ANI_OK     |   overridedMethodInChildWasCalled  |
    // |   Child     |  overridedMethod() from Parent class  |    ANI_OK     |   overridedMethodInChildWasCalled  |
    // |-------------|---------------------------------------|---------------|------------------------------------|

    CHECK_FIELD_AFTER_CALL_METHOD(parentObj, parentMethodInParent, parentMethodInParentWasCalled);
    CHECK_FIELD_AFTER_CALL_METHOD(parentObj, parentMethodInChild, parentMethodInParentWasCalled);
    CHECK_FIELD_AFTER_CALL_METHOD(parentObj, overridedMethodInChild, overridedMethodInParentWasCalled);
    CHECK_FIELD_AFTER_CALL_METHOD(parentObj, overridedMethodInParent, overridedMethodInParentWasCalled);

    CHECK_FIELD_AFTER_CALL_METHOD(childObj, parentMethodInParent, parentMethodInParentWasCalled);
    CHECK_FIELD_AFTER_CALL_METHOD(childObj, parentMethodInChild, parentMethodInParentWasCalled);
    CHECK_FIELD_AFTER_CALL_METHOD(childObj, childMethodInChild, childMethodInChildWasCalled);
    CHECK_FIELD_AFTER_CALL_METHOD(childObj, overridedMethodInChild, overridedMethodInChildWasCalled);
    CHECK_FIELD_AFTER_CALL_METHOD(childObj, overridedMethodInParent, overridedMethodInChildWasCalled);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
