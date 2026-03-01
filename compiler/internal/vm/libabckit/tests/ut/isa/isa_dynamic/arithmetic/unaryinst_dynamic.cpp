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

static void ValidTest(AbckitInst *(*unaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0),
                      AbckitIsaApiDynamicOpcode expectedOpcode, const std::string &expectedOutput)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/unaryinst_dynamic.abc",
                                             "unaryinst_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/unaryinst_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/unaryinst_dynamic_modified.abc", "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrUnaryInstValid(graph, unaryInstToCheck, 3);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForDynUnary(expectedOpcode));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arithmetic/unaryinst_dynamic_modified.abc",
                                        "unaryinst_dynamic");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

class LibAbcKitDynUnaryInstTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateNot, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynNotValid)
{
    ValidTest(g_dynG->iCreateNot, ABCKIT_ISA_API_DYNAMIC_OPCODE_NOT, "-11\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateNeg, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynNegValid)
{
    ValidTest(g_dynG->iCreateNeg, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEG, "-10\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateInc, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynIncValid)
{
    ValidTest(g_dynG->iCreateInc, ABCKIT_ISA_API_DYNAMIC_OPCODE_INC, "11\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDec, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynDecValid)
{
    ValidTest(g_dynG->iCreateDec, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEC, "9\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateIstrue, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynIstrueValid)
{
    ValidTest(g_dynG->iCreateIstrue, ABCKIT_ISA_API_DYNAMIC_OPCODE_ISTRUE, "true\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeIstrue, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynCallruntimeIstrueValid)
{
    ValidTest(g_dynG->iCreateCallruntimeIstrue, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISTRUE, "true\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateIsfalse, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynIsfalseValid)
{
    ValidTest(g_dynG->iCreateIsfalse, ABCKIT_ISA_API_DYNAMIC_OPCODE_ISFALSE, "false\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeIsfalse, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynCallruntimeIsfalseValid)
{
    ValidTest(g_dynG->iCreateCallruntimeIsfalse, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISFALSE, "false\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateTonumber, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynTonumberValid)
{
    ValidTest(g_dynG->iCreateTonumber, ABCKIT_ISA_API_DYNAMIC_OPCODE_TONUMBER, "10\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateTonumeric, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynTonumericValid)
{
    ValidTest(g_dynG->iCreateTonumeric, ABCKIT_ISA_API_DYNAMIC_OPCODE_TONUMERIC, "10\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateTypeof, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynUnaryInstTest, CreateDynTypeofValid)
{
    ValidTest(g_dynG->iCreateTypeof, ABCKIT_ISA_API_DYNAMIC_OPCODE_TYPEOF, "number\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
