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

#include "libabckit/c/abckit.h"
#include "adapter_static/ir_static.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/src/logger.h"

#include <gtest/gtest.h>

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

using CB = std::function<void(AbckitCoreFunction *)>;

void EnumerateAllMethodsInModule(AbckitFile *file, std::function<void(AbckitCoreNamespace *)> &cbNamespace,
                                 std::function<void(AbckitCoreClass *)> &cbClass,
                                 std::function<void(AbckitCoreFunction *)> &cbFunc)
{
    std::function<void(AbckitCoreModule *)> cbModule = [&](AbckitCoreModule *m) {
        g_implI->moduleEnumerateNamespaces(m, &cbNamespace, [](AbckitCoreNamespace *n, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
            return true;
        });
        g_implI->moduleEnumerateClasses(m, &cbClass, [](AbckitCoreClass *c, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
            return true;
        });
        g_implI->moduleEnumerateTopLevelFunctions(m, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
            return true;
        });
    };

    g_implI->fileEnumerateModules(file, &cbModule, [](AbckitCoreModule *m, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreModule *)> *>(cb))(m);
        return true;
    });
}

void EnumerateAllMethods(AbckitFile *file, const CB &cbUserFunc)
{
    CB cbFunc = [&](AbckitCoreFunction *f) {
        cbUserFunc(f);
        g_implI->functionEnumerateNestedFunctions(f, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
            (*reinterpret_cast<CB *>(cb))(m);
            return true;
        });
    };

    std::function<void(AbckitCoreClass *)> cbClass = [&](AbckitCoreClass *c) {
        g_implI->classEnumerateMethods(c, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
            (*reinterpret_cast<CB *>(cb))(m);
            return true;
        });
    };

    std::function<void(AbckitCoreNamespace *)> cbNamespace = [&](AbckitCoreNamespace *n) {
        g_implI->namespaceEnumerateNamespaces(n, &cbNamespace, [](AbckitCoreNamespace *n, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
            return true;
        });
        g_implI->namespaceEnumerateClasses(n, &cbClass, [](AbckitCoreClass *c, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
            return true;
        });
        g_implI->namespaceEnumerateTopLevelFunctions(n, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
            (*reinterpret_cast<CB *>(cb))(f);
            return true;
        });
    };

    EnumerateAllMethodsInModule(file, cbNamespace, cbClass, cbFunc);
}

std::string GetMethodName(AbckitCoreFunction *method)
{
    auto mname = g_implI->functionGetName(method);
    std::string fullSig = g_implI->abckitStringToString(mname);
    auto fullName = fullSig.substr(0, fullSig.find(':'));
    return fullName;
}

struct UserData {
    AbckitString *print = nullptr;
};

std::vector<AbckitBasicBlock *> BBgetPredBlocks(AbckitBasicBlock *bb)
{
    std::vector<AbckitBasicBlock *> predBBs;
    g_implG->bbVisitPredBlocks(bb, &predBBs, [](AbckitBasicBlock *succBasicBlock, void *d) {
        auto *preds = reinterpret_cast<std::vector<AbckitBasicBlock *> *>(d);
        preds->emplace_back(succBasicBlock);
        return true;
    });
    return predBBs;
}

std::vector<AbckitBasicBlock *> BBgetSuccBlocks(AbckitBasicBlock *bb)
{
    std::vector<AbckitBasicBlock *> succBBs;
    g_implG->bbVisitSuccBlocks(bb, &succBBs, [](AbckitBasicBlock *succBasicBlock, void *d) {
        auto *succs = reinterpret_cast<std::vector<AbckitBasicBlock *> *>(d);
        succs->emplace_back(succBasicBlock);
        return true;
    });
    return succBBs;
}

void TransformIr(AbckitGraph *graph, UserData *userData)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    AbckitBasicBlock *endBB = g_implG->gGetEndBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = BBgetSuccBlocks(startBB);
    AbckitBasicBlock *bb = succBBs[0];
    AbckitInst *initInst = g_implG->bbGetFirstInst(bb);
    AbckitInst *prevRetInst = g_implG->iGetPrev(g_implG->bbGetLastInst(bb));

    AbckitBasicBlock *tryBegin =
        g_implG->bbSplitBlockAfterInstruction(g_implG->iGetBasicBlock(initInst), initInst, true);
    AbckitBasicBlock *tryEnd =
        g_implG->bbSplitBlockAfterInstruction(g_implG->iGetBasicBlock(prevRetInst), prevRetInst, true);

    // Fill catchBlock
    AbckitBasicBlock *catchBlock = g_implG->bbCreateEmpty(graph);
    AbckitInst *catchPhi = g_implG->bbCreateCatchPhi(catchBlock, 0);
    AbckitInst *print = g_dynG->iCreateTryldglobalbyname(graph, userData->print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstBack(catchBlock, print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitInst *callArg = g_dynG->iCreateCallarg1(graph, print, catchPhi);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstBack(catchBlock, callArg);

    AbckitBasicBlock *epilogueBB = BBgetPredBlocks(endBB)[0];
    AbckitInst *retInst = g_implG->bbGetLastInst(epilogueBB);
    AbckitInst *firstPhiInput = g_implG->iGetInput(retInst, 0);
    AbckitInst *secondPhiInput = g_dynG->iCreateLdfalse(graph);
    g_implG->bbAddInstBack(bb, secondPhiInput);
    AbckitInst *phiInst = g_implG->bbCreatePhi(epilogueBB, 2, firstPhiInput, secondPhiInput);
    g_implG->iSetInput(retInst, phiInst, 0);

    g_implG->gInsertTryCatch(tryBegin, tryEnd, catchBlock, catchBlock);
}
}  // namespace

namespace libabckit::test {

class AbckitScenarioCTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioCTestClean, LibAbcKitTestDynamicAddTryCatchClean)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/add_try_catch/add_try_catch.abc";
    auto output = helpers::ExecuteDynamicAbc(INPUT_PATH, "add_try_catch");
    EXPECT_TRUE(helpers::Match(output, "THROW\n"));

    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitCoreFunction *runMethod;
    EnumerateAllMethods(file, [&](AbckitCoreFunction *method) {
        auto methodName = GetMethodName(method);
        if (methodName == "run") {
            runMethod = method;
        }
    });

    AbckitGraph *graph = g_implI->createGraphFromFunction(runMethod);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    UserData uData {};
    uData.print = g_implM->createString(file, "print", strlen("print"));
    TransformIr(graph, &uData);

    g_implM->functionSetGraph(runMethod, graph);
    g_impl->destroyGraph(graph);
    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/add_try_catch/add_try_catch_modified.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, "add_try_catch");
    EXPECT_TRUE(helpers::Match(output,
                               "THROW\n"
                               "Error: DUMMY_ERROR\n"
                               "false\n"));
}

}  // namespace libabckit::test
