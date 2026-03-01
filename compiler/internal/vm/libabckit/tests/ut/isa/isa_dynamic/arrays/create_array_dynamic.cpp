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

class LibAbcKitIcreateArrayTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreatearraywithbuffer, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitIcreateArrayTest, IcreateDynCreatearraywithbuffer_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arrays/create_array_dynamic.abc",
                                             "create_array_dynamic");
    EXPECT_TRUE(helpers::Match(output, "aa,10\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arrays/create_array_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arrays/create_array_dynamic_modified.abc", "test",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            // NOTE(mredkin): Remove CreateLiteralU8, CreateLiteralU16, CreateLiteralU64, CreateLiteralFloat API
            auto arr = std::vector<AbckitLiteral *>({
                g_implM->createLiteralBool(file, true),
                // g_implM->createLiteralU8(file, (uint8_t)((uint8_t)2 << 5) + 1),
                // g_implM->createLiteralU16(file, (uint16_t)((uint16_t)2 << 14) + 1),
                g_implM->createLiteralU32(file, (uint32_t)((uint32_t)2 << (uint32_t)30) + 1),
                // g_implM->createLiteralU64(file, (uint64_t)((uint64_t)2 << 62) + 1),
                // g_implM->createLiteralFloat(file, 3.21),
                g_implM->createLiteralDouble(file, 3.22),
                g_implM->createLiteralString(file, "asdf", strlen("asdf")),
            });
            auto litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *inst = g_dynG->iCreateCreatearraywithbuffer(graph, litArr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(inst, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            ASSERT_NE(ret, nullptr);
            g_implG->iInsertBefore(inst, ret);
            g_implG->iSetInput(ret, inst, 0);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arrays/create_array_dynamic_modified.abc",
                                        "create_array_dynamic");
    EXPECT_TRUE(helpers::Match(output, "true,2147483649,3.22,asdf\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateemptyarray, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIcreateArrayTest, IcreateDynCreateemptyarray)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arrays/create_array_dynamic.abc",
                                             "create_array_dynamic");
    EXPECT_TRUE(helpers::Match(output, "aa,10\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arrays/create_array_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arrays/create_array_dynamic_modified.abc", "test",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *inst = g_dynG->iCreateCreateemptyarray(graph);
            ASSERT_NE(inst, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            ASSERT_NE(ret, nullptr);
            g_implG->iInsertBefore(inst, ret);
            g_implG->iSetInput(ret, inst, 0);
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
                     {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER, {}},
                     {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYARRAY, {}},
                     {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {5}},
                 }},
                {{1}, {}, {}}};

            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/arrays/create_array_dynamic_modified.abc",
                                        "create_array_dynamic");
    EXPECT_TRUE(helpers::Match(output, "\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
