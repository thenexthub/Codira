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

class LibAbcKitCreateDynCall : public ::testing::Test {};

static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynCall(
    AbckitIsaApiDynamicOpcode opcode)
{
    auto paramOp = ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER;
    auto constOp = ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT;
    helpers::InstSchema<AbckitIsaApiDynamicOpcode> callInstSchema {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID, {}};
    std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> params {
        {0, paramOp, {}}, {1, paramOp, {}}, {2, paramOp, {}}};
    switch (opcode) {
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0:
            callInstSchema = {4, opcode, {3}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1:
            params = {{0, paramOp, {}}, {1, paramOp, {}}, {2, paramOp, {}}, {4, constOp, {}}};
            callInstSchema = {4, opcode, {3, 4}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARGS2:
            params = {
                {0, paramOp, {}}, {1, paramOp, {}}, {2, paramOp, {}}, {4, constOp, {}}, {5, constOp, {}},
            };
            callInstSchema = {4, opcode, {3, 4, 5}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARGS3:
            params = {
                {0, paramOp, {}}, {1, paramOp, {}}, {2, paramOp, {}},
                {4, constOp, {}}, {5, constOp, {}}, {6, constOp, {}},
            };
            callInstSchema = {4, opcode, {3, 4, 5, 6}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRANGE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_CALLRANGE:
            params = {
                {0, paramOp, {}}, {1, paramOp, {}}, {2, paramOp, {}}, {4, constOp, {}},
                {5, constOp, {}}, {6, constOp, {}}, {7, constOp, {}},
            };
            callInstSchema = {4, opcode, {3, 4, 5, 6, 7}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID:
            break;
        default:
            LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }
    return {{{}, {1}, params},
            {{0},
             {2},
             {
                 {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                 callInstSchema,
                 {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
             }},
            {{1}, {}, {}}};
}

static void TestHelper(void (*transformIrCall)(AbckitGraph *graph), AbckitIsaApiDynamicOpcode opcode,
                       const std::string &expectedOutput, bool execute = true)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/dyn_call/call_dynamic.abc", "call_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/dyn_call/call_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/dyn_call/call_dynamic_modified.abc", "call_dynamic.func_main_0",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            transformIrCall(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(CreateBBSchemaForDynCall(opcode));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    if (execute) {
        output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/dyn_call/call_dynamic_modified.abc",
                                            "call_dynamic");
        EXPECT_TRUE(helpers::Match(output, expectedOutput));
    }
}

static void TransformIrCallarg0(AbckitGraph *graph)
{
    auto func = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    ASSERT_NE(func, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call0 = g_dynG->iCreateCallarg0(graph, func);
    ASSERT_NE(call0, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call0, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCallarg1(AbckitGraph *graph)
{
    auto func = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    ASSERT_NE(func, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call1 = g_dynG->iCreateCallarg1(graph, func, param0);
    ASSERT_NE(call1, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call1, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCallargs2(AbckitGraph *graph)
{
    auto func = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    ASSERT_NE(func, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param1 = g_implG->gFindOrCreateConstantI64(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call2 = g_dynG->iCreateCallargs2(graph, func, param0, param1);
    ASSERT_NE(call2, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call2, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCallargs3(AbckitGraph *graph)
{
    auto func = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    ASSERT_NE(func, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param1 = g_implG->gFindOrCreateConstantI64(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param2 = g_implG->gFindOrCreateConstantI64(graph, 3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call3 = g_dynG->iCreateCallargs3(graph, func, param0, param1, param2);
    ASSERT_NE(call3, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call3, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCallrange(AbckitGraph *graph)
{
    auto func = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    ASSERT_NE(func, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param1 = g_implG->gFindOrCreateConstantI64(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param2 = g_implG->gFindOrCreateConstantI64(graph, 3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param3 = g_implG->gFindOrCreateConstantI64(graph, 4);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call4 = g_dynG->iCreateCallrange(graph, func, 4, param0, param1, param2, param3);
    ASSERT_NE(call4, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call4, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrWideCallrange(AbckitGraph *graph)
{
    auto func = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    ASSERT_NE(func, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param1 = g_implG->gFindOrCreateConstantI64(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param2 = g_implG->gFindOrCreateConstantI64(graph, 3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param3 = g_implG->gFindOrCreateConstantI64(graph, 4);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call4 = g_dynG->iCreateWideCallrange(graph, func, 4, param0, param1, param2, param3);
    ASSERT_NE(call4, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call4, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallarg0, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCall, CreateDynCallarg0)
{
    TestHelper(TransformIrCallarg0, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, "func\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallarg1, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCall, CreateDynCallarg1)
{
    TestHelper(TransformIrCallarg1, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, "func\n1\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallargs2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCall, CreateDynCallargs2)
{
    TestHelper(TransformIrCallargs2, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARGS2, "func\n1\n2\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallargs3, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCall, CreateDynCallargs3)
{
    TestHelper(TransformIrCallargs3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARGS3, "func\n1\n2\n3\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallrange, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCall, CreateDynCallrange)
{
    TestHelper(TransformIrCallrange, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRANGE, "func\n1\n2\n3\n4\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideCallrange, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCall, CreateDynWideCallrange)
{
    // Temporary doesn't execute due to Dynamic VM runtime bug #IA954E
    TestHelper(TransformIrWideCallrange, ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_CALLRANGE, "func\n1\n2\n3\n4\n", false);
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
