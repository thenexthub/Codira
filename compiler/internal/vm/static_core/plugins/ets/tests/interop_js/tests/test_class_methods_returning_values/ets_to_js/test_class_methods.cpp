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

class EtsInteropClassMethodsReturningValuesTestEtsToJs : public EtsInteropTest {};

TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, TestFunctionReturnStringSubsetByRef)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_string_subset_by_ref.js"));
}

// Note: Test is disabled until #17852 is resolved
TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, DISABLED_TestFunctionReturnMapSubsetByRef)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_map_subset_by_ref.js"));
}

TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, TestFunctionReturnStringSubsetByValue)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_string_subset_by_value.js"));
}

TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, TestFunctionReturnIntSubsetByValue)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_int_subset_by_value.js"));
}

TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, TestFunctionReturnBoolSubsetByValue)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_bool_subset_by_value.js"));
}

TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, TestFunctionReturnUndefSubsetByValue)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_undefined_subset_by_value.js"));
}

TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, TestFunctionReturnNullSubsetByValue)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_null_subset_by_value.js"));
}

TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, TestFunctionReturnUnion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_union.js"));
}

TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, TestFunctionReturnClass)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_class.js"));
}

TEST_F(EtsInteropClassMethodsReturningValuesTestEtsToJs, TestFunctionReturnInterface)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_interface.js"));
}

}  // namespace ark::ets::interop::js::testing
