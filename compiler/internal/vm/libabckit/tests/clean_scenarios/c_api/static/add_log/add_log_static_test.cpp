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
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_static.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/src/logger.h"

#include <string>
#include <vector>
#include <functional>
#include "metadata_inspect_impl.h"

#include <gtest/gtest.h>
#include <cstddef>

// NOTE:
// * Printed filename is "NOTE", should be real name
// * Use actual stdlib calls (instead of user's DateGetTime, ConsoleLogNum, ConsoleLogStr)
// * There are several issues related to SaveState in this test:
//   * Start calls are inserted after first SaveState (not at the beginning of function)
//   * SaveStates are manipulated by user explicitly
//   * SaveStates are "INVALID" operations in validation schema

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

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

struct UserData {
    AbckitString *str1 = nullptr;
    AbckitString *str2 = nullptr;
    AbckitCoreFunction *consoleLogStr = nullptr;
    AbckitCoreFunction *consoleLogNum = nullptr;
    AbckitCoreFunction *dateGetTime = nullptr;
    AbckitCoreFunction *handle = nullptr;
    AbckitInst *startTime = nullptr;
};

void TransformIr(AbckitGraph *graph, UserData *userData)
{
    // find first inst with opcode ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC
    AbckitInst *callOp = nullptr;
    std::vector<AbckitBasicBlock *> bbs;
    g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
        reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
        return true;
    });
    for (auto *bb : bbs) {
        auto *curInst = g_implG->bbGetFirstInst(bb);
        while (curInst != nullptr) {
            if (g_statG->iGetOpcode(curInst) == ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC) {
                callOp = curInst;
            }
            curInst = g_implG->iGetNext(curInst);
        }
    }

    // console.log("file: FileName; function: FunctionName")
    AbckitInst *loadString = g_statG->iCreateLoadString(graph, userData->str1);
    g_implG->iInsertBefore(loadString, callOp);
    AbckitInst *log1 = g_statG->iCreateCallStatic(graph, userData->consoleLogStr, 1, loadString);
    g_implG->iInsertAfter(log1, loadString);

    // const start = Date().getTime()
    userData->startTime = g_statG->iCreateCallStatic(graph, userData->dateGetTime, 0);
    g_implG->iInsertAfter(userData->startTime, log1);

    g_implG->gVisitBlocksRpo(graph, userData, [](AbckitBasicBlock *bb, void *data) {
        auto *inst = g_implG->bbGetFirstInst(bb);
        auto *userData = reinterpret_cast<UserData *>(data);
        auto *graph = g_implG->bbGetGraph(bb);
        while (inst != nullptr) {
            if (g_statG->iGetOpcode(inst) == ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID) {
                // const end = Date().getTime()
                AbckitInst *endTime = g_statG->iCreateCallStatic(graph, userData->dateGetTime, 0);
                g_implG->iInsertBefore(endTime, inst);

                // console.log("Elapsed time:")
                auto *loadString = g_statG->iCreateLoadString(graph, userData->str2);
                g_implG->iInsertAfter(loadString, endTime);
                AbckitInst *log = g_statG->iCreateCallStatic(graph, userData->consoleLogStr, 1, loadString);
                g_implG->iInsertAfter(log, loadString);

                // console.log(end - start)
                AbckitInst *sub = g_statG->iCreateSub(graph, endTime, userData->startTime);
                g_implG->iInsertAfter(sub, log);
                AbckitInst *log2 = g_statG->iCreateCallStatic(graph, userData->consoleLogNum, 1, sub);
                g_implG->iInsertAfter(log2, sub);
            }
            inst = g_implG->iGetNext(inst);
        }
        return true;
    });
}

std::string GetMethodName(AbckitCoreFunction *method)
{
    auto mname = g_implI->functionGetName(method);
    std::string fullSig = g_implI->abckitStringToString(mname);
    auto fullName = fullSig.substr(0, fullSig.find(':'));
    return fullName;
}
}  // namespace

namespace libabckit::test {

class AbckitScenarioCTestClean : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(AbckitScenarioCTestClean, LibAbcKitTestStaticAddLogClean)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "clean_scenarios/c_api/static/add_log/add_log_static.abc",
                                            "add_log_static", "main");
    EXPECT_TRUE(helpers::Match(output, "buisiness logic...\n"));

    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "clean_scenarios/c_api/static/add_log/add_log_static.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    UserData userData;
    EnumerateAllMethods(file, [&](AbckitCoreFunction *method) {
        auto methodName = GetMethodName(method);
        if (methodName == "ConsoleLogStr") {
            userData.consoleLogStr = method;
        }
        if (methodName == "ConsoleLogNum") {
            userData.consoleLogNum = method;
        }
        if (methodName == "DateGetTime") {
            userData.dateGetTime = method;
        }
        if (methodName == "handle") {
            userData.handle = method;
        }
    });

    std::string startMsg;
    EnumerateAllMethods(file, [&](AbckitCoreFunction *method) {
        auto methodName = GetMethodName(method);
        if (methodName != "handle") {
            return;
        }
        startMsg = "file: NOTE; function: " + methodName;
        userData.str1 = g_implM->createString(file, startMsg.c_str(), startMsg.size());
        userData.str2 = g_implM->createString(file, "Elapsed time:", strlen("Elapsed time:"));

        AbckitGraph *graph = g_implI->createGraphFromFunction(method);
        TransformIr(graph, &userData);
        g_implM->functionSetGraph(method, graph);
        g_impl->destroyGraph(graph);
    });

    constexpr auto OUTPUT_PATH = ABCKIT_ABC_DIR "clean_scenarios/c_api/static/add_log/add_log_static_modified.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    g_impl->closeFile(file);

    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "add_log_static", "main");
    EXPECT_TRUE(helpers::Match(output,
                               "file: NOTE; function: handle\n"
                               "buisiness logic...\n"
                               "Elapsed time:\n"
                               "\\d+\n"));
}

}  // namespace libabckit::test
