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
#include "libabckit/c/isa/isa_static.h"
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

class LibAbcKitGetConstantValueTest : public ::testing::Test {};

static void TransformMethodStaticValid(AbckitGraph *graph, const std::string &functionName)
{
    double firstConstVal = 7.1;
    AbckitInst *firstConst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT);
    if (functionName == "foo1") {
        auto constant = g_implG->iGetConstantValueU64(firstConst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(constant, 5U);
    } else if (functionName == "foo2") {
        auto constant = g_implG->iGetConstantValueF64(firstConst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(constant, firstConstVal);
    } else if (functionName == "foo3") {
        auto constant = g_implG->iGetConstantValueI64(firstConst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(constant, -5L);
    } else {
        LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }
}

static void TransformMethodDynamicValid(AbckitGraph *graph, const std::string &functionName)
{
    AbckitInst *firstConst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT);
    if (functionName == "foo1") {
        g_implG->gFindOrCreateConstantI64(graph, -5L);
        AbckitInst *lastConst = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT);
        auto constant = g_implG->iGetConstantValueI64(lastConst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(constant, -5L);
        AbckitInst *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
        g_implG->iSetInput(call, lastConst, 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    } else if (functionName == "foo2") {
        g_implG->gFindOrCreateConstantU64(graph, 6U);
        AbckitInst *lastConst = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT);
        auto constant = g_implG->iGetConstantValueU64(lastConst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(constant, 6U);
        AbckitInst *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
        g_implG->iSetInput(call, lastConst, 1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    } else if (functionName == "foo3") {
        auto constant = g_implG->iGetConstantValueF64(firstConst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        const size_t expected = 5235346573547;
        ASSERT_EQ(constant, expected);
    } else if (functionName == "foo4") {
        auto constant = g_implG->iGetConstantValueF64(firstConst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(constant, 71e-1);
    } else if (functionName == "foo5") {
        auto constant = g_implG->iGetConstantValueI32(firstConst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(constant, 1);
    } else {
        LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }
}

// Test: test-kind=api, api=GraphApiImpl::iGetConstantValueU64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitGetConstantValueTest, GetConstantValueU64Valid)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_static.abc", "foo1",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodStaticValid(graph, "foo1");
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetConstantValueF64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitGetConstantValueTest, GetConstantValueF64Valid)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_static.abc", "foo2",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodStaticValid(graph, "foo2");
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetConstantValueI64, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitGetConstantValueTest, GetConstantValueI64Valid)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_static.abc", "foo3",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodStaticValid(graph, "foo3");
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetConstantValueI64, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitGetConstantValueTest, GetConstantValueI64ValidDyn)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_dynamic.abc", "get_constant_value_dynamic");
    auto expected = "1\n2\n-5235346573547\n7.1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_dynamic_modified.abc", "foo1",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformMethodDynamicValid(graph, "foo1");
        },
        [](AbckitGraph * /*graph*/) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/ir_core/get_constant_value/get_constant_value_dynamic_modified.abc",
                                        "get_constant_value_dynamic");
    expected = "-5\n2\n-5235346573547\n7.1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=GraphApiImpl::iGetConstantValueU64, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitGetConstantValueTest, GetConstantValueU64ValidDyn)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_dynamic.abc", "get_constant_value_dynamic");
    auto expected = "1\n2\n-5235346573547\n7.1\n";
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_dynamic_modified.abc",
                             "foo2", [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 TransformMethodDynamicValid(graph, "foo2");
                             });
    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/ir_core/get_constant_value/get_constant_value_dynamic_modified.abc",
                                        "get_constant_value_dynamic");
    expected = "1\n6\n-5235346573547\n7.1\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=GraphApiImpl::iGetConstantValueF64, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitGetConstantValueTest, GetConstantValueF64ValidDyn)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_dynamic.abc", "foo3",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodDynamicValid(graph, "foo3");
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetConstantValueF64, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitGetConstantValueTest, GetConstantValueF64ValidDyn_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_dynamic.abc", "foo4",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodDynamicValid(graph, "foo4");
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetConstantValueI32, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitGetConstantValueTest, GetConstantValueI32ValidDyn)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/get_constant_value/get_constant_value_dynamic.abc", "foo5",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               TransformMethodDynamicValid(graph, "foo5");
                           });
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
