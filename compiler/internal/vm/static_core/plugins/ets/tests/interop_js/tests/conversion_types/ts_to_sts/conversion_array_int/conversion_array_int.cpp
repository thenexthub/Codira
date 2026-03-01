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

class EtsConversionArrayIntTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntToArrayChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionBinaryToArrayChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntStringToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntStringToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntStringToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntStringToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntStringToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntStringToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntStringToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionIntStringToArrayChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionIntStringToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionHexadecimalToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionHexadecimalToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionHexadecimalToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionHexadecimalToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionHexadecimalToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionHexadecimalToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionHexadecimalToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionHexadecimalToArrayChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionHexadecimalToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionFloatToArrayChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionByteToArrayChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionByteToArrayChar"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToArrayInt"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToArrayNumber"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToArrayFloat"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToArrayByte"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToArrayShort"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToArrayLong"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntArrayDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToArrayDouble"));
}
// NOTE issue (17741) - enable this after fix import bigInt
TEST_F(EtsConversionArrayIntTsToEtsTest, DISABLED_CheckConversionBigIntToArrayChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBigIntToArrayChar"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionExponentialToArrayInt"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionExponentialToArrayNumber"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionExponentialToArrayFloat"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionExponentialToArrayByte"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionExponentialToArrayShort"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionExponentialToArrayLong"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialArrayDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionExponentialToArrayDouble"));
}

TEST_F(EtsConversionArrayIntTsToEtsTest, checkConversionExponentialToArrayChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionExponentialToArrayChar"));
}

}  // namespace ark::ets::interop::js::testing
