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

class LibAbcKitCreateDynCreateIterResultObjInstTest : public ::testing::Test {};

static void TransformIr(AbckitGraph *graph)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *curInst = g_implG->iGetPrev(g_implG->bbGetLastInst(startBB));
    AbckitInst *firstInst = nullptr;
    AbckitInst *secondInst = nullptr;
    while (curInst != nullptr) {
        AbckitIsaApiDynamicOpcode curOpcode = g_dynG->iGetOpcode(curInst);
        if (curOpcode != ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER) {
            curInst = g_implG->iGetNext(curInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            continue;
        }
        firstInst = curInst;
        curInst = g_implG->iGetNext(curInst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        if (curOpcode != ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER) {
            curInst = g_implG->iGetNext(curInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            continue;
        }
        secondInst = curInst;
        break;
    }

    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitBasicBlock *first = succBBs[0];

    curInst = g_implG->bbGetFirstInst(first);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *inst = g_dynG->iCreateCreateiterresultobj(graph, firstInst, secondInst);
    while (curInst != nullptr) {
        AbckitIsaApiDynamicOpcode curOpcode = g_dynG->iGetOpcode(curInst);
        if (curOpcode != ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN) {
            curInst = g_implG->iGetNext(curInst);
            continue;
        }
        g_implG->iInsertBefore(inst, curInst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        g_implG->iSetInput(curInst, inst, 0);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        curInst = g_implG->iGetNext(curInst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    }
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateiterresultobj, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynCreateIterResultObjInstTest, CreateDynCreateiterresultobjValid)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/iterators/createiterresultobj_dynamic.abc", "createiterresultobj_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/iterators/createiterresultobj_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/iterators/createiterresultobj_dynamic_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIr(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                      {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                      {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                      {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                      {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  }},
                 {{0},
                  {2},
                  {{5, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2, {4, 3}},
                   {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEITERRESULTOBJ, {3, 4}},
                   {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {6}}}},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/iterators/createiterresultobj_dynamic_modified.abc",
                                        "createiterresultobj_dynamic");
    EXPECT_TRUE(helpers::Match(output, "\\[object Object\\]\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
