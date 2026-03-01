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

class EtsGetterEtsToJsTest : public EtsInteropTest {};

TEST_F(EtsGetterEtsToJsTest, check_public_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_public_getter_class.js"));
}

TEST_F(EtsGetterEtsToJsTest, check_protected_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_protected_getter_class.js"));
}

TEST_F(EtsGetterEtsToJsTest, check_protected_getter_inheritance_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_protected_getter_inheritance_class.js"));
}

TEST_F(EtsGetterEtsToJsTest, check_private_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_private_getter_class.js"));
}

TEST_F(EtsGetterEtsToJsTest, check_private_getter_inheritance_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_private_getter_inheritance_class.js"));
}

TEST_F(EtsGetterEtsToJsTest, check_union_type_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_union_type_getter_class.js"));
}

TEST_F(EtsGetterEtsToJsTest, check_any_type_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_any_type_getter_class.js"));
}
// NOTE (#24570): fix interop tests for tuples
TEST_F(EtsGetterEtsToJsTest, DISABLED_check_any_type_getter_class_tuple)
{
    ASSERT_TRUE(RunJsTestSuite("check_any_type_getter_class_tuple.js"));
}

TEST_F(EtsGetterEtsToJsTest, check_subset_by_ref_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_subset_by_ref_getter_class.js"));
}

TEST_F(EtsGetterEtsToJsTest, check_subset_by_value_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_subset_by_value_getter_class.js"));
}

TEST_F(EtsGetterEtsToJsTest, DISABLED_check_static_getter_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_static_getter_class.js"));
}

}  // namespace ark::ets::interop::js::testing
