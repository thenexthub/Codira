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

class EtsConstructorTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConstructorTsToEtsTest, checkNamedClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkNamedClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateNamedClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateNamedClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkNamedClassInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkNamedClassInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkAnonymousClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnonymousClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateAnonymousClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnonymousClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkAnonymousClassInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnonymousClassInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkFunctionConstructor)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkFunctionConstructor"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateFunctionConstructorFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateFunctionConstructorFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkFunctionConstructorInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkFunctionConstructorInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkIIFEClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIIFEClass"));
}
// NOTE issue(18074) fix IIFE without keyword 'new'.
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkIIFEClassError)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIIFEClassError"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateIIFEClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateIIFEClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkIIFEClassInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIIFEClassInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkIIFEConstructor)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIIFEConstructor"));
}

// NOTE issue(23306) change to catch exception.
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkIIFEConstructorUndefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIIFEConstructorUndefined"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateIIFEConstructorFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateIIFEConstructorFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkIIFEConstructorInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIIFEConstructorInstance"));
}
// NOTE issue(18077) fix try to call method with constructor.
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkMethodCreateConstructor)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMethodCreateConstructor"));
}
// NOTE issue(18077) fix try to call method with constructor.
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkCreateMethodConstructorClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateMethodConstructorClass"));
}
// NOTE issue(18077) fix try to call method with constructor.
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkMethodConstructorInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMethodConstructorInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkMethodCreateAnonymousClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMethodCreateAnonymousClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkMethodCreateClassInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMethodCreateClassInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkSimpleObject)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSimpleObject"));
}
// NOTE issue(18072) fix create object with keyword new .
TEST_F(EtsConstructorTsToEtsTest, DISABLED_checkSimpleArrowFunction)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSimpleArrowFunction"));
}

TEST_F(EtsConstructorTsToEtsTest, checkAbstractClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAbstractClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateAbstractClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAbstractClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkAbstractInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAbstractInstance"));
}

TEST_F(EtsConstructorTsToEtsTest, checkChildClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClass"));
}

TEST_F(EtsConstructorTsToEtsTest, checkCreateChildClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateChildClassFromTs"));
}

TEST_F(EtsConstructorTsToEtsTest, checkChildInstance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildInstance"));
}

}  // namespace ark::ets::interop::js::testing
