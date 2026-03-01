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

class EtsInteropScenariosEtsToJsIndirectCallValue : public EtsInteropTest {};

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_number_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_number_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_number_apply)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_number_apply.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_number_bind_with_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_number_bind_with_arg.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_number_bind_without_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_number_bind_without_arg.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_string_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_string_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_string_apply)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_string_apply.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_string_bind_with_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_string_bind_with_arg.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_string_bind_without_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_string_bind_without_arg.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_boolean_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_boolean_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_boolean_apply)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_boolean_apply.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_boolean_bind_with_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_boolean_bind_with_arg.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallValue, test_function_type_value_boolean_bind_without_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_value/test_function_type_value_boolean_bind_without_arg.js"));
}

}  // namespace ark::ets::interop::js::testing
