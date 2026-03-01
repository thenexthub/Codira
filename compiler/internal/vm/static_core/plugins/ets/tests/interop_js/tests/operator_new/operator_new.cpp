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

class EtsOperatorNew : public EtsInteropTest {};

TEST_F(EtsOperatorNew, noArgsClass)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "noArgsClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, noArgsClassBrackes)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "noArgsClassBrackes");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, oneArgsClass)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "oneArgsClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, twoArgsClass)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "twoArgsClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, manyArgsClass)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "manyArgsClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithSubClass)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "newWithSubClass");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithException)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "newWithException");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithFunction)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "newWithFunction");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsOperatorNew, newWithArrey)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "newWithArrey");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, newWithPrototype)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "newWithPrototype");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, DISABLED_newWithJustFunction)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "newWithJustFunction");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, DISABLED_newWithObj)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "newWithObj");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsOperatorNew, DISABLED_newWithPrimitive)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "newWithPrimitive");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
