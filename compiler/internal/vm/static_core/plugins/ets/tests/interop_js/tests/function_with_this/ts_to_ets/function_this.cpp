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

class TsMethodWithThisTsToEtsTest : public EtsInteropTest {};

TEST_F(TsMethodWithThisTsToEtsTest, testBaseClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBaseClass"));
}

TEST_F(TsMethodWithThisTsToEtsTest, testChildClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testChildClass"));
}

TEST_F(TsMethodWithThisTsToEtsTest, testBaseFunc)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testBaseFunc"));
}

TEST_F(TsMethodWithThisTsToEtsTest, testChildFunc)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testChildFunc"));
}

TEST_F(TsMethodWithThisTsToEtsTest, testFooBaseObjInvoke)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFooBaseObjInvoke"));
}

TEST_F(TsMethodWithThisTsToEtsTest, testFooBaseObjInvokeMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFooBaseObjInvokeMethod"));
}

TEST_F(TsMethodWithThisTsToEtsTest, testFooChildObjInvoke)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFooChildObjInvoke"));
}

TEST_F(TsMethodWithThisTsToEtsTest, testFooChildObjInvokeMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFooChildObjInvokeMethod"));
}

TEST_F(TsMethodWithThisTsToEtsTest, testArrowFuncInvoke)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testArrowFuncInvoke"));
}

}  // namespace ark::ets::interop::js::testing
