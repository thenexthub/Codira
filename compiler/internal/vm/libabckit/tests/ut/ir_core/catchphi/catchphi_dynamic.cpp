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

#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/abckit.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

class LibAbcKitDynamicCatchPhiTest : public ::testing::Test {};

namespace {
auto *g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto *g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

enum class TryCatchScenario {
    DEFAULT_POSITIVE = 0,
};

[[maybe_unused]] std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchema(TryCatchScenario scenario)
{
    helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb0 {{}, {1}, {}};

    helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb2 {{1}, {}, {}};

    switch (scenario) {
        case TryCatchScenario::DEFAULT_POSITIVE:
            return {bb0,
                    {{0},
                     {2},
                     {
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
        default:
            LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }
}
}  // namespace

// Test: test-kind=api, api=GraphApiImpl::bbCreateCatchPhi, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynamicCatchPhiTest, CatchPhiDynamicValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_dynamic.abc", "catchphi_dynamic");
    EXPECT_TRUE(helpers::Match(output, "CATCH\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_dynamic_modified.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *print = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
            auto *catchPhi = g_implG->bbCreateCatchPhi(g_implG->iGetBasicBlock(print), 0);
            g_implG->iSetInput(print, catchPhi, 1);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_dynamic_modified.abc",
                                        "catchphi_dynamic");
    EXPECT_TRUE(helpers::Match(output, "Error: abckit_error\n"));
}

// Test: test-kind=api, api=GraphApiImpl::bbCreateCatchPhi, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynamicCatchPhiTest, CatchPhiNoAccDynamicValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_dynamic.abc", "catchphi_dynamic");
    EXPECT_TRUE(helpers::Match(output, "CATCH\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_dynamic_noacc_modified.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *print = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
            auto constantI64Impl = g_implG->gFindOrCreateConstantI32(graph, 42);
            auto *catchPhi = g_implG->bbCreateCatchPhi(g_implG->iGetBasicBlock(print), 2, constantI64Impl, print);
            g_implG->iSetInput(print, catchPhi, 1);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/catchphi/catchphi_dynamic_noacc_modified.abc",
                                        "catchphi_dynamic");
    EXPECT_TRUE(helpers::Match(output, "42\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
