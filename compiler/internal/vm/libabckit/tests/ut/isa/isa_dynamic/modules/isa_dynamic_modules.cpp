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

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "ir_impl.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/statuses.h"

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitIsaDynamicModulesTest : public ::testing::Test {};

// ========================================
// Helpers
// ========================================

static void IsetModuleTest(AbckitGraph *graph, AbckitCoreModule *module, AbckitIsaApiDynamicOpcode opcode,
                           bool negative = false, bool wrongCtx = false)
{
    auto moduleRequestIdx = 0x2;
    AbckitInst *instWithModule = helpers::FindFirstInst(graph, opcode, [moduleRequestIdx](AbckitInst *inst) {
        auto imm = g_implG->iGetImmediate(inst, 0);
        return imm == moduleRequestIdx;
    });

    g_dynG->iSetModule(instWithModule, module);
    if (negative) {
        if (wrongCtx) {
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_CTX);
            return;
        }
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void IsetImportDescriptorTest(AbckitGraph *graph, AbckitCoreModule *m, AbckitIsaApiDynamicOpcode opcode,
                                     bool negative = false, AbckitCoreImportDescriptor *importToRemove = nullptr)
{
    auto moduleRequestIdx = 0x2;
    AbckitInst *instWithID = helpers::FindFirstInst(graph, opcode, [moduleRequestIdx](AbckitInst *inst) {
        auto imm = g_implG->iGetImmediate(inst, 0);
        return imm == moduleRequestIdx;
    });
    ASSERT_NE(instWithID, nullptr);

    if (importToRemove == nullptr) {
        helpers::ImportByAliasContext importFinder = {nullptr, "MF1M2"};
        g_implI->moduleEnumerateImports(m, &importFinder, helpers::ImportByAliasFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(importFinder.id, nullptr);
        importToRemove = importFinder.id;
    }

    g_dynG->iSetImportDescriptor(instWithID, importToRemove);
    if (negative) {
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void IsetExportDescriptorTest(AbckitGraph *graph, AbckitCoreModule *m, AbckitIsaApiDynamicOpcode opcode,
                                     bool negative = false, AbckitCoreExportDescriptor *exportToRemove = nullptr)
{
    auto moduleRequestIdx = 0x3;
    AbckitInst *instWithID = helpers::FindFirstInst(graph, opcode, [moduleRequestIdx](AbckitInst *inst) {
        auto imm = g_implG->iGetImmediate(inst, 0);
        return imm == moduleRequestIdx;
    });
    ASSERT_NE(instWithID, nullptr);

    if (exportToRemove == nullptr) {
        helpers::ExportByAliasContext exportFinder = {nullptr, "LocalExportLet"};
        g_implI->moduleEnumerateExports(m, &exportFinder, helpers::ExportByAliasFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(exportFinder.ed, nullptr);
        exportToRemove = exportFinder.ed;
    }

    g_dynG->iSetExportDescriptor(instWithID, exportToRemove);
    if (negative) {
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void IgetModuleTest(AbckitGraph *graph, AbckitIsaApiDynamicOpcode opcode)
{
    auto moduleRequestIdx = 0x0;
    AbckitInst *instWithModule = helpers::FindFirstInst(graph, opcode, [moduleRequestIdx](AbckitInst *inst) {
        auto imm = g_implG->iGetImmediate(inst, 0);
        return imm == moduleRequestIdx;
    });

    ASSERT_NE(instWithModule, nullptr);
    auto moduleI = g_dynG->iGetModule(instWithModule);
    if (opcode != ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE) {
        ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        return;
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module1"};
    g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *module = ctxFinder.module;
    ASSERT_EQ(moduleI, module);
}

static void IgetImportDescriptorTest(AbckitGraph *graph, AbckitCoreModule *m, AbckitIsaApiDynamicOpcode opcode)
{
    auto moduleRequestIdx = 0x3;
    AbckitInst *instWithID = helpers::FindFirstInst(graph, opcode, [moduleRequestIdx](AbckitInst *inst) {
        auto imm = g_implG->iGetImmediate(inst, 0);
        return imm == moduleRequestIdx;
    });

    ASSERT_NE(instWithID, nullptr);
    auto importI = g_dynG->iGetImportDescriptor(instWithID);
    if (opcode != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::ImportByAliasContext importFinder = {nullptr, "regularImportFunc1FromModule1"};
    g_implI->moduleEnumerateImports(m, &importFinder, helpers::ImportByAliasFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(importFinder.id, nullptr);

    ASSERT_EQ(importFinder.id, importI);
}

static void IgetExportDescriptorTest(AbckitGraph *graph, AbckitCoreModule *m, AbckitIsaApiDynamicOpcode opcode,
                                     size_t moduleRequestIdx)
{
    AbckitInst *instWithID = helpers::FindFirstInst(graph, opcode, [&](AbckitInst *inst) {
        auto imm = g_implG->iGetImmediate(inst, 0);
        return imm == moduleRequestIdx;
    });

    ASSERT_NE(instWithID, nullptr);
    auto exportI = g_dynG->iGetExportDescriptor(instWithID);
    if (opcode != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR) {
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::ExportByAliasContext exportFinder = {nullptr, "LocalExportLet"};
    g_implI->moduleEnumerateExports(m, &exportFinder, helpers::ExportByAliasFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(exportFinder.ed, nullptr);

    ASSERT_EQ(exportFinder.ed, exportI);
}

static void TransformIrGetModuleNamespace(AbckitGraph *graph, AbckitString *funcName, bool isWide = false)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module2"};
    g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);
    auto getModuleNamespaceInst = isWide ? g_dynG->iCreateWideGetmodulenamespace(graph, ctxFinder.module)
                                         : g_dynG->iCreateGetmodulenamespace(graph, ctxFinder.module);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto ldObjByNameInst = g_dynG->iCreateLdobjbyname(graph, getModuleNamespaceInst, funcName);
    auto callInst = g_dynG->iCreateCallthis0(graph, ldObjByNameInst, getModuleNamespaceInst);
    g_implG->iInsertBefore(getModuleNamespaceInst, lastInst);
    g_implG->iInsertBefore(ldObjByNameInst, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

static void TransformIrLdExternalModuleVar(AbckitGraph *graph, AbckitString *funcName, bool isWide = false)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
    g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    helpers::ImportByAliasContext importFinder = {nullptr, "MF1M2"};
    g_implI->moduleEnumerateImports(ctxFinder.module, &importFinder, helpers::ImportByAliasFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(importFinder.id, nullptr);

    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);
    auto ldExternalModuleVarInst = isWide ? g_dynG->iCreateWideLdexternalmodulevar(graph, importFinder.id)
                                          : g_dynG->iCreateLdexternalmodulevar(graph, importFinder.id);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto throwUndef = g_dynG->iCreateThrowUndefinedifholewithname(graph, ldExternalModuleVarInst, funcName);
    auto callInst = g_dynG->iCreateCallarg0(graph, ldExternalModuleVarInst);
    g_implG->iInsertBefore(ldExternalModuleVarInst, lastInst);
    g_implG->iInsertBefore(throwUndef, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

static void TransformIrLdLocalModuleVar(AbckitGraph *graph, AbckitString *localVarName, AbckitString *funcName,
                                        bool isWide = false)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
    g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    helpers::ExportByAliasContext exportFinder = {nullptr, "LocalExportLet"};
    g_implI->moduleEnumerateExports(ctxFinder.module, &exportFinder, helpers::ExportByAliasFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(exportFinder.ed, nullptr);

    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);
    auto tryLdGlobalByNameInst = g_dynG->iCreateTryldglobalbyname(graph, funcName);
    auto ldLocalModuleVarInst = isWide ? g_dynG->iCreateWideLdlocalmodulevar(graph, exportFinder.ed)
                                       : g_dynG->iCreateLdlocalmodulevar(graph, exportFinder.ed);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto throwUndef = g_dynG->iCreateThrowUndefinedifholewithname(graph, ldLocalModuleVarInst, localVarName);
    auto callInst = g_dynG->iCreateCallarg1(graph, tryLdGlobalByNameInst, ldLocalModuleVarInst);
    g_implG->iInsertBefore(tryLdGlobalByNameInst, lastInst);
    g_implG->iInsertBefore(ldLocalModuleVarInst, lastInst);
    g_implG->iInsertBefore(throwUndef, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

static void TransformIrStModuleVar(AbckitGraph *graph, AbckitString *localVarName, AbckitString *funcName,
                                   bool isWide = false)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
    g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    helpers::ExportByAliasContext exportFinder = {nullptr, "LocalExportConst"};
    g_implI->moduleEnumerateExports(ctxFinder.module, &exportFinder, helpers::ExportByAliasFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(exportFinder.ed, nullptr);

    auto *startBB = g_implG->gGetStartBasicBlock(graph);
    auto *const1Inst = g_implG->bbGetLastInst(startBB);
    auto *mainBB = g_implG->bbGetSuccBlock(startBB, 0);
    auto *lastInst = g_implG->bbGetLastInst(mainBB);
    auto tryLdGlobalByNameInst = g_dynG->iCreateTryldglobalbyname(graph, funcName);
    auto stModuleVarInst = isWide ? g_dynG->iCreateWideStmodulevar(graph, const1Inst, exportFinder.ed)
                                  : g_dynG->iCreateStmodulevar(graph, const1Inst, exportFinder.ed);
    auto ldLocalModuleVarInst = isWide ? g_dynG->iCreateWideLdlocalmodulevar(graph, exportFinder.ed)
                                       : g_dynG->iCreateLdlocalmodulevar(graph, exportFinder.ed);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto throwUndef = g_dynG->iCreateThrowUndefinedifholewithname(graph, ldLocalModuleVarInst, localVarName);
    auto callInst = g_dynG->iCreateCallarg1(graph, tryLdGlobalByNameInst, ldLocalModuleVarInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(tryLdGlobalByNameInst, lastInst);
    g_implG->iInsertBefore(stModuleVarInst, lastInst);
    g_implG->iInsertBefore(ldLocalModuleVarInst, lastInst);
    g_implG->iInsertBefore(throwUndef, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

static std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> g_bb2Insts {
    {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
    {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, {12}},
    {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED, {}},
    {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, {10}},
    {14, ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE, {}},
    {15, ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE, {}},
    {16, ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE, {}},
    {17, ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE, {}},
    {18, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, {17}},
    {19, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDHOLE, {}},
    {20, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER, {19}},
    {21, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {20}},
    {22, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, {20}},
    {23, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, {3}},
    {24, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, {4}},
    {25, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDHOLE, {}},
    {26, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER, {25}},
    {27, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {26}},
    {28, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, {26}},
    {29, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, {}},
    {30, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {29}},
    {31, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {29}},
    {32, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, {}},
    {33, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {32}},
    {34, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {32}},
    {35, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, {}},
    {36, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {35}},
    {37, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {35}},
    {38, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, {}},
    {39, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {38}},
    {40, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {38}},
    {41, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {14}},
    {42, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {41, 14}},
    {43, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {14}},
    {44, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {43, 14}},
    {45, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
    {46, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR, {}},
    {47, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {46}},
    {48, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {45, 46}},
    {49, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
    {50, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR, {}},
    {51, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {50}},
    {52, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {49, 50}},
    {53, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {15}},
    {54, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {53, 15}},
    {55, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {16}},
    {56, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {55, 16}},
    {57, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}}};

static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForModules()
{
    return {{{},
             {1},
             {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
              {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
            {
                {0},
                {2},
                g_bb2Insts,
            },
            {{1}, {}, {}}};
}

// ========================================
// Modules Tests
// ========================================

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetModule, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIsetModule)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module1"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);

            IsetModuleTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetModule, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIsetModuleWrongInst)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module1"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);

            IsetModuleTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, true);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetModule, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIsetModuleWrongModule)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                             "isa_dynamic_modules.func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto newM = new AbckitCoreModule();
                                 newM->impl = std::make_unique<AbckitArktsModule>();
                                 IsetModuleTest(graph, newM, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, true, true);
                                 delete newM;
                             });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetImportDescriptor, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIsetImportDescriptor)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);
            IsetImportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular import func1 from module2\nnamespace import func2 from module1\nnamespace import func3 from "
        "module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetImportDescriptor, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIsetImportDescriptorWrongInst)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);
            IsetImportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, true);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetImportDescriptor, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIsetImportDescriptorWrongImport)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *newID = new AbckitCoreImportDescriptor();
            newID->impl = std::make_unique<AbckitArktsImportDescriptor>();
            IsetImportDescriptorTest(graph, nullptr, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, true, newID);
            delete newID;
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetExportDescriptor, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIsetExportDescriptor)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);
            IsetExportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n2\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetExportDescriptor, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIsetExportDescriptorWrongInst)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);
            IsetExportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, true);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetExportDescriptor, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIsetExportDescriptorWrongExport)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *newED = new AbckitCoreExportDescriptor();
            newED->impl = std::make_unique<AbckitArktsExportDescriptor>();
            IsetExportDescriptorTest(graph, nullptr, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR, true, newED);
            delete newED;
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iGetModule, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIgetModule)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                             "isa_dynamic_modules.func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 IgetModuleTest(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE);
                             });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iGetModule, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIgetModuleWrongInst)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                             "isa_dynamic_modules.func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 IgetModuleTest(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR);
                             });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iGetImportDescriptor, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIgetImportDescriptor)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);
            IgetImportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iGetImportDescriptor, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIgetImportDescriptorWrongInst)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);
            IgetImportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iGetExportDescriptor, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIgetExportDescriptor)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);
            IgetExportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR, 0X5);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iGetExportDescriptor, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIsaDynamicModulesTest, DynamicIgetExportDescriptorWrongInst)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::ModuleByNameContext ctxFinder = {nullptr, "isa_dynamic_modules"};
            g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(ctxFinder.module, nullptr);
            IgetExportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, 0X3);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGetmodulenamespace, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, LibAbcKitTestCreateDynGetmodulenamespace)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    std::string expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *abckitstr =
                g_implM->createString(file, "sameFuncInDifferentModules", strlen("sameFuncInDifferentModules"));

            TransformIrGetModuleNamespace(graph, abckitstr);
        },
        [](AbckitGraph *graph) {
            auto bbSchema = CreateBBSchemaForModules();
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 47,
                                           {59, ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 48,
                                           {60, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {59}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 49,
                                           {61, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {60, 59}});
            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected += "the same func from module2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideGetmodulenamespace, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, LibAbcKitTestCreateDynWideGetmodulenamespace)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    std::string expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *abckitstr =
                g_implM->createString(file, "sameFuncInDifferentModules", strlen("sameFuncInDifferentModules"));

            TransformIrGetModuleNamespace(graph, abckitstr, true);
        },
        [](AbckitGraph *graph) {
            auto bbSchema = CreateBBSchemaForModules();
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 47,
                                           {59, ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_GETMODULENAMESPACE, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 48,
                                           {60, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {59}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 49,
                                           {61, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {60, 59}});
            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected += "the same func from module2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdexternalmodulevar, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, LibAbcKitTestCreateDynLdexternalmodulevar)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    std::string expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *abckitstr = g_implM->createString(file, "MF1M2", strlen("MF1M2"));

            TransformIrLdExternalModuleVar(graph, abckitstr);
        },
        [](AbckitGraph *graph) {
            auto bbSchema = CreateBBSchemaForModules();
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 47,
                                           {59, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 48,
                                           {60, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {59}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 49,
                                           {61, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {59}});
            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected += "regular import func1 from module2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideLdexternalmodulevar, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, LibAbcKitTestCreateDynWideLdexternalmodulevar)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    std::string expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *abckitstr = g_implM->createString(file, "MF1M2", strlen("MF1M2"));

            TransformIrLdExternalModuleVar(graph, abckitstr, true);
        },
        [](AbckitGraph *graph) {
            auto bbSchema = CreateBBSchemaForModules();
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 47,
                                           {59, ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDEXTERNALMODULEVAR, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 48,
                                           {60, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {59}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 49,
                                           {61, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {59}});
            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected += "regular import func1 from module2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdlocalmodulevar, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, LibAbcKitTestCreateDynLdlocalmodulevar)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    std::string expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *abckitstr = g_implM->createString(file, "LocalExportLet", strlen("LocalExportLet"));
            AbckitString *abckitcallStr = g_implM->createString(file, "print", strlen("print"));

            TransformIrLdLocalModuleVar(graph, abckitstr, abckitcallStr);
        },
        [](AbckitGraph *graph) {
            auto bbSchema = CreateBBSchemaForModules();
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 47,
                                           {59, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 48,
                                           {60, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 49,
                                           {61, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {60}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 50,
                                           {62, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {59, 60}});
            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected += "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideLdlocalmodulevar, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, LibAbcKitTestCreateDynWideLdlocalmodulevar)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    std::string expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *abckitstr = g_implM->createString(file, "LocalExportLet", strlen("LocalExportLet"));
            AbckitString *abckitcallStr = g_implM->createString(file, "print", strlen("print"));

            TransformIrLdLocalModuleVar(graph, abckitstr, abckitcallStr, true);
        },
        [](AbckitGraph *graph) {
            auto bbSchema = CreateBBSchemaForModules();
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 47,
                                           {59, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 48,
                                           {60, ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDLOCALMODULEVAR, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 49,
                                           {61, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {60}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 50,
                                           {62, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {59, 60}});
            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected += "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStmodulevar, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, LibAbcKitTestCreateDynStmodulevar)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    std::string expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *abckitstr = g_implM->createString(file, "LocalExportConst", strlen("LocalExportConst"));
            AbckitString *abckitcallStr = g_implM->createString(file, "print", strlen("print"));

            TransformIrStModuleVar(graph, abckitstr, abckitcallStr);
        },
        [](AbckitGraph *graph) {
            auto bbSchema = CreateBBSchemaForModules();
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 47,
                                           {59, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 48,
                                           {60, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, {4}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 49,
                                           {61, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 50,
                                           {62, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {61}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 51,
                                           {63, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {59, 61}});
            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected += "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideStmodulevar, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIsaDynamicModulesTest, LibAbcKitTestCreateDynWideStmodulevar)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
                                             "isa_dynamic_modules");
    std::string expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc", "isa_dynamic_modules.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *abckitstr = g_implM->createString(file, "LocalExportConst", strlen("LocalExportConst"));
            AbckitString *abckitcallStr = g_implM->createString(file, "print", strlen("print"));

            TransformIrStModuleVar(graph, abckitstr, abckitcallStr, true);
        },
        [](AbckitGraph *graph) {
            auto bbSchema = CreateBBSchemaForModules();
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 47,
                                           {59, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 48,
                                           {60, ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STMODULEVAR, {4}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 49,
                                           {61, ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDLOCALMODULEVAR, {}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 50,
                                           {62, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {61}});
            bbSchema[1].instSchemas.insert(bbSchema[1].instSchemas.begin() + 51,
                                           {63, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {59, 61}});
            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/modules/isa_dynamic_modules_modified.abc",
                                        "isa_dynamic_modules");
    expected += "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
