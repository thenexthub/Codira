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

#include <gtest/gtest.h>

#include "libabckit/c/abckit.h"
#include "libabckit/c/metadata_core.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitIrInstInputsTest : public ::testing::Test {};

namespace {

void TransformIrStatic(AbckitGraph *graph)
{
    auto constant1 = g_implG->gFindOrCreateConstantI64(graph, 10);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto constant2 = g_implG->gFindOrCreateConstantI64(graph, 20);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *callInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    ASSERT_NE(callInst, nullptr);

    g_implG->iSetInput(callInst, constant1, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iSetInput(callInst, constant2, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

void TransformIrStatic2(AbckitGraph *graph)
{
    auto constant1 = g_implG->gFindOrCreateConstantI64(graph, 10U);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto constant2 = g_implG->gFindOrCreateConstantI64(graph, 20U);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *callInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    ASSERT_NE(callInst, nullptr);

    g_implG->iSetInputs(callInst, 2U, constant1, constant2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

// Test: test-kind=api, api=GraphApiImpl::iSetInput, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstInputsTest, StaticSetInput)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_static.abc",
                                            "inst_inputs_static", "main");
    EXPECT_TRUE(helpers::Match(output, "1 2\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) { TransformIrStatic(graph); },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {5, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  }},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {0, 1}},
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_static_modified.abc",
                                       "inst_inputs_static", "main");
    EXPECT_TRUE(helpers::Match(output, "10 20\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iSetInputs, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstInputsTest, StaticSetInputs)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_static.abc",
                                            "inst_inputs_static", "main");
    EXPECT_TRUE(helpers::Match(output, "1 2\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) { TransformIrStatic2(graph); },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {5, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}},
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}},
                  }},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {0, 1}},
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_static_modified.abc",
                                       "inst_inputs_static", "main");
    EXPECT_TRUE(helpers::Match(output, "10 20\n"));
}

namespace {
void TransformIrDynamic(AbckitGraph *graph)
{
    auto constant1 = g_implG->gFindOrCreateConstantI64(graph, 10);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto constant2 = g_implG->gFindOrCreateConstantI64(graph, 20);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *callInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1);
    ASSERT_NE(callInst, nullptr);

    g_implG->iSetInput(callInst, constant1, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iSetInput(callInst, constant2, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

void TransformIrDynamic2(AbckitGraph *graph)
{
    auto constant1 = g_implG->gFindOrCreateConstantI64(graph, 10);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto constant2 = g_implG->gFindOrCreateConstantI64(graph, 20);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *callInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1);
    const size_t secondInput = 2;
    g_implG->iSetInputs(callInst, secondInput, constant1, constant2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

// Test: test-kind=api, api=GraphApiImpl::iSetInput, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstInputsTest, DynamicSetInput)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_dynamic_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) { TransformIrDynamic(graph); },
        [](AbckitGraph *graph) {
            std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> insts1(
                {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                 {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
                 {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}});
            helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb1({{}, {1}, insts1});
            std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> insts2({
                {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
                {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {5}},
                {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1, {10, 11, 3}},
                {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
            });
            helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb2({{0}, {2}, insts2});
            helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb3({{1}, {}, {}});
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas({bb1, bb2, bb3});
            helpers::VerifyGraph(graph, bbSchemas);
        });
}

// Test: test-kind=api, api=GraphApiImpl::iSetInputs, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstInputsTest, DynamicSetInputs)
{
    auto cb = [](AbckitGraph *graph) {
        // clang-format off
        std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> insts1({
            {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
            {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
            {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
            {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
            {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
            {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
            {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}
        });
        // clang-format on
        helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb1({{}, {1}, insts1});
        std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> insts2({
            {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
            {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {5}},
            {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1, {10, 11, 3}},
            {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
        });
        helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb2({{0}, {2}, insts2});
        helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb3({{1}, {}, {}});
        std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas({bb1, bb2, bb3});
        helpers::VerifyGraph(graph, bbSchemas);
    };
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/inst_inputs/inst_inputs_dynamic_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) { TransformIrDynamic2(graph); },
        cb);
}

}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
