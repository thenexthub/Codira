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

class EtsImportPrimitiveTypesTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChangeNumber"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChangeString"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeTrue)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChangeTrue"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeFalse)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChangeFalse"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeUndefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChangeUndefined"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeNull)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChangeNull"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeSymbol)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChangeSymbol"));
}

TEST_F(EtsImportPrimitiveTypesTsToEtsTest, checkChangeBigInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChangeBigInt"));
}

}  // namespace ark::ets::interop::js::testing
