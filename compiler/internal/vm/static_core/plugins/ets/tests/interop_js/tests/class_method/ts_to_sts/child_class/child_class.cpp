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

class EtsClassMethodChildClassTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkChildClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClassMethod"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkCreateChildClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateChildClassFromTs"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkInstanceChildClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildClass"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkChildClassMethodOwnMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClassMethodOwnMethod"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkCreateChildClassFromTsOwnMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateChildClassFromTsOwnMethod"));
}

TEST_F(EtsClassMethodChildClassTsToEtsTest, checkInstanceChildClassOwnMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildClassOwnMethod"));
}

}  // namespace ark::ets::interop::js::testing
