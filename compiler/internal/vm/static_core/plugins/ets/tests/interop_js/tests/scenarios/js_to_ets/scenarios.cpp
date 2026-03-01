/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropScenariosJsToEts : public EtsInteropTest {};

TEST_F(EtsInteropScenariosJsToEts, test_standalone_function_call)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestStandaloneFunctionCall");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, test_class_methodCall)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestClassMethodCall");
    ASSERT_EQ(ret, true);
}

[[maybe_unused]] constexpr int CAN_CREATE_CLASSES_AT_RUNTIME = 0;
#if CAN_CREATE_CLASSES_AT_RUNTIME
TEST_F(EtsInteropScenariosJsToEts, Test_interface_methodCall)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestInterfaceMethodCall");
    ASSERT_EQ(ret, true);
}
#endif

TEST_F(EtsInteropScenariosJsToEts, Test_class_getter)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestClassGetter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_class_setter)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestClassSetter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_lambda_function_call)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestLambdaFunctionCall");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_generic_function_call)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestGenericFunctionCall");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_any)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_unknown)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeUnknown");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_undefined)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeUndefined");
    ASSERT_EQ(ret, true);
}
// NOTE (#24570): fix interop tests for tuples
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_function_arg_type_tuple)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeTuple");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_any)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionReturnTypeAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_unknown)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionReturnTypeUnknown");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_undefined)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionReturnTypeUndefined");
    ASSERT_EQ(ret, true);
}

// NOTE #15891 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_function_arg_type_callable)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeCallable");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_default_value_define_for_parameter)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestDefaultValueDefineForParameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_default_value_int_define_for_parameter)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestDefaultValueIntDefineForParameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_generic_type_as_parameter)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestGenericTypeAsParameter");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_generic_type_as_return_value)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestGenericTypeAsReturnValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_optional_primitive_explicit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeOptionalPrimitiveExplicit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_optional_primitive_default)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeOptionalPrimitiveDefault");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_primitive)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypePrimitive");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_primitive)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionReturnTypePrimitive");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_optional_call)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testOptionals");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_rest_params)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestRestParams");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_union)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_return_type_union)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionReturnTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_class_method_arg_type_union)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestClassMethodArgTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_class_method_return_type_union)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestClassMethodReturnTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_interface_methodCall_return_type_union)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestInterfaceMethodCallReturnTypeUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_static_methodCall)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestStaticMethodCall");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_static_methodCall_return_type_union)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestStaticMethodCallReturnTypeUnion");
    ASSERT_EQ(ret, true);
}

// NOTE(nikitayegorov) #18381 disable after 'this' is fixed to return a reference instead of a value copy.
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_return_value_is_this)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testReturnIsThis");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_return_value_is_omitted)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testReturnIsOmitted");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_rest_parameter)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testFunctionRestParameter");
    ASSERT_EQ(ret, true);
}

// NOTE(nikitayegorov) #17339 enable after rest\spread is fixed
TEST_F(EtsInteropScenariosJsToEts, DISABLED_Test_function_spread_parameter)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testFunctionSpreadParameter");
    ASSERT_EQ(ret, true);
}

// #24756
TEST_F(EtsInteropScenariosJsToEts, DISABLED_testFunctionOverload)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testFunctionOverload");
    ASSERT_EQ(ret, true);
}

// NOTE #24130
TEST_F(EtsInteropScenariosJsToEts, DISABLED_testFunctionCallableReturnValue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testFunctionCallableReturnValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, testFunctionArgStringLiteralType)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testFunctionArgStringLiteralType");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, testFunctionIntersectionTypePrimitive)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testFunctionIntersectionTypePrimitive");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_class_in_place_field_declarations)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testClassInPlaceFieldDeclarations");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_returns_composite_type_int)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "testFunctionReturnsCompositeTypeInt");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
