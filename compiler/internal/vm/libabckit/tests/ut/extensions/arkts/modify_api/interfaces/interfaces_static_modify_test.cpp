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

#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/cpp/abckit_cpp.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "tests/helpers/helpers.h"
#include "tests/helpers/helpers_runtime.h"

namespace {
constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interfaces_static_modify.abc";
constexpr auto OUTPUT_PATH =
    ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interfaces_static_modify_modified.abc";
constexpr auto NEW_NAME = "New";
}  // namespace

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiInterfacesTest : public ::testing::Test {};

// Test: test-kind=api, api=ArktsModifyApiImpl::interfaceSetName, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiInterfacesTest, InterfaceSetNameStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "interfaces_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;

    helpers::InterfaceByNameContext interfaceFinder = {nullptr, "Interface1"};
    g_implI->moduleEnumerateInterfaces(module, &interfaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(interfaceFinder.iface, nullptr);
    auto arkI = g_implArkI->coreInterfaceToArktsInterface(interfaceFinder.iface);
    g_implArkM->interfaceSetName(arkI, NEW_NAME);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    helpers::ModuleByNameContext newFinder = {nullptr, "interfaces_static_modify"};
    g_implI->fileEnumerateModules(file, &newFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newFinder.module, nullptr);
    auto newModule = newFinder.module;

    helpers::InterfaceByNameContext newInterfaceFinder = {nullptr, NEW_NAME};
    g_implI->moduleEnumerateInterfaces(newModule, &newInterfaceFinder, helpers::InterfaceByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newInterfaceFinder.iface, nullptr);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "interfaces_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "interfaces_static_modify", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::createInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiInterfacesTest, CreatInterfaceTest0)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface.abc";
    std::string output = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface_out.abc";
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "interface"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    std::string ifName = "CreateInterface";
    auto interface = g_implArkM->createInterface(ctxFinder.module->GetArkTSImpl(), ifName.c_str());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    EXPECT_TRUE(g_implI->interfaceGetName(interface->core)->impl == ifName);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, ifName.c_str()};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);
    EXPECT_EQ(ifaceCtxFinder.iface, interface->core);

    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(output.c_str(), &file);

    ctxFinder = {nullptr, "interface"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    ifaceCtxFinder = {nullptr, ifName.c_str()};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);
    EXPECT_NE(ifaceCtxFinder.iface, nullptr);

    g_impl->closeFile(file);
}

}  // namespace libabckit::test
