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

class EtsInteropIncorrectConversionTest : public EtsInteropTest {};

TEST_F(EtsInteropIncorrectConversionTest, conversionTsObjToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsObjToStr"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsArrayNumToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsArrayNumToStr"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsArrayStrToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsArrayStrToStr"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsArrayNumToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsArrayNumToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToBool"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToFloat"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToDouble"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsStrToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToChar"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsStrToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsStrToBigInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToFloat"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToDouble"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToChar"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsBooleanToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToStr"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBooleanToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBooleanToBigInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToFloat"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToDouble"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToChar"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToBool"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsUndefinedToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToStr"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsUndefinedToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsUndefinedToBigInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToNum"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToFloat"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToDouble"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToChar"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNullToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToBool"), true);
}
TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNumToInt"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNumToByte"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNumToShort"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNumToLong"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNumToChar"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNumToBool"), true);
}

TEST_F(EtsInteropIncorrectConversionTest, conversionTsNumToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNumToStr"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsNumToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNumToBigInt"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsNullToBigInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsNullToBigInt"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToNum)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToNum"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToByte)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToByte"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToShort)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToShort"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToInt)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToInt"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToLong)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToLong"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToFloat)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToFloat"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToDouble)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToDouble"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToChar)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToChar"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToBool)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToBool"), true);
}

// Note: Test is disabled until #17741 is resolved. Need to enable this after fix import bigInt.
TEST_F(EtsInteropIncorrectConversionTest, DISABLED_conversionTsBigIntToStr)
{
    // NOLINTNEXTLINE(modernize-use-auto)
    ASSERT_EQ(CallEtsFunction<bool>(GetPackageName(), "conversionTsBigIntToStr"), true);
}

}  // namespace ark::ets::interop::js::testing
