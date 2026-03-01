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
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/bininst_dynamic.abc",
                                             "bininst_dynamic");
    EXPECT_TRUE(helpers::Match(output, "60\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/bininst_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/bininst_dynamic_modified.abc", "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrValid(graph, binaryInstToCheck, 4, 3);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForDynBinary(expectedOpcode));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/bininst_dynamic_modified.abc",
                                        "bininst_dynamic");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

class LibAbcKitCreateDynBinInstTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateAdd2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynAdd2Valid)
{
    ValidTest(g_dynG->iCreateAdd2, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2, "12\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateMul2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynMul2Valid)
{
    ValidTest(g_dynG->iCreateMul2, ABCKIT_ISA_API_DYNAMIC_OPCODE_MUL2, "20\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateMod2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynMod2Valid)
{
    ValidTest(g_dynG->iCreateMod2, ABCKIT_ISA_API_DYNAMIC_OPCODE_MOD2, "0\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDiv2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynDiv2Valid)
{
    ValidTest(g_dynG->iCreateDiv2, ABCKIT_ISA_API_DYNAMIC_OPCODE_DIV2, "5\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSub2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynSub2Valid)
{
    ValidTest(g_dynG->iCreateSub2, ABCKIT_ISA_API_DYNAMIC_OPCODE_SUB2, "8\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateExp, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynExpValid)
{
    ValidTest(g_dynG->iCreateExp, ABCKIT_ISA_API_DYNAMIC_OPCODE_EXP, "100\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateEq, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynEqValid)
{
    ValidTest(g_dynG->iCreateEq, ABCKIT_ISA_API_DYNAMIC_OPCODE_EQ, "false\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateNoteq, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynNoteqValid)
{
    ValidTest(g_dynG->iCreateNoteq, ABCKIT_ISA_API_DYNAMIC_OPCODE_NOTEQ, "true\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLess, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynLessValid)
{
    ValidTest(g_dynG->iCreateLess, ABCKIT_ISA_API_DYNAMIC_OPCODE_LESS, "false\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLesseq, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynLesseqValid)
{
    ValidTest(g_dynG->iCreateLesseq, ABCKIT_ISA_API_DYNAMIC_OPCODE_LESSEQ, "false\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGreater, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynGreaterValid)
{
    ValidTest(g_dynG->iCreateGreater, ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATER, "true\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGreatereq, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynGreatereqValid)
{
    ValidTest(g_dynG->iCreateGreatereq, ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATEREQ, "true\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStrictnoteq, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynStrictnoteqValid)
{
    ValidTest(g_dynG->iCreateStrictnoteq, ABCKIT_ISA_API_DYNAMIC_OPCODE_STRICTNOTEQ, "true\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStricteq, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynBinInstTest, CreateDynStricteqValid)
{
    ValidTest(g_dynG->iCreateStricteq, ABCKIT_ISA_API_DYNAMIC_OPCODE_STRICTEQ, "false\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
