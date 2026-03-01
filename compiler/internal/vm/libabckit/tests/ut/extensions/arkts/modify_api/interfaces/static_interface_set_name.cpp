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

#include <cstring>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <vector>

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/include/libabckit/c/abckit.h"
#include "libabckit/include/libabckit/c/declarations.h"
#include "libabckit/include/libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/include/libabckit/c/metadata_core.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_staticG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitModifyApiIfaceNameTests : public ::testing::Test {};

TEST_F(LibAbcKitModifyApiIfaceNameTests, InterfaceSetNameTest)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/static_interface_set_name.abc";
    std::string output = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/static_interface_set_name_out.abc";

    auto outputRst = helpers::ExecuteStaticAbc(input, "static_interface_set_name", "main");
    EXPECT_TRUE(helpers::Match(outputRst, "b\n"));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);
    helpers::ModuleByNameContext ctxFinder = {nullptr, "static_interface_set_name"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "User"};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    auto arktsIntrface = g_implArkI->coreInterfaceToArktsInterface(ifaceCtxFinder.iface);
    g_implArkM->interfaceSetName(arktsIntrface, "abc");

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(output.c_str(), &file);
    ctxFinder = {nullptr, "static_interface_set_name"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    ifaceCtxFinder = {nullptr, "abc"};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);
    ASSERT_NE(ifaceCtxFinder.iface, nullptr);
    g_impl->closeFile(file);
    outputRst = helpers::ExecuteStaticAbc(output, "static_interface_set_name", "main");
    EXPECT_TRUE(helpers::Match(outputRst, "b\n"));
}
}  // namespace libabckit::test