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
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformIR(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitBasicBlock *endBB = g_implG->gGetEndBasicBlock(graph);

    AbckitInst *param1 = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
    ASSERT_NE(param1, nullptr);
    AbckitInst *param0 = g_implG->iGetPrev(param1);
    ASSERT_NE(param0, nullptr);

    AbckitBasicBlock *ifBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(startBB, ifBB);
    AbckitInst *len = g_statG->iCreateLenArray(graph, param0);
    g_implG->bbAddInstBack(ifBB, len);
    AbckitInst *ifInst = g_statG->iCreateIf(graph, len, param1, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GT);

    g_implG->bbAddInstBack(ifBB, ifInst);

    g_implG->bbAppendSuccBlock(ifBB, succBBs[0]);
    g_implG->bbDisconnectSuccBlock(startBB, 0);

    AbckitBasicBlock *falseBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(ifBB, falseBB);
    auto *constm1 = g_implG->gFindOrCreateConstantI32(graph, -1);

    g_implG->bbAppendSuccBlock(falseBB, endBB);

    AbckitInst *ret = g_statG->iCreateReturn(graph, constm1);

    g_implG->bbAddInstBack(falseBB, ret);
}

class AbckitScenarioTest : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestStaticParameterCheck)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "scenarios/parameter_check/parameter_check_static.abc",
                                            "parameter_check_static", "main");
    EXPECT_TRUE(helpers::Match(output,
                               "buisiness logic...\n"
                               "1\n"
                               "buisiness logic...\n"
                               "2\n"
                               "buisiness logic...\n"
                               "3\n"));
    // NOTE: Add method search
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "scenarios/parameter_check/parameter_check_static.abc",
        ABCKIT_ABC_DIR "scenarios/parameter_check/parameter_check_static_modified.abc", "handle",
        // CC-OFFNXT(C_RULE_ID_ONE_STATEMENT_ONE_LINE) local codestyle conflict
        // CC-OFFNXT(G.FMT.04) project code style
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) { TransformIR(graph); },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "scenarios/parameter_check/parameter_check_static_modified.abc",
                                       "parameter_check_static", "main");
    EXPECT_TRUE(helpers::Match(output,
                               "buisiness logic...\n"
                               "1\n"
                               "-1\n"
                               "-1\n"));
}

}  // namespace libabckit::test
