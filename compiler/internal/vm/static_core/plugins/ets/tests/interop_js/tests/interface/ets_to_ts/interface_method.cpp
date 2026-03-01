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

class EtsInterfaceEtsToTsTest : public EtsInteropTest {};

TEST_F(EtsInterfaceEtsToTsTest, check_any_type_interface_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_any_type_interface_method.js"));
}

TEST_F(EtsInterfaceEtsToTsTest, check_union_type_interface_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_union_type_interface_method.js"));
}
// NOTE (issues 17749) fix class subset by ref
TEST_F(EtsInterfaceEtsToTsTest, DISABLED_check_subset_by_ref_interface_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_subset_by_ref_interface_method.js"));
}

TEST_F(EtsInterfaceEtsToTsTest, check_subset_by_value_interface_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_subset_by_value_interface_method.js"));
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsInterfaceEtsToTsTest, DISABLED_check_tuple_type_interface_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_tuple_type_interface_method.js"));
}
// NOTE (issues 17769) optional method interface
TEST_F(EtsInterfaceEtsToTsTest, DISABLED_check_optional_method_interface)
{
    ASSERT_TRUE(RunJsTestSuite("check_optional_method_interface.js"));
}
// NOTE: issue (1835) fix Literal type
TEST_F(EtsInterfaceEtsToTsTest, DISABLED_check_literal_type_interface_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_literal_type_interface_method.js"));
}

}  // namespace ark::ets::interop::js::testing
