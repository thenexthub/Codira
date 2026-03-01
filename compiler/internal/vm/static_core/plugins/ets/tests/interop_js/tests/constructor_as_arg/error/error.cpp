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

class EtsConstructorAsArgErrorTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassFunctionInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassFunctionString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_obj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassFunctionObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_arr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassFunctionArr"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_function_arrow)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassFunctionArrow"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassArrowFunctionInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassArrowFunctionString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_obj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassArrowFunctionObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_arr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassArrowFunctionArr"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_create_class_arrow_function_arrow)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkCreateClassArrowFunctionArrow"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkParentClassInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkParentClassString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_array)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkParentClassArray"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_obj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkParentClassObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_parent_class_arrow)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkParentClassArrow"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClassInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClassString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_array)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClassArray"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_obj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClassObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_child_class_arrow)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkChildClassArrow"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_int)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMethodClassInt"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMethodClassString"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_arr)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMethodClassArr"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_obj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMethodClassObj"));
}

TEST_F(EtsConstructorAsArgErrorTsToEtsTest, check_method_class_arrow)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkMethodClassArrow"));
}

}  // namespace ark::ets::interop::js::testing
