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

#include "../arithmetic/helpers_arithmetic.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateDynInstanceofInstTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateInstanceof, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynInstanceofInstTest, CreateDynInstanceofValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/instanceof/instanceof_dynamic.abc",
                                             "instanceof_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\n10\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/instanceof/instanceof_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/instanceof/instanceof_dynamic_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrValid(graph, g_dynG->iCreateInstanceof, 4, 3);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {14, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                      {15, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                      {16, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                      {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                      {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                      {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                      {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                  }},
                 {{0},
                  {3, 2},
                  {{4, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLEXVAR, {}},
                   {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {4}},
                   {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_EQ, {4, 1}},
                   {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF, {6, 2}}}},
                 {{1},
                  {3},
                  {{8, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
                   {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {8, 0}},
                   {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
                   {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {10, 3}}}},
                 {{1, 2},
                  {4},
                  {{12, ABCKIT_ISA_API_DYNAMIC_OPCODE_INSTANCEOF, {1, 0}},
                   {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {12}}}},
                 {{3}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/instanceof/instanceof_dynamic_modified.abc",
                                        "instanceof_dynamic");
    EXPECT_TRUE(helpers::Match(output, "true\nfalse\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
