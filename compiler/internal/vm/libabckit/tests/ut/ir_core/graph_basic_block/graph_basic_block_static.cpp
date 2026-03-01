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
#include "libabckit/c/abckit.h"
#include "logger.h"

#include <gtest/gtest.h>

namespace libabckit::test {

class LibAbcKitBasicBlocksTest : public ::testing::Test {};

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

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

    ASSERT_EQ(g_statG->iGetOpcode(inst), ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);
    ASSERT_EQ(g_implG->iGetId(inst), 0);

    g_impl->destroyGraph(graph);
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

    ASSERT_EQ(g_statG->iGetOpcode(inst), ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);
    ASSERT_EQ(g_implG->iGetId(inst), 0);

    startBB = bbs[1];

    // Verify bb predecessors
    ASSERT_EQ(helpers::BBgetPredBlocks(startBB).size(), 1);

    // Verify bb successors
    ASSERT_EQ(helpers::BBgetSuccBlocks(startBB).size(), 1);

    // Verify instructions
    inst = g_implG->bbGetFirstInst(startBB);

    ASSERT_EQ(g_statG->iGetOpcode(inst), ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_EQ(g_implG->iGetId(inst), 2U);

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

// Test: test-kind=api, api=GraphApiImpl::gGetStartBasicBlock, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, GgetStartBasicBlockValid)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    LIBABCKIT_LOG(DEBUG) << "LibAbcKitTestStaticGgetStartBasicBlock: " << INPUT_PATH << std::endl;

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    VerifyGraphStart(file);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=GraphApiImpl::gGetEndBasicBlock, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, GgetEndBasicBlockValid)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    LIBABCKIT_LOG(DEBUG) << "LibAbcKitTestStaticGgetEndBasicBlock: " << INPUT_PATH << std::endl;
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    VerifyGraphEnd(file);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=GraphApiImpl::gVisitBlocksRpo, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, GvisitBlocksRPO)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/graph_basic_block/graph_basic_block.abc";
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    LIBABCKIT_LOG(DEBUG) << "LibAbcKitTestStaticGgetEndBasicBlock: " << INPUT_PATH << std::endl;

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    VerifyGraphVisitBlocks(file);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
