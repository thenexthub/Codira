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
#include <vector>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static constexpr auto MODIFIED_DYNAMIC =
    ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic_modified.abc";
static constexpr auto MODIFIED_STATIC = ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static_modified.abc";

class LibAbcKitModifyApiValuesTest : public ::testing::Test {};

// Test: test-kind=api, api=ModifyApiImpl::createValueU1, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiValuesTest, CreateValueU1_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createValueDouble, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiValuesTest, CreateValueDouble_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueDouble(file, 1.0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createValueString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiValuesTest, CreateValueString_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralArrayValue, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiValuesTest, CreateLiteralArrayValue_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test", strlen("test")));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2);

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arr, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_DYNAMIC, strlen(MODIFIED_DYNAMIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createValueU1, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiValuesTest, CreateValueU1_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createValueDouble, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiValuesTest, CreateValueDouble_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueDouble(file, 1.0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createValueString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiValuesTest, CreateValueString_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralArrayValue, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiValuesTest, CreateLiteralArrayValue_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test", strlen("test")));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2);

    ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(arr, nullptr);
    // Write output file
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
