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

class EtsConversionFloatTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionFloatTsToEtsTest, checkConversionFloatToInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToInt"));
}

TEST_F(EtsConversionFloatTsToEtsTest, checkConversionFloatToNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToNumber"));
}

TEST_F(EtsConversionFloatTsToEtsTest, checkConversionFloatToFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToFloat"));
}

TEST_F(EtsConversionFloatTsToEtsTest, checkConversionFloatToByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToByte"));
}

TEST_F(EtsConversionFloatTsToEtsTest, checkConversionFloatToShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToShort"));
}

TEST_F(EtsConversionFloatTsToEtsTest, checkConversionFloatToLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToLong"));
}

TEST_F(EtsConversionFloatTsToEtsTest, checkConversionFloatToDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToDouble"));
}

TEST_F(EtsConversionFloatTsToEtsTest, checkConversionFloatToChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionFloatToChar"));
}

}  // namespace ark::ets::interop::js::testing
