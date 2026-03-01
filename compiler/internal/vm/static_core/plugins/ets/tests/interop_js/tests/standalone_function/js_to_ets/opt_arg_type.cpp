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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropScenariosJsToEts : public EtsInteropTest {};

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_double_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeDoubleOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_byte_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeByteOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_short_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeShortOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_int_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeIntOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_long_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeLongOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_float_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeFloatOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_char_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeCharOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_boolean_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeBooleanOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_string_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeStringOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_object_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeObjectOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_class_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeClassOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_array_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeArrayOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_tuple_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeTupleOpt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_callable_opt)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeCallableOpt");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
