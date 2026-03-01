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
#include "libabckit/c/ir_core.h"

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

void TransformLoadStringIr(AbckitFile *file, AbckitGraph *graph)
{
    auto *loadNullStr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    if (loadNullStr != nullptr) {
        g_implG->iRemove(loadNullStr);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    }

    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *str = g_implM->createString(file, "foo", strlen("foo"));
    ASSERT_NE(str, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *loadStr = g_statG->iCreateLoadString(graph, str);
    ASSERT_NE(loadStr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(loadStr, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetInput(ret, loadStr, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace

class LibAbcKitLoadStringStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateLoadString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitLoadStringStaticTest, LibAbcKitTestLoadString)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_string/load_string_static.abc",
                                            "load_string_static", "main");
    EXPECT_TRUE(helpers::Match(output, "\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_string/load_string_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_string/load_string_static_modified.abc", "foo",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformLoadStringIr(file, graph);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{}, {1}, {}},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {2}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_string/load_string_static_modified.abc",
                                       "load_string_static", "main");
    EXPECT_TRUE(helpers::Match(output, "foo\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
