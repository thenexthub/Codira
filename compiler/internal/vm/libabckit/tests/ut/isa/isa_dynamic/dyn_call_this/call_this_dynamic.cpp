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
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateDynCallThis : public ::testing::Test {};

static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynCallThisImpl(
    std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> &params,
    helpers::InstSchema<AbckitIsaApiDynamicOpcode> &callInstSchema)
{
    return {{{}, {1}, params},
            {{0},
             {2},
             {
                 {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDHOLE, {}},
                 {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER, {3}},
                 {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {4}},
                 {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE, {4}},
                 {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {6}},
                 callInstSchema,
                 {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
             }},
            {{1}, {}, {}}};
}

// CC-OFFNXT(huge_method, C_RULE_ID_FUNCTION_SIZE) test helper
static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynCallThis(
    AbckitIsaApiDynamicOpcode opcode)
{
    auto paramOp = ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER;
    auto constOp = ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT;
    helpers::InstSchema<AbckitIsaApiDynamicOpcode> callInstSchema {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID, {}};
    std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> params {
        {0, paramOp, {}}, {1, paramOp, {}}, {2, paramOp, {}}};
    switch (opcode) {
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0:
            callInstSchema = {10, opcode, {9, 6}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1:
            params = {{0, paramOp, {}}, {1, paramOp, {}}, {2, paramOp, {}}, {10, constOp, {}}};
            callInstSchema = {11, opcode, {9, 6, 10}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS2:
            params = {
                {0, paramOp, {}}, {1, paramOp, {}}, {2, paramOp, {}}, {10, constOp, {}}, {11, constOp, {}},
            };
            callInstSchema = {12, opcode, {9, 6, 10, 11}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS3:
            params = {
                {0, paramOp, {}},  {1, paramOp, {}},  {2, paramOp, {}},
                {10, constOp, {}}, {11, constOp, {}}, {12, constOp, {}},
            };
            callInstSchema = {13, opcode, {9, 6, 10, 11, 12}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHISRANGE:
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_CALLTHISRANGE:
            params = {
                {0, paramOp, {}},  {1, paramOp, {}},  {2, paramOp, {}},  {10, constOp, {}},
                {11, constOp, {}}, {12, constOp, {}}, {13, constOp, {}},
            };
            callInstSchema = {14, opcode, {9, 6, 10, 11, 12, 13}};
            break;
        case ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID:
            break;
        default:
            LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }
    return CreateBBSchemaForDynCallThisImpl(params, callInstSchema);
}

static void TestHelper(void (*transformIrCall)(AbckitGraph *graph, AbckitInst *obj, AbckitInst *func),
                       AbckitIsaApiDynamicOpcode opcode, const std::string &expectedOutput)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/dyn_call_this/call_this_dynamic.abc",
                                             "call_this_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/dyn_call_this/call_this_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/dyn_call_this/call_this_dynamic_modified.abc",
        "call_this_dynamic.func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE);
            ASSERT_NE(obj, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto func = g_dynG->iCreateLdobjbyname(graph, obj, g_implM->createString(file, "func", strlen("func")));
            ASSERT_NE(func, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->iInsertAfter(func, obj);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            transformIrCall(graph, obj, func);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(CreateBBSchemaForDynCallThis(opcode));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/dyn_call_this/call_this_dynamic_modified.abc", "call_this_dynamic");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

static void TransformIrCallThisarg0(AbckitGraph *graph, AbckitInst *obj, AbckitInst *func)
{
    auto call0 = g_dynG->iCreateCallthis0(graph, func, obj);
    ASSERT_NE(call0, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call0, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCallThisarg1(AbckitGraph *graph, AbckitInst *obj, AbckitInst *func)
{
    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call1 = g_dynG->iCreateCallthis1(graph, func, obj, param0);
    ASSERT_NE(call1, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call1, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCallThisargs2(AbckitGraph *graph, AbckitInst *obj, AbckitInst *func)
{
    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param1 = g_implG->gFindOrCreateConstantI64(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call2 = g_dynG->iCreateCallthis2(graph, func, obj, param0, param1);
    ASSERT_NE(call2, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call2, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCallThisargs3(AbckitGraph *graph, AbckitInst *obj, AbckitInst *func)
{
    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param1 = g_implG->gFindOrCreateConstantI64(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param2 = g_implG->gFindOrCreateConstantI64(graph, 3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call3 = g_dynG->iCreateCallthis3(graph, func, obj, param0, param1, param2);
    ASSERT_NE(call3, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call3, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCallThisrange(AbckitGraph *graph, AbckitInst *obj, AbckitInst *func)
{
    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param1 = g_implG->gFindOrCreateConstantI64(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param2 = g_implG->gFindOrCreateConstantI64(graph, 3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param3 = g_implG->gFindOrCreateConstantI64(graph, 4);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call3 = g_dynG->iCreateCallthisrange(graph, func, 4, obj, param0, param1, param2, param3);
    ASSERT_NE(call3, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call3, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrWideCallThisrange(AbckitGraph *graph, AbckitInst *obj, AbckitInst *func)
{
    auto param0 = g_implG->gFindOrCreateConstantI64(graph, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param1 = g_implG->gFindOrCreateConstantI64(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param2 = g_implG->gFindOrCreateConstantI64(graph, 3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto param3 = g_implG->gFindOrCreateConstantI64(graph, 4);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto call3 = g_dynG->iCreateWideCallthisrange(graph, func, 4, obj, param0, param1, param2, param3);
    ASSERT_NE(call3, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(call3, func);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallthis0, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCallThis, CreateDynCallthis0)
{
    TestHelper(TransformIrCallThisarg0, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, "func\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallthis1, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCallThis, CreateDynCallthis1)
{
    TestHelper(TransformIrCallThisarg1, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1, "func\n1\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallthis2, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCallThis, CreateDynCallthis2)
{
    TestHelper(TransformIrCallThisargs2, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS2, "func\n1\n2\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallthis3, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCallThis, CreateDynCallthis3)
{
    TestHelper(TransformIrCallThisargs3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS3, "func\n1\n2\n3\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallthisrange, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCallThis, CreateDynCallthisrange)
{
    TestHelper(TransformIrCallThisrange, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHISRANGE, "func\n1\n2\n3\n4\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideCallthisrange, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynCallThis, CreateDynWideCallthisrange)
{
    TestHelper(TransformIrWideCallThisrange, ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_CALLTHISRANGE, "func\n1\n2\n3\n4\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
