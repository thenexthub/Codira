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

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/include/libabckit/c/extensions/arkts/metadata_arkts.h"
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

class LibAbcKitModifyApiIfaceTests : public ::testing::Test {};

// Test: test-kind=api, api=ArktsModifyApiImpl::InterfaceRemoveField, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiIfaceTests, InterfaceRemoveFieldTest0)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface_remove_field_static.abc";
    std::string output =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface_remove_field_static_out.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);

    helpers::ModuleByNameContext moduleCtx = {nullptr, "interface_remove_field_static"};
    g_implI->fileEnumerateModules(file, &moduleCtx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext interfaceCtx = {nullptr, "MyInterface"};
    g_implI->moduleEnumerateInterfaces(moduleCtx.module, &interfaceCtx, helpers::InterfaceByNameFinder);
    helpers::InterfaceByNameContext interfaceCtx2 = {nullptr, "MyInterface2"};
    g_implI->moduleEnumerateInterfaces(moduleCtx.module, &interfaceCtx2, helpers::InterfaceByNameFinder);

    helpers::FieldByNameContext fieldCtx = {nullptr, "%%property-key"};
    g_implI->interfaceEnumerateFields(interfaceCtx.iface, &fieldCtx, helpers::FieldByNameFinder);
    ASSERT_TRUE(fieldCtx.field != nullptr);

    bool ret = g_implArkM->interfaceRemoveField(interfaceCtx2.iface->GetArkTSImpl(), fieldCtx.field->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_INTERNAL_ERROR);
    ASSERT_FALSE(ret);

    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    helpers::AssertOpenAbc(output.c_str(), &file);
    helpers::ModuleByNameContext moduleCtx2 = {nullptr, "interface_remove_field_static"};
    g_implI->fileEnumerateModules(file, &moduleCtx2, helpers::ModuleByNameFinder);
    helpers::InterfaceByNameContext interfaceCtx3 = {nullptr, "MyInterface"};
    g_implI->moduleEnumerateInterfaces(moduleCtx2.module, &interfaceCtx3, helpers::InterfaceByNameFinder);
    helpers::FieldByNameContext checkFieldCtx = {nullptr, "%%property-key"};
    g_implI->interfaceEnumerateFields(interfaceCtx3.iface, &checkFieldCtx, helpers::FieldByNameFinder);
    ASSERT_TRUE(checkFieldCtx.field != nullptr);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformInterfaceRemoveField(AbckitGraph *graph)
{
    auto *staticCall = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    if (staticCall != nullptr) {
        g_implG->iRemove(staticCall);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    }

    auto *virtualCall = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL);
    if (virtualCall != nullptr) {
        g_implG->iRemove(virtualCall);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    }
}

// Test: test-kind=api, api=ArktsModifyApiImpl::InterfaceRemoveField, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiIfaceTests, InterfaceRemoveFieldTest2)
{
    auto inputPath = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface_remove_field_static.abc";
    auto outputPath =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface_remove_field_static_output.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(inputPath, &file);
    helpers::TransformMethod(file, "main",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 TransformInterfaceRemoveField(graph);
                             });

    helpers::ModuleByNameContext moduleCtx = {nullptr, "interface_remove_field_static"};
    g_implI->fileEnumerateModules(file, &moduleCtx, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext interfaceCtx = {nullptr, "MyInterface"};
    g_implI->moduleEnumerateInterfaces(moduleCtx.module, &interfaceCtx, helpers::InterfaceByNameFinder);

    helpers::FieldByNameContext fieldCtx = {nullptr, "%%property-key"};
    g_implI->interfaceEnumerateFields(interfaceCtx.iface, &fieldCtx, helpers::FieldByNameFinder);
    ASSERT_TRUE(fieldCtx.field != nullptr);
    bool ret = g_implArkM->interfaceRemoveField(interfaceCtx.iface->GetArkTSImpl(), fieldCtx.field->GetArkTSImpl());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);

    g_impl->writeAbc(file, outputPath, strlen(outputPath));
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(outputPath, &file);
    helpers::ModuleByNameContext moduleCtx2 = {nullptr, "interface_remove_field_static"};
    g_implI->fileEnumerateModules(file, &moduleCtx2, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext interfaceCtx2 = {nullptr, "MyInterface"};
    g_implI->moduleEnumerateInterfaces(moduleCtx2.module, &interfaceCtx2, helpers::InterfaceByNameFinder);
    helpers::FieldByNameContext checkFieldCtx = {nullptr, "%%property-key"};
    g_implI->interfaceEnumerateFields(interfaceCtx2.iface, &checkFieldCtx, helpers::FieldByNameFinder);
    ASSERT_TRUE(checkFieldCtx.field == nullptr);
    g_impl->closeFile(file);

    auto output = helpers::ExecuteStaticAbc(inputPath, "interface_remove_field_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Key: test\nAccessing key: test\nValue: 42\n"));
    output = helpers::ExecuteStaticAbc(outputPath, "interface_remove_field_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Value: 42\n"));
}
}  // namespace libabckit::test