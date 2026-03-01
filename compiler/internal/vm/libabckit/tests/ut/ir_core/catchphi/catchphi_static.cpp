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

#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/abckit.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/statuses.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

class LibAbcKitStaticCatchPhiTest : public ::testing::Test {};

namespace {
auto *g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

enum class TryCatchScenario {
    DEFAULT_POSITIVE = 0,
};
}  // namespace

// Test: test-kind=api, api=GraphApiImpl::bbCreateCatchPhi, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitStaticCatchPhiTest, CatchPhiStaticValid)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static.abc", "catchphi_static", "main");
    EXPECT_TRUE(helpers::Match(output, "CATCH\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            auto *oldCall = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
            EXPECT_NE(oldCall, nullptr);
            auto *log = helpers::FindMethodByName(g_implI->functionGetFile(method), "logError");
            auto *catchPhi = g_implG->bbCreateCatchPhi(g_implG->iGetBasicBlock(oldCall), 0);
            auto *newCall = g_statG->iCreateCallStatic(graph, log, 1, catchPhi);
            g_implG->iInsertBefore(newCall, oldCall);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static_modified.abc",
                                       "catchphi_static", "main");
    EXPECT_TRUE(helpers::Match(output, "Error: abckit_error\nCATCH\n"));
}

// Test: test-kind=api, api=GraphApiImpl::bbCreateCatchPhi, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitStaticCatchPhiTest, CatchPhiNoAccStaticValid)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static.abc", "catchphi_static", "main0");
    EXPECT_TRUE(helpers::Match(output, "75\n13\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static_modified_noacc.abc", "bar",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            struct VisitorData {
                const AbckitGraphApi *implG;
                AbckitBasicBlock *returnBB;
            } vd {g_implG, nullptr};

            g_implG->gVisitBlocksRpo(graph, &vd, [](AbckitBasicBlock *curBB, void *data) {
                const AbckitGraphApi *gImplG = static_cast<struct VisitorData *>(data)->implG;
                if (gImplG->bbIsCatchBegin(curBB)) {
                    static_cast<struct VisitorData *>(data)->returnBB = curBB;
                    return false;
                }
                return true;
            });

            auto *throwableCall = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
            ASSERT_NE(throwableCall, nullptr);
            AbckitBasicBlock *returnBB = vd.returnBB;
            ASSERT_NE(returnBB, nullptr);
            AbckitInst *returnInst = g_implG->bbGetFirstInst(returnBB);
            ASSERT_NE(returnInst, nullptr);
            auto constantI64Impl = g_implG->gFindOrCreateConstantI32(graph, 42);
            AbckitInst *catchPhi = g_implG->bbCreateCatchPhi(returnBB, 2, constantI64Impl, throwableCall);
            ASSERT_NE(catchPhi, nullptr);
            g_implG->iSetInput(returnInst, catchPhi, 0);
        },
        [&]([[maybe_unused]] AbckitGraph *ctxG) {});

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static_modified_noacc.abc",
                                       "catchphi_static", "main0");
    EXPECT_TRUE(helpers::Match(output, "75\n42\n"));
}

// Test: test-kind=api, api=GraphApiImpl::bbCreateCatchPhi, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitStaticCatchPhiTest, CatchPhiNoAccStaticInvalid)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static.abc", "catchphi_static", "main0");
    EXPECT_TRUE(helpers::Match(output, "75\n13\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_static_modified_noacc_neg.abc", "bar",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            struct VisitorData {
                const AbckitGraphApi *implG;
                AbckitBasicBlock *returnBB;
            } vd {g_implG, nullptr};

            g_implG->gVisitBlocksRpo(graph, &vd, [](AbckitBasicBlock *curBB, void *data) {
                const AbckitGraphApi *gImplG = static_cast<struct VisitorData *>(data)->implG;
                if (gImplG->bbIsCatchBegin(curBB)) {
                    static_cast<struct VisitorData *>(data)->returnBB = curBB;
                    return false;
                }
                return true;
            });

            auto *throwableCall = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
            ASSERT_NE(throwableCall, nullptr);
            AbckitBasicBlock *returnBB = vd.returnBB;
            ASSERT_NE(returnBB, nullptr);
            AbckitInst *returnInst = g_implG->bbGetFirstInst(returnBB);
            ASSERT_NE(returnInst, nullptr);
            auto constantI64Impl = g_implG->gFindOrCreateConstantI64(graph, 42);
            // Mix integer and reference types as input
            AbckitInst *catchPhi =
                g_implG->bbCreateCatchPhi(returnBB, 4, constantI64Impl, throwableCall, throwableCall, throwableCall);
            ASSERT_EQ(catchPhi, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
            // Call no side-effect API to clear last error
            ASSERT_NE(g_implG->iGetInputCount(returnInst), 0);
        },
        [&]([[maybe_unused]] AbckitGraph *ctxG) {});
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
