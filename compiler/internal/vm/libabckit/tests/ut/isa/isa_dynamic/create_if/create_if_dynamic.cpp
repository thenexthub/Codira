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

void TransformIrCreateIfDynamic(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitInst *constOne = g_implG->gFindOrCreateConstantU64(graph, 1);
    AbckitInst *constZero = g_implG->gFindOrCreateConstantU64(graph, 0);

    AbckitBasicBlock *trueBB = succBBs[0];
    g_implG->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(falseBB, g_implG->gGetEndBasicBlock(graph));
    g_implG->bbAddInstBack(falseBB, g_dynG->iCreateReturnundefined(graph));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *intrinsicNoteq = g_dynG->iCreateNoteq(graph, constOne, constZero);
    AbckitInst *ifInst = g_dynG->iCreateIf(graph, intrinsicNoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    g_implG->bbAddInstBack(ifBB, intrinsicNoteq);
    g_implG->bbAddInstBack(ifBB, ifInst);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    g_implG->bbAppendSuccBlock(ifBB, trueBB);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
}

void TransformIrCreateIfDynamicNeg(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitInst *constOne = g_implG->gFindOrCreateConstantU64(graph, 1);
    AbckitInst *constZero = g_implG->gFindOrCreateConstantU64(graph, 0);

    AbckitBasicBlock *trueBB = succBBs[0];
    g_implG->bbDisconnectSuccBlock(startBB, ABCKIT_TRUE_SUCC_IDX);
    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(falseBB, g_implG->gGetEndBasicBlock(graph));
    g_implG->bbAddInstBack(falseBB, g_dynG->iCreateReturnundefined(graph));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *intrinsicNoteq = g_dynG->iCreateNoteq(graph, constOne, constZero);
    AbckitInst *ifInst = g_dynG->iCreateIf(graph, intrinsicNoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GT);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    ASSERT_EQ(ifInst, nullptr);
    ifInst = g_dynG->iCreateIf(graph, intrinsicNoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    g_implG->bbAddInstBack(ifBB, intrinsicNoteq);
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
    g_implG->bbAddInstBack(falseBB, g_dynG->iCreateReturnundefined(graph));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *intrinsicNoteq = g_dynG->iCreateNoteq(graph, constOne, constZero);
    AbckitInst *ifInst = g_dynG->iCreateIf(graph, intrinsicNoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    AbckitIsaApiDynamicConditionCode cc = g_dynG->iGetConditionCode(ifInst);
    ASSERT_EQ(cc, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    g_implG->bbAddInstBack(ifBB, intrinsicNoteq);
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
    g_implG->bbAddInstBack(falseBB, g_dynG->iCreateReturnundefined(graph));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *intrinsicNoteq = g_dynG->iCreateNoteq(graph, constOne, constZero);
    AbckitInst *ifInst = g_dynG->iCreateIf(graph, intrinsicNoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    AbckitIsaApiDynamicConditionCode cc = g_dynG->iGetConditionCode(nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    ASSERT_EQ(cc, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE);
    g_implG->bbAddInstBack(ifBB, intrinsicNoteq);
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
    g_implG->bbAddInstBack(falseBB, g_dynG->iCreateReturnundefined(graph));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *intrinsicNoteq = g_dynG->iCreateNoteq(graph, constOne, constZero);
    AbckitInst *ifInst = g_dynG->iCreateIf(graph, intrinsicNoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
    g_dynG->iSetConditionCode(ifInst, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstBack(ifBB, intrinsicNoteq);
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
    g_implG->bbAddInstBack(falseBB, g_dynG->iCreateReturnundefined(graph));
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *intrinsicNoteq = g_dynG->iCreateNoteq(graph, constOne, constZero);
    AbckitInst *ifInst = g_dynG->iCreateIf(graph, intrinsicNoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    g_dynG->iSetConditionCode(nullptr, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_dynG->iSetConditionCode(ifInst, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GT);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_implG->bbAddInstBack(ifBB, intrinsicNoteq);
    g_implG->bbAddInstBack(ifBB, ifInst);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    g_implG->bbAppendSuccBlock(ifBB, trueBB);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
}
}  // namespace

class LibAbcKitCreateIfDynamicTest : public ::testing::Test {};

static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> GetSchema()
{
    return {{{},
             {1},
             {
                 {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                 {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                 {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
             }},
            {{0},
             {3, 2},
             {
                 {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_NOTEQ, {0, 1}},
                 {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF, {3, 2}},
             }},
            {{1},
             {4},
             {
                 {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
             }},
            {{1},
             {4},
             {
                 {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {1}},
             }},
            {{2, 3}, {}, {}}};
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateIf, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateIfDynamicTest, LibAbcKitTestCreateIfDyn)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrCreateIfDynamic(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema()); });

    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified.abc", "create_if_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateIf, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitCreateIfDynamicTest, LibAbcKitTestCreateIfDynNeg)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_neg.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrCreateIfDynamicNeg(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema()); });

    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_neg.abc", "create_if_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iGetConditionCode, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateIfDynamicTest, LibAbcKitTestGetConditionCode)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_get.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrIgetConditionCode(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema()); });

    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_get.abc", "create_if_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iGetConditionCode, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitCreateIfDynamicTest, LibAbcKitTestGetConditionCodeNeg)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_get_neg.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrIgetConditionCodeNeg(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema()); });

    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_get_neg.abc", "create_if_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetConditionCode, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateIfDynamicTest, LibAbcKitTestSetConditionCode)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_set.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrIsetConditionCode(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema()); });

    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_set.abc", "create_if_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iSetConditionCode, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitCreateIfDynamicTest, LibAbcKitTestSetConditionCodeNeg)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_set_neg.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrIsetConditionCodeNeg(graph);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema()); });

    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create_if/create_if_dynamic_modified_set_neg.abc", "create_if_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
