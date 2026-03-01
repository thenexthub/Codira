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
#include "libabckit/c/isa/isa_dynamic.h"
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

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
}  // namespace

static constexpr auto CREATE_EMPTY_INPUT_PATH =
    ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/create_empty_dynamic.abc";

class LibAbcKitBasicBlocksDynamicTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::bbCreateEmpty, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBcreateEmptyBlock_1)
{
    helpers::TransformMethod(
        CREATE_EMPTY_INPUT_PATH, ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/create_empty_dynamic_modified.abc",
        "test",
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

            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
                 {{0}, {2}, {}},
                 {{1},
                  {3},
                  {
                      {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
                  }},
                 {{2}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::bbDisconnectSuccBlock, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBdisconnectSuccBlock)
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
        std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
            {{{},
              {1},
              {{4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
               {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
               {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
             {{0},
              {2},
              {
                  {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
              }},
             {{1}, {}, {}}});
        // CC-OFFNXT(G.FMT.02)
        helpers::VerifyGraph(graph, bbSchemas);
    };
    helpers::InspectMethod(CREATE_EMPTY_INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetGraph, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetGraph_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbCreateEmpty(graph);
        auto *end = g_implG->gGetEndBasicBlock(graph);

        g_implG->bbAppendSuccBlock(start, bb);
        g_implG->bbAppendSuccBlock(bb, end);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
            {{{},
              {2, 1},
              {{4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
               {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
               {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
             {{0}, {3}, {}},
             {{0},
              {3},
              {
                  {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
              }},
             {{2, 1}, {}, {}}});
        helpers::VerifyGraph(graph, bbSchemas);
        ASSERT_EQ(graph, graph);
    };
    helpers::InspectMethod(CREATE_EMPTY_INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetFirstInst, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetFirstInst_2)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];
        auto *inst = g_implG->bbGetFirstInst(bb);
        ASSERT_EQ(g_dynG->iGetOpcode(inst), ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
    };
    helpers::InspectMethod(CREATE_EMPTY_INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetLastInst, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetLastInst_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];
        auto *inst = g_implG->bbGetLastInst(bb);
        (void)inst;
        // CC-OFFNXT(G.FMT.02)
        ASSERT_EQ(g_dynG->iGetOpcode(inst), ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
    };
    helpers::InspectMethod(CREATE_EMPTY_INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetTrueBranch, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetTrueBranch_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *cmpB = helpers::BBgetSuccBlocks(start)[0];
        auto *trueB = g_implG->bbGetTrueBranch(cmpB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto *inst = g_implG->bbGetFirstInst(trueB);

        ASSERT_NE(inst, nullptr);
        ASSERT_EQ(g_dynG->iGetOpcode(inst), ABCKIT_ISA_API_DYNAMIC_OPCODE_NEG);

        std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
            {{{},
              {1},
              {
                  {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {14, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                  {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                  {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
              }},
             {{0},
              {3, 2},
              {
                  {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATER, {5, 3}},
                  {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF, {6, 8}},
              }},
             {{1},
              {4},
              {
                  {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
              }},
             {{1},
              {4},
              {
                  {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEG, {4}},
                  {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {10}},
              }},
             {{2, 3}, {}, {}}});
        helpers::VerifyGraph(graph, bbSchemas);
    };
    helpers::InspectMethod(CREATE_EMPTY_INPUT_PATH, "test2", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetFalseBranch, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetFalseBranch_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *cmpB = helpers::BBgetSuccBlocks(start)[0];
        auto *trueB = g_implG->bbGetFalseBranch(cmpB);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto *inst = g_implG->bbGetFirstInst(trueB);

        ASSERT_NE(inst, nullptr);
        ASSERT_EQ(g_dynG->iGetOpcode(inst), ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);

        std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
            {{{},
              {1},
              {
                  {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {14, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                  {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                  {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
              }},
             {{0},
              {3, 2},
              {
                  {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATER, {5, 3}},
                  {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF, {6, 8}},
              }},
             {{1},
              {4},
              {
                  {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
              }},
             {{1},
              {4},
              {
                  {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEG, {4}},
                  {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {10}},
              }},
             {{2, 3}, {}, {}}});
        helpers::VerifyGraph(graph, bbSchemas);
    };
    helpers::InspectMethod(CREATE_EMPTY_INPUT_PATH, "test2", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbAddInstFront, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBaddInstFront_4)
{
    helpers::TransformMethod(
        CREATE_EMPTY_INPUT_PATH, ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/create_empty_dynamic_instfront.abc",
        "test",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *bb = helpers::BBgetSuccBlocks(start)[0];
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT);
            auto *newInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
            auto *negInst = g_dynG->iCreateNeg(graph, newInst);
            g_implG->bbAddInstFront(bb, negInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{7, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {
                      {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEG, {3}},
                      {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(inst, g_implG->iGetNext(newInst));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::bbAddInstBack, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBaddInstBack_2)
{
    helpers::TransformMethod(
        CREATE_EMPTY_INPUT_PATH, ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/create_empty_dynamic_instback.abc",
        "test",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *bb = helpers::BBgetSuccBlocks(start)[0];
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT);
            auto *newInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
            auto *negInst = g_dynG->iCreateNeg(graph, newInst);

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
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{6, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {
                      {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEG, {3}},
                  }},
                 {{1},
                  {3},
                  {
                      {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
                  }},
                 {{2}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(inst, g_implG->iGetNext(newInst));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::bbAddInstBack, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBaddInstBack_4)
{
    helpers::TransformMethod(
        CREATE_EMPTY_INPUT_PATH, ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/create_empty_dynamic_instback.abc",
        "test",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *newIns = g_implG->gFindOrCreateConstantI64(graph, 1);

            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {
                      {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(newIns, g_implG->bbGetLastInst(start));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::bbVisitSuccBlocks, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBvisitSuccBlocks_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];

        uint32_t counter = 0;
        g_implG->bbVisitSuccBlocks(bb, &counter, [](AbckitBasicBlock *, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        });
        // CC-OFFNXT(G.FMT.02)
        ASSERT_EQ(counter, 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };
    helpers::InspectMethod(CREATE_EMPTY_INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbVisitPredBlocks, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBvisitPredBlocks_1)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];

        uint32_t counter = 0;
        g_implG->bbVisitPredBlocks(bb, &counter, [](AbckitBasicBlock *, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        });

        ASSERT_EQ(counter, 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };
    helpers::InspectMethod(CREATE_EMPTY_INPUT_PATH, "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbVisitDominatedBlocks, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBvisitDominatedBlocks_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];

        uint32_t counter = 0;
        g_implG->bbVisitDominatedBlocks(bb, &counter, [](AbckitBasicBlock *, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        });

        ASSERT_EQ(counter, 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetSuccBlock, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetSuccBlock_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);

        ASSERT_EQ(bb, helpers::BBgetSuccBlocks(start)[0]);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsStart, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBisStart_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);

        ASSERT_TRUE(g_implG->bbIsStart(start));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsEnd, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBisEnd_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *end = g_implG->gGetEndBasicBlock(graph);

        ASSERT_TRUE(g_implG->bbIsEnd(end));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetImmediateDominator, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetImmediateDominator_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];
        g_implG->bbGetImmediateDominator(bb);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetImmediateDominator, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetImmediateDominator_2)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto bb = g_implG->bbGetImmediateDominator(start);
        ASSERT_EQ(bb, nullptr);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetSuccBlockCount, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetSuccBlockCount_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);

        ASSERT_EQ(g_implG->bbGetSuccBlockCount(start), 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetPredBlockCount, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetPredBlockCount_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];

        ASSERT_EQ(g_implG->bbGetPredBlockCount(bb), 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbDump, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBdump_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        LIBABCKIT_LOG_DUMP(g_implG->bbDump(start, 0), DEBUG);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetNumberOfInstructions, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetNumberOfInstructions_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = helpers::BBgetSuccBlocks(start)[0];
        ASSERT_EQ(g_implG->bbGetNumberOfInstructions(bb), 1U);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetBasicBlock, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, GgetBasicBlock_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *bb = g_implG->gGetBasicBlock(graph, 1);
        ASSERT_EQ(bb, g_implG->gGetStartBasicBlock(graph));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetBasicBlock, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitBasicBlocksDynamicTest, GgetBasicBlockBigId)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto bbVectorSize = g_implG->gGetNumberOfBasicBlocks(graph);
        auto *bb = g_implG->gGetBasicBlock(graph, bbVectorSize + 1);
        ASSERT_EQ(bb, nullptr);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetParameter, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, GgetParameter_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *inst = g_implG->bbGetFirstInst(start);
        ASSERT_EQ(inst, g_implG->gGetParameter(graph, 0));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetParameter, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitBasicBlocksDynamicTest, GgetParameterBigId)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto size = g_implG->gGetNumberOfParameters(graph);
        auto *inst = g_implG->gGetParameter(graph, size + 1);
        ASSERT_EQ(inst, nullptr);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetNumberOfParameters, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, GgetNumberOfParameters)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        uint32_t num = g_implG->gGetNumberOfParameters(graph);
        ASSERT_EQ(num, 4U);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::gGetNumberOfBasicBlocks, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, GgetNumberOfBasicBlocks_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        g_implG->gGetNumberOfBasicBlocks(graph);
        ASSERT_EQ(g_implG->gGetNumberOfBasicBlocks(graph), 5U);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetId, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetId_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        ASSERT_EQ(g_implG->bbGetId(start), 3U);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbGetPredBlock, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBgetPredBlock_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        ASSERT_EQ(g_implG->bbGetPredBlock(bb, 0), helpers::BBgetPredBlocks(bb)[0]);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbInsertSuccBlock, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBinsertSuccBlock_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        auto *empty = g_implG->bbCreateEmpty(graph);
        g_implG->bbInsertSuccBlock(bb, empty, 2U);
        ASSERT_EQ(g_implG->bbGetSuccBlock(bb, 2U), helpers::BBgetSuccBlocks(bb)[2U]);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        g_implG->bbDisconnectSuccBlock(bb, 2U);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbAppendSuccBlock, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBappendSuccBlock_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        auto *empty = g_implG->bbCreateEmpty(graph);
        g_implG->bbAppendSuccBlock(bb, empty);
        ASSERT_EQ(g_implG->bbGetSuccBlock(bb, 2U), helpers::BBgetSuccBlocks(bb)[2U]);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        g_implG->bbDisconnectSuccBlock(bb, g_implG->bbGetSuccBlockCount(bb) - 1);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbRemoveAllInsts, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, bbRemoveAllInsts_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *empty = g_implG->bbCreateEmpty(graph);
        auto *constInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
        auto *newInst = g_dynG->iCreateNeg(graph, constInst);
        g_implG->bbAddInstFront(empty, newInst);
        ASSERT_EQ(g_implG->bbGetNumberOfInstructions(empty), 1);
        g_implG->bbRemoveAllInsts(empty);
        ASSERT_EQ(g_implG->bbGetNumberOfInstructions(empty), 0);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsLoopPrehead, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBisLoopPrehead_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test4", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        ASSERT_TRUE(g_implG->bbIsLoopPrehead(bb));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsLoopHead, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBisLoopHead_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
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

// Test: test-kind=api, api=GraphApiImpl::bbSplitBlockAfterInstruction, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBsplitBlockAfterInstruction_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
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

// Test: test-kind=api, api=GraphApiImpl::bbCheckDominance, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBcheckDominance_1)
{
    AbckitFile *file = g_impl->openAbc(CREATE_EMPTY_INPUT_PATH, strlen(CREATE_EMPTY_INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test2", [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        ASSERT_TRUE(g_implG->bbCheckDominance(bb, start));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::bbVisitSuccBlocks, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBvisitSuccBlocksDynamic)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/visit_succ_dynamic.abc",
                                             "visit_succ_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n1\n"));
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
        AbckitBasicBlock *ifBB = g_implG->bbGetSuccBlock(startBB, 0);
        std::vector<AbckitBasicBlock *> succBBs;
        struct VisitData {
            const AbckitIsaApiDynamic *gDynG;
            AbckitGraph *graph;
            int newPrintInt;
        } data {g_dynG, graph, 42};

        // CC-OFFNXT(G.FMT.02)
        auto simpleCb = [](AbckitBasicBlock *succBasicBlock, void *d) -> bool {
            // CC-OFFNXT(G.FMT.02)
            auto *gDynG = static_cast<struct VisitData *>(d)->gDynG;
            auto *graph = static_cast<struct VisitData *>(d)->graph;
            int newPrintInt = static_cast<struct VisitData *>(d)->newPrintInt++;
            auto *curInst = g_implG->bbGetFirstInst(succBasicBlock);
            while (curInst != nullptr &&
                   // CC-OFFNXT(G.FMT.02)
                   gDynG->iGetOpcode(curInst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1) {
                curInst = g_implG->iGetNext(curInst);
            }
            if (curInst == nullptr) {
                return false;
            }
            g_implG->iSetInput(curInst, g_implG->gFindOrCreateConstantU64(graph, newPrintInt), 1);
            return true;
        };

        // CC-OFFNXT(G.FMT.02)
        g_implG->bbVisitSuccBlocks(ifBB, &data, simpleCb);
    };
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/visit_succ_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/visit_succ_dynamic_modified.abc",
                             "visitSuccBlocks", cb);
    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/visit_succ_dynamic_modified.abc",
                                        "visit_succ_dynamic");
    EXPECT_TRUE(helpers::Match(output, "43\n42\n"));
}

// Test: test-kind=api, api=GraphApiImpl::bbIsTry, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBisTry)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *startBB = g_implG->gGetStartBasicBlock(graph);
        AbckitBasicBlock *tryBB = g_implG->bbGetSuccBlock(g_implG->bbGetSuccBlock(startBB, 0), 0);
        // CC-OFFNXT(G.FMT.02)
        ASSERT_TRUE(g_implG->bbIsTry(tryBB));
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/try_catch_blocks_dynamic.abc", "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsTryBegin, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBisTryBegin)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *startBB = g_implG->gGetStartBasicBlock(graph);
        AbckitBasicBlock *tryBegin = g_implG->bbGetSuccBlock(startBB, 0);
        ASSERT_TRUE(g_implG->bbIsTryBegin(tryBegin));
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/try_catch_blocks_dynamic.abc", "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsTryEnd, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBisTryEnd)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *endBB = g_implG->gGetEndBasicBlock(graph);
        AbckitBasicBlock *tryEnd = g_implG->bbGetPredBlock(endBB, 0);
        ASSERT_TRUE(g_implG->bbIsTryEnd(tryEnd));
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/try_catch_blocks_dynamic.abc", "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsCatchBegin, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBisCatchBegin)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *startBB = g_implG->gGetStartBasicBlock(graph);
        AbckitBasicBlock *tryBegin = g_implG->bbGetSuccBlock(startBB, 0);
        AbckitBasicBlock *catchBegin = g_implG->bbGetSuccBlock(tryBegin, 1);
        ASSERT_TRUE(g_implG->bbIsCatchBegin(catchBegin));
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/try_catch_blocks_dynamic.abc", "test", cb);
}

// Test: test-kind=api, api=GraphApiImpl::bbIsCatch, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitBasicBlocksDynamicTest, BBisCatch)
{
    auto cb = [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
        auto *endBB = g_implG->gGetEndBasicBlock(graph);
        AbckitBasicBlock *tryEnd = g_implG->bbGetPredBlock(endBB, 0);
        AbckitBasicBlock *catchBB = g_implG->bbGetSuccBlock(tryEnd, 1);
        ASSERT_TRUE(g_implG->bbIsCatch(catchBB));
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/js_src/try_catch_blocks_dynamic.abc", "test", cb);
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
