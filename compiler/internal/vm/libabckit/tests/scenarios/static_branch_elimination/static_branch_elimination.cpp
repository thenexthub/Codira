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
#include "logger.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_static.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

namespace libabckit::test {

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

bool IsCall(AbckitInst *cond, AbckitInst **ifInstr, AbckitInst *curInst)
{
    if (!g_implG->iCheckIsCall(cond)) {
        return false;
    }
    AbckitCoreFunction *method = g_implG->iGetFunction(cond);
    AbckitString *methodNameStr = g_implI->functionGetName(method);
    auto name = helpers::GetCropFuncName(helpers::AbckitStringToString(methodNameStr).data());
    if (name == "IsDebug") {
        *ifInstr = curInst;
        return true;
    }
    return false;
}

auto g_libAbcKitTestStaticBranchEliminationLambda0 = [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
    AbckitInst *ifInstr = nullptr;

    auto checkIfArg = [&ifInstr](AbckitFile *, AbckitBasicBlock *bb) -> bool {
        auto *curInst = g_implG->bbGetFirstInst(bb);
        while (curInst != nullptr) {
            AbckitIsaApiStaticOpcode op = g_statG->iGetOpcode(curInst);
            if (op != ABCKIT_ISA_API_STATIC_OPCODE_IF) {
                curInst = g_implG->iGetNext(curInst);
                continue;
            }
            auto *cond = g_implG->iGetInput(curInst, 0);
            if (IsCall(cond, &ifInstr, curInst)) {
                return true;
            }
            curInst = g_implG->iGetNext(curInst);
        }
        return false;
    };

    std::vector<AbckitBasicBlock *> bbs;
    g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
        reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
        return true;
    });

    for (auto i = bbs.rbegin(), j = bbs.rend(); i != j; ++i) {
        if (checkIfArg(file, *i)) {
            break;
        }
    }

    EXPECT_NE(ifInstr, nullptr);
    auto *ifBB = g_implG->iGetBasicBlock(ifInstr);
    EXPECT_NE(ifBB, nullptr);
    auto *falseBB = g_implG->bbGetFalseBranch(ifBB);
    EXPECT_NE(falseBB, nullptr);
    g_implG->bbDisconnectSuccBlock(ifBB, ABCKIT_FALSE_SUCC_IDX);
    g_implG->iRemove(ifInstr);
};

}  // namespace

class AbckitScenarioTest : public ::testing::Test {};

// Test: test-kind=scenario, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(AbckitScenarioTest, LibAbcKitTestStaticBranchElimination)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "scenarios/static_branch_elimination/static_branch_elimination.abc";
    auto output = helpers::ExecuteStaticAbc(INPUT_PATH, "static_branch_elimination", "main");
    EXPECT_TRUE(helpers::Match(output,
                               "MyFunc start...\n"
                               "MyFunc start...\n"
                               "Debug buisness logic...\n"
                               "Debug buisness logic...\n"
                               "MyFunc end...\n"
                               "MyFunc end...\n"));

    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    // Transform method
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "MyFunc", g_libAbcKitTestStaticBranchEliminationLambda0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    constexpr auto OUTPUT_PATH =
        ABCKIT_ABC_DIR "scenarios/static_branch_elimination/static_branch_elimination_transformed.abc";
    // Write output file
    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    output = helpers::ExecuteStaticAbc(OUTPUT_PATH, "static_branch_elimination", "main");
    EXPECT_TRUE(helpers::Match(output,
                               "MyFunc start...\n"
                               "MyFunc start...\n"
                               "Release buisness logic...\n"
                               "Release buisness logic...\n"
                               "MyFunc end...\n"
                               "MyFunc end...\n"));
}

}  // namespace libabckit::test
