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

class EtsOptionalOperatorTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsOptionalOperatorTsToEtsTest, checkOptionalOperatorWithValue)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorWithValue"));
}
// NOTE (17745) - enable after fix alias undefined
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorReturnUndefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorReturnUndefined"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_check_optional_operator_with_value_by_string_index)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "check_optional_operator_with_value_by_string_index"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorReturnUndefined_by_string_index)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorReturnUndefined_by_string_index"));
}

TEST_F(EtsOptionalOperatorTsToEtsTest, checkOptionalOperatorInsideObjectWithValue)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorInsideObjectWithValue"));
}
// NOTE (17745) - enable after fix alias undefined
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorInsideObjectReturnUndefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorInsideObjectReturnUndefined"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorInsideObjectWithValueByStringIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorInsideObjectWithValueByStringIndex"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorInsideReturnUndefinedByStringIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorInsideReturnUndefinedByStringIndex"));
}
// NOTE  (alexanderpolenov) issue(18447) enable after fix call function with ?. operator
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorFunction)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorFunction"));
}

// There is a strict type check in 1.2, it will be CTE if there not exist attribute.
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorFunctionUndefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorFunctionUndefined"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorFunctionByStringIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorFunctionByStringIndex"));
}
// NOTE: (alexanderpolenov) issue(18238) access by string index
TEST_F(EtsOptionalOperatorTsToEtsTest, DISABLED_checkOptionalOperatorFunctionReturnUndefinedByStringIndex)
{
    ASSERT_EQ(true,
              CallEtsFunction<bool>(GetPackageName(), "checkOptionalOperatorFunctionReturnUndefinedByStringIndex"));
}

}  // namespace ark::ets::interop::js::testing
