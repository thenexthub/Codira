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
#include "metadata_inspect_impl.h"
#include "libabckit/c/statuses.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiModulesTest : public ::testing::Test {};

constexpr auto MAIN_MODULE_NAME = "modules_dynamic_modify";

// ========================================
// Helpers
// ========================================

static void ModifyMetaDynModuleRemoveImport(AbckitFile *file, const std::string &name, bool shouldBeRemoved)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
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
            g_implArkM->moduleRemoveImport(g_implArkI->coreModuleToArktsModule(module),
                                           g_implArkI->coreImportDescriptorToArktsImportDescriptor(gotImport));
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
    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
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
            g_implArkM->moduleRemoveExport(g_implArkI->coreModuleToArktsModule(module),
                                           g_implArkI->coreExportDescriptorToArktsExportDescriptor(gotExport));
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

static void TransformIrDynModuleRemoveImport(AbckitGraph *graph, const std::string &name)
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

static void TransformIrDynModuleRemoveNSImport(AbckitGraph *graph, const std::string &name)
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

static void TransformIrDynModuleRemoveExport(AbckitGraph *graph, const std::string &exportName)
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

    AbckitArktsImportFromDynamicModuleCreateParams params {};
    params.name = (userTransformerData->name).c_str();
    params.alias = (userTransformerData->alias).c_str();

    auto newImport = g_implArkM->moduleAddImportFromArktsV1ToArktsV1(
        g_implArkI->coreModuleToArktsModule(module), g_implArkI->coreModuleToArktsModule(ctxFinder.module), &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<AbckitCoreImportDescriptor *> gotImports;
    g_implI->moduleEnumerateImports(module, &gotImports, helpers::ModuleImportsCollector);

    bool existed = false;
    for (auto &gotImport : gotImports) {
        if (g_implArkI->coreImportDescriptorToArktsImportDescriptor(gotImport) != newImport) {
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
        ASSERT_EQ(gotImport->GetArkTSImpl()->payload.GetDynId().isRegularImport, userTransformerData->isRegular);
    }

    ASSERT_EQ(existed, true);
}

static void DynamicModuleAddExportImpl(AbckitCoreModule *module, void *data)
{
    auto userTransformerData = reinterpret_cast<UserTransformerData *>(data);
    AbckitFile *file = g_implI->moduleGetFile(module);

    AbckitArktsDynamicModuleExportCreateParams params {};

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

    auto newExport = g_implArkM->moduleAddExportFromArktsV1ToArktsV1(
        g_implArkI->coreModuleToArktsModule(module), g_implArkI->coreModuleToArktsModule(exported), &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<AbckitCoreExportDescriptor *> gotExports;
    g_implI->moduleEnumerateExports(module, &gotExports, helpers::ModuleExportsCollector);

    bool existed = false;
    for (auto &gotExport : gotExports) {
        if (g_implArkI->coreExportDescriptorToArktsExportDescriptor(gotExport) != newExport) {
            continue;
        }
        existed = true;

        auto expName = g_implI->exportDescriptorGetName(gotExport);
        auto strName = helpers::AbckitStringToString(expName);
        auto expAlias = g_implI->exportDescriptorGetAlias(gotExport);
        auto strAlias = helpers::AbckitStringToString(expAlias);
        ASSERT_EQ(userTransformerData->name, strName);
        ASSERT_EQ(userTransformerData->alias, strAlias);
        ASSERT_EQ(userTransformerData->kind, gotExport->GetArkTSImpl()->payload.GetDynamicPayload().kind);
        if (userTransformerData->kind == AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT) {
            ASSERT_EQ(g_implI->exportDescriptorGetExportedModule(gotExport), module);
        } else {
            ASSERT_EQ(g_implI->exportDescriptorGetExportedModule(gotExport), exported);
        }
        ASSERT_EQ(g_implI->exportDescriptorGetExportingModule(gotExport), module);
    }

    ASSERT_EQ(existed, true);
}

static void TransformIrAddImportedFunctionCall(AbckitGraph *graph, AbckitFile *file, const std::string &funcName)
{
    auto abckitStrName = g_implM->createString(file, funcName.c_str(), funcName.size());
    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
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

static void TransformIrAddImportedFunctionCallNS(AbckitGraph *graph, AbckitFile *file, const std::string &funcName,
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

static void TransformIrAddLocalExport(AbckitGraph *graph, AbckitFile *file, const std::string &varName,
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

static void TransformIrAddIndirectExport(AbckitGraph *graph, AbckitFile *file, const std::string &varName,
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

static void TransformIrAddStarExport(AbckitGraph *graph, AbckitFile *file, const std::string &funcName,
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

static void TransformIrAddStarExportFunc(AbckitGraph *graph, AbckitFile *file, const std::string &exportName,
                                         const std::string &moduleName)
{
    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);

    helpers::ModuleByNameContext ctxFinder = {nullptr, moduleName.c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ModuleByNameContext ctxFinder2 = {nullptr, "modules/module4"};
    g_implI->fileEnumerateModules(file, &ctxFinder2, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder2.module, nullptr);

    helpers::ExportByAliasContext exportFinder = {nullptr, exportName.c_str()};
    g_implI->moduleEnumerateExports(ctxFinder2.module, &exportFinder, helpers::ExportByAliasFinder);
    ASSERT_NE(exportFinder.ed, nullptr);

    auto getModuleNamespaceInst = g_dynG->iCreateGetmodulenamespace(graph, ctxFinder.module);           // module3
    auto stModuleVarInst = g_dynG->iCreateStmodulevar(graph, getModuleNamespaceInst, exportFinder.ed);  // NewStarExport
    g_implG->iInsertBefore(getModuleNamespaceInst, lastInst);
    g_implG->iInsertBefore(stModuleVarInst, lastInst);
}

static void TransformIrAddStarExportFunc2(AbckitGraph *graph, AbckitFile *file, const std::string &funcName,
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

constexpr auto UNMODIFIED_EXPECTED_OUTPUT =
    "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
    "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
    "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n";

constexpr auto OUTPUT_PATH =
    ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_dynamic_modify_modified.abc";

constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_dynamic_modify.abc";

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveImport, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveImport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
                                 TransformIrDynModuleRemoveImport(graph, "regularImportFunc1FromModule1");
                                 ModifyMetaDynModuleRemoveImport(file, "regularImportFunc1FromModule1", true);
                             });

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module2\nregular default import func1 from module3\nregular default import func2 "
        "from module2\nnamespace import func2 from module1\nnamespace import func3 from module1\n1\n2\nnamespace "
        "import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveImport, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveNSImport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
                                 TransformIrDynModuleRemoveNSImport(graph, "namespaceImportFunc2FromModule1");
                                 TransformIrDynModuleRemoveNSImport(graph, "namespaceImportFunc3FromModule1");
                                 ModifyMetaDynModuleRemoveImport(file, "NS1", true);
                             });

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\n1\n2\nnamespace import func3 from module2\nthe same func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveImport, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveImportAndModule)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
                                 TransformIrDynModuleRemoveImport(graph, "regularDefaultImportFunc1FromModule3");
                                 ModifyMetaDynModuleRemoveImport(file, "regularDefaultImportFunc1FromModule3", true);
                             });

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func2 from "
        "module2\nnamespace import func2 from module1\nnamespace import func3 from module1\n1\n2\nnamespace import "
        "func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveImport, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveImportWithWrongName)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
                                 ModifyMetaDynModuleRemoveImport(file, "R", false);
                             });

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveImport, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveWrongImport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        INPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *) {
            auto *newID = new AbckitCoreImportDescriptor();
            auto newAID = std::make_unique<AbckitArktsImportDescriptor>();
            newAID->core = newID;
            newID->impl = std::move(newAID);
            newID->importingModule = method->owningModule;
            newID->importedModule = method->owningModule;  // just to make object newID valid
            helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
            g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            EXPECT_NE(ctxFinder.module, nullptr);
            auto *module = ctxFinder.module;
            g_implArkM->moduleRemoveImport(g_implArkI->coreModuleToArktsModule(module),
                                           g_implArkI->coreImportDescriptorToArktsImportDescriptor(newID));
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
            g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);  // just to clear last error
            delete newID;
        });

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveImport, abc-kind=ArkTS1, category=negative, extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveImport_WrongTargets)
{
    // importDescriptorGetAlias has no ArkTS_V2 support, aliases for imports are not implemented

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *module = ctxFinder.module;

    // Find import from module/module4 in module/module3
    ctxFinder = {nullptr, "modules/module3"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *module2 = ctxFinder.module;
    // Set another module target
    module2->target = ABCKIT_TARGET_ARK_TS_V2;

    std::set<AbckitCoreImportDescriptor *> gotImports;
    g_implI->moduleEnumerateImports(module2, &gotImports, helpers::ModuleImportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(gotImports.empty(), false);

    // Try to remove found import from main module
    auto importName = "regularImportFunc1FromModule1";
    for (auto &gotImport : gotImports) {
        auto impName = g_implI->importDescriptorGetAlias(gotImport);
        if (impName == nullptr) {
            // importDescriptorGetAlias may return null for ArkTS_V2, skip null aliases
            continue;
        }
        auto strName = helpers::AbckitStringToString(impName);
        if (strName == importName) {
            g_implArkM->moduleRemoveImport(g_implArkI->coreModuleToArktsModule(module),
                                           g_implArkI->coreImportDescriptorToArktsImportDescriptor(gotImport));
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
            break;
        }
    }

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveExport, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveExport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
                                 TransformIrDynModuleRemoveExport(graph, "LocalExportConst");
                                 ModifyMetaDynModuleRemoveExport(file, "LocalExportConst", true);
                             });

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n2\nnamespace import func3 from module2\nthe same func from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveExport, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveExportWithWrongName)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(INPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
                             [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *) {
                                 ModifyMetaDynModuleRemoveExport(file, "R", false);
                             });

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveExport, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveWrongExport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        INPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *) {
            auto *newED = new AbckitCoreExportDescriptor();
            auto newAED = std::make_unique<AbckitArktsExportDescriptor>();
            newAED->core = newED;
            newED->impl = std::move(newAED);
            newED->exportingModule = method->owningModule;
            newED->exportedModule = method->owningModule;  // just to make object newED valid
            helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
            g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            EXPECT_NE(ctxFinder.module, nullptr);
            auto *module = ctxFinder.module;
            g_implArkM->moduleRemoveExport(g_implArkI->coreModuleToArktsModule(module),
                                           g_implArkI->coreExportDescriptorToArktsExportDescriptor(newED));
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
            g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);  // just to clear last error
            delete newED;
        });

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleRemoveExport, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleRemoveExport_WrongTargets)
{
    // exportDescriptorGetAlias has no ArkTS_V2 support, aliases for exports are not implemented

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *module = ctxFinder.module;

    // Find export in module/module3
    ctxFinder = {nullptr, "modules/module3"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *module2 = ctxFinder.module;
    // Set another module target
    module2->target = ABCKIT_TARGET_ARK_TS_V2;

    std::set<AbckitCoreExportDescriptor *> gotExports;
    g_implI->moduleEnumerateExports(module2, &gotExports, helpers::ModuleExportsCollector);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(gotExports.empty(), false);

    auto exportName = "regularDefaultImportFunc1FromModule3";
    for (auto &gotExport : gotExports) {
        auto expName = g_implI->exportDescriptorGetAlias(gotExport);
        if (expName == nullptr) {
            // exportDescriptorGetAlias has no ArkTS_V2 support, skip null aliases
            continue;
        }
        auto strName = helpers::AbckitStringToString(expName);
        if (strName == exportName) {
            g_implArkM->moduleRemoveExport(g_implArkI->coreModuleToArktsModule(module),
                                           g_implArkI->coreExportDescriptorToArktsExportDescriptor(gotExport));
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);
            break;
        }
    }

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddImportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleAddImportFromDynamicModule_RegularImport)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "newImportedFunc";
    utd.alias = "newImportedFunc";
    utd.moduleName = "modules/module3";
    utd.isRegular = true;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCall(graph, g_implI->functionGetFile(method), "newImportedFunc");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddImportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleAddImportFromDynamicModule_RegularImport2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "newImportedFunc";
    utd.alias = "NewImportedFuncAlias";
    utd.moduleName = "modules/module3";
    utd.isRegular = true;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCall(graph, g_implI->functionGetFile(method), "NewImportedFuncAlias");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddImportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleAddImportFromDynamicModule_RegularImport3)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
    AbckitFile *file = nullptr;

    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "newImportedFuncFromModule4";
    utd.alias = "NewImportedFuncAlias";
    utd.moduleName = "modules/module4";
    utd.isRegular = true;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCall(graph, g_implI->functionGetFile(method), "NewImportedFuncAlias");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddImportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleAddImportFromDynamicModule_RegularImport4)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
    AbckitFile *file = nullptr;

    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "default";
    utd.alias = "newImportedDefaultFuncFromModule4";
    utd.moduleName = "modules/module4";
    utd.isRegular = true;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCall(graph, g_implI->functionGetFile(method),
                                               "newImportedDefaultFuncFromModule4");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported "
        "default func from module4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddImportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleAddImportFromDynamicModule_NamespaceImport)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
    AbckitFile *file = nullptr;

    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "*";
    utd.alias = "NewImport";
    utd.moduleName = "modules/module3";
    utd.isRegular = false;
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCallNS(graph, g_implI->functionGetFile(method), "newImportedFunc",
                                                 "modules/module3");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddImportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleAddImportFromDynamicModule_NamespaceImport2)
{
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
    AbckitFile *file = nullptr;

    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd = {"*", "NewImport", "modules/module4", false};
    AddImportFromDynamicModuleImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddImportedFunctionCallNS(graph, g_implI->functionGetFile(method), "newImportedFuncFromModule4",
                                                 "modules/module4");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew imported func "
        "from module4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddImportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=negative
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleAddImportFromDynamicModule_WrongTargets)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *module = ctxFinder.module;

    helpers::ModuleByNameContext ctxFinder2 = {nullptr, "modules/module4"};
    g_implI->fileEnumerateModules(file, &ctxFinder2, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder2.module, nullptr);
    auto *importedModule = ctxFinder2.module;

    AbckitArktsImportFromDynamicModuleCreateParams params {};
    params.name = "*";
    params.alias = "NewImport";

    importedModule->target = ABCKIT_TARGET_ARK_TS_V2;

    g_implArkM->moduleAddImportFromArktsV1ToArktsV1(g_implArkI->coreModuleToArktsModule(module),
                                                    g_implArkI->coreModuleToArktsModule(importedModule), &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleAddExport_LocalExport)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "NewExportedVar";
    utd.alias = "NewExportedVar";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    utd.moduleName = MAIN_MODULE_NAME;
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [&utd](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddLocalExport(graph, g_implI->functionGetFile(method), utd.alias, MAIN_MODULE_NAME);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n5\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleAddExport_LocalExport2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "NewExportedVar";
    utd.alias = "NewExportedVarAlias";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    utd.moduleName = MAIN_MODULE_NAME;
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [&utd](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddLocalExport(graph, g_implI->functionGetFile(method), utd.alias, MAIN_MODULE_NAME);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n5\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleAddExport_LocalExport3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "default";
    utd.alias = "NewExportedVarDefault";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    utd.moduleName = MAIN_MODULE_NAME;
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [&utd](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddLocalExport(graph, g_implI->functionGetFile(method), utd.alias, MAIN_MODULE_NAME);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n5\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleAddExport_IndirectExport)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module2"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "NewLocalExportLet";
    utd.alias = "NewLocalExportLet";
    utd.moduleName = "modules/module1";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT;
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    UserTransformerData utd2;
    utd2.name = utd.name;
    utd2.alias = utd.alias;
    utd2.moduleName = "modules/module2";
    utd2.isRegular = true;
    ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    AddImportFromDynamicModuleImpl(ctxFinder.module, &utd2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [&utd2](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddIndirectExport(graph, g_implI->functionGetFile(method), utd2.alias, MAIN_MODULE_NAME);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleAddExport_IndirectExport2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module2"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "NewLocalExportVar";
    utd.alias = "NewLocalExportVarAlias";
    utd.moduleName = "modules/module4";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT;
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    UserTransformerData utd2 = {utd.alias, "NewLocalExportVarFromModule4", "modules/module2", true};
    ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    AddImportFromDynamicModuleImpl(ctxFinder.module, &utd2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [&utd2](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddIndirectExport(graph, g_implI->functionGetFile(method), utd2.alias, MAIN_MODULE_NAME);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\n6\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleAddExport_StarExport)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module4"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd = {"*", "", "modules/module3", false,
                               AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT};
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    module = ctxFinder.module;
    UserTransformerData utd2 = {"*", "NS4", "modules/module4", false};
    AddImportFromDynamicModuleImpl(module, &utd2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [&utd2](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddStarExport(graph, g_implI->functionGetFile(method), "newExportedFunc", utd2.moduleName);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew exported func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleAddExport_StarExport2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module3"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    UserTransformerData utd;
    utd.name = "*";
    utd.moduleName = "modules/module4";
    utd.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT;
    DynamicModuleAddExportImpl(module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddStarExport(graph, g_implI->functionGetFile(method), "newExportedFuncFromModule4",
                                     "modules/module3");
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew exported func "
        "from module4\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleAddExport_StarExport3)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module4"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    UserTransformerData utd = {"*", "NewStarExport", "modules/module3", false,
                               AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT};
    DynamicModuleAddExportImpl(ctxFinder.module, &utd);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ctxFinder = {nullptr, MAIN_MODULE_NAME};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    UserTransformerData utd2 = {"NewStarExport", "NewStarExport", "modules/module4", true};
    AddImportFromDynamicModuleImpl(ctxFinder.module, &utd2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules/module4.func_main_0",
        [&utd2, &utd](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddStarExportFunc(graph, g_implI->functionGetFile(method), utd2.alias, utd.moduleName);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        OUTPUT_PATH, OUTPUT_PATH, "modules_dynamic_modify.func_main_0",
        [](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            TransformIrAddStarExportFunc2(graph, g_implI->functionGetFile(method), "newExportedFunc", MAIN_MODULE_NAME);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n1\n2\nnamespace import func3 from module2\nthe same func from module3\nnew exported func "
        "from module3\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddExportFromArktsV1ToArktsV1, abc-kind=ArkTS1,
// category=negative, extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesTest, DynamicModuleAddExport_WrongTargets)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules/module2"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *module = ctxFinder.module;

    helpers::ModuleByNameContext ctxFinder2 = {nullptr, "modules/module1"};
    g_implI->fileEnumerateModules(file, &ctxFinder2, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder2.module, nullptr);
    auto *exportedModule = ctxFinder2.module;

    AbckitArktsDynamicModuleExportCreateParams params {};
    params.name = "NewLocalExportLet";
    params.alias = "NewLocalExportLet";

    exportedModule->target = ABCKIT_TARGET_ARK_TS_V2;

    g_implArkM->moduleAddExportFromArktsV1ToArktsV1(g_implArkI->coreModuleToArktsModule(module),
                                                    g_implArkI->coreModuleToArktsModule(exportedModule), &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_WRONG_TARGET);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::fileAddExternalModuleArktsV1, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesTest, fileAddExternalModuleArktsV1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    AbckitArktsV1ExternalModuleCreateParams params {};
    params.name = "ExternalModule";
    AbckitArktsModule *arkTsModule = g_implArkM->fileAddExternalModuleArktsV1(file, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arkTsModule, nullptr);

    AbckitCoreModule *coreModule = g_implArkI->arktsModuleToCoreModule(arkTsModule);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(coreModule, nullptr);

    helpers::ModuleByNameContext ctxFinder = {nullptr, params.name};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    ASSERT_EQ(g_implI->moduleIsExternal(ctxFinder.module), true);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, MAIN_MODULE_NAME);
    auto expected = UNMODIFIED_EXPECTED_OUTPUT;
    EXPECT_TRUE(helpers::Match(output, expected));
}

}  // namespace libabckit::test
