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
#include "libabckit/c/ir_core.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

struct ReplaceContext {
    AbckitGraph *ctx;
    AbckitCoreFunction *oldMethod;
    AbckitCoreFunction *newMethod;
};

void EnumerateInsts(ReplaceContext *rctx, AbckitBasicBlock *bb)
{
    auto *inst = g_implG->bbGetFirstInst(bb);
    while (inst != nullptr) {
        if (g_implG->iCheckIsCall(inst) && g_implG->iGetFunction(inst) == rctx->oldMethod) {
            auto *call = g_statG->iCreateCallStatic(rctx->ctx, rctx->newMethod, 2, g_implG->iGetInput(inst, 0),
                                                    g_implG->iGetInput(inst, 1));
            helpers::ReplaceInst(inst, call);
        }
        inst = g_implG->iGetNext(inst);
    }
}

void VisitAllBBs(ReplaceContext *rctx, AbckitBasicBlock *bb, std::set<AbckitBasicBlock *> &visitedBB)
{
    if (visitedBB.find(bb) != visitedBB.end()) {
        return;
    }
    EnumerateInsts(rctx, bb);
    visitedBB.emplace(bb);
    for (int i = 0; i < g_implG->bbGetSuccBlockCount(bb); i++) {
        VisitAllBBs(rctx, g_implG->bbGetSuccBlock(bb, i), visitedBB);
    }
}

void TransformIR(ReplaceContext *rctx)
{
    std::set<AbckitBasicBlock *> visitedBlocks;
    auto *start = g_implG->gGetStartBasicBlock(rctx->ctx);
    VisitAllBBs(rctx, start, visitedBlocks);
}

class AbckitScenarioTest : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestReplaceCallSite)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "scenarios/replace_callsite/replace_callsite_static.abc";
    auto output = helpers::ExecuteStaticAbc(INPUT_PATH, "replace_callsite_static", "main");
    EXPECT_TRUE(helpers::Match(output, "4\n3\n"));
    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));

    helpers::TransformMethod(file, "main", [file](AbckitFile *, AbckitCoreFunction *, AbckitGraph *ctxG) {
        auto *newMethod = helpers::FindMethodByName(file, "fixedApi");
        ASSERT_NE(newMethod, nullptr);
        auto *oldMethod = helpers::FindMethodByName(file, "api");
        ASSERT_NE(oldMethod, nullptr);
        ReplaceContext rctx = {ctxG, oldMethod, newMethod};
        TransformIR(&rctx);
    });
    constexpr auto OUTPUT_PATH = ABCKIT_ABC_DIR "scenarios/replace_callsite/replace_callsite_static_modified.abc";
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    g_impl->closeFile(file);
    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "replace_callsite_static", "main");
    EXPECT_TRUE(helpers::Match(output, "5\n4\n"));
}

}  // namespace libabckit::test
