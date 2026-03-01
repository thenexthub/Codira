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

class EtsIntersectionArgAgeNameNegativeResultTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkArgFuFromSts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkArgFuFromSts"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkAgeNameClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAgeNameClass"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkAgeNameClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAgeNameClassMethod"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkInstanceAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAgeNameInterfaceClass"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkChildAgeNameClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildAgeNameClass"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkChildAgeNameClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildAgeNameClassMethod"));
}

TEST_F(EtsIntersectionArgAgeNameNegativeResultTsToEtsTest, checkInstanceChildAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildAgeNameInterfaceClass"));
}

}  // namespace ark::ets::interop::js::testing
