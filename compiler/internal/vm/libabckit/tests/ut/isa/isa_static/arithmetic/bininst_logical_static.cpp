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

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"
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
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_logical_static.abc",
                                            "bininst_logical_static", "main");
    EXPECT_TRUE(helpers::Match(output, "60\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_logical_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_logical_static_modified.abc", "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrValid(graph, binaryInstToCheck);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForBinary(expectedOpcode));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_logical_static_modified.abc",
                                  "bininst_logical_static", "main");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

class LibAbcKitCreateBinInstLogicalTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateAnd, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateBinInstLogicalTest, CreateAndValid)
{
    ValidTest(g_statG->iCreateAnd, ABCKIT_ISA_API_STATIC_OPCODE_AND, "2\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateOr, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateBinInstLogicalTest, CreateOrValid)
{
    ValidTest(g_statG->iCreateOr, ABCKIT_ISA_API_STATIC_OPCODE_OR, "10\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateXor, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateBinInstLogicalTest, CreateXorValid)
{
    ValidTest(g_statG->iCreateXor, ABCKIT_ISA_API_STATIC_OPCODE_XOR, "8\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
