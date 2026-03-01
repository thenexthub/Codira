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
#include <cstring>

#include "libabckit/c/abckit.h"
#include "libabckit/c/metadata_core.h"
#include "metadata_inspect_impl.h"  // NOTE(mshimenkov)

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitInspectApiStringsTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::abckitStringToString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiStringsTest, AbckitStringToStringStatic)
{
    ASSERT_NE(g_impl, nullptr);
    ASSERT_NE(g_implI, nullptr);

    g_implI->abckitStringToString(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    std::string testString = "testString";

    auto *abcTestString = new AbckitString {testString};  // NOTE
    auto name = g_implI->abckitStringToString(abcTestString);
    delete abcTestString;

    ASSERT_EQ(name, testString);
}

// Test: test-kind=api, api=InspectApiImpl::abckitStringToString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiStringsTest, AbckitStringToStringDynamic)
{
    ASSERT_NE(g_impl, nullptr);
    ASSERT_NE(g_implI, nullptr);

    g_implI->abckitStringToString(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    std::string testString = "testString";

    auto *abcTestString = new AbckitString {testString};  // NOTE
    auto name = g_implI->abckitStringToString(abcTestString);
    delete abcTestString;

    ASSERT_EQ(name, testString);
}

}  // namespace libabckit::test
