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
#include "adapter_static/ir_static.h"
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
};

static void TransformIr(AbckitGraph *graph, UserData *userData)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    AbckitBasicBlock *endBB = g_implG->gGetEndBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitBasicBlock *bb = succBBs[0];
    AbckitInst *initInst = g_implG->bbGetFirstInst(bb);
    AbckitInst *prevRetInst = g_implG->iGetPrev(g_implG->bbGetLastInst(bb));

    AbckitBasicBlock *tryBegin =
        g_implG->bbSplitBlockAfterInstruction(g_implG->iGetBasicBlock(initInst), initInst, true);
    AbckitBasicBlock *tryEnd =
        g_implG->bbSplitBlockAfterInstruction(g_implG->iGetBasicBlock(prevRetInst), prevRetInst, true);

    // Fill catchBlock
    AbckitBasicBlock *catchBlock = g_implG->bbCreateEmpty(graph);
    AbckitInst *catchPhi = g_implG->bbCreateCatchPhi(catchBlock, 0);
    AbckitInst *print = g_dynG->iCreateTryldglobalbyname(graph, userData->print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstBack(catchBlock, print);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitInst *callArg = g_dynG->iCreateCallarg1(graph, print, catchPhi);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstBack(catchBlock, callArg);

    AbckitBasicBlock *epilogueBB = helpers::BBgetPredBlocks(endBB)[0];
    AbckitInst *retInst = g_implG->bbGetLastInst(epilogueBB);
    AbckitInst *firstPhiInput = g_implG->iGetInput(retInst, 0);
    AbckitInst *secondPhiInput = g_dynG->iCreateLdfalse(graph);
    g_implG->bbAddInstBack(bb, secondPhiInput);
    AbckitInst *phiInst = g_implG->bbCreatePhi(epilogueBB, 2, firstPhiInput, secondPhiInput);
    g_implG->iSetInput(retInst, phiInst, 0);

    g_implG->gInsertTryCatch(tryBegin, tryEnd, catchBlock, catchBlock);
}

// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestDynamicAddTryCatch)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "scenarios/add_try_catch/dynamic/add_try_catch.abc", "add_try_catch");
    EXPECT_TRUE(helpers::Match(output, "THROW\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "scenarios/add_try_catch/dynamic/add_try_catch.abc",
        ABCKIT_ABC_DIR "scenarios/add_try_catch/dynamic/add_try_catch_modified.abc", "run",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            UserData uData {};
            uData.print = g_implM->createString(file, "print", strlen("print"));
            TransformIr(graph, &uData);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "scenarios/add_try_catch/dynamic/add_try_catch_modified.abc",
                                        "add_try_catch");
    EXPECT_TRUE(helpers::Match(output,
                               "THROW\n"
                               "Error: DUMMY_ERROR\n"
                               "false\n"));
}

}  // namespace libabckit::test
