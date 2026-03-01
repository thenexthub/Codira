/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "libabckit/cpp/abckit_cpp.h"
#include "tests/helpers/helpers.h"
#include "tests/helpers/helpers_runtime.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiNamespacesTest : public ::testing::Test {};

// Helper function to verify namespace name change
static void VerifyNamespaceRenamed(AbckitFile *file, const std::string &moduleName, const std::string &oldName,
                                   const std::string &newName)
{
    helpers::ModuleByNameContext moduleCtx = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &moduleCtx, helpers::ModuleByNameFinder);
    ASSERT_NE(moduleCtx.module, nullptr);

    // Verify old namespace no longer exists
    helpers::NamespaceByNameContext oldNsCtx = {nullptr, oldName.c_str()};
    g_implI->moduleEnumerateNamespaces(moduleCtx.module, &oldNsCtx, helpers::NamespaceByNameFinder);
    EXPECT_EQ(oldNsCtx.n, nullptr);

    // Verify new namespace exists
    helpers::NamespaceByNameContext newNsCtx = {nullptr, newName.c_str()};
    g_implI->moduleEnumerateNamespaces(moduleCtx.module, &newNsCtx, helpers::NamespaceByNameFinder);
    EXPECT_NE(newNsCtx.n, nullptr);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::namespaceSetName, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiNamespacesTest, NamespaceSetNameStatic)
{
    constexpr const char *INPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/namespaces/namespace_static_modify.abc";
    constexpr const char *OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/namespaces/namespace_static_modify_modified.abc";
    constexpr const char *MODULE_NAME = "namespace_static_modify";
    constexpr const char *OLD_NAMESPACE = "NS1";
    constexpr const char *NEW_NAMESPACE = "nnnnnns";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext moduleCtx = {nullptr, MODULE_NAME};
    g_implI->fileEnumerateModules(file, &moduleCtx, helpers::ModuleByNameFinder);
    ASSERT_NE(moduleCtx.module, nullptr);

    helpers::NamespaceByNameContext nsCtx = {nullptr, OLD_NAMESPACE};
    g_implI->moduleEnumerateNamespaces(moduleCtx.module, &nsCtx, helpers::NamespaceByNameFinder);
    ASSERT_NE(nsCtx.n, nullptr);

    helpers::NamespaceByNameContext nsCtx2 = {nullptr, "NS2"};
    g_implI->namespaceEnumerateNamespaces(nsCtx.n, &nsCtx2, helpers::NamespaceByNameFinder);
    ASSERT_NE(nsCtx2.n, nullptr);
    auto arktsNs = g_implArkI->coreNamespaceToArktsNamespace(nsCtx.n);
    g_implArkM->namespaceSetName(arktsNs, NEW_NAMESPACE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::EnumByNameContext enumCtx = {nullptr, "E1"};
    g_implI->namespaceEnumerateEnums(nsCtx2.n, &enumCtx, helpers::EnumByNameFinder);
    ASSERT_NE(enumCtx.enm, nullptr);
    auto arktsEnum = g_implArkI->coreEnumToArktsEnum(enumCtx.enm);
    g_implArkM->enumSetName(arktsEnum, "newenum");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    VerifyNamespaceRenamed(file, MODULE_NAME, OLD_NAMESPACE, NEW_NAMESPACE);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::ExecuteStaticAbc(OUTPUT_PATH, MODULE_NAME, "main");
}

}  // namespace libabckit::test