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

class ArrayTsToEtsTest : public EtsInteropTest {};

TEST_F(ArrayTsToEtsTest, indexAccess)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "indexAccess"));
}

TEST_F(ArrayTsToEtsTest, testLength)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testLength"));
}

TEST_F(ArrayTsToEtsTest, testArrayTypeof)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "typeofArray"));
}

TEST_F(ArrayTsToEtsTest, testInstanceof)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "instanceofArray"));
}

TEST_F(ArrayTsToEtsTest, testInstanceofObj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "instanceofArrayObject"));
}

TEST_F(ArrayTsToEtsTest, testAt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testAt"));
}

TEST_F(ArrayTsToEtsTest, testConcat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testConcat"));
}

TEST_F(ArrayTsToEtsTest, testCopyWithin)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testCopyWithin"));
}

TEST_F(ArrayTsToEtsTest, testEvery)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testEvery"));
}

TEST_F(ArrayTsToEtsTest, testFill)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFill"));
}

TEST_F(ArrayTsToEtsTest, testFilter)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFilter"));
}

TEST_F(ArrayTsToEtsTest, testFind)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFind"));
}

TEST_F(ArrayTsToEtsTest, testFindIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFindIndex"));
}

TEST_F(ArrayTsToEtsTest, testFindLast)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFindLast"));
}

TEST_F(ArrayTsToEtsTest, testFindLastIndex)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFindLastIndex"));
}

TEST_F(ArrayTsToEtsTest, testFlat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFlat"));
}

TEST_F(ArrayTsToEtsTest, testFlatMap)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testFlatMap"));
}

TEST_F(ArrayTsToEtsTest, testForEach)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testForEach"));
}

TEST_F(ArrayTsToEtsTest, testIncludes)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testIncludes"));
}

TEST_F(ArrayTsToEtsTest, testIndexOf)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testIndexOf"));
}

TEST_F(ArrayTsToEtsTest, testJoin)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testJoin"));
}

TEST_F(ArrayTsToEtsTest, testLastIndexOf)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testLastIndexOf"));
}

TEST_F(ArrayTsToEtsTest, testMap)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMap"));
}

TEST_F(ArrayTsToEtsTest, testPop)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testPop"));
}

TEST_F(ArrayTsToEtsTest, testPushOne)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testPushOne"));
}

TEST_F(ArrayTsToEtsTest, testPushArray)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testPushArray"));
}

TEST_F(ArrayTsToEtsTest, testReduce)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testReduce"));
}

TEST_F(ArrayTsToEtsTest, testReduceRight)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testReduceRight"));
}

TEST_F(ArrayTsToEtsTest, testReverse)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testReverse"));
}

TEST_F(ArrayTsToEtsTest, testShift)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testShift"));
}

TEST_F(ArrayTsToEtsTest, testSlice)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSlice"));
}

TEST_F(ArrayTsToEtsTest, testSome)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSome"));
}

TEST_F(ArrayTsToEtsTest, testSort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSort"));
}

TEST_F(ArrayTsToEtsTest, testSplice)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testSplice"));
}

TEST_F(ArrayTsToEtsTest, testToLocaleString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToLocaleString"));
}

TEST_F(ArrayTsToEtsTest, testToReversed)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToReversed"));
}

TEST_F(ArrayTsToEtsTest, testToSorted)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToSorted"));
}

TEST_F(ArrayTsToEtsTest, testToSpliced)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testToSpliced"));
}

TEST_F(ArrayTsToEtsTest, testUnShift)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testUnShift"));
}

TEST_F(ArrayTsToEtsTest, testWith)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testWith"));
}

}  // namespace ark::ets::interop::js::testing