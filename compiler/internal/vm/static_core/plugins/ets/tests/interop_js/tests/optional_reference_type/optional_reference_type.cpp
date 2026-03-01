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

class EtsPassingReferenceType : public EtsInteropTest {};

TEST_F(EtsPassingReferenceType, referenceAnyParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceAnyParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceAnyWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceAnyWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceLiteralParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceLiteralParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceLiteralWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceLiteralWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceExtraSetParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceExtraSetParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceExtraSetWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceExtraSetWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetPickParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceSubsetPickParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetPickWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceSubsetPickWithoutParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceSubsetOmitParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceSubsetOmitParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetOmitWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceSubsetOmitWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetPartialParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceSubsetPartialParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceSubsetPartialWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceSubsetPartialWithoutParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceUnionArrParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceUnionArrParam");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsPassingReferenceType, referenceUnionObjParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceUnionObjParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceUnionWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceUnionWithoutParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceClassParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceClassParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceClassWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceClassWithoutParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceInterfaceParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceInterfaceParam");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsPassingReferenceType, referenceInterfaceWithoutParam)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceInterfaceWithoutParam");
    ASSERT_EQ(ret, true);
}

// NOTE(pishin) enable after fixibng #17878
TEST_F(EtsPassingReferenceType, DISABLED_referenceObjectWithArr)
{
    auto ret = CallEtsFunction<bool>(GetPackageName(), "referenceObjectWithArr");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing