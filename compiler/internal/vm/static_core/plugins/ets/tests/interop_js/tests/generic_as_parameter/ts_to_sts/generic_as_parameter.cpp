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

class EtsGenericAsParameterTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterBool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterArr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterObj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterUnion)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterUnion"));
}
// NOTE (#24570): fix interop tests
TEST_F(EtsGenericAsParameterTsToEtsTest, DISABLED_checkAnyTypeParameterTuple)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterExplicitCallInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterExplicitCallString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallBool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterExplicitCallBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallArr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterExplicitCallArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallObj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterExplicitCallObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallUnion)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterExplicitCallUnion"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallTuple)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterExplicitCallTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkAnyTypeParameterExplicitCallLiteral)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnyTypeParameterExplicitCallLiteral"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionReturnAnyInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionReturnAnyString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyBool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionReturnAnyBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyArr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionReturnAnyArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyObj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionReturnAnyObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionReturnAnyUnion)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionReturnAnyUnion"));
}

// NOTE (#24570): fix interop test
TEST_F(EtsGenericAsParameterTsToEtsTest, DISABLED_checkGenericTypeFunctionReturnAnyTuple)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionReturnAnyTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionExplicitCallInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionExplicitCallString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallBool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionExplicitCallBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallArr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionExplicitCallArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallObj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionExplicitCallObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallUnion)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionExplicitCallUnion"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallTuple)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionExplicitCallTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericTypeFunctionExplicitCallLiteral)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericTypeFunctionExplicitCallLiteral"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendGenericNumber"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendGenericString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericBool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendGenericBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericArr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendGenericArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericObj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendGenericObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericUnion)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendGenericUnion"));
}

// NOTE (#24570): add issue, fix interop test
TEST_F(EtsGenericAsParameterTsToEtsTest, DISABLED_checkExtendGenericTuple)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendGenericTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkExtendGenericLiteral)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkExtendGenericLiteral"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericExtendInterface)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericExtendInterface"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericExtendType)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericExtendType"));
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsGenericAsParameterTsToEtsTest, DISABLED_checkTupleGeneric)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTupleGeneric"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCollectGenericInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCollectGenericInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCollectGenericString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCollectGenericString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCollectGenericBool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCollectGenericBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCollectGenericObj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCollectGenericObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericUserClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericUserClass"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkCreateClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkUserClassInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUserClassInstance"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultInt"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultString"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultBool)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultBool"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultArr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultArr"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultObj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultObj"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultUnion)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultUnion"));
}
// NOTE (#24570): fix interop tests for tuples
TEST_F(EtsGenericAsParameterTsToEtsTest, DISABLED_checkGenericDefaultTuple)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultTuple"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultLiteral)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultLiteral"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultIntCallFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultIntCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultStringCallFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultStringCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultBoolCallFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultBoolCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultArrCallFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultArrCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultObjCallFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultObjCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultUnionCallFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultUnionCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultTupleCallFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultTupleCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericDefaultLiteralCallFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericDefaultLiteralCallFromTs"));
}

TEST_F(EtsGenericAsParameterTsToEtsTest, checkGenericExtendClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGenericExtendClass"));
}

}  // namespace ark::ets::interop::js::testing
