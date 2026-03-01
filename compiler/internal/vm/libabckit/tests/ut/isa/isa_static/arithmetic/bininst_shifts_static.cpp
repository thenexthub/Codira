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
#include "libabckit/c/abckit.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "ut/isa/isa_dynamic/arithmetic/helpers_arithmetic.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static void ValidTest(AbckitInst *(*binaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1),
                      AbckitIsaApiStaticOpcode expectedOpcode, const std::string &expectedOutput)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_static.abc",
                                            "bininst_shifts_static", "main");
    EXPECT_TRUE(helpers::Match(output, "60\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_static_modified.abc", "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrValid(graph, binaryInstToCheck);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForBinary(expectedOpcode));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_shifts_static_modified.abc",
                                       "bininst_shifts_static", "main");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

class LibAbcKitCreateBinInstShiftsTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateShl, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateBinInstShiftsTest, CreateShlValid)
{
    ValidTest(g_statG->iCreateShl, ABCKIT_ISA_API_STATIC_OPCODE_SHL, "40\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateAShr, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateBinInstShiftsTest, CreateAShrValid)
{
    ValidTest(g_statG->iCreateAShr, ABCKIT_ISA_API_STATIC_OPCODE_ASHR, "2\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateShr, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateBinInstShiftsTest, CreateShrValid)
{
    ValidTest(g_statG->iCreateShr, ABCKIT_ISA_API_STATIC_OPCODE_SHR, "2\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
