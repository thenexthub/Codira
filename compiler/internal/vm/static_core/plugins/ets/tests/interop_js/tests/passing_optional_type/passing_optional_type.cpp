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

class EtsPassingOptionalType : public EtsInteropTest {};

TEST_F(EtsPassingOptionalType, optionalAnyParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalAnyParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalAnyWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalAnyWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalAnyObjectParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalAnyObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalAnyObjectWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalAnyObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalLiteralParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalLiteralParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalLiteralWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalLiteralWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalLiteralObjectParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalLiteralObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalLiteralObjectWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalLiteralObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalExtraSetParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalExtraSetParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalExtraSetWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalExtraSetWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalSubSetObjectReduseParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalSubSetObjectReduseParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalSubSetObjectPartialParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalSubSetObjectPartialParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalExtraSetObjectParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalExtraSetObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalExtraSetObjectWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalExtraSetObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalSubSetObjectParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalSubSetObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalSubSetObjectWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalSubSetObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionStringParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalUnionStringParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionStringWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalUnionStringWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionNumberParam)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "optionalUnionNumberParam");
    constexpr int EXPECTED_VALUE = 999;
    ASSERT_EQ(ret, EXPECTED_VALUE);
}

TEST_F(EtsPassingOptionalType, optionalUnionWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalUnionWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionStringObjectParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalUnionStringObjectParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingOptionalType, optionalUnionNumberObjectParam)
{
    auto ret = CallEtsFunction<int64_t>(GetPackageName(), "optionalUnionNumberObjectParam");
    constexpr int EXPECTED_VALUE = 777;
    ASSERT_EQ(ret, EXPECTED_VALUE);
}

TEST_F(EtsPassingOptionalType, optionalUnionObjectWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "optionalUnionObjectWithoutParam");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing