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

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"
#include "libabckit/src/logger.h"
#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)

namespace libabckit::test {

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void CreateLoopDynamic(AbckitGraph *graph, AbckitFile *file)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitBasicBlock *endBB = g_implG->gGetEndBasicBlock(graph);

    AbckitInst *forConst = g_implG->gFindOrCreateConstantU64(graph, 0xa);
    AbckitInst *zero = g_implG->gFindOrCreateConstantU64(graph, 0x0);
    AbckitInst *forStep = g_implG->gFindOrCreateConstantU64(graph, 0x2);

    g_implG->bbAddInstBack(startBB, forConst);
    g_implG->bbAddInstBack(startBB, zero);
    g_implG->bbAddInstBack(startBB, forStep);
    g_implG->bbDisconnectSuccBlock(startBB, 0);

    AbckitBasicBlock *forBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(startBB, forBB);

    AbckitInst *phi = g_implG->bbCreatePhi(forBB, 1, zero);

    AbckitInst *intrinsicLess = g_dynG->iCreateLess(graph, phi, forConst);
    g_implG->bbAddInstBack(forBB, intrinsicLess);

    AbckitInst *ifInst = g_dynG->iCreateIf(graph, intrinsicLess, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
    g_implG->bbAddInstBack(forBB, ifInst);

    AbckitBasicBlock *trueBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *add = g_dynG->iCreateAdd2(graph, phi, forStep);

    g_implG->bbAddInstBack(trueBB, add);
    g_implG->iAppendInput(phi, add);

    auto *print = g_dynG->iCreateTryldglobalbyname(graph, g_implM->createString(file, "print", strlen("print")));
    g_implG->bbAddInstBack(trueBB, print);
    auto *call = g_dynG->iCreateCallarg1(graph, print, phi);
    g_implG->bbAddInstBack(trueBB, call);
    g_implG->bbAppendSuccBlock(forBB, trueBB);

    AbckitBasicBlock *retBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(forBB, retBB);
    g_implG->bbAppendSuccBlock(trueBB, forBB);
    g_implG->bbAppendSuccBlock(retBB, endBB);

    AbckitInst *ret = g_dynG->iCreateReturnundefined(graph);

    g_implG->bbAddInstBack(retBB, ret);
}

void CreateLoopStatic(AbckitGraph *graph, AbckitCoreFunction *consoleLogInt)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitBasicBlock *endBB = g_implG->gGetEndBasicBlock(graph);

    AbckitInst *forConst = g_implG->gFindOrCreateConstantI32(graph, 0xa);
    AbckitInst *zero = g_implG->gFindOrCreateConstantI32(graph, 0x0);
    AbckitInst *forStep = g_implG->gFindOrCreateConstantI32(graph, 0x1);

    g_implG->bbAddInstBack(startBB, forConst);
    g_implG->bbAddInstBack(startBB, zero);
    g_implG->bbAddInstBack(startBB, forStep);

    g_implG->bbDisconnectSuccBlock(startBB, 0);

    AbckitBasicBlock *forBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(startBB, forBB);

    AbckitInst *phi = g_implG->bbCreatePhi(forBB, 1, zero);

    AbckitInst *ifInst = g_statG->iCreateIf(graph, phi, forConst, ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LT);
    g_implG->bbAddInstBack(forBB, ifInst);

    AbckitBasicBlock *trueBB = g_implG->bbCreateEmpty(graph);
    AbckitInst *add = g_statG->iCreateAdd(graph, phi, forStep);

    g_implG->bbAddInstBack(trueBB, add);
    g_implG->iAppendInput(phi, add);

    AbckitInst *callLogInt = g_statG->iCreateCallStatic(graph, consoleLogInt, 1, phi);
    g_implG->bbAddInstBack(trueBB, callLogInt);

    g_implG->bbAppendSuccBlock(forBB, trueBB);

    AbckitBasicBlock *retBB = g_implG->bbCreateEmpty(graph);
    g_implG->bbAppendSuccBlock(forBB, retBB);
    g_implG->bbAppendSuccBlock(trueBB, forBB);
    g_implG->bbAppendSuccBlock(retBB, endBB);

    AbckitInst *ret = g_statG->iCreateReturnVoid(graph);
    g_implG->bbAddInstBack(retBB, ret);
}

}  // namespace

class LibAbcKitLoopStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::bbCreatePhi, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitLoopStaticTest, LibAbcKitLoopTestStatic)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/loops/loop_static.abc", "loop_static", "main");
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/loops/loop_static.abc", ABCKIT_ABC_DIR "ut/ir_core/loops/loop_static_modified.abc",
        "bar",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            auto *consoleLogInt = helpers::FindMethodByName(g_implI->functionGetFile(method), "console_log_int");
            CreateLoopStatic(graph, consoleLogInt);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/loops/loop_static_modified.abc", "loop_static", "main");
}

// Test: test-kind=api, api=GraphApiImpl::bbCreatePhi, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitLoopStaticTest, LibAbcKitLoopDynamic)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/loops/loop_dynamic.abc", "loop_dynamic");
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/loops/loop_dynamic.abc", ABCKIT_ABC_DIR "ut/ir_core/loops/loop_dynamic_modified.abc",
        "bar", [&](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) { CreateLoopDynamic(graph, file); },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/loops/loop_dynamic_modified.abc", "loop_dynamic");
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
