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

class EtsIntersectionArgLiteralTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkArgFuFromSts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkArgFuFromSts"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntersectionTypeClass"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkIntersectionClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIntersectionClassMethod"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkCreateIntersectionClassTypeFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateIntersectionClassTypeFromTs"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkCallMethodIntersectionClassTypeFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCallMethodIntersectionClassTypeFromTs"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkInstanceIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceIntersectionTypeClass"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkChildIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildIntersectionTypeClass"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkChildIntersectionTypeClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildIntersectionTypeClassMethod"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkCreateChildIntersectionClassTypeFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateChildIntersectionClassTypeFromTs"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkCallMethodChildIntersectionClassTypeFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCallMethodChildIntersectionClassTypeFromTs"));
}

TEST_F(EtsIntersectionArgLiteralTsToEtsTest, checkInstanceChildIntersectionTypeClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildIntersectionTypeClass"));
}

}  // namespace ark::ets::interop::js::testing
