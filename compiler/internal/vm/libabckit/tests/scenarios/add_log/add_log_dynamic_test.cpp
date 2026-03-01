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

#include <gtest/gtest.h>

namespace libabckit::test {

class AbckitScenarioTest : public ::testing::Test {};

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

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

static void TransformIr(AbckitGraph *graph, UserData *userData)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
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

// CC-OFFNXT(G.FUN.01-CPP) test function
static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> GetCorrectBBSchema()
{
    // NOLINTBEGIN(readability-magic-numbers)
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
        {{{},
          {1},
          {
              {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
              {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
              {14, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
          }},
         {{0},
          {3, 2},
          {
              {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
              {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
              {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {4, 3}},
              {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
              {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE, {6}},
              {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {7}},
              {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {8, 7}},
              {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_LESS, {11, 10}},
              {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF, {12, 14}},
          }},
         {{1},
          {4},
          {
              {15, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
              {16, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
              {17, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {15, 16}},
          }},
         {{1},
          {4},
          {
              {18, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
              {19, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
              {20, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {18, 19}},
          }},
         {{2, 3},
          {5},
          {
              {24, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
              {25, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE, {24}},
              {26, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {25}},
              {27, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {26, 25}},
              {28, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
              {29, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
              {30, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {29, 28}},
              {31, ABCKIT_ISA_API_DYNAMIC_OPCODE_SUB2, {9, 27}},
              {32, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
              {33, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {32, 31}},
              {34, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
          }},
         {{4}, {}, {}}});
    return bbSchemas;
}
// CC-OFFNXT(huge_method, C_RULE_ID_FUNCTION_SIZE) test, solid logic
// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestDynamicAddLog)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "scenarios/add_log/add_log_dynamic.abc", "add_log_dynamic");
    EXPECT_TRUE(helpers::Match(output, "abckit\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "scenarios/add_log/add_log_dynamic.abc",
        ABCKIT_ABC_DIR "scenarios/add_log/add_log_dynamic_modified.abc", "handle",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            UserData data = {};
            data.print = g_implM->createString(file, "print", strlen("print"));
            data.date = g_implM->createString(file, "Date", strlen("Date"));
            data.getTime = g_implM->createString(file, "getTime", strlen("getTime"));
            data.str = g_implM->createString(file, "file: src/MyClass, function: MyClass.handle",
                                             strlen("file: src/MyClass, function: MyClass.handle"));
            data.consume = g_implM->createString(file, "Ellapsed time:", strlen("Ellapsed time:"));

            TransformIr(graph, &data);
        },
        [](AbckitGraph *graph) {
            // NOLINTEND(readability-magic-numbers)
            helpers::VerifyGraph(graph, GetCorrectBBSchema());
        });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "scenarios/add_log/add_log_dynamic_modified.abc", "add_log_dynamic");
    EXPECT_TRUE(helpers::Match(output,
                               "file: src/MyClass, function: MyClass.handle\n"
                               "abckit\n"
                               "Ellapsed time:\n"
                               "\\d+\n"));
}

}  // namespace libabckit::test
