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

class EtsInteropClassMethodsReturningValuesTest : public EtsInteropTest {};

// Note: Several tests disabled until #17741 is resolved

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnIntegerAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnIntegerAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnStringAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnStringAsAny");
    ASSERT_EQ(ret, true);
}

// Note: Test is disabled until #17741 is resolved
TEST_F(EtsInteropClassMethodsReturningValuesTest, DISABLED_eTS_call_TS_TestReturnBigIntegerAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnBigIntegerAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnBooleanAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnBooleanAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnUndefinedAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnUndefinedAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnNullAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnNullAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnMapAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnMapAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnSetAsAny)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnSetAsAny");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnStringAsLiteral)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnStringAsLiteral");
    ASSERT_EQ(ret, true);
}

// Note: Test is disabled until #17741 is resolved
TEST_F(EtsInteropClassMethodsReturningValuesTest, DISABLED_eTS_call_TS_TestReturnBigNAsLiteral)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnBigNAsLiteral");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnIntAsLiteral)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnIntAsLiteral");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnBoolAsLiteral)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnBoolAsLiteral");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnMap)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnMap");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnSet)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnSet");
    ASSERT_EQ(ret, true);
}
// NOTE (#24570): fix interop tests with tuples
TEST_F(EtsInteropClassMethodsReturningValuesTest, DISABLED_TestReturnTuple)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnTuple");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnStringSubsetByRef)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnStringSubsetByRef");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnMapSubsetByRef)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnMapSubsetByRef");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnStringSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnStringSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnIntSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnIntSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnLongIntSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnLongIntSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnBoolSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnBoolSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnUndefinedSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnUndefinedSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnNullSubsetByValue)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnNullSubsetByValue");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnUnion)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnUnion");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnClass)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnClass");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropClassMethodsReturningValuesTest, TestReturnInterface)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    auto ret = CallEtsFunction<bool>(GetPackageName(), "TestReturnInterface");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
