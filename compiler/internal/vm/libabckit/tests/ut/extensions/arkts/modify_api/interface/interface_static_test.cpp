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

#include <cstring>
#include <gtest/gtest.h>

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"
#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiInterfaceTests : public ::testing::Test {};

// Test: test-kind=api, api=ArktsModifyApiImpl::InterfaceAddSuperInterface, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiInterfaceTests, InterfaceAddSuperInterface)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/addsuperinterface.abc", &file);
    auto output = helpers::ExecuteStaticAbc(
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/addsuperinterface.abc", "addsuperinterface", "main");
    EXPECT_TRUE(helpers::Match(output, "600\n"));

    helpers::ModuleByNameContext mdFinder = {nullptr, "addsuperinterface"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::InterfaceByNameContext ifaceFinder = {nullptr, "ExtendedStyle"};
    g_implI->moduleEnumerateInterfaces(module, &ifaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ifaceFinder.iface, nullptr);
    auto *iface = ifaceFinder.iface;

    helpers::InterfaceByNameContext superifaceFinder = {nullptr, "Style"};
    g_implI->moduleEnumerateInterfaces(module, &superifaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(superifaceFinder.iface, nullptr);
    auto *superIface = superifaceFinder.iface;

    bool ret = g_implArkM->interfaceAddSuperInterface(iface->GetArkTSImpl(), superIface->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/interface_static_modify_AddSuperInterface.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "addsuperinterface", "main");
    EXPECT_TRUE(helpers::Match(output, "600\n"));
    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    mdFinder = {nullptr, "addsuperinterface"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_NE(mdFinder.module, nullptr);
    module = mdFinder.module;
    ifaceFinder = {nullptr, "ExtendedStyle"};
    g_implI->moduleEnumerateInterfaces(module, &ifaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_NE(ifaceFinder.iface, nullptr);
    iface = ifaceFinder.iface;
    ASSERT_EQ(iface->superInterfaces.size(), 1);
    auto name = g_implI->abckitStringToString(g_implI->interfaceGetName(iface->superInterfaces[0]));
    ASSERT_EQ(std::string(name), "Style");
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::InterfaceRemoveSuperInterface, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiInterfaceTests, InterfaceRemoveSuperInterface)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/removesuperinterface.abc", &file);
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/removesuperinterface.abc",
                                  "removesuperinterface", "main");
    EXPECT_TRUE(helpers::Match(output, "600\nred\n"));

    helpers::ModuleByNameContext mdFinder = {nullptr, "removesuperinterface"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;
    helpers::InterfaceByNameContext ifaceFinder = {nullptr, "ExtendedStyle"};
    g_implI->moduleEnumerateInterfaces(module, &ifaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ifaceFinder.iface, nullptr);
    auto *iface = ifaceFinder.iface;

    helpers::InterfaceByNameContext superifaceFinder = {nullptr, "Style"};
    g_implI->moduleEnumerateInterfaces(module, &superifaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(superifaceFinder.iface, nullptr);
    auto *superIface = superifaceFinder.iface;

    bool ret = g_implArkM->interfaceRemoveSuperInterface(iface->GetArkTSImpl(), superIface->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/interface_static_modify_RemoveSuperInterface.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    mdFinder = {nullptr, "removesuperinterface"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_NE(mdFinder.module, nullptr);
    module = mdFinder.module;
    ifaceFinder = {nullptr, "ExtendedStyle"};
    g_implI->moduleEnumerateInterfaces(module, &ifaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_NE(ifaceFinder.iface, nullptr);
    iface = ifaceFinder.iface;
    ASSERT_EQ(iface->superInterfaces.size(), 0);
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::InterfaceRemoveMethod, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiInterfaceTests, InterfaceRemoveMethod)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/interfaceset_static.abc", &file);

    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/interfaceset_static.abc",
                                  "interfaceset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "red\n"));

    helpers::ModuleByNameContext mdFinder = {nullptr, "interfaceset_static"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::InterfaceByNameContext ifaceFinder = {nullptr, "ExtendedStyle"};
    g_implI->moduleEnumerateInterfaces(module, &ifaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ifaceFinder.iface, nullptr);
    auto *iface = ifaceFinder.iface;

    helpers::MethodByNameContext methodCtxFinder = {nullptr, "showColor"};
    g_implI->interfaceEnumerateMethods(iface, &methodCtxFinder, helpers::MethodByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(methodCtxFinder.method, nullptr);
    auto *method = methodCtxFinder.method;

    bool ret = g_implArkM->interfaceRemoveMethod(iface->GetArkTSImpl(), method->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/interface_static_modify_RemoveMethod.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "interfaceset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "red\n"));
    constexpr auto OUTPUT_PATH_1 =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/interface_static_modify_RemoveMethod_1.abc";

    helpers::TransformMethod(OUTPUT_PATH, OUTPUT_PATH_1, "main",
                             [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
                                 auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 g_implG->iRemove(call);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });
    output = helpers::ExecuteStaticAbc(OUTPUT_PATH_1, "interfaceset_static", "main");
    EXPECT_TRUE(helpers::Match(output, ""));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::InterfaceSetOwningModule, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiInterfaceTests, InterfaceSetOwningModule)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/interfaceset_static.abc", &file);
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/interfaceset_static.abc",
                                  "interfaceset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "red\n"));

    helpers::ModuleByNameContext mdFinder = {nullptr, "interfaceset_static"};
    g_implI->fileEnumerateModules(file, &mdFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(mdFinder.module, nullptr);
    auto *module = mdFinder.module;

    helpers::InterfaceByNameContext ifaceFinder = {nullptr, "ExtendedStyle"};
    g_implI->moduleEnumerateInterfaces(module, &ifaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ifaceFinder.iface, nullptr);
    auto *iface = ifaceFinder.iface;

    // create a new module
    auto m = std::make_unique<AbckitCoreModule>();
    m->file = file;
    m->target = ABCKIT_TARGET_ARK_TS_V2;
    const char *moduleName = "LocalModule";
    m->moduleName = CreateStringStatic(file, moduleName, strlen(moduleName));
    m->impl = std::make_unique<AbckitArktsModule>();
    m->GetArkTSImpl()->core = m.get();

    bool ret = g_implArkM->interfaceSetOwningModule(nullptr, m.get()->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    ASSERT_FALSE(ret);
    ret = g_implArkM->interfaceSetOwningModule(iface->GetArkTSImpl(), nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    ASSERT_FALSE(ret);
    ret = g_implArkM->interfaceSetOwningModule(iface->GetArkTSImpl(), m.get()->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);
    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interface/interface_static_modify_SetOwningModule.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "interfaceset_static", "main");
    EXPECT_TRUE(helpers::Match(output, "red\n"));
}
}  // namespace libabckit::test
