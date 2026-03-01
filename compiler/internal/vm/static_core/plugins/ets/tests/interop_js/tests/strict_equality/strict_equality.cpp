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

class EtsInteropStrictEqualityTest : public EtsInteropTest {};

TEST_F(EtsInteropStrictEqualityTest, TestEqualTrue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestEqualTrue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestEqualFalse)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestEqualFalse");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotEqualTrue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestNotEqualTrue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotEqualFalse)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestNotEqualFalse");
    ASSERT_EQ(ret, true);
}

// Behaviour for equal is the same as strict equal
TEST_F(EtsInteropStrictEqualityTest, TestNotStrictEqualTrue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestNotStrictEqualTrue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotStrictEqualFalse)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestNotStrictEqualFalse");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotStrictNotEqualTrue)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestNotStrictNotEqualTrue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropStrictEqualityTest, TestNotStrictNotEqualFalse)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestNotStrictNotEqualFalse");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
