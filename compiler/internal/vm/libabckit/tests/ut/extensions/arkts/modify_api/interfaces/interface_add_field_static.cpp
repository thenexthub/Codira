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

class LibAbcKitModifyApiIfaceTests : public ::testing::Test {};

struct CaptureData {
    AbckitGraph *graph = nullptr;
    AbckitCoreFunction *mainFunc = nullptr;
    AbckitCoreFunction *targetFunc = nullptr;
};

void ClassAddInterfaceTransformMainIr(AbckitCoreFunction *mainFunction, AbckitCoreFunction *targetFunc)
{
    AbckitGraph *graph = g_implI->createGraphFromFunction(mainFunction);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    CaptureData capData;
    capData.graph = graph;
    capData.targetFunc = targetFunc;
    g_implG->gVisitBlocksRpo(graph, &capData, [](AbckitBasicBlock *bb, void *data) -> bool {
        CaptureData *capData = reinterpret_cast<CaptureData *>(data);
        for (auto inst = g_implG->bbGetFirstInst(bb); inst != nullptr; inst = g_implG->iGetNext(inst)) {
            if (g_staticG->iGetOpcode(inst) == ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID) {
                auto preInst = g_implG->iGetPrev(inst);
                auto objRef = g_implG->iGetInput(preInst, 0);
                auto newInst = g_staticG->iCreateCallVirtual(capData->graph, objRef, capData->targetFunc, 0);
                g_implG->iInsertBefore(newInst, inst);
            }
        }
        return true;
    });

    g_implM->functionSetGraph(mainFunction, graph);
    g_impl->destroyGraph(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

TEST_F(LibAbcKitModifyApiIfaceTests, InterfaceAddFieldTest0)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface_add_field_static.abc";
    std::string output = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface_add_field_static_out.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);
    helpers::ModuleByNameContext ctxFinder = {nullptr, "interface_add_field_static"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "MyInterface"};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    struct AbckitArktsInterfaceFieldCreateParams params;
    params.name = "newInterfaceField";
    params.type = g_implM->createType(file, AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    params.isReadOnly = false;
    params.fieldVisibility = AbckitArktsFieldVisibility::PUBLIC;

    auto arktsInterface = g_implArkI->coreInterfaceToArktsInterface(ifaceCtxFinder.iface);
    g_implArkM->interfaceAddField(arktsInterface, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // open output file
    helpers::AssertOpenAbc(output.c_str(), &file);
    ctxFinder = {nullptr, "interface_add_field_static"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    ifaceCtxFinder = {nullptr, "MyInterface"};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    std::vector<std::string> FieldNames;
    g_implI->interfaceEnumerateFields(
        ifaceCtxFinder.iface, &FieldNames, [](AbckitCoreInterfaceField *field, void *data) {
            auto ctx = static_cast<std::vector<std::string> *>(data);
            auto filedName = g_implI->abckitStringToString(g_implI->interfaceFieldGetName(field));
            (*ctx).emplace_back(filedName);
            return true;
        });
    std::vector<std::string> testFileds = {"%%property-newInterfaceField", "%%property-key"};
    ASSERT_EQ(FieldNames, testFileds);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(input, "interface_add_field_static", "main"),
              helpers::ExecuteStaticAbc(output, "interface_add_field_static", "main"));
}

TEST_F(LibAbcKitModifyApiIfaceTests, InterfaceAddMethodTest0)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface_add_method_static.abc";
    std::string output = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/interfaces/interface_add_field_static_out.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "interface_add_method_static"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder = {nullptr, "Test"};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder, helpers::InterfaceByNameFinder);

    helpers::InterfaceByNameContext ifaceCtxFinder1 = {nullptr, "MyInterface"};
    g_implI->moduleEnumerateInterfaces(ctxFinder.module, &ifaceCtxFinder1, helpers::InterfaceByNameFinder);

    helpers::MethodByNameContext methodCtxFinder = {nullptr, "sayHello:interface_add_method_static.Test;f64;void;",
                                                    true};
    g_implI->interfaceEnumerateMethods(ifaceCtxFinder.iface, &methodCtxFinder, helpers::MethodByNameFinder);

    auto arkInterface = g_implArkI->coreInterfaceToArktsInterface(ifaceCtxFinder1.iface);
    auto arkFunc = g_implArkI->coreFunctionToArktsFunction(methodCtxFinder.method);
    g_implArkM->interfaceAddMethod(arkInterface, arkFunc);

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test