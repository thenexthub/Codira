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
#include "libabckit/c/statuses.h"

#include <gtest/gtest.h>
#include <cstdint>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateDynTestin : public ::testing::Test {};

static void TransformIr(AbckitGraph *graph, uint64_t slot)
{
    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *par = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
    ASSERT_NE(par, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *testin = g_dynG->iCreateTestin(graph, par, 0x0, slot);
    ASSERT_NE(testin, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(testin, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetInput(ret, testin, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void VerifyIr(AbckitGraph *graph)
{
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema = {
        {{},
         {1},
         {
             {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
         }},
        {{0},
         {2},
         {
             {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_TESTIN, {3}},
             {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void TestImpl(uint64_t slot, const char *expectedOutput)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/testin/testin_dynamic.abc", "testin_dynamic");
    EXPECT_TRUE(helpers::Match(output, "\\[object Object\\]\n\\[object Object\\]\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/testin/testin_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/testin/testin_dynamic_modified.abc", "testin_dynamic.foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) { TransformIr(graph, slot); },
        [&](AbckitGraph *graph) { VerifyIr(graph); });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/testin/testin_dynamic_modified.abc",
                                        "testin_dynamic");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateTestin, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynTestin, CreateTestIn_1)
{
    TestImpl(0x0, "true\nfalse\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateTestin, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitCreateDynTestin, CreateTestIn_2)
{
    TestImpl(0x1, "false\nfalse\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
