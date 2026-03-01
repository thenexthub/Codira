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
#include <gtest/gtest.h>
#include <cstdint>

// NOLINTBEGIN(readability-magic-numbers)

namespace libabckit::test {

class LibAbcKitTest : public ::testing::Test {};

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static void TransformMethodStaticValid(AbckitGraph *graph, const std::string &functionName)
{
    AbckitInst *constant = nullptr;
    if (functionName == "foo1") {
        constant = g_implG->gFindOrCreateConstantU64(graph, 0xa);
    } else if (functionName == "foo2") {
        constant = g_implG->gFindOrCreateConstantF64(graph, 12.16);
    } else if (functionName == "foo3") {
        constant = g_implG->gFindOrCreateConstantI64(graph, -5);
    } else if (functionName == "foo5") {
        constant = g_implG->gFindOrCreateConstantI64(graph, 5);
    } else {
        LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }

    AbckitInst *retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    g_implG->iSetInput(retOp, constant, 0);
}

static void TransformMethodStaticUnconnected(AbckitGraph *graph, const std::string &typeName)
{
    AbckitBasicBlock *startBB = g_implG->gGetStartBasicBlock(graph);

    AbckitInst *constant = nullptr;
    if (typeName == "u64") {
        constant = g_implG->gFindOrCreateConstantU64(graph, 0xa);
    } else if (typeName == "f64") {
        constant = g_implG->gFindOrCreateConstantF64(graph, 12.16);
    } else if (typeName == "i64") {
        constant = g_implG->gFindOrCreateConstantI64(graph, -5);
    } else if (typeName == "i32") {
        constant = g_implG->gFindOrCreateConstantI32(graph, 5);
    } else {
        LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }

    ASSERT_NE(constant, nullptr);
    g_implG->bbAddInstFront(startBB, constant);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
}

static void VerifyTransformations(AbckitGraph *graph)
{
    helpers::InstSchema constantInstr {
        2, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}};  // NOTE: NEED TO VERIFY VALUE OF CONSTANT

    // Verify graph
    std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
        {{{},
          {1},
          {
              {4, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
              constantInstr,
          }},
         {{0},
          {2},
          {
              {3, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {2}},
          }},
         {{1}, {}, {}}});
    helpers::VerifyGraph(graph, bbSchemas);
}

class LibAbcKitCreateConstantTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::gFindOrCreateConstantU64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateConstantTest, StaticCreateConstantU64Valid)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static_modify.abc", "foo1",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformMethodStaticValid(graph, "foo1");
        },
        [](AbckitGraph *graph) { VerifyTransformations(graph); });
}

// Test: test-kind=api, api=GraphApiImpl::gFindOrCreateConstantF64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateConstantTest, StaticCreateConstantF64Valid)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static_modified.abc", "foo2",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformMethodStaticValid(graph, "foo2");
        },
        [](AbckitGraph *graph) { VerifyTransformations(graph); });
}

// Test: test-kind=api, api=GraphApiImpl::gFindOrCreateConstantI64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateConstantTest, StaticCreateConstantI64Valid)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static_modified.abc", "foo3",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformMethodStaticValid(graph, "foo3");
        },
        [](AbckitGraph *graph) { VerifyTransformations(graph); });
}

// Test: test-kind=api, api=GraphApiImpl::gFindOrCreateConstantU64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateConstantTest, StaticCreateConstantU64Unconnected)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static.abc", "foo4",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodStaticUnconnected(graph, "u64");
                           });
}

// Test: test-kind=api, api=GraphApiImpl::gFindOrCreateConstantI64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateConstantTest, StaticCreateConstantI64Unconnected)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static.abc", "foo4",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodStaticUnconnected(graph, "i64");
                           });
}

// Test: test-kind=api, api=GraphApiImpl::gFindOrCreateConstantF64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateConstantTest, StaticCreateConstantF64Unconnected)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static.abc", "foo4",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodStaticUnconnected(graph, "f64");
                           });
}

// Test: test-kind=api, api=GraphApiImpl::gFindOrCreateConstantI32, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateConstantTest, StaticCreateConstantI32Valid)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static_modified.abc", "foo5",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformMethodStaticValid(graph, "foo5");
        },
        [](AbckitGraph *graph) { VerifyTransformations(graph); });
}

// Test: test-kind=api, api=GraphApiImpl::gFindOrCreateConstantI32, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitCreateConstantTest, StaticCreateConstantI32Unconnected)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/create_constant/create_constant_static.abc", "foo4",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodStaticUnconnected(graph, "i32");
                           });
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
