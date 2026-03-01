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

class EtsIndexedSignatureTsToEtsTest : public EtsInteropTest {};

// NOTE: (alexanderpolenov)issue(18238) access by string index
TEST_F(EtsIndexedSignatureTsToEtsTest, DISABLED_checkValueDynamicObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkValueDynamicObject"));
}
// NOTE: (alexanderpolenov)issue(18238) access by string index
TEST_F(EtsIndexedSignatureTsToEtsTest, DISABLED_checkAddNewPropertyDynamicObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAddNewPropertyDynamicObject"));
}
// // NOTE: (alexanderpolenov)issue(18238) access by string index
TEST_F(EtsIndexedSignatureTsToEtsTest, DISABLED_checkAddArrayDynamicObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAddArrayDynamicObject"));
}
// // NOTE: (alexanderpolenov)issue(18238) access by string index
TEST_F(EtsIndexedSignatureTsToEtsTest, DISABLED_checkPushToArrayDynamicObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkPushToArrayDynamicObject"));
}
// // NOTE: (alexanderpolenov)issue(18239) access by number index
TEST_F(EtsIndexedSignatureTsToEtsTest, DISABLED_checkAddToArrayByIndexDynamicObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAddToArrayByIndexDynamicObject"));
}
// // NOTE: (alexanderpolenov)issue(18238) access by string index
TEST_F(EtsIndexedSignatureTsToEtsTest, DISABLED_checkCallFunctionDynamicObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCallFunctionDynamicObject"));
}

}  // namespace ark::ets::interop::js::testing
