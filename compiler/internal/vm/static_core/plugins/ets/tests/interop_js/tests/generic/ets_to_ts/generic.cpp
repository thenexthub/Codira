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

class EtsGenericEtsToTsTest : public EtsInteropTest {};

TEST_F(EtsGenericEtsToTsTest, check_generic_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_class.js"));
}

TEST_F(EtsGenericEtsToTsTest, check_generic_any)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_any.js"));
}
// NOTE (#24570): fix interop tests with tuple
TEST_F(EtsGenericEtsToTsTest, DISABLED_check_generic_tuple)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_tuple.js"));
}

TEST_F(EtsGenericEtsToTsTest, check_generic_union)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_union.js"));
}

TEST_F(EtsGenericEtsToTsTest, check_generic_interface)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_interface.js"));
}
// NOTE (issues 1805) fix extend interface
TEST_F(EtsGenericEtsToTsTest, DISABLED_check_generic_subset_ref)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_subset_ref.js"));
}
// NOTE (issues 1806) fix check_explicitly declared type
TEST_F(EtsGenericEtsToTsTest, DISABLED_check_generic_explicitly_declared)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_explicitly_declared.js"));
}

TEST_F(EtsGenericEtsToTsTest, check_generic_abstract)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_abstract.js"));
}

}  // namespace ark::ets::interop::js::testing
