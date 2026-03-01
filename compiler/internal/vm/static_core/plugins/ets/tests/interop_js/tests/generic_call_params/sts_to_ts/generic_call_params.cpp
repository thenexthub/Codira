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

class EtsGenericCallParamsEtsToTsTest : public EtsInteropTest {};

// NOTE: issue (18006) fix function as param
TEST_F(EtsGenericCallParamsEtsToTsTest, DISABLED_check_apply_function_generic)
{
    ASSERT_TRUE(RunJsTestSuite("check_apply_function_generic.js"));
}
// NOTE: issue (18006) fix function as param
TEST_F(EtsGenericCallParamsEtsToTsTest, DISABLED_check_apply_function_generics)
{
    ASSERT_TRUE(RunJsTestSuite("check_apply_function_generics.js"));
}
// NOTE: issue (18006) fix function as param
TEST_F(EtsGenericCallParamsEtsToTsTest, DISABLED_check_generic_extends_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_extends_class.js"));
}
// NOTE: issue (18006) fix function as param
TEST_F(EtsGenericCallParamsEtsToTsTest, DISABLED_check_apply_function_generic_array)
{
    ASSERT_TRUE(RunJsTestSuite("check_apply_function_generic_array.js"));
}
// NOTE: issue (18007) fix primitive constrain
TEST_F(EtsGenericCallParamsEtsToTsTest, DISABLED_apply_function_with_constraints)
{
    ASSERT_TRUE(RunJsTestSuite("apply_function_with_constraints.js"));
}
// NOTE: issue (18006) fix function as param
TEST_F(EtsGenericCallParamsEtsToTsTest, DISABLED_check_generic_subset_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_subset_class.js"));
}
// NOTE: issue (18006) fix function as param
TEST_F(EtsGenericCallParamsEtsToTsTest, DISABLED_check_apply_function_generic_union)
{
    ASSERT_TRUE(RunJsTestSuite("check_apply_function_generic_union.js"));
}

}  // namespace ark::ets::interop::js::testing
