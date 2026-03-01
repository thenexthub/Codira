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

class LibAbcKitCreateReturnInstTest : public ::testing::Test {};

static void TransformIrCreateReturnInstValid(AbckitGraph *graph)
{
    AbckitInst *firstInst = g_implG->gFindOrCreateConstantI64(graph, 5);
    AbckitInst *mainInst = g_statG->iCreateReturn(graph, firstInst);
    ASSERT_NE(mainInst, nullptr);

    auto retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iRemove(retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCreateReturnVoidInstValid(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    AbckitInst *inputInst = g_implG->iGetNext(g_implG->bbGetFirstInst(startBB));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *mainInst = g_statG->iCreateReturnVoid(graph);
    ASSERT_NE(mainInst, nullptr);

    auto callVirt = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    ASSERT_NE(callVirt, nullptr);
    AbckitInst *nextInst = g_implG->iGetNext(callVirt);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iRemove(nextInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertAfter(mainInst, callVirt);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iSetInput(callVirt, inputInst, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateReturn, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateReturnInstTest, CreateReturnValid)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/return/return_static.abc", "return_static", "main");
    EXPECT_TRUE(helpers::Match(output, "10\n11\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/return/return_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/return/return_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrCreateReturnInstValid(graph);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{4, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                   {5, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                   {2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}}},
                 {{0}, {2}, {{3, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {2}}}},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/return/return_static_modified.abc",
                                       "return_static", "main");
    EXPECT_TRUE(helpers::Match(output, "5\n11\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateReturnVoid, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateReturnInstTest, CreateReturnVoidValid)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/return/return_static.abc", "return_static", "main");
    EXPECT_TRUE(helpers::Match(output, "10\n11\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/return/return_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/return/return_static_modified.abc", "foo1",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrCreateReturnVoidInstValid(graph);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{7, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}, {1, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}}},
                 {{0},
                  {2},
                  {{2, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTATIC, {}},
                   {5, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {2, 1}},
                   {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}}}},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/return/return_static_modified.abc",
                                       "return_static", "main");
    EXPECT_TRUE(helpers::Match(output, "10\n10\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
