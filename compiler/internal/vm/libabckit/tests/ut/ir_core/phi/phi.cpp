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
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitGetPhiTest : public ::testing::Test {};

namespace {

std::pair<AbckitInst *, AbckitInst *> IRFindParameters(AbckitGraph *graph, bool isDynamic)
{
    AbckitInst *param0 = nullptr;
    AbckitInst *param1 = nullptr;
    if (isDynamic) {
        param1 = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
        param0 = g_implG->iGetPrev(param1);
    } else {
        param0 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);
        param1 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER,
                                        [counter = 0](AbckitInst * /*i*/) mutable { return counter++ > 0; });
    }

    return {param0, param1};
}

void TransformIR(AbckitGraph *graph, bool isDynamic = false)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitBasicBlock *endBB = g_implG->gGetEndBasicBlock(graph);

    auto [param0, param1] = IRFindParameters(graph, isDynamic);

    ASSERT_NE(param0, nullptr);
    ASSERT_NE(param1, nullptr);

    g_implG->bbDisconnectSuccBlock(startBB, 0);
    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    AbckitInst *ifInst = nullptr;
    if (isDynamic) {
        AbckitInst *intrinsicEq = g_dynG->iCreateEq(graph, param1, param0);
        g_implG->bbAddInstBack(ifBB, intrinsicEq);
        ifInst = g_dynG->iCreateIf(graph, intrinsicEq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
    } else {
        ifInst = g_statG->iCreateIf(graph, param0, param1, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE);
    }
    g_implG->bbAddInstBack(ifBB, ifInst);

    AbckitBasicBlock *trueBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(ifBB, trueBB);
    AbckitInst *dec = nullptr;
    if (isDynamic) {
        dec = g_dynG->iCreateSub2(graph, param1, param0);
    } else {
        dec = g_statG->iCreateSub(graph, param0, param1);
    }
    g_implG->bbAddInstBack(trueBB, dec);

    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
    AbckitInst *inc = nullptr;
    if (isDynamic) {
        inc = g_dynG->iCreateAdd2(graph, param1, param0);
    } else {
        inc = g_statG->iCreateAdd(graph, param0, param1);
    }
    g_implG->bbAddInstBack(falseBB, inc);

    AbckitBasicBlock *retBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(trueBB, retBB);
    g_implG->bbAppendSuccBlock(falseBB, retBB);
    g_implG->bbAppendSuccBlock(retBB, endBB);
    AbckitInst *phi = g_implG->bbCreatePhi(retBB, 2, dec, inc);
    AbckitInst *ret = nullptr;
    if (isDynamic) {
        ret = g_dynG->iCreateReturn(graph, phi);
    } else {
        ret = g_statG->iCreateReturn(graph, phi);
    }
    g_implG->bbAddInstBack(retBB, ret);
}

}  // namespace

// Test: test-kind=api, api=GraphApiImpl::bbCreatePhi, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitGetPhiTest, StaticBBcreatePhi)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/phi/phi_static.abc", "phi_static", "main");
    EXPECT_TRUE(helpers::Match(output, "2\n4\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/phi/phi_static.abc", ABCKIT_ABC_DIR "ut/ir_core/phi/phi_static_modified.abc", "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) { TransformIR(graph); },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> schema = {
                {{},
                 {1},
                 {
                     {0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                     {1, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                 }},
                {{0},
                 {3, 2},
                 {
                     {2, ABCKIT_ISA_API_STATIC_OPCODE_IF, {0, 1}},
                 }},
                {{1},
                 {4},
                 {
                     {3, ABCKIT_ISA_API_STATIC_OPCODE_ADD, {0, 1}},
                 }},
                {{1},
                 {4},
                 {
                     {4, ABCKIT_ISA_API_STATIC_OPCODE_SUB, {0, 1}},
                 }},
                {{2, 3},
                 {5},
                 {
                     {5, ABCKIT_ISA_API_STATIC_OPCODE_PHI, {3, 4}},
                     {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {5}},
                 }},
                {{4}, {}, {}}};
            helpers::VerifyGraph(graph, schema);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/phi/phi_static_modified.abc", "phi_static", "main");
    EXPECT_TRUE(helpers::Match(output, "-1\n4\n"));
}

// Test: test-kind=api, api=GraphApiImpl::bbCreatePhi, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitGetPhiTest, DynamicBBcreatePhi)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/phi/phi_dynamic.abc", "phi_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\n4\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/phi/phi_dynamic.abc", ABCKIT_ABC_DIR "ut/ir_core/phi/phi_dynamic_modified.abc",
        "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) { TransformIR(graph, true); },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> schema = {
                {{},
                 {1},
                 {
                     {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                     {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                     {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                     {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                     {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                     {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                 }},
                {{0},
                 {3, 2},
                 {
                     {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_EQ, {1, 0}},
                     {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF, {3, 2}},
                 }},
                {{1},
                 {4},
                 {
                     {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2, {1, 0}},
                 }},
                {{1},
                 {4},
                 {
                     {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_SUB2, {1, 0}},
                 }},
                {{2, 3},
                 {5},
                 {
                     {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_PHI, {5, 6}},
                     {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {7}},
                 }},
                {{4}, {}, {}}};
            helpers::VerifyGraph(graph, schema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/phi/phi_dynamic_modified.abc", "phi_dynamic");
    EXPECT_TRUE(helpers::Match(output, "-1\n4\n"));
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
