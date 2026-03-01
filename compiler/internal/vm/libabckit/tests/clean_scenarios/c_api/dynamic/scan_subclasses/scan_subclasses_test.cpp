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

#include <sstream>

#include <gtest/gtest.h>

#include "libabckit/c/abckit.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/src/logger.h"

#ifndef ABCKIT_ABC_DIR
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABCKIT_ABC_DIR ""
#endif

namespace {

constexpr AbckitApiVersion VERSION = ABCKIT_VERSION_RELEASE_1_0_0;
auto *g_impl = AbckitGetApiImpl(VERSION);
const AbckitInspectApi *g_implI = AbckitGetInspectApiImpl(VERSION);
const AbckitGraphApi *g_implG = AbckitGetGraphApiImpl(VERSION);
const AbckitIsaApiDynamic *g_dynG = AbckitGetIsaApiDynamicImpl(VERSION);

struct ClassInfo {
    std::string path;
    std::string className;
};

struct CapturedData {
    void *callback = nullptr;
    const AbckitGraphApi *gImplG = nullptr;
};

template <class ModuleCallBack>
inline void EnumerateModules(const ModuleCallBack &cb, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->fileEnumerateModules(file, (void *)(&cb), [](AbckitCoreModule *mod, void *data) {
        const auto &cb = *((ModuleCallBack *)(data));
        cb(mod);
        return true;
    });
}

template <class FunctionCallBack>
inline void EnumerateModuleTopLevelFunctions(AbckitCoreModule *mod, const FunctionCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->moduleEnumerateTopLevelFunctions(mod, (void *)(&cb), [](AbckitCoreFunction *method, void *data) {
        const auto &cb = *((FunctionCallBack *)data);
        cb(method);
        return true;
    });
}

template <class ClassCallBack>
inline void EnumerateModuleClasses(AbckitCoreModule *mod, const ClassCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->moduleEnumerateClasses(mod, (void *)(&cb), [](AbckitCoreClass *klass, void *data) {
        const auto &cb = *((ClassCallBack *)data);
        cb(klass);
        return true;
    });
}

template <class ImportCallBack>
inline void EnumerateModuleImports(AbckitCoreModule *mod, const ImportCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implI->moduleEnumerateImports(mod, (void *)(&cb), [](AbckitCoreImportDescriptor *i, void *data) {
        const auto &cb = *((ImportCallBack *)(data));
        cb(i);
        return true;
    });
}

void GetImportDescriptors(AbckitCoreModule *mod,
                          std::vector<std::pair<AbckitCoreImportDescriptor *, size_t>> *importDescriptors,
                          std::vector<ClassInfo> *baseClasses)
{
    LIBABCKIT_LOG_FUNC;

    EnumerateModuleImports(mod, [&](AbckitCoreImportDescriptor *id) {
        auto importName = g_implI->abckitStringToString(g_implI->importDescriptorGetName(id));
        auto *importedModule = g_implI->importDescriptorGetImportedModule(id);
        auto source = g_implI->abckitStringToString(g_implI->moduleGetName(importedModule));

        for (size_t i = 0; i < baseClasses->size(); ++i) {
            const auto baseClass = (*baseClasses)[i];
            if (source != baseClass.path) {
                continue;
            }
            if (importName == baseClass.className) {
                importDescriptors->emplace_back(id, i);
            }
        }
    });
}

template <class InstCallBack>
inline bool VisitBlock(AbckitBasicBlock *bb, void *data)
{
    auto *captured = reinterpret_cast<CapturedData *>(data);
    const auto &cb = *reinterpret_cast<InstCallBack *>(captured->callback);
    auto *implG = captured->gImplG;
    for (auto *inst = implG->bbGetFirstInst(bb); inst != nullptr; inst = implG->iGetNext(inst)) {
        cb(inst);
    }
    return true;
}

template <class InstCallBack>
inline void EnumerateGraphInsts(AbckitGraph *graph, const InstCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    CapturedData captured {(void *)(&cb), g_implG};

    g_implG->gVisitBlocksRpo(graph, &captured,
                             [](AbckitBasicBlock *bb, void *data) { return VisitBlock<InstCallBack>(bb, data); });
}

template <class InstCallBack>
inline void EnumerateFunctionInsts(AbckitCoreFunction *func, const InstCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    AbckitGraph *graph = g_implI->createGraphFromFunction(func);
    EnumerateGraphInsts(graph, cb);
    g_impl->destroyGraph(graph);
}

template <class UserCallBack>
inline void EnumerateInstUsers(AbckitInst *inst, const UserCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    g_implG->iVisitUsers(inst, (void *)(&cb), [](AbckitInst *user, void *data) {
        const auto &cb = *((UserCallBack *)data);
        cb(user);
        return true;
    });
}

bool IsLoadApi(AbckitCoreImportDescriptor *id, AbckitInst *inst, ClassInfo &subclassInfo)
{
    LIBABCKIT_LOG_FUNC;

    if (g_dynG->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return false;
    }

    if (g_dynG->iGetImportDescriptor(inst) != id) {
        return false;
    }

    bool found = false;
    EnumerateInstUsers(inst, [&](AbckitInst *user) {
        if (g_dynG->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER) {
            auto method = g_implG->iGetFunction(user);
            auto klass = g_implI->functionGetParentClass(method);
            auto module = g_implI->classGetModule(klass);
            subclassInfo.className = g_implI->abckitStringToString(g_implI->classGetName(klass));
            subclassInfo.path = g_implI->abckitStringToString(g_implI->moduleGetName(module));
            found = true;
        }
    });

    return found;
}

void CollectSubClasses(AbckitCoreFunction *method,
                       const std::vector<std::pair<AbckitCoreImportDescriptor *, size_t>> &impDescrs,
                       std::vector<ClassInfo> *subClasses)
{
    LIBABCKIT_LOG_FUNC;

    EnumerateFunctionInsts(method, [&](AbckitInst *inst) {
        for (const auto &[impDescr, idx] : impDescrs) {
            ClassInfo classInfo;
            if (IsLoadApi(impDescr, inst, classInfo)) {
                subClasses->emplace_back(classInfo);
            }
        }
    });
}

template <class MethodCallBack>
inline void EnumerateClassMethods(AbckitCoreClass *klass, const MethodCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;
    g_implI->classEnumerateMethods(klass, (void *)(&cb), [](AbckitCoreFunction *method, void *data) {
        const auto &cb = *((MethodCallBack *)data);
        cb(method);
        return true;
    });
}

template <class FunctionCallBack>
inline void EnumerateModuleFunctions(AbckitCoreModule *mod, const FunctionCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;
    // NOTE: currently we can only enumerate class methods and top level functions. need to update.
    EnumerateModuleTopLevelFunctions(mod, cb);
    EnumerateModuleClasses(mod, [&](AbckitCoreClass *klass) { EnumerateClassMethods(klass, cb); });
}

bool IsEqualsSubClasses(const std::vector<ClassInfo> &otherSubClasses, std::vector<ClassInfo> &subClasses)
{
    LIBABCKIT_LOG_FUNC;

    for (auto &otherSubClass : otherSubClasses) {
        auto iter = std::find_if(subClasses.begin(), subClasses.end(), [&otherSubClass](const ClassInfo &classInfo) {
            return (otherSubClass.className == classInfo.className) && (otherSubClass.path == classInfo.path);
        });
        if (iter == subClasses.end()) {
            return false;
        }
    }
    return true;
}

}  // namespace

namespace libabckit::test {

class AbckitScenarioCTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioCTestClean, LibAbcKitTestScanSubclassesClean)
{
    // CC-OFFNXT(G.NAM.03) project code style
    AbckitFile *file =
        g_impl->openAbc(ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/scan_subclasses/scan_subclasses.abc",
                        strlen(ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/scan_subclasses/scan_subclasses.abc"));
    ASSERT_NE(file, nullptr);

    std::vector<ClassInfo> subClasses;
    std::vector<ClassInfo> baseClasses = {{"modules/base", "Base"}};

    EnumerateModules(
        [&](AbckitCoreModule *mod) {
            std::vector<std::pair<AbckitCoreImportDescriptor *, size_t>> impDescriptors;
            GetImportDescriptors(mod, &impDescriptors, &baseClasses);
            if (impDescriptors.empty()) {
                return;
            }
            EnumerateModuleFunctions(
                mod, [&](AbckitCoreFunction *method) { CollectSubClasses(method, impDescriptors, &subClasses); });
        },
        file);

    ASSERT_FALSE(subClasses.empty());

    const std::vector<ClassInfo> expectedSubClasses = {{"scan_subclasses", "Child1"}, {"scan_subclasses", "Child2"}};

    ASSERT_TRUE(IsEqualsSubClasses(expectedSubClasses, subClasses));
    g_impl->closeFile(file);
}

}  // namespace libabckit::test
