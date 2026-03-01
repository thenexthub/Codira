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

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

namespace {
void TransformIrDynReturnundefinedInstValid(AbckitGraph *graph,
                                            AbckitInst *(*returnundefinedInstToCheck)(AbckitGraph *graph))
{
    AbckitInst *retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(retOp, nullptr);

    auto *mainInst = returnundefinedInstToCheck(graph);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(mainInst, retOp);
    g_implG->iRemove(retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

void TransformIrDynReturnInstValid(AbckitGraph *graph,
                                   AbckitInst *(*returnInstToCheck)(AbckitGraph *graph, AbckitInst *acc))
{
    AbckitInst *retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(retOp, nullptr);

    auto *constZero = g_implG->gFindOrCreateConstantU64(graph, 0);
    auto *mainInst = returnInstToCheck(graph, constZero);
    ASSERT_NE(mainInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(mainInst, retOp);
    g_implG->iRemove(retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynReturnundefined()
{
    return {{{},
             {1},
             {{1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
            {{0}, {2}, {{5, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}}}},
            {{1}, {}, {}}};
}

std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynReturn()
{
    return {{{},
             {1},
             {{1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
            {{0}, {2}, {{6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {5}}}},
            {{1}, {}, {}}};
}
}  // namespace

class LibAbcKitCreateDynReturnInstTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateReturnundefined, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynReturnInstTest, CreateDynReturnundefinedValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/return/return_dynamic.abc", "return_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/return/return_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/return/return_dynamic_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrDynReturnundefinedInstValid(graph, g_dynG->iCreateReturnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(CreateBBSchemaForDynReturnundefined());
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/return/return_dynamic_modified.abc",
                                        "return_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateReturn, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynReturnInstTest, CreateDynReturnValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/return/return_dynamic.abc", "return_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/return/return_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/return/return_dynamic_modified_0.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrDynReturnInstValid(graph, g_dynG->iCreateReturn);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->gGetStartBasicBlock(graph);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(CreateBBSchemaForDynReturn());
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/return/return_dynamic_modified_0.abc",
                                        "return_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
