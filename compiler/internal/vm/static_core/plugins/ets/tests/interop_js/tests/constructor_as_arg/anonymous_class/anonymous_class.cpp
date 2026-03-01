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

class EtsAnonymousClassTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsAnonymousClassTsToEtsTest, check_anonymous_class_create_class)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnonymousClassCreateClass"));
}

TEST_F(EtsAnonymousClassTsToEtsTest, check_create_anonymous_class_create_class_with_arg_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnonymousClassCreateClassWithArgFromTs"));
}

TEST_F(EtsAnonymousClassTsToEtsTest, check_create_anonymous_class_create_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnonymousClassCreateClassFromTs"));
}

TEST_F(EtsAnonymousClassTsToEtsTest, check_anonymous_class_create_main_instance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnonymousClassCreateMainInstance"));
}

TEST_F(EtsAnonymousClassTsToEtsTest, check_create_anonymous_class_create_IIFE_class_from_ts)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateAnonymousClassCreateIIFEClassFromTs"));
}

TEST_F(EtsAnonymousClassTsToEtsTest, check_anonymous_class_create_IIFE_instance)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAnonymousClassCreateIIFEInstance"));
}

}  // namespace ark::ets::interop::js::testing
