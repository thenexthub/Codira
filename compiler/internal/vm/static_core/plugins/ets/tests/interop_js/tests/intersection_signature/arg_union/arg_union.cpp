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

class EtsIntersectionArgUnionTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkArgFuFromSts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkArgFuFromSts"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntersectionTypeClass"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkIntersectionClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntersectionClassMethod"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkCreateIntersectionClassTypeFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateIntersectionClassTypeFromTs"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkCallMethodIntersectionClassTypeFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCallMethodIntersectionClassTypeFromTs"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkInstanceIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceIntersectionTypeClass"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkChildIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildIntersectionTypeClass"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkChildIntersectionTypeClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildIntersectionTypeClassMethod"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkCreateChildIntersectionClassTypeFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateChildIntersectionClassTypeFromTs"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkCallMethodChildIntersectionClassTypeFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCallMethodChildIntersectionClassTypeFromTs"));
}

TEST_F(EtsIntersectionArgUnionTsToEtsTest, checkInstanceChildIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildIntersectionTypeClass"));
}

}  // namespace ark::ets::interop::js::testing
