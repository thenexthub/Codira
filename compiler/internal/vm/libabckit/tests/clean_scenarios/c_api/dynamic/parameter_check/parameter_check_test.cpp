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

#include "libabckit/c/abckit.h"
#include "libabckit/c/metadata_core.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/src/logger.h"

namespace {

constexpr AbckitApiVersion VERSION = ABCKIT_VERSION_RELEASE_1_0_0;
auto *g_impl = AbckitGetApiImpl(VERSION);
const AbckitInspectApi *g_implI = AbckitGetInspectApiImpl(VERSION);
const AbckitGraphApi *g_implG = AbckitGetGraphApiImpl(VERSION);
const AbckitIsaApiDynamic *g_dynG = AbckitGetIsaApiDynamicImpl(VERSION);
const AbckitModifyApi *g_implM = AbckitGetModifyApiImpl(VERSION);

template <class FunctionCallBack>
inline void TransformMethod(AbckitCoreFunction *f, const FunctionCallBack &cb)
{
    cb(g_implI->functionGetFile(f), f);
}

void AddParamChecker(AbckitCoreFunction *method)
{
    AbckitGraph *graph = g_implI->createGraphFromFunction(method);

    TransformMethod(method, [&](AbckitFile *file, AbckitCoreFunction *method) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        AbckitInst *idx = g_implG->bbGetLastInst(startBB);
        AbckitInst *arr = g_implG->iGetPrev(idx);

        std::vector<AbckitBasicBlock *> succBBs;
        g_implG->bbVisitSuccBlocks(startBB, &succBBs, [](AbckitBasicBlock *succBasicBlock, void *d) {
            auto *succs = reinterpret_cast<std::vector<AbckitBasicBlock *> *>(d);
            succs->emplace_back(succBasicBlock);
            return true;
        });

        AbckitString *str = g_implM->createString(file, "length", strlen("length"));

        AbckitInst *constant = g_implG->gFindOrCreateConstantI32(graph, -1);
        AbckitInst *arrLength = g_dynG->iCreateLdobjbyname(graph, arr, str);

        AbckitBasicBlock *trueBB = succBBs[0];
        g_implG->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
        AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
        g_implG->bbAppendSuccBlock(falseBB, g_implG->gGetEndBasicBlock(graph));
        g_implG->bbAddInstBack(falseBB, g_dynG->iCreateReturn(graph, constant));
        AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
        AbckitInst *intrinsicGreatereq = g_dynG->iCreateGreatereq(graph, arrLength, idx);
        AbckitInst *ifInst = g_dynG->iCreateIf(graph, intrinsicGreatereq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
        g_implG->bbAddInstBack(ifBB, arrLength);
        g_implG->bbAddInstBack(ifBB, intrinsicGreatereq);
        g_implG->bbAddInstBack(ifBB, ifInst);
        g_implG->bbAppendSuccBlock(startBB, ifBB);
        g_implG->bbAppendSuccBlock(ifBB, trueBB);
        g_implG->bbAppendSuccBlock(ifBB, falseBB);

        g_implM->functionSetGraph(method, graph);
        g_impl->destroyGraph(graph);
    });
}

struct CapturedData {
    void *callback = nullptr;
    const AbckitGraphApi *gImplG = nullptr;
};

struct MethodInfo {
    std::string path;
    std::string className;
    std::string methodName;
};

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

AbckitCoreImportDescriptor *GetImportDescriptor(AbckitCoreModule *module, MethodInfo &methodInfo)
{
    AbckitCoreImportDescriptor *impDescriptor = nullptr;
    EnumerateModuleImports(module, [&](AbckitCoreImportDescriptor *id) {
        auto importName = g_implI->abckitStringToString(g_implI->importDescriptorGetName(id));
        auto *importedModule = g_implI->importDescriptorGetImportedModule(id);
        auto source = g_implI->abckitStringToString(g_implI->moduleGetName(importedModule));
        if (source != methodInfo.path) {
            return false;
        }
        if (importName == methodInfo.className) {
            impDescriptor = id;
            return true;
        }
        return false;
    });
    return impDescriptor;
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

template <class FunctionCallBack>
inline void EnumerateModuleFunctions(AbckitCoreModule *mod, const FunctionCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;
    // NOTE: currently we can only enumerate class methods and top level functions. need to update.
    EnumerateModuleTopLevelFunctions(mod, cb);
    EnumerateModuleClasses(mod, [&](AbckitCoreClass *klass) { EnumerateClassMethods(klass, cb); });
}

template <class InstCallBack>
inline void EnumerateFunctionInsts(AbckitCoreFunction *func, const InstCallBack &cb)
{
    LIBABCKIT_LOG_FUNC;

    AbckitGraph *graph = g_implI->createGraphFromFunction(func);
    EnumerateGraphInsts(graph, cb);
    g_impl->destroyGraph(graph);
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

AbckitCoreFunction *GetMethodToModify(AbckitCoreClass *klass, MethodInfo &methodInfo)
{
    AbckitCoreFunction *foundMethod = nullptr;
    EnumerateClassMethods(klass, [&](AbckitCoreFunction *method) {
        auto name = g_implI->abckitStringToString(g_implI->functionGetName(method));
        if (name == methodInfo.methodName) {
            foundMethod = method;
            return true;
        }
        return false;
    });
    return foundMethod;
}

AbckitCoreFunction *GetSubclassMethod(AbckitCoreImportDescriptor *id, AbckitInst *inst, MethodInfo &methodInfo)
{
    if (g_dynG->iGetOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR) {
        return nullptr;
    }

    if (g_dynG->iGetImportDescriptor(inst) != id) {
        return nullptr;
    }

    AbckitCoreFunction *foundMethod = nullptr;
    EnumerateInstUsers(inst, [&](AbckitInst *user) {
        if (g_dynG->iGetOpcode(user) == ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER) {
            auto method = g_implG->iGetFunction(user);
            auto klass = g_implI->functionGetParentClass(method);
            foundMethod = GetMethodToModify(klass, methodInfo);
        }
    });

    return foundMethod;
}
void ModifyFunction(AbckitCoreFunction *method, AbckitCoreImportDescriptor *id, MethodInfo &methodInfo)
{
    EnumerateFunctionInsts(method, [&](AbckitInst *inst) {
        auto subclassMethod = GetSubclassMethod(id, inst, methodInfo);
        if (subclassMethod != nullptr) {
            AddParamChecker(subclassMethod);
        }
    });
}

void Modify(MethodInfo &methodInfo, AbckitFile *file)
{
    EnumerateModules(
        [&](AbckitCoreModule *mod) {
            AbckitCoreImportDescriptor *impDescriptor = GetImportDescriptor(mod, methodInfo);
            if (impDescriptor == nullptr) {
                return;
            }
            EnumerateModuleFunctions(mod, [&](AbckitCoreFunction *method) {
                ModifyFunction(method, impDescriptor, methodInfo);
                return true;
            });
        },
        file);
}

}  // namespace

namespace libabckit::test {

class AbckitScenarioCTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioCTestClean, LibAbcKitTestDynamicParameterCheckClean)
{
    constexpr const auto INPUT_PATH =
        ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/parameter_check/parameter_check.abc";
    constexpr const auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/parameter_check/parameter_check_modified.abc";
    const std::string entryPoint = "parameter_check";

    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_NE(file, nullptr);

    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, entryPoint);
    EXPECT_TRUE(helpers::Match(output,
                               "str1\n"
                               "str2\n"
                               "str3\n"
                               "undefined\n"
                               "str3\n"
                               "str2\n"
                               "str4\n"
                               "undefined\n"));

    MethodInfo method = {"modules/base", "Handler", "handle"};

    Modify(method, file);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    g_impl->closeFile(file);

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, entryPoint);
    EXPECT_TRUE(helpers::Match(output,
                               "str1\n"
                               "str2\n"
                               "str3\n"
                               "-1\n"
                               "str3\n"
                               "str2\n"
                               "str4\n"
                               "-1\n"));
}

}  // namespace libabckit::test
