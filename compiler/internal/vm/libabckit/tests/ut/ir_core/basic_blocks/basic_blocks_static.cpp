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
#include "libabckit/c/statuses.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_static.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>

// NOLINTBEGIN(readability-magic-numbers)

namespace libabckit::test {
static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static.abc";

class LibAbcKitBasicBlocksTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::bbCreateEmpty, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBcreateEmptyBlock_1)
{
    helpers::TransformMethod(
        INPUT_PATH, ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static_modified.abc", "test",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *main = helpers::BBgetSuccBlocks(start)[0];
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *bb = g_implG->bbCreateEmpty(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->bbDisconnectSuccBlock(start, 0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbInsertSuccBlock(start, bb, 0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbInsertSuccBlock(bb, main, 0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{}, {1}, {{1, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}}},
                 {{0}, {2}, {}},
                 {{1},
                  {3},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{2}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::bbDisconnectSuccBlock, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBdisconnectSuccBlock)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *next = helpers::BBgetSuccBlocks(start)[0];
        g_implG->bbDisconnectSuccBlock(start, 0);
        ASSERT_TRUE(helpers::BBgetSuccBlocks(start).empty());
        ASSERT_TRUE(helpers::BBgetPredBlocks(next).empty());
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        g_implG->bbAppendSuccBlock(start, next);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
            {{{}, {1}, {{3, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}}},
             {{0},
              {2},
              {
                  {2, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
              }},
             {{1}, {}, {}}});
        // CC-OFFNXT(G.FMT.02)
        helpers::VerifyGraph(graph, bbSchemas);
    };
    helpers::InspectMethod(INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetGraph, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetGraph_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbCreateEmpty(graph);
        auto *end = g_implG->gGetEndBasicBlock(graph);

        g_implG->bbAppendSuccBlock(start, bb);
        g_implG->bbAppendSuccBlock(bb, end);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
            {{{}, {2, 1}, {{3, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}}},
             {{0}, {3}, {}},
             {{0},
              {3},
              {
                  {2, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
              }},
             {{2, 1}, {}, {}}});
        helpers::VerifyGraph(graph, bbSchemas);
        ASSERT_EQ(graph, graph);
    };
    helpers::InspectMethod(INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetFirstInst, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetFirstInst_2)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];
        auto *inst = g_implG->bbGetFirstInst(bb);

        ASSERT_EQ(g_statG->iGetOpcode(inst), ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    };
    helpers::InspectMethod(INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetLastInst, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetLastInst_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];
        auto *inst = g_implG->bbGetLastInst(bb);

        ASSERT_EQ(g_statG->iGetOpcode(inst), ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    };
    helpers::InspectMethod(INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetTrueBranch, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetTrueBranch_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *cmpB = helpers::BBgetSuccBlocks(start)[0];
        auto *trueB = g_implG->bbGetTrueBranch(cmpB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto *inst = g_implG->bbGetFirstInst(trueB);

        ASSERT_NE(inst, nullptr);
        ASSERT_EQ(g_statG->iGetOpcode(inst), ABCKIT_ISA_API_STATIC_OPCODE_RETURN);

        std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
            {{{},
              {1},
              {
                  {9, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                  {1, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                  {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  {5, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  {7, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
              }},
             {{0},
              {3, 2},
              {
                  {4, ABCKIT_ISA_API_STATIC_OPCODE_IF, {3, 1}},
              }},
             {{1},
              {4},
              {
                  {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {5}},
              }},
             {{1},
              {4},
              {
                  {8, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {7}},
              }},
             {{2, 3}, {}, {}}});
        helpers::VerifyGraph(graph, bbSchemas);
    };
    helpers::InspectMethod(INPUT_PATH, "test2", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetFalseBranch, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetFalseBranch_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *cmpB = helpers::BBgetSuccBlocks(start)[0];
        auto *trueB = g_implG->bbGetFalseBranch(cmpB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto *inst = g_implG->bbGetFirstInst(trueB);

        ASSERT_NE(inst, nullptr);
        ASSERT_EQ(g_statG->iGetOpcode(inst), ABCKIT_ISA_API_STATIC_OPCODE_RETURN);

        std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
            {{{},
              {1},
              {
                  {9, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                  {1, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                  {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  {5, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  {7, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
              }},
             {{0},
              {3, 2},
              {
                  {4, ABCKIT_ISA_API_STATIC_OPCODE_IF, {3, 1}},
              }},
             {{1},
              {4},
              {
                  {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {5}},
              }},
             {{1},
              {4},
              {
                  {8, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {7}},
              }},
             {{2, 3}, {}, {}}});
        helpers::VerifyGraph(graph, bbSchemas);
    };
    helpers::InspectMethod(INPUT_PATH, "test2", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbAddInstFront, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBaddInstFront_4)
{
    helpers::TransformMethod(
        INPUT_PATH, ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static_modified.abc", "test",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *bb = helpers::BBgetSuccBlocks(start)[0];
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT);
            auto *newInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
            auto *negInst = g_statG->iCreateNeg(graph, newInst);
            g_implG->bbAddInstFront(bb, negInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{5, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}, {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_NEG, {3}},
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(inst, g_implG->iGetNext(newInst));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::bbAddInstBack, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBaddInstBack_2)
{
    helpers::TransformMethod(
        INPUT_PATH, ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static_modified.abc", "test",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *bb = helpers::BBgetSuccBlocks(start)[0];
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT);
            auto *newInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
            auto *negInst = g_statG->iCreateNeg(graph, newInst);

            g_implG->bbDisconnectSuccBlock(start, 0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *empty = g_implG->bbCreateEmpty(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAppendSuccBlock(start, empty);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAppendSuccBlock(empty, bb);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAddInstBack(empty, negInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{5, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}, {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_NEG, {3}},
                  }},
                 {{1},
                  {3},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{2}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(inst, g_implG->iGetNext(newInst));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::bbAddInstBack, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBaddInstBack_4)
{
    helpers::TransformMethod(
        INPUT_PATH, ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static_modified.abc", "test",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *newIns = g_implG->gFindOrCreateConstantI64(graph, 1);

            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{4, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}, {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(newIns, g_implG->bbGetLastInst(start));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::bbVisitSuccBlocks, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBvisitSuccBlocks_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];

        uint32_t counter = 0;
        auto cb = [](AbckitBasicBlock *, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        };
        g_implG->bbVisitSuccBlocks(bb, &counter, cb);
        // CC-OFFNXT(G.FMT.02)
        ASSERT_EQ(counter, 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };
    helpers::InspectMethod(INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbVisitPredBlocks, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBvisitPredBlocks_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];

        uint32_t counter = 0;
        auto cb = [](AbckitBasicBlock *, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        };
        g_implG->bbVisitPredBlocks(bb, &counter, cb);

        ASSERT_EQ(counter, 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };

    helpers::InspectMethod(INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbVisitDominatedBlocks, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBvisitDominatedBlocks_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];

        uint32_t counter = 0;
        auto cb = [](AbckitBasicBlock *, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        };
        g_implG->bbVisitDominatedBlocks(bb, &counter, cb);

        ASSERT_EQ(counter, 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetSuccBlock, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetSuccBlock_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);

        ASSERT_EQ(bb, helpers::BBgetSuccBlocks(start)[0]);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsStart, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBisStart_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);

        ASSERT_TRUE(g_implG->bbIsStart(start));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsEnd, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBisEnd_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *end = g_implG->gGetEndBasicBlock(graph);

        ASSERT_TRUE(g_implG->bbIsEnd(end));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetImmediateDominator, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetImmediateDominator_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];
        g_implG->bbGetImmediateDominator(bb);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetImmediateDominator, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetImmediateDominator_2)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto bb = g_implG->bbGetImmediateDominator(start);
        ASSERT_EQ(bb, nullptr);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetSuccBlockCount, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetSuccBlockCount_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);

        ASSERT_EQ(g_implG->bbGetSuccBlockCount(start), 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetPredBlockCount, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetPredBlockCount_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];

        ASSERT_EQ(g_implG->bbGetPredBlockCount(bb), 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbDump, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBdump_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        LIBABCKIT_LOG_DUMP(g_implG->bbDump(start, 0), DEBUG);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetNumberOfInstructions, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetNumberOfInstructions_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];
        ASSERT_EQ(g_implG->bbGetNumberOfInstructions(bb), 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetBasicBlock, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, GgetBasicBlock_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *bb = g_implG->gGetBasicBlock(graph, 1);
        ASSERT_EQ(bb, g_implG->gGetStartBasicBlock(graph));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetBasicBlock, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitBasicBlocksTest, GgetBasicBlockBigId)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto bbVectorSize = g_implG->gGetNumberOfBasicBlocks(graph);
        auto *bb = g_implG->gGetBasicBlock(graph, bbVectorSize + 1);
        ASSERT_EQ(bb, nullptr);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetParameter, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, GgetParameter_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *inst = g_implG->bbGetFirstInst(start);
        ASSERT_EQ(inst, g_implG->gGetParameter(graph, 0));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetParameter, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitBasicBlocksTest, GgetParameterBigId)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto size = g_implG->gGetNumberOfParameters(graph);
        auto *inst = g_implG->gGetParameter(graph, size + 1);
        ASSERT_EQ(inst, nullptr);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetNumberOfParameters, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, GgetNumberOfParameters)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        uint32_t num = g_implG->gGetNumberOfParameters(graph);
        ASSERT_EQ(num, 2U);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetNumberOfBasicBlocks, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, GgetNumberOfBasicBlocks_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        g_implG->gGetNumberOfBasicBlocks(graph);
        ASSERT_EQ(g_implG->gGetNumberOfBasicBlocks(graph), 5U);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetId, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetId_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_implG->bbGetId(start), 3U);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetPredBlock, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBgetPredBlock_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0U);
        ASSERT_EQ(g_implG->bbGetPredBlock(bb, 0U), helpers::BBgetPredBlocks(bb)[0U]);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbInsertSuccBlock, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBinsertSuccBlock_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0U);
        auto *empty = g_implG->bbCreateEmpty(graph);
        g_implG->bbInsertSuccBlock(bb, empty, 2U);
        ASSERT_EQ(g_implG->bbGetSuccBlock(bb, 2U), helpers::BBgetSuccBlocks(bb)[2U]);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        g_implG->bbDisconnectSuccBlock(bb, 2U);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbAppendSuccBlock, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBappendSuccBlock_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0U);
        auto *empty = g_implG->bbCreateEmpty(graph);
        g_implG->bbAppendSuccBlock(bb, empty);
        ASSERT_EQ(g_implG->bbGetSuccBlock(bb, 2U), helpers::BBgetSuccBlocks(bb)[2U]);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        g_implG->bbDisconnectSuccBlock(bb, g_implG->bbGetSuccBlockCount(bb) - 1U);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbRemoveAllInsts, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, bbRemoveAllInsts_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *empty = g_implG->bbCreateEmpty(graph);
        auto *constInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
        auto *newInst = g_statG->iCreateNeg(graph, constInst);
        g_implG->bbAddInstFront(empty, newInst);
        ASSERT_EQ(g_implG->bbGetNumberOfInstructions(empty), 1);
        g_implG->bbRemoveAllInsts(empty);
        ASSERT_EQ(g_implG->bbGetNumberOfInstructions(empty), 0);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsLoopPrehead, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBisLoopPrehead_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test4", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        ASSERT_TRUE(g_implG->bbIsLoopPrehead(bb));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsLoopHead, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBisLoopHead_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test4", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        auto *bb2 = g_implG->bbGetSuccBlock(bb, 0);
        ASSERT_TRUE(g_implG->bbIsLoopHead(bb2));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbSplitBlockAfterInstruction, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBsplitBlockAfterInstruction_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        auto *bb2 = g_implG->bbGetSuccBlock(bb, 0);
        auto *inst = g_implG->bbGetFirstInst(bb2);
        ASSERT_EQ(g_implG->gGetNumberOfBasicBlocks(graph), 5U);
        auto *newBb = g_implG->bbSplitBlockAfterInstruction(bb2, inst, false);
        ASSERT_EQ(g_implG->gGetNumberOfBasicBlocks(graph), 6U);
        g_implG->bbAppendSuccBlock(bb2, newBb);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbCheckDominance, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBcheckDominance_1)
{
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        ASSERT_TRUE(g_implG->bbCheckDominance(bb, start));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbVisitSuccBlocks, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBvisitSuccBlocksStatic_2)
{
    auto output = helpers::ExecuteStaticAbc(INPUT_PATH, "basic_blocks_static", "visit_call");
    EXPECT_TRUE(helpers::Match(output, "0\n1\n"));
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        AbckitBasicBlock *ifBB = g_implG->bbGetSuccBlock(startBB, 0);
        std::vector<AbckitBasicBlock *> succBBs;
        struct VisitData {
            const AbckitIsaApiStatic *gStatG;
            const AbckitGraphApi *implG;
            AbckitGraph *graph;
            int newPrintInt;
        } data {g_statG, g_implG, graph, 42};
        g_implG->bbVisitSuccBlocks(ifBB, &data, [](AbckitBasicBlock *succBasicBlock, void *d) -> bool {
            auto *gStatG = static_cast<struct VisitData *>(d)->gStatG;
            auto *gImplG = static_cast<struct VisitData *>(d)->implG;
            auto *graph = static_cast<struct VisitData *>(d)->graph;
            int newPrintInt = static_cast<struct VisitData *>(d)->newPrintInt++;
            auto *curInst = gImplG->bbGetFirstInst(succBasicBlock);
            while (curInst != nullptr && gStatG->iGetOpcode(curInst) != ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC) {
                curInst = gImplG->iGetNext(curInst);
            }
            if (curInst == nullptr) {
                return false;
            }
            gImplG->iSetInput(curInst, gImplG->gFindOrCreateConstantI32(graph, newPrintInt), 1);
            return true;
        });
    };
    auto abcIn = INPUT_PATH;
    auto abcOut = ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static_visit.abc";
    helpers::TransformMethod(abcIn, abcOut, "visit_succ_blocks", cb);
    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static_visit.abc",
                                       "basic_blocks_static", "visit_call");
    EXPECT_TRUE(helpers::Match(output, "43\n42\n"));
}

// Test: test-kind=api, api=GraphApiImpl::bbIsTryBegin, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBisTryBegin)
{
    helpers::InspectMethod(INPUT_PATH, "test3", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *startBB = g_implG->gGetStartBasicBlock(graph);
        AbckitBasicBlock *tryBegin = g_implG->bbGetSuccBlock(startBB, 0);
        ASSERT_TRUE(g_implG->bbIsTryBegin(tryBegin));
    });
}

// Test: test-kind=api, api=GraphApiImpl::bbIsTryEnd, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBisTryEnd)
{
    helpers::InspectMethod(INPUT_PATH, "test3", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *endBB = g_implG->gGetEndBasicBlock(graph);
        AbckitBasicBlock *tryEnd = g_implG->bbGetPredBlock(g_implG->bbGetPredBlock(endBB, 2), 0);
        ASSERT_TRUE(g_implG->bbIsTryEnd(tryEnd));
    });
}

// Test: test-kind=api, api=GraphApiImpl::bbIsCatchBegin, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBisCatchBegin)
{
    helpers::InspectMethod(INPUT_PATH, "test3", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *startBB = g_implG->gGetStartBasicBlock(graph);
        AbckitBasicBlock *tryBegin = g_implG->bbGetSuccBlock(startBB, 0);
        AbckitBasicBlock *catchBegin = g_implG->bbGetSuccBlock(tryBegin, 1);
        ASSERT_TRUE(g_implG->bbIsCatchBegin(catchBegin));
    });
}

// Test: test-kind=api, api=GraphApiImpl::bbIsCatch, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBisCatch)
{
    helpers::InspectMethod(INPUT_PATH, "test3", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *endBB = g_implG->gGetEndBasicBlock(graph);
        AbckitBasicBlock *catchBB = g_implG->bbGetPredBlock(endBB, 1);
        ASSERT_TRUE(g_implG->bbIsCatch(catchBB));
    });
}

// Test: test-kind=api, api=GraphApiImpl::bbIsTry, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksTest, BBisTry)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *startBB = g_implG->gGetStartBasicBlock(graph);
        AbckitBasicBlock *tryBB = g_implG->bbGetSuccBlock(g_implG->bbGetSuccBlock(startBB, 0), 0);
        // CC-OFFNXT(G.FMT.02)
        ASSERT_TRUE(g_implG->bbIsTry(tryBB));
    };
    helpers::InspectMethod(INPUT_PATH, "test3", cb);
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
