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

class EtsIntersectionArgInterfaceTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkArgFuFromSts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkArgFuFromSts"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkAgeNameClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAgeNameClass"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkAgeNameClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAgeNameClassMethod"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkCreateAgeNameClassInterfaceFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAgeNameClassInterfaceFromTs"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkCallMethodAgeNameClassInterfaceFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCallMethodAgeNameClassInterfaceFromTs"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkInstanceAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAgeNameInterfaceClass"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkChildAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildAgeNameInterfaceClass"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkChildAgeNameInterfaceClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildAgeNameInterfaceClassMethod"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkCreateChildAgeNameClassInterfaceFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateChildAgeNameClassInterfaceFromTs"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkCallMethodChildAgeNameClassInterfaceFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCallMethodChildAgeNameClassInterfaceFromTs"));
}

TEST_F(EtsIntersectionArgInterfaceTsToEtsTest, checkInstanceChildAgeNameInterfaceClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildAgeNameInterfaceClass"));
}

}  // namespace ark::ets::interop::js::testing
