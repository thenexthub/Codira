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
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/abckit.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "helpers_arithmetic.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static void ValidTest(AbckitInst *(*binaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1),
                      AbckitIsaApiDynamicOpcode expectedOpcode, const std::string &expectedOutput)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/bininst_logical_dynamic.abc",
                                             "bininst_logical_dynamic");
    EXPECT_TRUE(helpers::Match(output, "60\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/bininst_logical_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/bininst_logical_dynamic_modified.abc", "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrValid(graph, binaryInstToCheck, 4, 3);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForDynBinary(expectedOpcode));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/bininst_logical_dynamic_modified.abc", "bininst_logical_dynamic");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

class LibAbcKitCreateDynBinInstLogicalTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAnd2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstLogicalTest, CreateDynAnd2Valid)
{
    ValidTest(g_dynG->iCreateAnd2, ABCKIT_ISA_API_DYNAMIC_OPCODE_AND2, "2\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateOr2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstLogicalTest, CreateDynOr2Valid)
{
    ValidTest(g_dynG->iCreateOr2, ABCKIT_ISA_API_DYNAMIC_OPCODE_OR2, "10\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateXor2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstLogicalTest, CreateDynXor2Valid)
{
    ValidTest(g_dynG->iCreateXor2, ABCKIT_ISA_API_DYNAMIC_OPCODE_XOR2, "8\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
