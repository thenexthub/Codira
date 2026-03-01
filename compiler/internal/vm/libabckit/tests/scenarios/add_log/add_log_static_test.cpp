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

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>
#include <cstddef>

// NOTE:
// * Printed filename is "NOTE", should be real name
// * Use actual stdlib calls (instead of user's DateGetTime, ConsoleLogNum, ConsoleLogStr)
// * There are several issues related to SaveState in this test:
//   * Start calls are inserted after first SaveState (not at the beginning of function)
//   * SaveStates are manipulated by user explicitly
//   * SaveStates are "INVALID" operations in validation schema

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

struct UserData {
    AbckitString *str1 = nullptr;
    AbckitString *str2 = nullptr;
    AbckitCoreFunction *consoleLogStr = nullptr;
    AbckitCoreFunction *consoleLogNum = nullptr;
    AbckitCoreFunction *dateGetTime = nullptr;
    AbckitInst *startTime = nullptr;
};

static void TransformIr(AbckitGraph *graph, UserData *userData)
{
    AbckitInst *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);

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

class AbckitScenarioTest : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestStaticAddLog)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "scenarios/add_log/add_log_static.abc", "add_log_static", "main");
    EXPECT_TRUE(helpers::Match(output, "buisiness logic...\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "scenarios/add_log/add_log_static.abc",
        ABCKIT_ABC_DIR "scenarios/add_log/add_log_static_modified.abc", "handle",
        []([[maybe_unused]] AbckitFile *file, [[maybe_unused]] AbckitCoreFunction *method,
           [[maybe_unused]] AbckitGraph *graph) {
            UserData userData;
            userData.consoleLogStr = helpers::FindMethodByName(file, "ConsoleLogStr");
            userData.consoleLogNum = helpers::FindMethodByName(file, "ConsoleLogNum");
            userData.dateGetTime = helpers::FindMethodByName(file, "DateGetTime");
            auto mname = g_implI->functionGetName(method);
            auto methodName = helpers::GetCropFuncName(helpers::AbckitStringToString(mname).data());
            std::string startMsg = "file: NOTE; function: " + methodName;
            userData.str1 = g_implM->createString(file, startMsg.c_str(), startMsg.size());
            userData.str2 = g_implM->createString(file, "Elapsed time:", strlen("Elapsed time:"));

            TransformIr(graph, &userData);
        },
        [](AbckitGraph *graph) {
            // NOLINTBEGIN(readability-magic-numbers)
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{}, {1}, {{0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}}},
                 {{0},
                  {2},
                  {
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                      {5, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                      {8, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {5}},
                      {11, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {}},
                      {14, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {3}},
                      {17, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {}},
                      {19, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                      {22, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {19}},
                      {23, ABCKIT_ISA_API_STATIC_OPCODE_SUB, {17, 11}},
                      {26, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {23}},
                      {27, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            // NOLINTEND(readability-magic-numbers)
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "scenarios/add_log/add_log_static_modified.abc", "add_log_static",
                                       "main");
    EXPECT_TRUE(helpers::Match(output,
                               "file: NOTE; function: handle\n"
                               "buisiness logic...\n"
                               "Elapsed time:\n"
                               "\\d+\n"));
}

}  // namespace libabckit::test
