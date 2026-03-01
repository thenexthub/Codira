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

#include "libabckit/c/abckit.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/statuses.h"
#include "statuses_impl.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformIrIcreateCmp(AbckitGraph *graph, AbckitInst *firstConst, AbckitInst *secondConst)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);
    std::vector<AbckitBasicBlock *> succBBs = helpers::BBgetSuccBlocks(startBB);
    AbckitBasicBlock *firstBB = succBBs[0];
    AbckitInst *cmp = g_statG->iCreateCmp(graph, firstConst, secondConst);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstFront(firstBB, cmp);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    AbckitInst *ret = g_implG->bbGetLastInst(firstBB);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
    g_implG->iSetInput(ret, cmp, 0);
    EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

class LibAbcKitCreateCmpStaticTest : public ::testing::Test {};

template <bool IS_EQ_TEST>
static std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> GetSchema()
{
    size_t firstConstIdx = 1;
    size_t secondConstIdx = 2;
    size_t cmpIdx = 3;
    size_t retIdx = 4;
    if constexpr (IS_EQ_TEST) {
        secondConstIdx = firstConstIdx;
        cmpIdx--;
        retIdx--;
    }
    std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> constInsts = {
        {0, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}, {firstConstIdx, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}};
    if constexpr (!IS_EQ_TEST) {
        constInsts.push_back({secondConstIdx, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}});
    }
    return {{{}, {1}, {constInsts}},
            {{0},
             {2},
             {
                 {cmpIdx, ABCKIT_ISA_API_STATIC_OPCODE_CMP, {firstConstIdx, secondConstIdx}},
                 {retIdx, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {cmpIdx}},
             }},
            {{1}, {}, {}}};
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCmp, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateCmpStaticTest, LibAbcKitTestCmpI1)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *firstConst = g_implG->gFindOrCreateConstantI64(graph, 10);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            AbckitInst *secondConst = g_implG->gFindOrCreateConstantI64(graph, 7);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            TransformIrIcreateCmp(graph, firstConst, secondConst);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema<false>()); });

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc",
                                            "create_cmp_static", "main");
    EXPECT_TRUE(helpers::Match(output, "1\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCmp, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateCmpStaticTest, LibAbcKitTestCmpI2)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *firstConst = g_implG->gFindOrCreateConstantI64(graph, 0);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            AbckitInst *secondConst = g_implG->gFindOrCreateConstantI64(graph, 24);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            TransformIrIcreateCmp(graph, firstConst, secondConst);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema<false>()); });

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc",
                                            "create_cmp_static", "main");
    EXPECT_TRUE(helpers::Match(output, "-1\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCmp, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateCmpStaticTest, LibAbcKitTestCmpI3)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *firstConst = g_implG->gFindOrCreateConstantI64(graph, 7);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            TransformIrIcreateCmp(graph, firstConst, firstConst);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema<true>()); });

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc",
                                            "create_cmp_static", "main");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCmp, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateCmpStaticTest, LibAbcKitTestCmpD1)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *firstConst = g_implG->gFindOrCreateConstantF64(graph, 12.4);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            AbckitInst *secondConst = g_implG->gFindOrCreateConstantF64(graph, 4.5);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            TransformIrIcreateCmp(graph, firstConst, secondConst);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema<false>()); });

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc",
                                            "create_cmp_static", "main");
    EXPECT_TRUE(helpers::Match(output, "1\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCmp, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateCmpStaticTest, LibAbcKitTestCmpD2)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *firstConst = g_implG->gFindOrCreateConstantF64(graph, 4.3);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            AbckitInst *secondConst = g_implG->gFindOrCreateConstantF64(graph, 13.6);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            TransformIrIcreateCmp(graph, firstConst, secondConst);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema<false>()); });

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc",
                                            "create_cmp_static", "main");
    EXPECT_TRUE(helpers::Match(output, "-1\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCmp, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateCmpStaticTest, LibAbcKitTestCmpD3)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *firstConst = g_implG->gFindOrCreateConstantF64(graph, 7.6);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            TransformIrIcreateCmp(graph, firstConst, firstConst);
        },
        [](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema<true>()); });

    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc",
                                            "create_cmp_static", "main");
    EXPECT_TRUE(helpers::Match(output, "0\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateCmp, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitCreateCmpStaticTest, LibAbcKitTestCmpNegative)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/cmp/create_cmp_static_modified_set.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *firstConst = g_implG->gFindOrCreateConstantI64(graph, 10);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            AbckitInst *secondConst = g_implG->gFindOrCreateConstantF64(graph, 7.2);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
            AbckitInst *cmp = g_statG->iCreateCmp(graph, firstConst, secondConst);
            EXPECT_TRUE(cmp == nullptr);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_BAD_ARGUMENT);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
