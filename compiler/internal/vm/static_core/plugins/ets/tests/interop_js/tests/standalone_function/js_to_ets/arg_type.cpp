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

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_double)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeDouble");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_byte)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeByte");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_short)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeShort");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_int)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeInt");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_long)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeLong");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_float)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeFloat");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_char)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeChar");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_boolean)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeBoolean");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_string)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeString");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_object)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeObject");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_class)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeClass");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_array)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeArray");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_tuple)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeTuple");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_callable)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeCallable");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_double_lit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeDoubleLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_byte_lit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeByteLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_short_lit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeShortLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_int_lit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeIntLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_long_lit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeLongLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_float_lit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeFloatLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_char_lit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeCharLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_boolean_lit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeBooleanLit");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEts, Test_function_arg_type_string_lit)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestFunctionArgTypeStringLit");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
