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

class EtsInteropScenariosEtsToJsIndirectCallRef : public EtsInteropTest {};

TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, test_function_type_ref_array_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_array_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, test_function_type_ref_array_apply)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_array_apply.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, test_function_type_ref_array_bind_with_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_array_bind_with_arg.js"));
}

TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, test_function_type_ref_array_bind_without_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_array_bind_without_arg.js"));
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, DISABLED_test_function_type_ref_tuple_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_tuple_call.js"));
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, DISABLED_test_function_type_ref_tuple_apply)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_tuple_apply.js"));
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, DISABLED_test_function_type_ref_tuple_bind_with_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_tuple_bind_with_arg.js"));
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, DISABLED_test_function_type_ref_tuple_bind_without_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_tuple_bind_without_arg.js"));
}

// NOTE #17852 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, DISABLED_test_function_type_ref_map_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_map_call.js"));
}

// NOTE #17852 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, DISABLED_test_function_type_ref_map_apply)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_map_apply.js"));
}

// NOTE #17852 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, DISABLED_test_function_type_ref_map_bind_with_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_map_bind_with_arg.js"));
}

// NOTE #17852 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJsIndirectCallRef, DISABLED_test_function_type_ref_map_bind_without_arg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/type_ref/test_function_type_ref_map_bind_without_arg.js"));
}

}  // namespace ark::ets::interop::js::testing
