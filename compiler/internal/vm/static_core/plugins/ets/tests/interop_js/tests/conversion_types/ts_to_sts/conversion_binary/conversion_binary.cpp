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

class EtsConversionBinaryTsToEtsTest : public EtsInteropTest {};

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToInt)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToInt"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToNumber)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToNumber"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToFloat)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToFloat"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToByte)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToByte"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToShort)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToShort"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToLong)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToLong"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToDouble)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToDouble"));
}

TEST_F(EtsConversionBinaryTsToEtsTest, checkConversionBinaryToChar)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "checkConversionBinaryToChar"));
}

}  // namespace ark::ets::interop::js::testing
