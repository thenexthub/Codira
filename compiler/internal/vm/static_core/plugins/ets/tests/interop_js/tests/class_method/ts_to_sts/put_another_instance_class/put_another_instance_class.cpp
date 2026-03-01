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

class EtsClassMethodPutAnotherInstanceClassTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkChildClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateChildClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateChildClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceChildClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkChildClassMethodOwnMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClassMethodOwnMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateChildClassFromTsOwnMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateChildClassFromTsOwnMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceChildClassOwnMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildClassOwnMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkUserClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkUserClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateUserClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateUserClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceUserClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceUserClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInterfaceClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInterfaceClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateInterfaceClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateInterfaceClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceInterfaceClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceInterfaceClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkStaticClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkStaticClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateStaticClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateStaticClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkPrivateClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkPrivateClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreatePrivateClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreatePrivateClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstancePrivateClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstancePrivateClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkChildProtectedClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildProtectedClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateChildProtectedClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateChildProtectedClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceChildProtectedClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceChildProtectedClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkProtectedClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkProtectedClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateProtectedClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateProtectedClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceProtectedClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceProtectedClass"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkAbstractClassMethod)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAbstractClassMethod"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkCreateAbstractClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAbstractClassFromTs"));
}

TEST_F(EtsClassMethodPutAnotherInstanceClassTsToEtsTest, checkInstanceAbstractClass)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceAbstractClass"));
}

}  // namespace ark::ets::interop::js::testing
