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

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/abckit.h"
#include "logger.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)

namespace libabckit::test {

class LibAbcKitBasicBlocksDynTest : public ::testing::Test {};

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

}  // namespace

static void VerifyBbSchem1(AbckitGraph *graph)
{
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas = {
        {{},
         {1},
         {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
          {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
          {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
        {{0},
         {3, 2},
         {{6, ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATER, {5, 3}}, {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF, {6, 9}}}},
        {{1},
         {4},
         {{11, ABCKIT_ISA_API_DYNAMIC_OPCODE_LESS, {10, 3}},
          {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
          {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2, {10, 3}},
          {14, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {12, 13}}}},
        {{1},
         {4},
         {{15, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
          {16, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {15, 3}}}},
        {{2, 3}, {5}, {{20, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}}}},
        {{4}, {}, {}}};
    helpers::VerifyGraph(graph, bbSchemas);
}

// Test: test-kind=api, api=GraphApiImpl::gRunPassRemoveUnreachableBlocks, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitBasicBlocksDynTest, GrunPassRemoveUnreachableBlocks_1)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic.abc", "graph_basic_block_dynamic");
    EXPECT_TRUE(helpers::Match(output, "31\n51\n9\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic_modified.abc", "test",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *bb2 = g_implG->gGetBasicBlock(graph, 2);
            g_implG->iRemove(g_implG->bbGetLastInst(bb2));
            g_implG->bbDisconnectSuccBlock(bb2, 0x0);
            g_implG->gRunPassRemoveUnreachableBlocks(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) { VerifyBbSchem1(graph); });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic_modified.abc",
                                   "graph_basic_block_dynamic");
    EXPECT_TRUE(helpers::Match(output, "31\n41\n9\n"));
}

static void VerifyBbSchem2(AbckitGraph *graph)
{
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas = {
        {{},
         {1},
         {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
          {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
          {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
          {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
        {{0},
         {3, 2},
         {{5, ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATER, {4, 3}}, {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF, {5, 8}}}},
        {{1},
         {4},
         {{10, ABCKIT_ISA_API_DYNAMIC_OPCODE_LESS, {9, 3}},
          {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
          {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2, {12, 3}},
          {14, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {11, 13}}}},
        {{1},
         {4},
         {{15, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
          {16, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {15, 3}}}},
        {{2, 3}, {5}, {{20, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}}}},
        {{4}, {}, {}}};
    helpers::VerifyGraph(graph, bbSchemas);
}

// Test: test-kind=api, api=GraphApiImpl::gRunPassRemoveUnreachableBlocks, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitBasicBlocksDynTest, GrunPassRemoveUnreachableBlocks_2)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic.abc", "graph_basic_block_dynamic");
    EXPECT_TRUE(helpers::Match(output, "31\n51\n9\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic_modified.abc", "test",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *bb2 = g_implG->gGetBasicBlock(graph, 2);
            g_implG->iRemove(g_implG->bbGetLastInst(bb2));
            g_implG->bbDisconnectSuccBlock(bb2, 0x1);
            g_implG->gRunPassRemoveUnreachableBlocks(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) { VerifyBbSchem2(graph); });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic_modified.abc",
                                   "graph_basic_block_dynamic");
    EXPECT_TRUE(helpers::Match(output, "41\n51\n9\n"));
}

static void VerifyBbSchem3(AbckitGraph *graph)
{
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas = {
        {{},
         {1},
         {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
          {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
          {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
          {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
          {15, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
        {{0},
         {3, 2},
         {{5, ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATER, {4, 3}},
          {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_LESS, {6, 3}},
          {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF, {7, 10}}}},
        {{1},
         {4},
         {{11, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
          {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2, {6, 3}},
          {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {11, 12}}}},
        {{1},
         {4},
         {{14, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
          {16, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2, {15, 3}},
          {17, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {14, 16}}}},
        {{2, 3}, {5}, {{21, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}}}},
        {{4}, {}, {}}};
    helpers::VerifyGraph(graph, bbSchemas);
}

// Test: test-kind=api, api=GraphApiImpl::gRunPassRemoveUnreachableBlocks, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitBasicBlocksDynTest, GrunPassRemoveUnreachableBlocks_3)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic.abc", "graph_basic_block_dynamic");
    EXPECT_TRUE(helpers::Match(output, "31\n51\n9\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic_modified.abc", "test",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *bb0 = g_implG->gGetBasicBlock(graph, 0);
            g_implG->iRemove(g_implG->bbGetLastInst(bb0));
            g_implG->bbDisconnectSuccBlock(bb0, 0x0);
            g_implG->gRunPassRemoveUnreachableBlocks(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) { VerifyBbSchem3(graph); });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_dynamic_modified.abc",
                                   "graph_basic_block_dynamic");
    EXPECT_TRUE(helpers::Match(output, "31\n51\n29\n"));
}

static void VerifyGraphStart(AbckitFile *file)
{
    // Find transformed method
    AbckitCoreFunction *testMethod = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(testMethod, nullptr);

    // Get graph
    AbckitGraph *graph = g_implI->createGraphFromFunction(testMethod);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    ASSERT_NE(startBB, nullptr);

    // Verify bb predecessors
    ASSERT_EQ(helpers::BBgetPredBlocks(startBB).size(), 0);

    // Verify bb successors
    ASSERT_EQ(helpers::BBgetSuccBlocks(startBB).size(), 1);

    // Verify instructions
    auto *inst = g_implG->bbGetFirstInst(startBB);

    ASSERT_EQ(g_dynG->iGetOpcode(inst), ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(g_implG->iGetId(inst), 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->destroyGraph(graph);
}

// Test: test-kind=api, api=GraphApiImpl::gGetStartBasicBlock, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynTest, GgetStartBasicBlockValid)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_simple.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    LIBABCKIT_LOG(DEBUG) << "LibAbcKitBasicBlocksDynTest: " << INPUT_PATH << std::endl;

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    VerifyGraphStart(file);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void VerifyGraphEnd(AbckitFile *file)
{
    // Find transformed method
    AbckitCoreFunction *testMethod = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(testMethod, nullptr);

    // Get graph
    AbckitGraph *graph = g_implI->createGraphFromFunction(testMethod);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitBasicBlock *endBB = g_implG->gGetEndBasicBlock(graph);
    ASSERT_NE(endBB, nullptr);

    // Verify bb predecessors
    ASSERT_EQ(helpers::BBgetPredBlocks(endBB).size(), 1);

    // Verify bb successors
    ASSERT_EQ(helpers::BBgetSuccBlocks(endBB).size(), 0);

    // Verify instructions
    ASSERT_EQ(g_implG->bbGetFirstInst(endBB), nullptr);

    g_impl->destroyGraph(graph);
}

// Test: test-kind=api, api=GraphApiImpl::gGetEndBasicBlock, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynTest, GgetEndBasicBlockValid)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_simple.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    LIBABCKIT_LOG(DEBUG) << "LibAbcKitBasicBlocksDynTest: " << INPUT_PATH << std::endl;
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    VerifyGraphEnd(file);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void VerifyGraphVisitBlocks(AbckitFile *file)
{
    // Find transformed method
    AbckitCoreFunction *testMethod = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(testMethod, nullptr);

    // Get graph
    AbckitGraph *graph = g_implI->createGraphFromFunction(testMethod);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::vector<AbckitBasicBlock *> bbs;

    // Collect basic blocks
    g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
        reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
        return true;
    });

    ASSERT_EQ(bbs.size(), 3U);

    auto *startBB = bbs[0];

    // Verify bb predecessors
    ASSERT_EQ(helpers::BBgetPredBlocks(startBB).size(), 0);

    // Verify bb successors
    ASSERT_EQ(helpers::BBgetSuccBlocks(startBB).size(), 1);

    // Verify instructions
    auto *inst = g_implG->bbGetFirstInst(startBB);

    ASSERT_EQ(g_dynG->iGetOpcode(inst), ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
    ASSERT_EQ(g_implG->iGetId(inst), 0);

    startBB = bbs[1];

    // Verify bb predecessors
    ASSERT_EQ(helpers::BBgetPredBlocks(startBB).size(), 1);

    // Verify bb successors
    ASSERT_EQ(helpers::BBgetSuccBlocks(startBB).size(), 1);

    // Verify instructions
    inst = g_implG->bbGetFirstInst(startBB);

    ASSERT_EQ(g_dynG->iGetOpcode(inst), ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_EQ(g_implG->iGetId(inst), 4U);

    startBB = bbs[2U];
    ASSERT_NE(startBB, nullptr);

    // Verify bb predecessors
    ASSERT_EQ(helpers::BBgetPredBlocks(startBB).size(), 1);

    // Verify bb successors
    ASSERT_EQ(helpers::BBgetSuccBlocks(startBB).size(), 0);

    // Verify instructions
    inst = g_implG->bbGetFirstInst(startBB);
    ASSERT_EQ(inst, nullptr);

    g_impl->destroyGraph(graph);
}

// Test: test-kind=api, api=GraphApiImpl::gVisitBlocksRpo, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynTest, GvisitBlocksRPO)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block_simple.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    LIBABCKIT_LOG(DEBUG) << "LibAbcKitBasicBlocksDynTest: " << INPUT_PATH << std::endl;

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    VerifyGraphVisitBlocks(file);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
