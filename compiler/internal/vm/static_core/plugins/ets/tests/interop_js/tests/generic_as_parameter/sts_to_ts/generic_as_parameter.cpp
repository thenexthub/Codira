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

class EtsGenericAsParameterEtsToTsTest : public EtsInteropTest {};

TEST_F(EtsGenericAsParameterEtsToTsTest, check_any_type_parameter)
{
    ASSERT_TRUE(RunJsTestSuite("check_any_type_parameter.js"));
}

TEST_F(EtsGenericAsParameterEtsToTsTest, DISABLED_check_any_type_parameter_tuple)
{
    ASSERT_TRUE(RunJsTestSuite("check_any_type_parameter_tuple.js"));
}
// NOTE: issue (17921) fix class implements interface
TEST_F(EtsGenericAsParameterEtsToTsTest, DISABLED_check_generic_extend_interface)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_extend_interface.js"));
}
// NOTE: issue (17924) fix class implements interface
TEST_F(EtsGenericAsParameterEtsToTsTest, DISABLED_check_generic_extend_type)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_extend_type.js"));
}

TEST_F(EtsGenericAsParameterEtsToTsTest, check_generic_extend_class)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_extend_class.js"));
}
// NOTE: issue (1835) fix Literal type
TEST_F(EtsGenericAsParameterEtsToTsTest, DISABLED_check_generic_default_type)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_default_type.js"));
}
// NOTE (#24570): fix interop tests
TEST_F(EtsGenericAsParameterEtsToTsTest, DISABLED_check_tuple_parameter_generic)
{
    ASSERT_TRUE(RunJsTestSuite("check_tuple_parameter_generic.js"));
}

TEST_F(EtsGenericAsParameterEtsToTsTest, check_collect_generic)
{
    ASSERT_TRUE(RunJsTestSuite("check_collect_generic.js"));
}

TEST_F(EtsGenericAsParameterEtsToTsTest, check_generic_constructor)
{
    ASSERT_TRUE(RunJsTestSuite("check_generic_constructor.js"));
}

}  // namespace ark::ets::interop::js::testing
