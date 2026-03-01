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

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformIrIcreateIf(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitInst *constOne = g_implG->gFindOrCreateConstantU64(graph, 1);
    AbckitInst *constZero = g_implG->gFindOrCreateConstantU64(graph, 0);

    AbckitBasicBlock *trueBB = succBBs[0];
    g_implG->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(falseBB, g_implG->gGetEndBasicBlock(graph));
    g_implG->bbAddInstBack(falseBB, g_statG->iCreateReturn(graph, constZero));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *ifInst = g_statG->iCreateIf(graph, constOne, constZero, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE);
    g_implG->bbAddInstBack(ifBB, ifInst);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    g_implG->bbAppendSuccBlock(ifBB, trueBB);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
}

void TransformIrIcreateIfNeg(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitInst *constOne = g_implG->gFindOrCreateConstantU64(graph, 1);
    AbckitInst *constZero = g_implG->gFindOrCreateConstantU64(graph, 0);

    AbckitBasicBlock *trueBB = succBBs[0];
    g_implG->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(falseBB, g_implG->gGetEndBasicBlock(graph));
    g_implG->bbAddInstBack(falseBB, g_statG->iCreateReturn(graph, constZero));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *ifInst = g_statG->iCreateIf(graph, constOne, constZero, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);
    g_implG->bbAddInstBack(ifBB, ifInst);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    g_implG->bbAppendSuccBlock(ifBB, trueBB);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
}

void TransformIrIsetConditionCode(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitInst *constOne = g_implG->gFindOrCreateConstantU64(graph, 1);
    AbckitInst *constZero = g_implG->gFindOrCreateConstantU64(graph, 0);

    AbckitBasicBlock *trueBB = succBBs[0];
    g_implG->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(falseBB, g_implG->gGetEndBasicBlock(graph));
    g_implG->bbAddInstBack(falseBB, g_statG->iCreateReturn(graph, constZero));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *ifInst = g_statG->iCreateIf(graph, constOne, constZero, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);
    g_statG->iSetConditionCode(ifInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstBack(ifBB, ifInst);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    g_implG->bbAppendSuccBlock(ifBB, trueBB);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
}

void TransformIrIsetConditionCodeNeg(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitInst *constOne = g_implG->gFindOrCreateConstantU64(graph, 1);
    AbckitInst *constZero = g_implG->gFindOrCreateConstantU64(graph, 0);

    AbckitBasicBlock *trueBB = succBBs[0];
    g_implG->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(falseBB, g_implG->gGetEndBasicBlock(graph));
    g_implG->bbAddInstBack(falseBB, g_statG->iCreateReturn(graph, constZero));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *ifInst = g_statG->iCreateIf(graph, constOne, constZero, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);
    g_statG->iSetConditionCode(nullptr, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_implG->bbAddInstBack(ifBB, ifInst);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    g_implG->bbAppendSuccBlock(ifBB, trueBB);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
}

void TransformIrIgetConditionCode(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitInst *constOne = g_implG->gFindOrCreateConstantU64(graph, 1);
    AbckitInst *constZero = g_implG->gFindOrCreateConstantU64(graph, 0);

    AbckitBasicBlock *trueBB = succBBs[0];
    g_implG->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(falseBB, g_implG->gGetEndBasicBlock(graph));
    g_implG->bbAddInstBack(falseBB, g_statG->iCreateReturn(graph, constZero));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *ifInst = g_statG->iCreateIf(graph, constOne, constZero, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);
    AbckitIsaApiStaticConditionCode cc = g_statG->iGetConditionCode(ifInst);
    ASSERT_EQ(cc, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);
    g_statG->iSetConditionCode(ifInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE);
    g_implG->bbAddInstBack(ifBB, ifInst);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    g_implG->bbAppendSuccBlock(ifBB, trueBB);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
}

void TransformIrIgetConditionCodeNeg(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitInst *constOne = g_implG->gFindOrCreateConstantU64(graph, 1);
    AbckitInst *constZero = g_implG->gFindOrCreateConstantU64(graph, 0);

    AbckitBasicBlock *trueBB = succBBs[0];
    g_implG->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(falseBB, g_implG->gGetEndBasicBlock(graph));
    g_implG->bbAddInstBack(falseBB, g_statG->iCreateReturn(graph, constZero));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *ifInst = g_statG->iCreateIf(graph, constOne, constZero, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ);
    AbckitIsaApiStaticConditionCode cc = g_statG->iGetConditionCode(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    ASSERT_EQ(cc, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE);
    g_statG->iSetConditionCode(ifInst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE);
    g_implG->bbAddInstBack(ifBB, ifInst);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    g_implG->bbAppendSuccBlock(ifBB, trueBB);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
}
}  // namespace

class LibAbcKitCreateIfStaticTest : public ::testing::Test {};

static std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> GetSchema1()
{
    return {{{},
             {1},
             {
                 {9, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                 {0, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                 {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                 {2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                 {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                 {4, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
             }},
            {{0},
             {3, 2},
             {
                 {5, ABCKIT_ISA_API_STATIC_OPCODE_CMP, {1, 0}},
                 {6, ABCKIT_ISA_API_STATIC_OPCODE_IF, {5, 2}},
             }},
            {{1},
             {4},
             {
                 {7, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {4}},
             }},
            {{1},
             {4},
             {
                 {8, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {3}},
             }},
            {{2, 3}, {}, {}}};
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateIf, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateIfStaticTest, LibAbcKitTestCreateIf)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) { TransformIrIcreateIf(graph); },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema1()); });
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified.abc",
                                            "create_if_static", "main");
    EXPECT_TRUE(helpers::Match(output, "3.14\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateIf, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitCreateIfStaticTest, LibAbcKitTestCreateIfNeg)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified_neg.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrIcreateIfNeg(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema1()); });
    auto output = helpers::ExecuteStaticAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified_neg.abc", "create_if_static", "main");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

static std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> GetSchema2()
{
    return {{{},
             {1},
             {
                 {9, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                 {0, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                 {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                 {2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                 {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                 {4, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
             }},
            {{0},
             {3, 2},
             {
                 {5, ABCKIT_ISA_API_STATIC_OPCODE_CMP, {1, 0}},
                 {6, ABCKIT_ISA_API_STATIC_OPCODE_IF, {5, 2}},
             }},
            {{1},
             {4},
             {
                 {7, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {4}},
             }},
            {{1},
             {4},
             {
                 {8, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {3}},
             }},
            {{2, 3}, {}, {}}};
}

// Test: test-kind=api, api=IsaApiStaticImpl::iSetConditionCode, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateIfStaticTest, LibAbcKitTestSetCondidionCodePos)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified_set.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrIsetConditionCode(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema2()); });
    auto output = helpers::ExecuteStaticAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified_set.abc", "create_if_static", "main");
    EXPECT_TRUE(helpers::Match(output, "3.14\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iSetConditionCode, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitCreateIfStaticTest, LibAbcKitTestSetCondidionCodeNeg)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified_set_neg.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrIsetConditionCodeNeg(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema2()); });
}

// Test: test-kind=api, api=IsaApiStaticImpl::iGetConditionCode, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateIfStaticTest, LibAbcKitTestGetCondidionCodePos)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified_get.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrIgetConditionCode(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema2()); });
    auto output = helpers::ExecuteStaticAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified_get.abc", "create_if_static", "main");
    EXPECT_TRUE(helpers::Match(output, "3.14\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iGetConditionCode, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitCreateIfStaticTest, LibAbcKitTestGetCondidionCodeNeg)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_if/create_if_static_modified_get_neg.abc", "handle",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrIgetConditionCodeNeg(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema2()); });
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
