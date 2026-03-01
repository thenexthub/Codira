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

class EtsGenericStaticEtsToTsTest : public EtsInteropTest {};

// NOTE: issue (1835) fix Literal type
TEST_F(EtsGenericStaticEtsToTsTest, DISABLED_check_generic_static_literal)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_static_literal.js"));
}

TEST_F(EtsGenericStaticEtsToTsTest, check_static_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_static_method.js"));
}

TEST_F(EtsGenericStaticEtsToTsTest, check_class_extends)
{
    ASSERT_TRUE(RunJsTestSuite("check_class_extends.js"));
}

TEST_F(EtsGenericStaticEtsToTsTest, check_generic_union_static)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_union_static.js"));
}

TEST_F(EtsGenericStaticEtsToTsTest, check_user_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_user_class.js"));
}
// // NOTE: issue (18183) extends interface
TEST_F(EtsGenericStaticEtsToTsTest, DISABLED_check_interface_static)
{
    ASSERT_TRUE(RunJsTestSuite("check_interface_static.js"));
}
// NOTE: issue (18007) fix constrain primitive
TEST_F(EtsGenericStaticEtsToTsTest, DISABLED_check_generic_subset_by_value)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_subset_by_value.js"));
}

TEST_F(EtsGenericStaticEtsToTsTest, check_generic_subset_by_ref)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_subset_by_ref.js"));
}
// NOTE: issue (18007) fix constrain
TEST_F(EtsGenericStaticEtsToTsTest, DISABLED_check_generic_static_extra_set)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_static_extra_set.js"));
}

}  // namespace ark::ets::interop::js::testing
