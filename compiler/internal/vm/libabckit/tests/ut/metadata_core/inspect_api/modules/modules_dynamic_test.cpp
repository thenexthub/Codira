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
#include "helpers/helpers_runtime.h"
#include "libabckit/c/metadata_core.h"
#include "metadata_inspect_impl.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "ir_impl.h"
#include "logger.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static constexpr auto MODIFIED = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic_modified.abc";

class LibAbcKitInspectApiModulesTest : public ::testing::Test {};

// ========================================
// Helpers
// ========================================

static bool ModuleNamesCollector(AbckitCoreModule *module, void *data)
{
    helpers::AssertModuleVisitor(module, data);

    AbckitFile *file = g_implI->moduleGetFile(module);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(file != nullptr);
    auto names = reinterpret_cast<std::set<std::string> *>(data);
    auto moduleName = g_implI->moduleGetName(module);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name = helpers::AbckitStringToString(moduleName);
    LIBABCKIT_LOG(DEBUG) << "module name: " << name << std::endl;
    EXPECT_TRUE(names->find(name.data()) == names->end());
    names->insert(name.data());

    return true;
}

static bool ModuleCollector(AbckitCoreModule *module, void *data)
{
    helpers::AssertModuleVisitor(module, data);

    auto modules = reinterpret_cast<std::set<AbckitCoreModule *> *>(data);
    EXPECT_TRUE(modules->find(module) == modules->end());
    modules->insert(module);

    return true;
}

static bool ModuleImportNamesCollector(AbckitCoreImportDescriptor *id, void *data)
{
    helpers::AssertImportVisitor(id, data);

    AbckitFile *file = g_implI->importDescriptorGetFile(id);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(file != nullptr);
    auto names = reinterpret_cast<std::unordered_map<std::string, size_t> *>(data);
    auto importName = g_implI->importDescriptorGetName(id);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name = helpers::AbckitStringToString(importName);
    LIBABCKIT_LOG(DEBUG) << "import name: " << name << std::endl;
    auto found = names->find(name.data());
    if (found != names->end()) {
        found->second += 1;
    } else {
        names->emplace(name, 1);
    }

    return true;
}

static bool ModuleImportAliasCollector(AbckitCoreImportDescriptor *id, void *data)
{
    helpers::AssertImportVisitor(id, data);

    AbckitFile *file = g_implI->importDescriptorGetFile(id);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(file != nullptr);
    auto names = reinterpret_cast<std::set<std::string> *>(data);
    auto importName = g_implI->importDescriptorGetAlias(id);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name = helpers::AbckitStringToString(importName);
    LIBABCKIT_LOG(DEBUG) << "import alias: " << name << std::endl;
    EXPECT_TRUE(names->find(name.data()) == names->end());
    names->insert(name.data());

    return true;
}

static bool ModuleExportNamesCollector(AbckitCoreExportDescriptor *ed, void *data)
{
    helpers::AssertExportVisitor(ed, data);

    AbckitFile *file = g_implI->exportDescriptorGetFile(ed);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(file != nullptr);
    auto names = reinterpret_cast<std::unordered_map<std::string, size_t> *>(data);
    auto exportName = g_implI->exportDescriptorGetName(ed);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name = helpers::AbckitStringToString(exportName);
    LIBABCKIT_LOG(DEBUG) << "export name: " << name << std::endl;
    auto found = names->find(name.data());
    if (found != names->end()) {
        found->second += 1;
    } else {
        names->emplace(name, 1);
    }

    return true;
}

static bool ModuleExportAliasCollector(AbckitCoreExportDescriptor *ed, void *data)
{
    helpers::AssertExportVisitor(ed, data);

    AbckitFile *file = g_implI->exportDescriptorGetFile(ed);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(file != nullptr);
    auto names = reinterpret_cast<std::set<std::string> *>(data);
    auto exportName = g_implI->exportDescriptorGetAlias(ed);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

    auto name = helpers::AbckitStringToString(exportName);
    LIBABCKIT_LOG(DEBUG) << "export alias: " << name << std::endl;
    EXPECT_TRUE(names->find(name.data()) == names->end());
    names->insert(name.data());

    return true;
}

static AbckitTarget GetFileTarget(AbckitFile *file, const char *moduleName)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, moduleName};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    return g_implI->moduleGetTarget(m);
}

// ========================================
// Modules Tests
// ========================================

// Test: test-kind=api, api=InspectApiImpl::moduleGetName, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicModuleGetName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    std::set<std::string> gotNames;

    g_implI->fileEnumerateModules(file, &gotNames, ModuleNamesCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expectedNames = {"modules_dynamic", "modules/module1", "modules/module2", "modules/module3"};
    ASSERT_EQ(expectedNames.size(), gotNames.size());
    for (auto &expectedName : expectedNames) {
        ASSERT_NE(gotNames.find(expectedName), gotNames.end());
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// NOTE: fix after ets2bundle integrated into abckit unit tests
// Test: test-kind=api, api=InspectApiImpl::moduleIsExternal, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicModuleIsExternal)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic_external.abc", &file);

    std::set<AbckitCoreModule *> gotModules;

    g_implI->fileEnumerateModules(file, &gotModules, ModuleCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::unordered_map<std::string, bool> expected = {{"modules_dynamic_external", false},
                                                      {"modules/module1", false},
                                                      {"modules/module2", false},
                                                      {"modules/module3", false},
                                                      {"external_module", true}};
    ASSERT_EQ(expected.size(), gotModules.size());
    for (auto *module : gotModules) {
        auto name = helpers::AbckitStringToString(g_implI->moduleGetName(module));
        ASSERT_NE(expected.find(name.data()), expected.end());
        ASSERT_EQ(g_implI->moduleIsExternal(module), expected[name.data()]);
    }

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic_external_modified.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicModuleGetFile)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    std::set<AbckitCoreModule *> gotModules;

    g_implI->fileEnumerateModules(file, &gotModules, ModuleCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(gotModules.size(), 4U);
    for (auto *module : gotModules) {
        auto file = g_implI->moduleGetFile(module);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(file, file);
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// ========================================
// Imports
// ========================================

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateImports, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicModuleEnumerateImports)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::set<AbckitCoreImportDescriptor *> gotImports;
    g_implI->moduleEnumerateImports(m, &gotImports, helpers::ModuleImportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(gotImports.size(), 5U);

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::importDescriptorGetName, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicImportDescriptorGetName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::unordered_map<std::string, size_t> gotImportNames;
    g_implI->moduleEnumerateImports(m, &gotImportNames, ModuleImportNamesCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expectedNames = {"*", "regularImportFunc1FromModule1", "regularImportFunc1FromModule2",
                                           "default"};
    ASSERT_EQ(expectedNames.size(), gotImportNames.size());
    for (auto &expectedName : expectedNames) {
        [[maybe_unused]] auto found = gotImportNames.find(expectedName);
        ASSERT_NE(found, gotImportNames.end());
        if (expectedName == "default") {
            ASSERT_EQ(found->second, 2U);
        } else {
            ASSERT_EQ(found->second, 1);
        }
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::importDescriptorGetAlias, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicImportDescriptorGetAlias)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::set<std::string> gotImportAliases;
    g_implI->moduleEnumerateImports(m, &gotImportAliases, ModuleImportAliasCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expectedNames = {"NS", "regularImportFunc1FromModule1",
                                           "regularDefaultImportFunc2FromModule2",
                                           "regularDefaultImportFunc1FromModule3", "MF1M2"};
    ASSERT_EQ(expectedNames.size(), gotImportAliases.size());
    for (auto &expectedName : expectedNames) {
        ASSERT_NE(gotImportAliases.find(expectedName), gotImportAliases.end());
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::importDescriptorGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicImportDescriptorGetFile)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::set<AbckitCoreImportDescriptor *> gotImports;
    g_implI->moduleEnumerateImports(m, &gotImports, helpers::ModuleImportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(gotImports.size(), 5U);
    for (auto &gotImport : gotImports) {
        auto file = g_implI->importDescriptorGetFile(gotImport);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(file, g_implI->moduleGetFile(m));
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::importDescriptorGetImportingModule, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicImportDescriptorGetImportingModule)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::set<AbckitCoreImportDescriptor *> gotImports;
    g_implI->moduleEnumerateImports(m, &gotImports, helpers::ModuleImportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(gotImports.size(), 5U);
    for (auto &gotImport : gotImports) {
        auto impM = g_implI->importDescriptorGetImportingModule(gotImport);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(impM, m);
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::importDescriptorGetImportedModule, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicImportDescriptorGetImportedModule)
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

    std::set<AbckitCoreImportDescriptor *> gotImports;
    g_implI->moduleEnumerateImports(gotModulesMap.find("modules_dynamic")->second, &gotImports,
                                    helpers::ModuleImportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    std::unordered_map<std::string, AbckitCoreModule *> expected = {
        {"NS", gotModulesMap.find("modules/module1")->second},
        {"regularImportFunc1FromModule1", gotModulesMap.find("modules/module1")->second},
        {"regularDefaultImportFunc2FromModule2", gotModulesMap.find("modules/module2")->second},
        {"regularDefaultImportFunc1FromModule3", gotModulesMap.find("modules/module3")->second},
        {"MF1M2", gotModulesMap.find("modules/module2")->second}};

    ASSERT_EQ(gotImports.size(), 5U);
    for (auto &gotImport : gotImports) {
        auto impM = g_implI->importDescriptorGetImportedModule(gotImport);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto impName = g_implI->importDescriptorGetAlias(gotImport);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto strName = helpers::AbckitStringToString(impName);
        ASSERT_NE(expected.find(strName.data()), expected.end());
        ASSERT_EQ(impM, expected.find(strName.data())->second);
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// ========================================
// Exports
// ========================================

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateExports, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicModuleEnumerateExports)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::set<AbckitCoreExportDescriptor *> gotExports;
    g_implI->moduleEnumerateExports(m, &gotExports, helpers::ModuleExportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(gotExports.size(), 10U);

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::exportDescriptorGetName, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicExportDescriptorGetName)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::unordered_map<std::string, size_t> gotExportNames;
    g_implI->moduleEnumerateExports(m, &gotExportNames, ModuleExportNamesCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expectedNames = {"*",
                                           "default",
                                           "LocalExportClass",
                                           "LocalExportFunc",
                                           "LocalExportLet",
                                           "LocalExportVar",
                                           "indirectExportFunc1FromModule1",
                                           "indirectExportFunc2FromModule1"};
    ASSERT_EQ(expectedNames.size(), gotExportNames.size());
    for (auto &expectedName : expectedNames) {
        [[maybe_unused]] auto found = gotExportNames.find(expectedName);
        ASSERT_NE(found, gotExportNames.end());
        if (expectedName == "*") {
            ASSERT_EQ(found->second, 3U);
        } else {
            ASSERT_EQ(found->second, 1);
        }
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::exportDescriptorGetAlias, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicExportDescriptorGetAlias)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::set<std::string> gotExportAliases;
    g_implI->moduleEnumerateExports(m, &gotExportAliases, ModuleExportAliasCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expectedNames = {"",
                                           "indirectExportFunc1FromModule1",
                                           "LocalDefaultExportClass",
                                           "LocalExportClass",
                                           "LocalExportFunc",
                                           "LocalExportLet",
                                           "LocalExportVar",
                                           "StarExport",
                                           "StarExport2",
                                           "MF2M1"};
    ASSERT_EQ(expectedNames.size(), gotExportAliases.size());
    for (auto &expectedName : expectedNames) {
        ASSERT_NE(gotExportAliases.find(expectedName), gotExportAliases.end());
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::exportDescriptorGetFile, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicExportDescriptorGetFile)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::set<AbckitCoreExportDescriptor *> gotExports;
    g_implI->moduleEnumerateExports(m, &gotExports, helpers::ModuleExportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(gotExports.size(), 10U);
    for (auto &gotExport : gotExports) {
        auto file = g_implI->exportDescriptorGetFile(gotExport);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(file, g_implI->moduleGetFile(m));
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::exportDescriptorGetExportingModule, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicExportDescriptorGetExportingModule)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    std::set<AbckitCoreExportDescriptor *> gotExports;
    g_implI->moduleEnumerateExports(m, &gotExports, helpers::ModuleExportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(gotExports.size(), 10U);
    for (auto &gotExport : gotExports) {
        auto expM = g_implI->exportDescriptorGetExportingModule(gotExport);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(expM, m);
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::exportDescriptorGetExportedModule, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicExportDescriptorGetExportedModule)
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

    std::set<AbckitCoreExportDescriptor *> gotExports;
    g_implI->moduleEnumerateExports(gotModulesMap.find("modules_dynamic")->second, &gotExports,
                                    helpers::ModuleExportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    std::unordered_map<std::string, AbckitCoreModule *> expected = {
        {"", gotModulesMap.find("modules/module2")->second},
        {"indirectExportFunc1FromModule1", gotModulesMap.find("modules/module1")->second},
        {"MF2M1", gotModulesMap.find("modules/module1")->second},
        {"LocalExportClass", gotModulesMap.find("modules_dynamic")->second},
        {"LocalExportFunc", gotModulesMap.find("modules_dynamic")->second},
        {"LocalExportLet", gotModulesMap.find("modules_dynamic")->second},
        {"LocalExportVar", gotModulesMap.find("modules_dynamic")->second},
        {"StarExport", gotModulesMap.find("modules/module1")->second},
        {"StarExport2", gotModulesMap.find("modules/module2")->second},
        {"LocalDefaultExportClass", gotModulesMap.find("modules_dynamic")->second}};

    ASSERT_EQ(gotExports.size(), 10U);
    for (auto &gotExport : gotExports) {
        auto impM = g_implI->exportDescriptorGetExportedModule(gotExport);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto impName = g_implI->exportDescriptorGetAlias(gotExport);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto strName = helpers::AbckitStringToString(impName);
        ASSERT_NE(expected.find(strName.data()), expected.end());
        ASSERT_EQ(impM, expected.find(strName.data())->second);
    }

    g_impl->writeAbc(file, MODIFIED, strlen(MODIFIED));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::fileEnumerateExternalModules, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicFileEnumerateExternalModules)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic_external.abc", &file);

    std::set<AbckitCoreModule *> gotModules;

    g_implI->fileEnumerateExternalModules(file, &gotModules, ModuleCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expected = {"external_module"};
    ASSERT_EQ(expected.size(), gotModules.size());
    for (auto *module : gotModules) {
        auto name = helpers::AbckitStringToString(g_implI->moduleGetName(module));
        ASSERT_NE(expected.find(name.data()), expected.end());
    }

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_dynamic_external_modified.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleGetTarget, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, DynamicModuleGetTarget)
{
    AbckitFile *jsFile = nullptr;
    AbckitFile *tsFile = nullptr;

    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/targets/JS_target.abc", &jsFile);
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/targets/TS_target.abc", &tsFile);

    EXPECT_EQ(GetFileTarget(jsFile, "JS_target"), ABCKIT_TARGET_JS);
    EXPECT_EQ(GetFileTarget(tsFile, "TS_target"), ABCKIT_TARGET_ARK_TS_V1);

    g_impl->closeFile(jsFile);
    g_impl->closeFile(tsFile);
}

}  // namespace libabckit::test
