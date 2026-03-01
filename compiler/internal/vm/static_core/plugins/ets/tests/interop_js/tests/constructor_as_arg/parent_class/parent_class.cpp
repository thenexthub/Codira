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

class EtsParentClassTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsParentClassTsToEtsTest, check_main_class_as_arg)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMainClassAsArg"));
}

TEST_F(EtsParentClassTsToEtsTest, check_anonymous_class_as_arg)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnonymousClassAsArg"));
}

TEST_F(EtsParentClassTsToEtsTest, check_IIFE_class_as_arg)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIIFEClassAsArg"));
}

TEST_F(EtsParentClassTsToEtsTest, check_create_main_class_with_arg_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateMainClassWithArgFromTs"));
}

TEST_F(EtsParentClassTsToEtsTest, check_create_main_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateMainClassFromTs"));
}

TEST_F(EtsParentClassTsToEtsTest, check_main_class_instance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMainClassInstance"));
}

TEST_F(EtsParentClassTsToEtsTest, check_create_anonymous_class_with_arg_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnonymousClassWithArgFromTs"));
}

TEST_F(EtsParentClassTsToEtsTest, checkCreateAnonymousClassFromTs)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnonymousClassFromTs"));
}

TEST_F(EtsParentClassTsToEtsTest, check_anonymous_class_instance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnonymousClassInstance"));
}

TEST_F(EtsParentClassTsToEtsTest, check_create_iife_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateIifeClassFromTs"));
}

TEST_F(EtsParentClassTsToEtsTest, check_iife_class_instance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIifeClassInstance"));
}

}  // namespace ark::ets::interop::js::testing
