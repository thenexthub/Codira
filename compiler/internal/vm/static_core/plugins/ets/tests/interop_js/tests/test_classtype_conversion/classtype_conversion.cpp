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
#include "plugins/ets/runtime/interop_js/js_value.h"

namespace ark::ets::interop::js::testing {

class EtsESValueTypeOfTest : public EtsInteropTest {};

TEST_F(EtsESValueTypeOfTest, check_is_boolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsBoolean"));
}

TEST_F(EtsESValueTypeOfTest, check_is_number)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsNumber"));
}

TEST_F(EtsESValueTypeOfTest, check_is_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsString"));
}

TEST_F(EtsESValueTypeOfTest, check_is_bigint)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsBigInt"));
}

TEST_F(EtsESValueTypeOfTest, check_is_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsUndefined"));
}

TEST_F(EtsESValueTypeOfTest, check_is_function)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsFunction"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_boolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfBoolean"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_number)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfNumber"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfString"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_bigint)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfBigInt"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfUndefined"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_null)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfNull"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_function)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfFunction"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_symbol)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfSymbol"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_array)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfArray"));
}

TEST_F(EtsESValueTypeOfTest, check_typeof_object)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfObject"));
}
TEST_F(EtsESValueTypeOfTest, check_typeof_external)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOfExternal"));
}
}  // namespace ark::ets::interop::js::testing
