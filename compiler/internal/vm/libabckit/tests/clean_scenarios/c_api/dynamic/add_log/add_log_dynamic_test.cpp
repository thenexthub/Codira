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
#include "ir_impl.h"
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
    CB cbFunc;
    std::function<void(AbckitCoreClass *)> cbClass;

    cbFunc = [&](AbckitCoreFunction *f) {
        cbUserFunc(f);
        g_implI->functionEnumerateNestedFunctions(f, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
            (*reinterpret_cast<CB *>(cb))(f);
            return true;
        });
        g_implI->functionEnumerateNestedClasses(f, &cbClass, [](AbckitCoreClass *c, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
            return true;
        });
    };

    cbClass = [&](AbckitCoreClass *c) {
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

}  // namespace

namespace libabckit::test {

class AbckitScenarioCTestClean : public ::testing::Test {};

struct UserData {
    AbckitString *print = nullptr;
    AbckitString *date = nullptr;
    AbckitString *getTime = nullptr;
    AbckitString *str = nullptr;
    AbckitString *consume = nullptr;
};

struct VisitData {
    UserData *ud = nullptr;
    AbckitInst *time = nullptr;
};

// CC-OFFNXT(G.FUN.01-CPP) huge function
static void CreateEpilog(AbckitInst *inst, AbckitBasicBlock *bb, UserData *userData, AbckitInst *time)
{
    auto *graph = g_implG->bbGetGraph(bb);
    while (inst != nullptr) {
        if (g_dynG->iGetOpcode(inst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED) {
            // Epilog
            auto *dateClass2 = g_dynG->iCreateTryldglobalbyname(graph, userData->date);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(dateClass2, inst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *dateObj2 = g_dynG->iCreateNewobjrange(graph, 1, dateClass2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertAfter(dateObj2, dateClass2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *getTime2 = g_dynG->iCreateLdobjbyname(graph, dateObj2, userData->getTime);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertAfter(getTime2, dateObj2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *time2 = g_dynG->iCreateCallthis0(graph, getTime2, dateObj2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertAfter(time2, getTime2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *consume = g_dynG->iCreateLoadString(graph, userData->consume);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertAfter(consume, time2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *print2 = g_dynG->iCreateTryldglobalbyname(graph, userData->print);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertAfter(print2, consume);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *callArg2 = g_dynG->iCreateCallarg1(graph, print2, consume);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertAfter(callArg2, print2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *sub = g_dynG->iCreateSub2(graph, time, time2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertAfter(sub, callArg2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *print3 = g_dynG->iCreateTryldglobalbyname(graph, userData->print);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertAfter(print3, sub);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *callArg3 = g_dynG->iCreateCallarg1(graph, print3, sub);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertAfter(callArg3, print3);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        }
        inst = g_implG->iGetNext(inst);
    }
}

static std::vector<AbckitBasicBlock *> BBgetSuccBlocks(AbckitBasicBlock *bb)
{
    std::vector<AbckitBasicBlock *> succBBs;
    g_implG->bbVisitSuccBlocks(bb, &succBBs, [](AbckitBasicBlock *succBasicBlock, void *d) {
        auto *succs = reinterpret_cast<std::vector<AbckitBasicBlock *> *>(d);
        succs->emplace_back(succBasicBlock);
        return true;
    });
    return succBBs;
}

static void TransformIr(AbckitGraph *graph, UserData *userData)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = BBgetSuccBlocks(startBB);
    auto *bb = succBBs[0];

    // Prolog
    auto *str = g_dynG->iCreateLoadString(graph, userData->str);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstFront(bb, str);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *print = g_dynG->iCreateTryldglobalbyname(graph, userData->print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(print, str);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *callArg = g_dynG->iCreateCallarg1(graph, print, str);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(callArg, print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *dateClass = g_dynG->iCreateTryldglobalbyname(graph, userData->date);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(dateClass, callArg);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *dateObj = g_dynG->iCreateNewobjrange(graph, 1, dateClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(dateObj, dateClass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *getTime = g_dynG->iCreateLdobjbyname(graph, dateObj, userData->getTime);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(getTime, dateObj);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *time = g_dynG->iCreateCallthis0(graph, getTime, dateObj);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(time, getTime);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    VisitData vd;
    vd.ud = userData;
    vd.time = time;

    g_implG->gVisitBlocksRpo(graph, &vd, [](AbckitBasicBlock *bb, void *data) {
        auto *inst = g_implG->bbGetFirstInst(bb);

        auto *visitData = reinterpret_cast<VisitData *>(data);
        CreateEpilog(inst, bb, visitData->ud, visitData->time);
        return true;
    });
}

static std::string GetMethodName(AbckitCoreFunction *method)
{
    auto mname = g_implI->functionGetName(method);
    std::string fullSig = g_implI->abckitStringToString(mname);
    auto fullName = fullSig.substr(0, fullSig.find(':'));
    return fullName;
}

// CC-OFFNXT(huge_method, C_RULE_ID_FUNCTION_SIZE) test, solid logic
// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioCTestClean, LibAbcKitTestDynamicAddLogClean)
{
    // ExecuteDynamicAbc is helper function needed for testing. Вoes not affect the logic of IR transformation
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/add_log/add_log_dynamic.abc",
                                             "add_log_dynamic");
    EXPECT_TRUE(helpers::Match(output, "abckit\n"));

    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/add_log/add_log_dynamic.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    UserData data = {};
    data.print = g_implM->createString(file, "print", strlen("print"));
    data.date = g_implM->createString(file, "Date", strlen("Date"));
    data.getTime = g_implM->createString(file, "getTime", strlen("getTime"));
    data.str = g_implM->createString(file, "file: src/MyClass, function: MyClass.handle",
                                     strlen("file: src/MyClass, function: MyClass.handle"));
    data.consume = g_implM->createString(file, "Ellapsed time:", strlen("Ellapsed time:"));

    AbckitCoreFunction *handleMethod;
    EnumerateAllMethods(file, [&](AbckitCoreFunction *method) {
        auto methodName = GetMethodName(method);
        if (methodName == "handle") {
            handleMethod = method;
        }
    });

    AbckitGraph *graph = g_implI->createGraphFromFunction(handleMethod);
    TransformIr(graph, &data);
    g_implM->functionSetGraph(handleMethod, graph);
    constexpr auto OUTPUT_PATH = ABCKIT_ABC_DIR "clean_scenarios/c_api/dynamic/add_log/add_log_dynamic_modified.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);

    output = helpers::ExecuteDynamicAbc(OUTPUT_PATH, "add_log_dynamic");
    EXPECT_TRUE(helpers::Match(output,
                               "file: src/MyClass, function: MyClass.handle\n"
                               "abckit\n"
                               "Ellapsed time:\n"
                               "\\d+\n"));
}

}  // namespace libabckit::test
