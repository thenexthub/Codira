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

class EtsConversionBigIntTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionBigIntTsToEtsTest, checkTypeBigInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkTypeBigInt"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToInt"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToNumber"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToFloat"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToByte"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToShort"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToLong"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToDouble"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionBigIntTsToEtsTest, DISABLED_checkConversionBigIntToChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToChar"));
}

TEST_F(EtsConversionBigIntTsToEtsTest, checkReturnBigInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkReturnBigInt"));
}

TEST_F(EtsConversionBigIntTsToEtsTest, checkReturnBigIntObj)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkReturnBigIntObj"));
}

}  // namespace ark::ets::interop::js::testing
