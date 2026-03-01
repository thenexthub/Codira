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

class EtsMimicIteratorEtsToTsTest : public EtsInteropTest {};

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorNumberForOf)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorNumberForOf"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorNumberKeys)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorNumberKeys"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorNumberValues)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorNumberValues"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorString)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorString"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorBoolean)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorBoolean"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorBooleanEntries)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorBooleanEntries"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorStringEntries)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorStringEntries"));
}

TEST_F(EtsMimicIteratorEtsToTsTest, testMimicIteratorNumberEntries)
{
    ASSERT_EQ(true, CallEtsFunction<bool>(GetPackageName(), "testMimicIteratorNumberEntries"));
}

}  // namespace ark::ets::interop::js::testing
