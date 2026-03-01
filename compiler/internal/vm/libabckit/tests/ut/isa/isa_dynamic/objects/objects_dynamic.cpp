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
#include "libabckit/c/statuses.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitObjectsTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateemptyobject, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitObjectsTest, IcreateDynCreateemptyobject)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/objects/objects_dynamic.abc", "objects_dynamic");
    EXPECT_TRUE(helpers::Match(output, "\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/objects/objects_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/objects/objects_dynamic_modified.abc", "test",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *callarg0 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0);
            ASSERT_NE(callarg0, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *print = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME);
            ASSERT_NE(print, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->iRemove(callarg0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *emptyobj = g_dynG->iCreateCreateemptyobject(graph);
            ASSERT_NE(emptyobj, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *callarg1 = g_dynG->iCreateCallarg1(graph, print, emptyobj);
            ASSERT_NE(callarg1, nullptr);

            g_implG->iInsertAfter(callarg1, print);
            g_implG->iInsertBefore(emptyobj, print);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema = {
                {{},
                 {1},
                 {
                     {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                     {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                     {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 }},
                {{0},
                 {2},
                 {
                     {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                     {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
                     {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {4, 3}},
                     {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
                 }},
                {{1}, {}, {}}};

            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/objects/objects_dynamic_modified.abc",
                                        "objects_dynamic");
    EXPECT_TRUE(helpers::Match(output, "\\[object Object\\]\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
