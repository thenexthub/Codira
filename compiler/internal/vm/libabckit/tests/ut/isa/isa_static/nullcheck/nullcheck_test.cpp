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
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/metadata_core.h"

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
}  // namespace

class LibAbcKitNullCheckStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateNullCheck, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitNullCheckStaticTest, LibAbcKitTestNullCheck)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/nullcheck/nullcheck_static.abc",
                                            "nullcheck_static", "main");
    EXPECT_NE(output.find("nullcheck_static.A"), std::string::npos);
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/nullcheck/nullcheck_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/nullcheck/nullcheck_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            ASSERT_NE(ret, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            AbckitInst *inputObj = g_implG->iGetInput(ret, 0);

            AbckitInst *nullCheck = g_statG->iCreateNullCheck(graph, inputObj);
            ASSERT_NE(nullCheck, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(nullCheck, ret);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->gDump(graph, 1);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts1({});

            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts0({
                {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                {2, ABCKIT_ISA_API_STATIC_OPCODE_NULLCHECK, {0}},
                {1, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {0}},
            });

            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts2({});

            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb1({{}, {1}, insts1});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb0({{0}, {2}, insts0});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb2({{1}, {}, insts2});

            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas({bb1, bb0, bb2});

            helpers::VerifyGraph(graph, bbSchemas);
        });

    auto modified_output = helpers::ExecuteStaticAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_static/nullcheck/nullcheck_static_modified.abc", "nullcheck_static", "main");
    EXPECT_EQ(modified_output, output);
}

TEST_F(LibAbcKitNullCheckStaticTest, LibAbcKitTestNullCheck2)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/nullcheck/nullcheck_static.abc",
                                            "nullcheck_static", "main");
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/nullcheck/nullcheck_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/nullcheck/nullcheck_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            ASSERT_NE(ret, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *ldaNull = g_statG->gCreateNullPtr(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *nullCheck = g_statG->iCreateNullCheck(graph, ldaNull);
            ASSERT_NE(nullCheck, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(nullCheck, ret);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *createReturn = g_statG->iCreateReturn(graph, ldaNull);
            ASSERT_NE(createReturn, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            helpers::ReplaceInst(ret, createReturn);
            g_implG->gDump(graph, 1);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts1({
                {2, ABCKIT_ISA_API_STATIC_OPCODE_NULLPTR, {}},
            });

            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts0({
                {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                {3, ABCKIT_ISA_API_STATIC_OPCODE_NULLCHECK, {2}},
                {4, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {2}},
            });

            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts2({});

            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb1({{}, {1}, insts1});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb0({{0}, {2}, insts0});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb2({{1}, {}, insts2});

            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas({bb1, bb0, bb2});
            helpers::VerifyGraph(graph, bbSchemas);
        });
    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/nullcheck/nullcheck_static_modified.abc",
                                       "nullcheck_static", "main");
    EXPECT_TRUE(helpers::Match(output, "NullPointerError\n"));
}

}  // namespace libabckit::test
   // NOLINTEND(readability-magic-numbers)