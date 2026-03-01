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

class EtsESValueJsToEtsTest : public EtsInteropTest {};

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_null)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetNull"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_boolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapBoolean"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapString"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_number)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapNumber"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_bigint)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapBigInt"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_wrap_Double)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkWrapDouble"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_boolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsBoolean"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsString"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_number)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsNumber"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_bigint)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsBigInt"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_Function)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsFunction"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_boolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToBoolean"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_string)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToString"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_Number)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToNumber"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_bigint)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToBigInt"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_to_null)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkToNull"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_equal_to)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsEqualTo"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_are_strict_equal)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkAreStrictEqual"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_is_equal_to_safe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIsEqualToSafe"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_by_name)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByName"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyStaticObj"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_by_name_safe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByNameSafe"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_by_index)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByIndex"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByIndexDouble"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_by_index_safe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertyByIndexSafe"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetProperty"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_get_property_safe)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkGetPropertySafe"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_set_property_by_name)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetPropertyByName"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_set_property_by_index)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetPropertyByIndex"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetPropertyByIndexDouble"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_set_property)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkSetProperty"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_has_property)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkHasProperty"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_has_property_by_name)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkHasPropertyByName"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_type_of)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeOf"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_invoke)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInvokeNoParam"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInvokeMethod"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInvokeHasParam"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInvokeMethodHasParam"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_iterator)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkIterator"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkKeys"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkValues"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkEntries"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_instanceOf)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfStaticObj"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfNumeric"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfStaticPrimitive"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfDynamic"));
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstanceOfNullOrUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_check_instaniate)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkInstaniate"));
}

TEST_F(EtsESValueJsToEtsTest, test_esvalue_test_undefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testUndefined"));
}

TEST_F(EtsESValueJsToEtsTest, checkThrowNullOrUndefined)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkThrowNullOrUndefined"));
}

}  // namespace ark::ets::interop::js::testing