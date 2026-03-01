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

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateNullPtrInstTest : public ::testing::Test {};

static void TransformIrCreateNullPtrInstValid(AbckitGraph *graph)
{
    AbckitInst *mainInst = g_statG->gCreateNullPtr(graph);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    g_implG->iSetInput(callOp, mainInst, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=IsaApiStaticImpl::gCreateNullPtr, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateNullPtrInstTest, CreateNullPtrValid)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/create_nullptr/create_nullptr_static.abc",
                                            "create_nullptr_static", "main");
    EXPECT_TRUE(helpers::Match(output, "10\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_nullptr/create_nullptr_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/create_nullptr/create_nullptr_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrCreateNullPtrInstValid(graph);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_NULLPTR, {}},
                  }},
                 {{0},
                  {2},
                  {{2, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {1}},
                   {3, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}}}},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/create_nullptr/create_nullptr_static_modified.abc",
                                  "create_nullptr_static", "main");
    EXPECT_TRUE(helpers::Match(output, "undefined\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
