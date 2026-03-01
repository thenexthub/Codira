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

#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "helpers/helpers.h"
#include "metadata_inspect_impl.h"

#include <functional>

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformMethod(AbckitCoreFunction *method)
{
    if (g_implI->moduleGetTarget(g_implI->functionGetModule(method)) == ABCKIT_TARGET_ARK_TS_V2) {
        auto *arktsMethod = g_implArkI->coreFunctionToArktsFunction(method);
        if (g_implArkI->functionIsNative(arktsMethod)) {
            return;
        }
    }
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::cout << "[function name]: " << g_implI->abckitStringToString((g_implI->functionGetName(method))) << std::endl;

    AbckitGraph *graph = g_implI->createGraphFromFunction(method);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::function<bool(AbckitBasicBlock *)> enumerateInsts = [&](AbckitBasicBlock *bb) -> bool {
        auto inst = g_implG->bbGetFirstInst(bb);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto instNum = g_implG->bbGetNumberOfInstructions(bb);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        for (int i = 0; i < instNum; ++i) {
            g_implG->iDump(inst, 1);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto isCallInst = g_implG->iCheckIsCall(inst);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::cout << "[isCallInst]: " << isCallInst << std::endl;
            inst = g_implG->iGetNext(inst);
            EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        }
        return true;
    };

    g_implG->gVisitBlocksRpo(graph, &enumerateInsts, [](AbckitBasicBlock *bb, void *cb) -> bool {
        return (*reinterpret_cast<std::function<bool(AbckitBasicBlock *)> *>(cb))(bb);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implM->functionSetGraph(method, graph);
    g_impl->destroyGraph(graph);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

namespace libabckit::test {

class AbckitRegressionTestCAPI : public ::testing::Test {};

static std::function<bool(AbckitCoreClass *)> g_cbClass;

static std::function<bool(AbckitCoreFunction *)> g_cbFunc = [](AbckitCoreFunction *f) -> bool {
    g_implI->functionEnumerateNestedFunctions(f, &g_cbFunc, [](AbckitCoreFunction *f, void *cb) {
        return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(f);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implI->functionEnumerateNestedClasses(f, &g_cbClass, [](AbckitCoreClass *c, void *cb) -> bool {
        return (*reinterpret_cast<std::function<bool(AbckitCoreClass *)> *>(cb))(c);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    TransformMethod(f);

    return true;
};

static std::function<void(AbckitCoreNamespace *)> g_cbNamespce = [](AbckitCoreNamespace *n) {
    g_implI->namespaceEnumerateNamespaces(n, &g_cbNamespce, [](AbckitCoreNamespace *n, void *cb) -> bool {
        return (*reinterpret_cast<std::function<bool(AbckitCoreNamespace *)> *>(cb))(n);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implI->namespaceEnumerateClasses(n, &g_cbClass, [](AbckitCoreClass *c, void *cb) -> bool {
        return (*reinterpret_cast<std::function<bool(AbckitCoreClass *)> *>(cb))(c);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implI->namespaceEnumerateTopLevelFunctions(n, (void *)&g_cbFunc, [](AbckitCoreFunction *f, void *cb) -> bool {
        return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(f);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
};

static std::function<void(AbckitCoreModule *)> g_cbModule = [](AbckitCoreModule *m) -> bool {
    g_implI->moduleEnumerateNamespaces(m, &g_cbNamespce, [](AbckitCoreNamespace *n, void *cb) {
        return (*reinterpret_cast<std::function<bool(AbckitCoreNamespace *)> *>(cb))(n);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implI->moduleEnumerateClasses(m, &g_cbClass, [](AbckitCoreClass *c, void *cb) {
        return (*reinterpret_cast<std::function<bool(AbckitCoreClass *)> *>(cb))(c);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implI->moduleEnumerateTopLevelFunctions(m, (void *)&g_cbFunc, [](AbckitCoreFunction *m, void *cb) {
        return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(m);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    return true;
};

// Test: test-kind=regression, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(AbckitRegressionTestCAPI, LibAbcKitTestIssueIB2T4M)
{
    std::string inputPath = ABCKIT_ABC_DIR "regression/issue_IB2T4M/issue_IB2T4M.abc";
    AbckitFile *file = g_impl->openAbc(inputPath.c_str(), inputPath.size());
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_cbClass = [](AbckitCoreClass *c) -> bool {
        g_implI->classEnumerateMethods(c, (void *)&g_cbFunc, [](AbckitCoreFunction *m, void *cb) -> bool {
            return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(m);
        });
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        return true;
    };

    g_implI->fileEnumerateModules(file, &g_cbModule, [](AbckitCoreModule *m, void *cb) -> bool {
        return (*reinterpret_cast<std::function<bool(AbckitCoreModule *)> *>(cb))(m);
    });
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->closeFile(file);
}

}  // namespace libabckit::test
