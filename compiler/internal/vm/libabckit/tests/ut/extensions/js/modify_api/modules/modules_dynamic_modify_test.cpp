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
#include "libabckit/c/extensions/js/metadata_js.h"
#include "ir_impl.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/statuses.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implJsI = AbckitGetJsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implJsM = AbckitGetJsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static constexpr auto EXPECTED0 =
    "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
    "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
    "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew exported func "
    "from module3\n";
static constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/js/modify_api/modules/JSmodules_dynamic_modify.abc";
static constexpr auto MODIFIED_PATH =
    ABCKIT_ABC_DIR "ut/extensions/js/modify_api/modules/JSmodules_dynamic_modify_modified.abc";
;

class LibAbcKitJSModifyApiModulesTest : public ::testing::Test {};

// ========================================
// Helpers
// ========================================

template <class T>
static void TestNoError(T *ptr)
{
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ptr, nullptr);
}

static void ModifyMetaDynModuleRemoveImport(AbckitFile *file, const std::string &name, bool shouldBeRemoved)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *module = ctxFinder.module;

    std::set<AbckitCoreImportDescriptor *> gotImports;
    g_implI->moduleEnumerateImports(module, &gotImports, helpers::ModuleImportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(gotImports.empty(), false);

    bool removed = false;
    for (auto &gotImport : gotImports) {
        auto impName = g_implI->importDescriptorGetAlias(gotImport);
        auto strName = helpers::AbckitStringToString(impName);
        if (strName == name) {
            g_implJsM->moduleRemoveImport(g_implJsI->coreModuleToJsModule(module),
                                          g_implJsI->coreImportDescriptorToJsImportDescriptor(gotImport));
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            removed = true;
            break;
        }
    }

    ASSERT_EQ(removed, shouldBeRemoved);

    std::set<AbckitCoreImportDescriptor *> gotImportsAfter;
    g_implI->moduleEnumerateImports(module, &gotImportsAfter, helpers::ModuleImportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(gotImportsAfter.empty(), false);

    for (auto &gotImportAfter : gotImportsAfter) {
        auto impName = g_implI->importDescriptorGetAlias(gotImportAfter);
        auto strName = helpers::AbckitStringToString(impName);
        ASSERT_NE(strName, name);
    }
}

static void ModifyMetaDynModuleRemoveExport(AbckitFile *file, const std::string &exportName, bool shouldBeRemoved)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *module = ctxFinder.module;

    std::set<AbckitCoreExportDescriptor *> gotExports;
    g_implI->moduleEnumerateExports(module, &gotExports, helpers::ModuleExportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    bool removed = false;
    for (auto &gotExport : gotExports) {
        auto expName = g_implI->exportDescriptorGetAlias(gotExport);
        auto strName = helpers::AbckitStringToString(expName);
        if (strName == exportName) {
            g_implJsM->moduleRemoveExport(g_implJsI->coreModuleToJsModule(module),
                                          g_implJsI->coreExportDescriptorToJsExportDescriptor(gotExport));
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            removed = true;
            break;
        }
    }

    ASSERT_EQ(removed, shouldBeRemoved);

    std::set<AbckitCoreExportDescriptor *> gotExportsAfter;
    g_implI->moduleEnumerateExports(module, &gotExportsAfter, helpers::ModuleExportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    for (auto &gotExportAfter : gotExportsAfter) {
        auto expName = g_implI->exportDescriptorGetAlias(gotExportAfter);
        auto strName = helpers::AbckitStringToString(expName);
        ASSERT_NE(strName, exportName);
    }
}

void TransformIrDynModuleRemoveImport(AbckitGraph *graph, const std::string &name)
{
    AbckitInst *instWithName = helpers::FindFirstInst(
        graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, [&](AbckitInst *inst) {
            auto nameOffset = g_implG->iGetImmediate(inst, 0);
            auto nameStr = graph->irInterface->GetStringIdByOffset(nameOffset);
            return nameStr == name;
        });

    ASSERT_NE(instWithName, nullptr);
    auto ldExternalModuleVarInst = g_implG->iGetPrev(instWithName);
    ASSERT_NE(ldExternalModuleVarInst, nullptr);
    auto callArg0Inst = g_implG->iGetNext(instWithName);
    ASSERT_NE(callArg0Inst, nullptr);

    g_implG->iRemove(instWithName);
    g_implG->iRemove(ldExternalModuleVarInst);
    g_implG->iRemove(callArg0Inst);
}

void TransformIrDynModuleRemoveNSImport(AbckitGraph *graph, const std::string &name)
{
    AbckitInst *ldObjByNameInst =
        helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, [&](AbckitInst *inst) {
            auto nameOffset = g_implG->iGetImmediate(inst, 1);
            auto nameStr = graph->irInterface->GetStringIdByOffset(nameOffset);
            return nameStr == name;
        });

    ASSERT_NE(ldObjByNameInst, nullptr);
    auto callThis0Inst = g_implG->iGetNext(ldObjByNameInst);
    ASSERT_NE(callThis0Inst, nullptr);

    g_implG->iRemove(ldObjByNameInst);
    g_implG->iRemove(callThis0Inst);
}

void TransformIrDynModuleRemoveExport(AbckitGraph *graph, const std::string &exportName)
{
    AbckitInst *instWithName = helpers::FindFirstInst(
        graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, [&](AbckitInst *inst) {
            auto nameOffset = g_implG->iGetImmediate(inst, 0);
            auto nameStr = graph->irInterface->GetStringIdByOffset(nameOffset);
            return nameStr == exportName;
        });

    ASSERT_NE(instWithName, nullptr);
    auto ldLocalModuleVarInst = g_implG->iGetPrev(instWithName);
    ASSERT_NE(ldLocalModuleVarInst, nullptr);
    auto callArg1Inst = g_implG->iGetNext(instWithName);
    ASSERT_NE(callArg1Inst, nullptr);

    g_implG->iRemove(instWithName);
    g_implG->iRemove(ldLocalModuleVarInst);
    g_implG->iRemove(callArg1Inst);

    AbckitInst *stModuleVarInst =
        helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, [&](AbckitInst *inst) {
            return g_implG->iGetImmediate(inst, 0) == g_implG->iGetImmediate(ldLocalModuleVarInst, 0);
        });

    ASSERT_NE(stModuleVarInst, nullptr);
    g_implG->iRemove(stModuleVarInst);
}

void IsetModuleTest(AbckitGraph *graph, AbckitCoreModule *module, AbckitIsaApiDynamicOpcode opcode,
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

void IsetImportDescriptorTest(AbckitGraph *graph, AbckitCoreModule *m, AbckitIsaApiDynamicOpcode opcode,
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

void IsetExportDescriptorTest(AbckitGraph *graph, AbckitCoreModule *m, AbckitIsaApiDynamicOpcode opcode,
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

struct UserTransformerData {
    std::string name;
    std::string alias;
    std::string moduleName;
    bool isRegular = false;
    AbckitDynamicExportKind kind = ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
};

static void AddImportFromDynamicModuleImpl(AbckitCoreModule *module, void *data)
{
    auto userTransformerData = reinterpret_cast<UserTransformerData *>(data);
    AbckitFile *file = g_implI->moduleGetFile(module);

    helpers::ModuleByNameContext ctxFinder = {nullptr, (userTransformerData->moduleName).c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    AbckitJsImportFromDynamicModuleCreateParams params {};
    params.name = (userTransformerData->name).c_str();
    params.alias = (userTransformerData->alias).c_str();

    auto newImport = g_implJsM->moduleAddImportFromJsToJs(g_implJsI->coreModuleToJsModule(module),
                                                          g_implJsI->coreModuleToJsModule(ctxFinder.module), &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<AbckitCoreImportDescriptor *> gotImports;
    g_implI->moduleEnumerateImports(module, &gotImports, helpers::ModuleImportsCollector);

    bool existed = false;
    for (auto &gotImport : gotImports) {
        if (g_implJsI->coreImportDescriptorToJsImportDescriptor(gotImport) != newImport) {
            continue;
        }
        existed = true;

        auto impName = g_implI->importDescriptorGetName(gotImport);
        auto strName = helpers::AbckitStringToString(impName);
        auto impAlias = g_implI->importDescriptorGetAlias(gotImport);
        auto strAlias = helpers::AbckitStringToString(impAlias);
        ASSERT_EQ(userTransformerData->name, strName);
        ASSERT_EQ(userTransformerData->alias, strAlias);
        ASSERT_EQ(g_implI->importDescriptorGetImportedModule(gotImport), ctxFinder.module);
        ASSERT_EQ(g_implI->importDescriptorGetImportingModule(gotImport), module);
        ASSERT_EQ(gotImport->GetJsImpl()->payload.GetDynId().isRegularImport, userTransformerData->isRegular);
    }

    ASSERT_EQ(existed, true);
}

static void DynamicModuleAddExportImpl(AbckitCoreModule *module, void *data)
{
    auto userTransformerData = reinterpret_cast<UserTransformerData *>(data);
    AbckitFile *file = g_implI->moduleGetFile(module);

    AbckitJsDynamicModuleExportCreateParams params {};

    AbckitCoreModule *exported = nullptr;
    if (!userTransformerData->moduleName.empty()) {
        helpers::ModuleByNameContext ctxFinder = {nullptr, (userTransformerData->moduleName).c_str()};
        g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ctxFinder.module, nullptr);
        exported = ctxFinder.module;
    }
    params.name = (userTransformerData->name).c_str();

    if (userTransformerData->alias.empty()) {
        params.alias = nullptr;
    } else {
        params.alias = (userTransformerData->alias).c_str();
    }

    auto newExport = g_implJsM->moduleAddExportFromJsToJs(g_implJsI->coreModuleToJsModule(module),
                                                          g_implJsI->coreModuleToJsModule(exported), &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<AbckitCoreExportDescriptor *> gotExports;
    g_implI->moduleEnumerateExports(module, &gotExports, helpers::ModuleExportsCollector);

    bool existed = false;
    for (auto &gotExport : gotExports) {
        if (g_implJsI->coreExportDescriptorToJsExportDescriptor(gotExport) != newExport) {
            continue;
        }
        existed = true;

        auto expName = g_implI->exportDescriptorGetName(gotExport);
        auto strName = helpers::AbckitStringToString(expName);
        auto expAlias = g_implI->exportDescriptorGetAlias(gotExport);
        auto strAlias = helpers::AbckitStringToString(expAlias);
        ASSERT_EQ(userTransformerData->name, strName);
        ASSERT_EQ(userTransformerData->alias, strAlias);
        ASSERT_EQ(userTransformerData->kind, gotExport->GetJsImpl()->payload.GetDynamicPayload().kind);
        if (userTransformerData->kind == AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT) {
            ASSERT_EQ(g_implI->exportDescriptorGetExportedModule(gotExport), module);
        } else {
            ASSERT_EQ(g_implI->exportDescriptorGetExportedModule(gotExport), exported);
        }
        ASSERT_EQ(g_implI->exportDescriptorGetExportingModule(gotExport), module);
    }

    ASSERT_EQ(existed, true);
}

void TransformIrAddImportedFunctionCall(AbckitGraph *graph, AbckitFile *file, const std::string &funcName)
{
    auto abckitStrName = g_implM->createString(file, funcName.c_str(), funcName.size());
    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    helpers::ImportByAliasContext importFinder = {nullptr, funcName.c_str()};
    g_implI->moduleEnumerateImports(ctxFinder.module, &importFinder, helpers::ImportByAliasFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(importFinder.id, nullptr);

    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);
    auto ldExternalModuleVarInst = g_dynG->iCreateLdexternalmodulevar(graph, importFinder.id);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto throwUndef = g_dynG->iCreateThrowUndefinedifholewithname(graph, ldExternalModuleVarInst, abckitStrName);
    auto callInst = g_dynG->iCreateCallarg0(graph, ldExternalModuleVarInst);
    g_implG->iInsertBefore(ldExternalModuleVarInst, lastInst);
    g_implG->iInsertBefore(throwUndef, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

void TransformIrAddImportedFunctionCallNS(AbckitGraph *graph, AbckitFile *file, const std::string &funcName,
                                          const std::string &moduleName)
{
    auto abckitStrName = g_implM->createString(file, funcName.c_str(), funcName.size());
    helpers::ModuleByNameContext ctxFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);
    auto getModuleNamespaceInst = g_dynG->iCreateGetmodulenamespace(graph, ctxFinder.module);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto ldObjByNameInst = g_dynG->iCreateLdobjbyname(graph, getModuleNamespaceInst, abckitStrName);
    auto callInst = g_dynG->iCreateCallthis0(graph, ldObjByNameInst, getModuleNamespaceInst);
    g_implG->iInsertBefore(getModuleNamespaceInst, lastInst);
    g_implG->iInsertBefore(ldObjByNameInst, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

void TransformIrAddLocalExport(AbckitGraph *graph, AbckitFile *file, const std::string &varName,
                               const std::string &moduleName)
{
    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);

    AbckitString *abckitStr = g_implM->createString(file, "print", strlen("print"));
    auto tryLdGlobalByNameInst = g_dynG->iCreateTryldglobalbyname(graph, abckitStr);

    helpers::ModuleByNameContext ctxFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ExportByAliasContext exportFinder = {nullptr, varName.c_str()};
    g_implI->moduleEnumerateExports(ctxFinder.module, &exportFinder, helpers::ExportByAliasFinder);
    ASSERT_NE(exportFinder.ed, nullptr);

    auto constant = g_implG->gFindOrCreateConstantI32(graph, 5);
    auto stModuleVarInst = g_dynG->iCreateStmodulevar(graph, constant, exportFinder.ed);
    auto ldLocalModuleVarInst = g_dynG->iCreateLdlocalmodulevar(graph, exportFinder.ed);
    AbckitString *varNameStr = g_implM->createString(file, varName.c_str(), varName.size());
    auto throwUndef = g_dynG->iCreateThrowUndefinedifholewithname(graph, ldLocalModuleVarInst, varNameStr);
    auto callInst = g_dynG->iCreateCallarg1(graph, tryLdGlobalByNameInst, ldLocalModuleVarInst);

    g_implG->iInsertBefore(stModuleVarInst, lastInst);
    g_implG->iInsertBefore(tryLdGlobalByNameInst, lastInst);
    g_implG->iInsertBefore(ldLocalModuleVarInst, lastInst);
    g_implG->iInsertBefore(throwUndef, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

void TransformIrAddIndirectExport(AbckitGraph *graph, AbckitFile *file, const std::string &varName,
                                  const std::string &moduleName)
{
    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);

    AbckitString *abckitStr = g_implM->createString(file, "print", strlen("print"));
    auto tryLdGlobalByNameInst = g_dynG->iCreateTryldglobalbyname(graph, abckitStr);

    helpers::ModuleByNameContext ctxFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ImportByAliasContext importFinder = {nullptr, varName.c_str()};
    g_implI->moduleEnumerateImports(ctxFinder.module, &importFinder, helpers::ImportByAliasFinder);
    ASSERT_NE(importFinder.id, nullptr);

    auto ldExternalModuleVarInst = g_dynG->iCreateLdexternalmodulevar(graph, importFinder.id);
    AbckitString *varNameStr = g_implM->createString(file, varName.c_str(), varName.size());
    auto throwUndef = g_dynG->iCreateThrowUndefinedifholewithname(graph, ldExternalModuleVarInst, varNameStr);
    auto callInst = g_dynG->iCreateCallarg1(graph, tryLdGlobalByNameInst, ldExternalModuleVarInst);
    g_implG->iInsertBefore(tryLdGlobalByNameInst, lastInst);
    g_implG->iInsertBefore(ldExternalModuleVarInst, lastInst);
    g_implG->iInsertBefore(throwUndef, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

void TransformIrAddStarExport(AbckitGraph *graph, AbckitFile *file, const std::string &funcName,
                              const std::string &moduleName)
{
    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);

    helpers::ModuleByNameContext ctxFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    auto getModuleNamespaceInst = g_dynG->iCreateGetmodulenamespace(graph, ctxFinder.module);
    AbckitString *funcNameStr = g_implM->createString(file, funcName.c_str(), funcName.size());
    auto ldObjByNameInst = g_dynG->iCreateLdobjbyname(graph, getModuleNamespaceInst, funcNameStr);
    auto callInst = g_dynG->iCreateCallthis0(graph, ldObjByNameInst, getModuleNamespaceInst);
    g_implG->iInsertBefore(getModuleNamespaceInst, lastInst);
    g_implG->iInsertBefore(ldObjByNameInst, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

void TransformIrAddStarExportFunc(AbckitGraph *graph, AbckitFile *file, const std::string &exportName,
                                  const std::string &moduleName)
{
    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);

    helpers::ModuleByNameContext ctxFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ModuleByNameContext ctxFinder2 = {nullptr, "modules/JSmodule4"};
    g_implI->fileEnumerateModules(file, &ctxFinder2, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder2.module, nullptr);

    helpers::ExportByAliasContext exportFinder = {nullptr, exportName.c_str()};
    g_implI->moduleEnumerateExports(ctxFinder2.module, &exportFinder, helpers::ExportByAliasFinder);
    ASSERT_NE(exportFinder.ed, nullptr);

    auto getModuleNamespaceInst = g_dynG->iCreateGetmodulenamespace(graph, ctxFinder.module);           // JSmodule3
    auto stModuleVarInst = g_dynG->iCreateStmodulevar(graph, getModuleNamespaceInst, exportFinder.ed);  // NewStarExport
    g_implG->iInsertBefore(getModuleNamespaceInst, lastInst);
    g_implG->iInsertBefore(stModuleVarInst, lastInst);
}

void TransformIrAddStarExportFunc2(AbckitGraph *graph, AbckitFile *file, const std::string &funcName,
                                   const std::string &moduleName)
{
    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);

    helpers::ModuleByNameContext ctxFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ImportByAliasContext importFinder = {nullptr, "NewStarExport"};
    g_implI->moduleEnumerateImports(ctxFinder.module, &importFinder, helpers::ImportByAliasFinder);
    ASSERT_NE(importFinder.id, nullptr);

    auto ldExternalModuleVarInst = g_dynG->iCreateLdexternalmodulevar(graph, importFinder.id);
    auto exportStarName = g_implM->createString(file, "NewStarExport", strlen("NewStarExport"));
    auto throwUndef = g_dynG->iCreateThrowUndefinedifholewithname(graph, ldExternalModuleVarInst, exportStarName);
    auto funcNameStr = g_implM->createString(file, funcName.c_str(), funcName.size());
    auto ldObjByNameInst = g_dynG->iCreateLdobjbyname(graph, ldExternalModuleVarInst, funcNameStr);
    auto callInst = g_dynG->iCreateCallthis0(graph, ldObjByNameInst, ldExternalModuleVarInst);
    g_implG->iInsertBefore(ldExternalModuleVarInst, lastInst);
    g_implG->iInsertBefore(throwUndef, lastInst);
    g_implG->iInsertBefore(ldObjByNameInst, lastInst);
    g_implG->iInsertBefore(callInst, lastInst);
}

// ========================================
// Modules Tests
// ========================================

constexpr char const *UNMODIFIED_EXPECTED_OUTPUT =
    "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
    "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
    "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";

// Test: test-kind=api, api=JsModifyApiImpl::moduleRemoveImport, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleRemoveImport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
                                 TransformIrDynModuleRemoveImport(graph, "regularImportFunc1FromModule1");
                                 ModifyMetaDynModuleRemoveImport(file, "regularImportFunc1FromModule1", true);
                             });

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module2\nregular default import func1 from module3\nregular default import func2 "
        "from module2\nnamespace import func2 from module1\nnamespace import func3 from module1\n1\n2\nnamespace "
        "import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleRemoveImport, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleRemoveNSImport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
                                 TransformIrDynModuleRemoveNSImport(graph, "namespaceImportFunc2FromModule1");
                                 TransformIrDynModuleRemoveNSImport(graph, "namespaceImportFunc3FromModule1");
                                 ModifyMetaDynModuleRemoveImport(file, "NS1", true);
                             });

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\n1\n2\nnamespace import func3 from module2\nthe same func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleRemoveImport, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleRemoveImportAndModule)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
                                 TransformIrDynModuleRemoveImport(graph, "regularDefaultImportFunc1FromModule3");
                                 ModifyMetaDynModuleRemoveImport(file, "regularDefaultImportFunc1FromModule3", true);
                             });

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func2 from "
        "module2\nnamespace import func2 from module1\nnamespace import func3 from module1\n1\n2\nnamespace import "
        "func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleRemoveImport, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleRemoveImportWithWrongName)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
                                 ModifyMetaDynModuleRemoveImport(file, "R", false);
                             });

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleRemoveImport, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleRemoveWrongImport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *) {
            auto *newID = new AbckitCoreImportDescriptor();
            auto newAID = std::make_unique<AbckitJsImportDescriptor>();
            newAID->core = newID;
            newID->impl = std::move(newAID);
            newID->importingModule = method->owningModule;
            helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
            g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            EXPECT_NE(ctxFinder.module, nullptr);
            auto *module = ctxFinder.module;
            g_implJsM->moduleRemoveImport(g_implJsI->coreModuleToJsModule(module),
                                          g_implJsI->coreImportDescriptorToJsImportDescriptor(newID));
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
            g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);  // just to clear last error
            delete newID;
        });

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleRemoveExport, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleRemoveExport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
                                 TransformIrDynModuleRemoveExport(graph, "LocalExportConst");
                                 ModifyMetaDynModuleRemoveExport(file, "LocalExportConst", true);
                             });

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleRemoveExport, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleRemoveExportWithWrongName)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
                                 ModifyMetaDynModuleRemoveExport(file, "R", false);
                             });

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleRemoveExport, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleRemoveWrongExport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *) {
            auto *newED = new AbckitCoreExportDescriptor();
            auto newAED = std::make_unique<AbckitJsExportDescriptor>();
            newAED->core = newED;
            newED->impl = std::move(newAED);
            newED->exportingModule = method->owningModule;
            helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
            g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            EXPECT_NE(ctxFinder.module, nullptr);
            auto *module = ctxFinder.module;
            g_implJsM->moduleRemoveExport(g_implJsI->coreModuleToJsModule(module),
                                          g_implJsI->coreExportDescriptorToJsExportDescriptor(newED));
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
            g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);  // just to clear last error
            delete newED;
        });

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetModule, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIsetModule)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/JSmodule1"};
        g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ctxFinder.module, nullptr);

        IsetModuleTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE);
    };

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetModule, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIsetModuleWrongInst)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/JSmodule1"};
        g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ctxFinder.module, nullptr);

        IsetModuleTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, true);
    };

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetModule, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIsetModuleWrongModule)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
                             [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
                                 auto *newM = new AbckitCoreModule();
                                 newM->impl = std::make_unique<AbckitJsModule>();
                                 IsetModuleTest(graph, newM, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, true, true);
                                 delete newM;
                             });

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetImportDescriptor, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIsetImportDescriptor)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
        g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ctxFinder.module, nullptr);
        IsetImportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR);
    };

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular import func1 from module2\nnamespace import func2 from module1\nnamespace import func3 from "
        "module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetImportDescriptor, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIsetImportDescriptorWrongInst)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
        g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ctxFinder.module, nullptr);
        IsetImportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR, true);
    };

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetImportDescriptor, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIsetImportDescriptorWrongImport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *newID = new AbckitCoreImportDescriptor();
        newID->impl = std::make_unique<AbckitJsImportDescriptor>();
        IsetImportDescriptorTest(graph, nullptr, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, true, newID);
        delete newID;
    };

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetExportDescriptor, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIsetExportDescriptor)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
        g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ctxFinder.module, nullptr);
        IsetExportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR);
    };

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n2\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetExportDescriptor, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIsetExportDescriptorWrongInst)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
        g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(ctxFinder.module, nullptr);
        IsetExportDescriptorTest(graph, ctxFinder.module, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR, true);
    };

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetExportDescriptor, abc-kind=JS, category=negative
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIsetExportDescriptorWrongExport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *newED = new AbckitCoreExportDescriptor();
        newED->impl = std::make_unique<AbckitJsExportDescriptor>();
        IsetExportDescriptorTest(graph, nullptr, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR, true, newED);
        delete newED;
    };

    helpers::TransformMethod(INPUT_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddImportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, ModuleAddImportFromDynamicModule_RegularImport)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "newImportedFunc";
    utd.alias = "newImportedFunc";
    utd.moduleName = "modules/JSmodule3";
    utd.isRegular = true;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCall(graph, g_implI->functionGetFile(method), "newImportedFunc");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddImportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, ModuleAddImportFromDynamicModule_RegularImport2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "newImportedFunc";
    utd.alias = "NewImportedFuncAlias";
    utd.moduleName = "modules/JSmodule3";
    utd.isRegular = true;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCall(graph, g_implI->functionGetFile(method), "NewImportedFuncAlias");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddImportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, ModuleAddImportFromDynamicModule_RegularImport3)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
    AbckitFile *file = nullptr;

    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "newImportedFuncFromModule4";
    utd.alias = "NewImportedFuncAlias";
    utd.moduleName = "modules/JSmodule4";
    utd.isRegular = true;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCall(graph, g_implI->functionGetFile(method), "NewImportedFuncAlias");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddImportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, ModuleAddImportFromDynamicModule_RegularImport4)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
    AbckitFile *file = nullptr;

    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "default";
    utd.alias = "newImportedDefaultFuncFromModule4";
    utd.moduleName = "modules/JSmodule4";
    utd.isRegular = true;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCall(graph, g_implI->functionGetFile(method),
                                               "newImportedDefaultFuncFromModule4");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported "
        "default func from module4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddImportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, ModuleAddImportFromDynamicModule_NamespaceImport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
    AbckitFile *file = nullptr;

    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "*";
    utd.alias = "NewImport";
    utd.moduleName = "modules/JSmodule3";
    utd.isRegular = false;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCallNS(graph, g_implI->functionGetFile(method), "newImportedFunc",
                                                 "modules/JSmodule3");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddImportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, ModuleAddImportFromDynamicModule_NamespaceImport2)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
    AbckitFile *file = nullptr;

    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "*";
    utd.alias = "NewImport";
    utd.moduleName = "modules/JSmodule4";
    utd.isRegular = false;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCallNS(graph, g_implI->functionGetFile(method), "newImportedFuncFromModule4",
                                                 "modules/JSmodule4");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddExportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleAddExport_LocalExport)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "NewExportedVar";
    utd.alias = "NewExportedVar";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    utd.moduleName = "JSmodules_dynamic_modify";
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [&utd](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddLocalExport(graph, g_implI->functionGetFile(method), utd.alias, "JSmodules_dynamic_modify");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n5\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddExportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleAddExport_LocalExport2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "NewExportedVar";
    utd.alias = "NewExportedVarAlias";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    utd.moduleName = "JSmodules_dynamic_modify";
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [&utd](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddLocalExport(graph, g_implI->functionGetFile(method), utd.alias, "JSmodules_dynamic_modify");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n5\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddExportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleAddExport_LocalExport3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "default";
    utd.alias = "NewExportedVarDefault";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    utd.moduleName = "JSmodules_dynamic_modify";
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [&utd](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddLocalExport(graph, g_implI->functionGetFile(method), utd.alias, "JSmodules_dynamic_modify");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n5\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddExportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleAddExport_IndirectExport)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/JSmodule2"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    TestNoError(ctxFinder.module);
    auto module = ctxFinder.module;
    UserTransformerData utd {"NewLocalExportLet", "NewLocalExportLet", "modules/JSmodule1", false,
                             AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT};
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    UserTransformerData utd2 {utd.name, utd.alias, "modules/JSmodule2", true};
    ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    TestNoError(ctxFinder.module);
    AddImportFromDynamicModuleImpl(ctxFinder.module, &utd2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [&utd2](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddIndirectExport(graph, g_implI->functionGetFile(method), utd2.alias,
                                         "JSmodules_dynamic_modify");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddExportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleAddExport_IndirectExport2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/JSmodule2"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd = {"NewLocalExportVar", "NewLocalExportVarAlias", "modules/JSmodule4", false,
                               AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT};
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    UserTransformerData utd2 = {utd.alias, "NewLocalExportVarFromModule4", "modules/JSmodule2", true};
    ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    AddImportFromDynamicModuleImpl(ctxFinder.module, &utd2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [&utd2](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddIndirectExport(graph, g_implI->functionGetFile(method), utd2.alias,
                                         "JSmodules_dynamic_modify");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n6\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddExportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleAddExport_StarExport)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/JSmodule4"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "*";
    utd.moduleName = "modules/JSmodule3";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT;
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    module = ctxFinder.module;
    UserTransformerData utd2 = {"*", "NS4", "modules/JSmodule4", false};
    AddImportFromDynamicModuleImpl(module, &utd2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [&utd2](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddStarExport(graph, g_implI->functionGetFile(method), "newExportedFunc", utd2.moduleName);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    EXPECT_TRUE(helpers::Match(output, EXPECTED0));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddExportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleAddExport_StarExport2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/JSmodule3"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "*";
    utd.moduleName = "modules/JSmodule4";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT;
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddStarExport(graph, g_implI->functionGetFile(method), "newExportedFuncFromModule4",
                                     "modules/JSmodule3");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew exported func "
        "from module4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddExportFromJsToJs, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicModuleAddExport_StarExport3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/JSmodule4"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    TestNoError(ctxFinder.module);
    auto module = ctxFinder.module;
    UserTransformerData utd {"*", "NewStarExport", "modules/JSmodule3", false,
                             AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT};
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ctxFinder = {nullptr, "JSmodules_dynamic_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    TestNoError(ctxFinder.module);
    module = ctxFinder.module;
    UserTransformerData utd2 {"NewStarExport", "NewStarExport", "modules/JSmodule4", true};
    AddImportFromDynamicModuleImpl(module, &utd2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "modules/JSmodule4.func_main_0",
        [&utd2, &utd](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddStarExportFunc(graph, g_implI->functionGetFile(method), utd2.alias, utd.moduleName);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        MODIFIED_PATH, MODIFIED_PATH, "JSmodules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddStarExportFunc2(graph, g_implI->functionGetFile(method), "newExportedFunc",
                                          "JSmodules_dynamic_modify");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew exported func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=JsModifyApiImpl::fileAddExternalModule, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, FileAddExternalModule)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    AbckitJsExternalModuleCreateParams params {};
    params.name = "ExternalModule";
    g_implJsM->fileAddExternalModule(file, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitJsModule *jsModule = g_implJsM->fileAddExternalModule(file, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(jsModule, nullptr);

    AbckitCoreModule *coreModule = g_implJsI->jsModuleToCoreModule(jsModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(coreModule, nullptr);

    helpers::ModuleByNameContext ctxFinder = {nullptr, params.name};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    ASSERT_EQ(g_implI->moduleIsExternal(ctxFinder.module), true);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(MODIFIED_PATH, "JSmodules_dynamic_modify");
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

}  // namespace libabckit::test
