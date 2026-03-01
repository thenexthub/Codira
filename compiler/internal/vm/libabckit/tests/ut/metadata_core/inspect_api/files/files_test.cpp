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

#include <cassert>
#include <cstring>
#include <gtest/gtest.h>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"
#include "logger.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitInspectApiFilesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::fileGetVersion, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFilesTest, StaticFileGetVersion)
{
    LIBABCKIT_LOG(DEBUG) << "StaticFileGetVersion source: " << ABCKIT_TEST_DIR "ut/metadata_core/inspect_api/files\n";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/files/file_static.abc", &file);

    AbckitFileVersion version = g_implI->fileGetVersion(file);
    std::array<uint8_t, ABCKIT_VERSION_SIZE> expectedVersion = {0, 0, 0, 6};

    const auto versionsEquality = std::memcmp(expectedVersion.data(), version, sizeof(uint8_t) * ABCKIT_VERSION_SIZE);
    ASSERT_EQ(versionsEquality, 0);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::fileGetVersion, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFilesTest, DynamicFileGetVersion)
{
    LIBABCKIT_LOG(DEBUG) << "StaticFileGetVersion source: " << ABCKIT_TEST_DIR "ut/metadata_core/inspect_api/files\n";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/files/file_dynamic.abc", &file);

    AbckitFileVersion version = g_implI->fileGetVersion(file);
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::array<uint8_t, ABCKIT_VERSION_SIZE> expectedVersion = {13, 0, 1, 0};
    const auto versionsEquality = std::memcmp(expectedVersion.data(), version, sizeof(uint8_t) * ABCKIT_VERSION_SIZE);
    ASSERT_EQ(versionsEquality, 0);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::fileEnumerateModules, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiFilesTest, DynamicFileEnumerateModules)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    std::unordered_map<std::string, AbckitCoreModule *> gotModulesMap;

    g_implI->fileEnumerateModules(file, &gotModulesMap, helpers::NameToModuleCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expectedNames = {"modules_dynamic", "modules/module1", "modules/module2", "modules/module3"};
    ASSERT_EQ(expectedNames.size(), gotModulesMap.size());
    for (auto &expectedName : expectedNames) {
        ASSERT_NE(gotModulesMap.find(expectedName), gotModulesMap.end());
    }

    constexpr auto OUTPUT_PATH = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic_modified.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
