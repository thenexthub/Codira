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

class ToStringTest : public EtsInteropTest {};

TEST_F(ToStringTest, implicit_to_string)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "implicitToString"), "optionalValA");
}

TEST_F(ToStringTest, test_functional1)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "testFunctional1"), "Str1A");
}

TEST_F(ToStringTest, test_functional2)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "testFunctional2"), "123A");
}

TEST_F(ToStringTest, test_nan_call)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "testNanCall"), "NaNA");
}

TEST_F(ToStringTest, test_throwing)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "testThrowing"), "123A");
}

// NOTE(www): #ICMNFA, Currently unable to get method object in 1.2.
TEST_F(ToStringTest, DISABLED_ets_jsvalue_to_str)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "etsJsvalueToStr"), "123AA");
}

// NOTE(www): #ICMNFA, Currently unable to get method object in 1.2.
TEST_F(ToStringTest, DISABLED_ets_object_to_str)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "etsObjectToStr"), "123AA");
}

TEST_F(ToStringTest, jsval_object_to_str)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "jsvalObjectToStr"), "123A");
}

TEST_F(ToStringTest, concat_null_to_string)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "concatNullToString"), "nullA");
}

// #21832: produces NPE
TEST_F(ToStringTest, DISABLED_concat_undefined_to_string)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "concatUndefinedToString"), "undefinedA");
}

TEST_F(ToStringTest, concat_nan_to_string)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "concatNanToString"), "NaNA");
}

// #21832: expects NPE
TEST_F(ToStringTest, DISABLED_explicit_optional_to_string)
{
    ASSERT_EQ(CallEtsFunction<std::string>(GetPackageName(), "explicitOptionalToString"), "NPE");
}

}  // namespace ark::ets::interop::js::testing
