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

class EtsInteropScenariosEtsToJs : public EtsInteropTest {};

// NOTE(oignatenko) return and arg types any, unknown, undefined need real TS because transpiling cuts off
// important details. Have a look at declgen_ets2ts
TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_any_call)
{
    // Note this also covers scenario of return type any
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_any_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_unknown_call)
{
    // Note this also covers scenario of return type unknown
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_unknown_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_undefined_call)
{
    // Note this also covers scenario of return type undefined
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_undefined_call.js"));
}

// Literal set
TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_boolean_lit_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_boolean_lit_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_byte_lit_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_byte_lit_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_float_lit_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_float_lit_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_int_lit_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_int_lit_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_long_lit_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_long_lit_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_number_lit_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_number_lit_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_short_lit_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_short_lit_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_string_lit_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_string_lit_call.js"));
}

// TS extra set
TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_byte_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_byte_call.js"));
}

// NOTE (nikitayegorov) Negative test, causes InteropCtx::Fatal by design.
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_byte_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_byte_call_neg.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_short_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_short_call.js"));
}

// NOTE (nikitayegorov) Negative test, causes InteropCtx::Fatal by design.
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_short_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_short_call_neg.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_int_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_int_call.js"));
}

// NOTE (nikitayegorov) Negative test, causes InteropCtx::Fatal by design.
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_int_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_int_call_neg.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_long_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_long_call.js"));
}

// NOTE (nikitayegorov) Negative test, causes InteropCtx::Fatal by design.
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_long_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_long_call_neg.js"));
}

// NOTE #18627 (nikitayegorov) Enable when ArkTS -> JS floating point precision is fixed
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_float_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_float_call.js"));
}

// NOTE (nikitayegorov) Negative test, causes InteropCtx::Fatal by design.
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_float_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_float_call_neg.js"));
}

// NOTE #18621 (nikitayegorov) Enable when JS -> Ark char casting is fixed
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_char_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_char_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_char_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_char_call_neg.js"));
}

// TS subset by value
TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_boolean_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_boolean_call.js"));
}

// NOTE (nikitayegorov) Negative test, causes InteropCtx::Fatal by design.
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_boolean_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_boolean_call_neg.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_string_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_string_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_string_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_string_call_neg.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_number_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_number_call.js"));
}

// NOTE (nikitayegorov) Negative test, causes InteropCtx::Fatal by design.
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_number_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_number_call_neg.js"));
}

// NOTE #18628 (nikitayegorov) Enable when interop typing is fixed
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_object_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_object_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_class_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_class_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_class_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_class_call_neg.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_array_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_array_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_array_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_array_call_neg.js"));
}

// NOTE(oignatenko) enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_tuple_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_tuple_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_tuple_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_tuple_call_neg.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_union_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_union_call.js"));
}

// NOTE #18630 (nikitayegorov) Enable when typechecks against unions are fixed
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_union_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_union_call_neg.js"));
}

// NOTE #18081(oignatenko) enable this when callables are o
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_callable_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_callable_call.js"));
}

// NOTE #18081 (vladimiralpavlov) enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_callable_call_neg)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_callable_call_neg.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_interface_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_interface_call.js"));
}

}  // namespace ark::ets::interop::js::testing
