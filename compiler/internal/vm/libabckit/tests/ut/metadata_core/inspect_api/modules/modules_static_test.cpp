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
#include <vector>
#include <string>

#include "helpers/helpers.h"
#include "libabckit/cpp/abckit_cpp.h"
#include "libabckit/src/metadata_inspect_impl.h"

#include <set>

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitInspectApiModulesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateFields, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleGetFieldsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc");

    std::vector<std::string> fieldNames;

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &field : module.GetFields()) {
            fieldNames.emplace_back(field.GetName());
        }
    }

    ASSERT_EQ(fieldNames[0], "moduleField");
    ASSERT_EQ(fieldNames.size(), 1);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateInterfaces, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleGetInterfacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc");

    std::vector<std::string> interfacesNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &iface : module.GetInterfaces()) {
            interfacesNames.emplace_back(iface.GetName());
        }
    }

    ASSERT_EQ(interfacesNames[0], "InterfaceA");
    ASSERT_EQ(interfacesNames.size(), 1);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateEnums, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleGetEnumsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc");

    std::vector<std::string> enumNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            enumNames.emplace_back(enm.GetName());
        }
    }

    ASSERT_EQ(enumNames[0], "EnumA");
    ASSERT_EQ(enumNames.size(), 1);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateNamespaces, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleGetNamespacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc");

    std::set<std::string> gotNamespaceNames;
    std::set<std::string> expectNamespacesNames = {"Ns1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            gotNamespaceNames.emplace(ns.GetName());
        }
    }

    ASSERT_EQ(gotNamespaceNames, expectNamespacesNames);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateInterfaces, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleEnumerateInterfaces)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    auto callBack = [](AbckitCoreInterface *method, void *data) -> bool { return false; };
    ctx.module->target = ABCKIT_TARGET_ARK_TS_V1;
    auto result = g_implI->moduleEnumerateInterfaces(ctx.module, nullptr, callBack);
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    ctx.module->target = ABCKIT_TARGET_JS;
    result = g_implI->moduleEnumerateInterfaces(ctx.module, nullptr, callBack);
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    ctx.module->target = ABCKIT_TARGET_UNKNOWN;
    result = g_implI->moduleEnumerateInterfaces(ctx.module, nullptr, callBack);
    EXPECT_TRUE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateEnums, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleEnumerateEnums)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    auto callBack = [](AbckitCoreEnum *method, void *data) -> bool { return false; };
    ctx.module->target = ABCKIT_TARGET_ARK_TS_V1;
    auto result = g_implI->moduleEnumerateEnums(ctx.module, nullptr, callBack);
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    ctx.module->target = ABCKIT_TARGET_JS;
    result = g_implI->moduleEnumerateEnums(ctx.module, nullptr, callBack);
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    ctx.module->target = ABCKIT_TARGET_UNKNOWN;
    result = g_implI->moduleEnumerateEnums(ctx.module, nullptr, callBack);
    EXPECT_TRUE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateFields, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleEnumerateFields)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    auto callBack = [](AbckitCoreModuleField *method, void *data) -> bool { return false; };
    ctx.module->target = ABCKIT_TARGET_ARK_TS_V1;
    auto result = g_implI->moduleEnumerateFields(ctx.module, nullptr, callBack);
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    ctx.module->target = ABCKIT_TARGET_JS;
    result = g_implI->moduleEnumerateFields(ctx.module, nullptr, callBack);
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    ctx.module->target = ABCKIT_TARGET_UNKNOWN;
    result = g_implI->moduleEnumerateFields(ctx.module, nullptr, callBack);
    EXPECT_TRUE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateAnonymousFunctions, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleEnumerateAnonymousFunctions_1)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    auto callBack = [](AbckitCoreFunction *method, void *data) -> bool { return false; };
    ctx.module->target = ABCKIT_TARGET_UNKNOWN;
    auto result = g_implI->moduleEnumerateAnonymousFunctions(ctx.module, nullptr, callBack);
    EXPECT_TRUE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateAnonymousFunctions, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleEnumerateAnonymousFunctions_2)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    auto callBack = [](AbckitCoreFunction *method, void *data) -> bool { return false; };
    auto result = g_implI->moduleEnumerateAnonymousFunctions(ctx.module, nullptr, callBack);
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateAnnotationInterfaces, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleEnumerateAnnotationInterfaces)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    auto callBack = [](AbckitCoreAnnotationInterface *method, void *data) -> bool { return false; };
    ctx.module->target = ABCKIT_TARGET_JS;
    auto result = g_implI->moduleEnumerateAnnotationInterfaces(ctx.module, nullptr, callBack);
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceGetName, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, NamespaceGetName)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    std::vector<AbckitString *> nsNames;
    for (auto it = ctx.module->nt.begin(); it != ctx.module->nt.end(); it++) {
        nsNames.push_back(g_implI->namespaceGetName(it->second.get()));
    }

    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
    ASSERT_TRUE(nsNames.size() == 1);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateNamespaces, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, NamespaceEnumerateNamespaces)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    bool result = false;
    for (auto it = ctx.module->nt.begin(); it != ctx.module->nt.end(); it++) {
        result = g_implI->namespaceEnumerateNamespaces(
            it->second.get(), nullptr, [](AbckitCoreNamespace *method, void *data) -> bool { return false; });
    }

    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateClasses, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, NamespaceEnumerateClasses)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    bool result = false;
    for (auto it = ctx.module->nt.begin(); it != ctx.module->nt.end(); it++) {
        result = g_implI->namespaceEnumerateClasses(it->second.get(), nullptr,
                                                    [](AbckitCoreClass *, void *) -> bool { return false; });
    }

    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateInterfaces, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, NamespaceEnumerateInterfaces)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    bool result = false;
    for (auto it = ctx.module->nt.begin(); it != ctx.module->nt.end(); it++) {
        result = g_implI->namespaceEnumerateInterfaces(it->second.get(), nullptr,
                                                       [](AbckitCoreInterface *, void *) -> bool { return false; });
    }

    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateEnums, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, NamespaceEnumerateEnums)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    bool result = false;
    for (auto it = ctx.module->nt.begin(); it != ctx.module->nt.end(); it++) {
        result = g_implI->namespaceEnumerateEnums(it->second.get(), nullptr,
                                                  [](AbckitCoreEnum *, void *) -> bool { return false; });
    }

    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateFields, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, NamespaceEnumerateFields)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    bool result = false;
    for (auto it = ctx.module->nt.begin(); it != ctx.module->nt.end(); it++) {
        result = g_implI->namespaceEnumerateFields(it->second.get(), nullptr,
                                                   [](AbckitCoreNamespaceField *, void *) -> bool { return false; });
    }

    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::importDescriptorGetName, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ImportDescriptorGetName)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    AbckitCoreImportDescriptor imDes;
    imDes.importingModule = ctx.module;
    auto adcStr = g_implI->importDescriptorGetName(&imDes);

    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    EXPECT_EQ(adcStr, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::importDescriptorGetAlias, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ImportDescriptorGetAlias)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    AbckitCoreImportDescriptor imDes;
    imDes.importingModule = ctx.module;
    auto adcStr = g_implI->importDescriptorGetAlias(&imDes);

    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    EXPECT_EQ(adcStr, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::exportDescriptorGetName, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ExportDescriptorGetName)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    AbckitCoreExportDescriptor expDes;
    expDes.exportingModule = ctx.module;
    auto adcStr = g_implI->exportDescriptorGetName(&expDes);

    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    EXPECT_EQ(adcStr, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::exportDescriptorGetAlias, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ExportDescriptorGetAlias)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    AbckitCoreExportDescriptor expDes;
    expDes.exportingModule = ctx.module;
    auto adcStr = g_implI->exportDescriptorGetAlias(&expDes);

    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    EXPECT_EQ(adcStr, nullptr);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateAnnotations, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ClassEnumerateAnnotations)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    bool result = false;
    for (auto it = ctx.module->ct.begin(); it != ctx.module->ct.end(); it++) {
        result = g_implI->classEnumerateAnnotations(
            it->second.get(), nullptr, [](AbckitCoreAnnotation *method, void *data) -> bool { return false; });
    }
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateSubClasses, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ClassEnumerateSubClasses)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    bool result = false;
    for (auto it = ctx.module->ct.begin(); it != ctx.module->ct.end(); it++) {
        result = g_implI->classEnumerateSubClasses(it->second.get(), nullptr,
                                                   [](AbckitCoreClass *, void *) -> bool { return false; });
    }
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateInterfaces, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ClassEnumerateInterfaces)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    bool result = false;
    for (auto it = ctx.module->ct.begin(); it != ctx.module->ct.end(); it++) {
        result = g_implI->classEnumerateInterfaces(it->second.get(), nullptr,
                                                   [](AbckitCoreInterface *, void *) -> bool { return false; });
    }
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateFields, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ClassEnumerateFields)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    ctx.module->target = ABCKIT_TARGET_JS;
    bool result = false;
    for (auto it = ctx.module->ct.begin(); it != ctx.module->ct.end(); it++) {
        result = g_implI->classEnumerateFields(it->second.get(), nullptr,
                                               [](AbckitCoreClassField *, void *) -> bool { return false; });
    }
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceGetModule, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, NamespaceGetModuleWithNullModule)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    EXPECT_EQ(g_implI->namespaceGetModule(nullptr), nullptr);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateInterfaces, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ClassEnumerateInterfacesWithNullCallback)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    bool result = false;
    for (auto it = ctx.module->ct.begin(); it != ctx.module->ct.end(); it++) {
        result = g_implI->classEnumerateInterfaces(it->second.get(), nullptr, nullptr);
    }
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateInterfaces, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ClassEnumerateInterfacesWithNullKclass)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    auto result = g_implI->classEnumerateInterfaces(nullptr, nullptr, nullptr);
    EXPECT_FALSE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);

    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateImports, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleEnumerateImports)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    bool result = g_implI->moduleEnumerateImports(
        ctx.module, nullptr, [](AbckitCoreImportDescriptor *method, void *data) -> bool { return false; });
    EXPECT_TRUE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateExports, abc-kind=ArkTS2, category=negative,
// extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleEnumerateExports)
{
    auto input = ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static_test.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input, &file);

    helpers::ModuleByNameContext ctx = {nullptr, "modules_static_test"};
    g_implI->fileEnumerateModules(file, &ctx, helpers::ModuleByNameFinder);
    ASSERT_NE(ctx.module, nullptr);

    bool result = g_implI->moduleEnumerateExports(
        ctx.module, nullptr, [](AbckitCoreExportDescriptor *method, void *data) -> bool { return false; });
    EXPECT_TRUE(result);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->closeFile(file);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace libabckit::test