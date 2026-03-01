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

static void ValidTest(AbckitInst *(*binaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm),
                      AbckitIsaApiStaticOpcode expectedOpcode, const std::string &expectedOutput)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_imm_static.abc",
                                            "bininst_imm_static", "main");
    EXPECT_TRUE(helpers::Match(output, "100\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_imm_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_imm_static_modified.abc", "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrWithImmValid(graph, binaryInstToCheck);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForBinaryWithImm(expectedOpcode));
            helpers::VerifyGraph(graph, bbSchemas);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_imm_static_modified.abc",
                                       "bininst_imm_static", "main");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

class LibAbcKitBinaryInstImmTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateAddI, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBinaryInstImmTest, CreateAddIValid)
{
    ValidTest(g_statG->iCreateAddI, ABCKIT_ISA_API_STATIC_OPCODE_ADDI, "15\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateMulI, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBinaryInstImmTest, CreateMulIValid)
{
    ValidTest(g_statG->iCreateMulI, ABCKIT_ISA_API_STATIC_OPCODE_MULI, "50\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateModI, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBinaryInstImmTest, CreateModIValid)
{
    ValidTest(g_statG->iCreateModI, ABCKIT_ISA_API_STATIC_OPCODE_MODI, "0\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateDivI, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBinaryInstImmTest, CreateDivIValid)
{
    ValidTest(g_statG->iCreateDivI, ABCKIT_ISA_API_STATIC_OPCODE_DIVI, "2\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateSubI, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitBinaryInstImmTest, CreateSubIValid)
{
    ValidTest(g_statG->iCreateSubI, ABCKIT_ISA_API_STATIC_OPCODE_SUBI, "5\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
