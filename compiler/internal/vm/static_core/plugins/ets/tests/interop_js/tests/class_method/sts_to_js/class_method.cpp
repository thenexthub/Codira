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

class EtsMethodClassEtsToTsTest : public EtsInteropTest {};

TEST_F(EtsMethodClassEtsToTsTest, check_user_class_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_user_class_method.js"));
}

TEST_F(EtsMethodClassEtsToTsTest, check_child_class_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_child_class_method.js"));
}

TEST_F(EtsMethodClassEtsToTsTest, check_interface_class_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_interface_class_method.js"));
}

TEST_F(EtsMethodClassEtsToTsTest, check_private_class_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_private_class_method.js"));
}

TEST_F(EtsMethodClassEtsToTsTest, check_protected_class_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_protected_class_method.js"));
}

TEST_F(EtsMethodClassEtsToTsTest, check_child_protected_class_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_child_protected_class_method.js"));
}

TEST_F(EtsMethodClassEtsToTsTest, check_abstract_class_method)
{
    ASSERT_TRUE(RunJsTestSuite("check_abstract_class_method.js"));
}

TEST_F(EtsMethodClassEtsToTsTest, check_put_another_instance_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_put_another_instance_class.js"));
}

}  // namespace ark::ets::interop::js::testing