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
#include <fstream>

#include "libabckit/c/abckit.h"
#include "adapter_static/ir_static.h"
#include "metadata_inspect_impl.h"
#include "libabckit/c/statuses.h"
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
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitIrInstTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::iSetInput, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IsetInput_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *input = g_implG->iGetNext(g_implG->bbGetFirstInst(start));
                               auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_IF);

                               g_implG->iSetInput(inst, input, 0);
                               ASSERT_EQ(g_implG->iGetInput(inst, 0), input);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iSetInput, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IsetInput_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *input = g_implG->iGetNext(g_implG->bbGetFirstInst(start));
                               auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF);

                               g_implG->iSetInput(inst, input, 0);
                               ASSERT_EQ(g_implG->iGetInput(inst, 0), input);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetInput, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetInput_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *input = g_implG->iGetNext(g_implG->iGetNext(g_implG->bbGetFirstInst(start)));
                               auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_IF);
                               ASSERT_NE(inst, nullptr);
                               ASSERT_EQ(g_implG->iGetInput(inst, 0), input);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetInput, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitIrInstTest, IgetInput_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_IF);

                               ASSERT_EQ(g_implG->iGetInput(inst, -1), nullptr);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               ASSERT_EQ(g_implG->iGetInput(inst, 2), nullptr);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetInput, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetInput_3)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *input = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATER);
                               auto *instIf = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF);

                               ASSERT_EQ(g_implG->iGetInput(instIf, 0), input);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetInput, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IgetInput_4)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF);

                               ASSERT_EQ(g_implG->iGetInput(inst, -1), nullptr);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               ASSERT_EQ(g_implG->iGetInput(inst, 2), nullptr);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetInputCount, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetInputCount_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_IF);

                               ASSERT_EQ(g_implG->iGetInputCount(inst), 2);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetInputCount, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetInputCount_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF);

                               ASSERT_EQ(g_implG->iGetInputCount(inst), 2);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediateCount, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetImmediateCount_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto const2 = g_implG->gFindOrCreateConstantU64(graph, 2);
                               auto *addi = g_statG->iCreateAddI(graph, const1, 2);
                               auto *add = g_statG->iCreateAdd(graph, const1, const2);

                               ASSERT_EQ(g_implG->iGetImmediateCount(addi), 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               ASSERT_EQ(g_implG->iGetImmediateCount(add), 0);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediateCount, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetImmediateCount_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 2);
                               auto *getnextpropname = g_dynG->iCreateGetnextpropname(graph, const1);

                               ASSERT_EQ(g_implG->iGetImmediateCount(newlexenv), 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               ASSERT_EQ(g_implG->iGetImmediateCount(getnextpropname), 0);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediate, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetImmediate_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto *addi = g_statG->iCreateAddI(graph, const1, 2);

                               ASSERT_EQ(g_implG->iGetImmediate(addi, 0), 2);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediate, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitIrInstTest, IgetImmediate_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto const2 = g_implG->gFindOrCreateConstantU64(graph, 2);
                               auto *addi = g_statG->iCreateAddI(graph, const1, 2);
                               auto *add = g_statG->iCreateAdd(graph, const1, const2);

                               ASSERT_EQ(g_implG->iGetImmediate(addi, -1), 0);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               ASSERT_EQ(g_implG->iGetImmediate(addi, 1), 0);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               ASSERT_EQ(g_implG->iGetImmediate(add, 0), 0);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediate, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetImmediate_3)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 2);

                               ASSERT_EQ(g_implG->iGetImmediate(newlexenv, 0), 2);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediate, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IgetImmediate_4)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 2);
                               auto *getnextpropname = g_dynG->iCreateGetnextpropname(graph, const1);

                               ASSERT_EQ(g_implG->iGetImmediate(newlexenv, -1), 0);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               ASSERT_EQ(g_implG->iGetImmediate(newlexenv, 1), 0);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               ASSERT_EQ(g_implG->iGetImmediate(getnextpropname, 0), 0);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iSetImmediate, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IsetImmediate_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto *addi = g_statG->iCreateAddI(graph, const1, 2);

                               ASSERT_EQ(g_implG->iGetImmediate(addi, 0), 2);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               g_implG->iSetImmediate(addi, 0, 3);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               ASSERT_EQ(g_implG->iGetImmediate(addi, 0), 3);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iSetImmediate, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitIrInstTest, IsetImmediate_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto const2 = g_implG->gFindOrCreateConstantU64(graph, 2);
                               auto *addi = g_statG->iCreateAddI(graph, const1, 2);
                               auto *add = g_statG->iCreateAdd(graph, const1, const2);

                               g_implG->iSetImmediate(addi, -1, 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               g_implG->iSetImmediate(addi, 1, 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               g_implG->iSetImmediate(add, 0, 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iSetImmediate, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IsetImmediate_3)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 2);

                               ASSERT_EQ(g_implG->iGetImmediate(newlexenv, 0), 2);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               g_implG->iSetImmediate(newlexenv, 0, 3);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               ASSERT_EQ(g_implG->iGetImmediate(newlexenv, 0), 3);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iSetImmediate, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IsetImmediate_4)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 2);
                               auto *getnextpropname = g_dynG->iCreateGetnextpropname(graph, const1);

                               g_implG->iSetImmediate(newlexenv, -1, 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               g_implG->iSetImmediate(newlexenv, 1, 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               g_implG->iSetImmediate(getnextpropname, 0, 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iSetImmediate, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IsetImmediate_5)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 2);

                               ASSERT_EQ(g_implG->iGetImmediate(newlexenv, 0), 2);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                               g_implG->iSetImmediate(newlexenv, 0, 0x100);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                               ASSERT_EQ(g_implG->iGetImmediate(newlexenv, 0), 2);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediateSize, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetImmediateSize_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 2);

                               ASSERT_EQ(g_implG->iGetImmediateSize(newlexenv, 0), AbckitBitImmSize::BITSIZE_8);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediateSize, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetImmediateSize_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *wideldlexvar = g_dynG->iCreateWideLdlexvar(graph, 1000, 1);

                               ASSERT_EQ(g_implG->iGetImmediateSize(wideldlexvar, 1), AbckitBitImmSize::BITSIZE_16);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediateSize, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetImmediateSize_3)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto *addi = g_statG->iCreateAddI(graph, const1, 2);

                               ASSERT_EQ(g_implG->iGetImmediateSize(addi, 0), AbckitBitImmSize::BITSIZE_8);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetImmediateSize, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetImmediateSize_4)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
                               auto *xori = g_statG->iCreateXorI(graph, const1, 2);

                               ASSERT_EQ(g_implG->iGetImmediateSize(xori, 0), AbckitBitImmSize::BITSIZE_32);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetNext, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetNext_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);
                               auto *next = g_implG->iGetNext(inst);
                               ASSERT_EQ(g_statG->iGetOpcode(next), ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetNext, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetNext_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "Test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);
                               auto *next = g_implG->iGetNext(inst);
                               ASSERT_EQ(g_dynG->iGetOpcode(next), ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetPrev, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetPrev_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);
                               auto *next = g_implG->iGetNext(inst);
                               auto *prev = g_implG->iGetPrev(next);
                               ASSERT_EQ(g_statG->iGetOpcode(prev), ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetPrev, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetPrev_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "Test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);
                               auto *next = g_implG->iGetNext(inst);
                               auto *prev = g_implG->iGetPrev(next);
                               ASSERT_EQ(g_dynG->iGetOpcode(prev), ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iGetOpcode, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetOpcode_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);
                               ASSERT_EQ(g_statG->iGetOpcode(inst), ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iInsertBefore, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IinsertBefore_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = helpers::BBgetSuccBlocks(g_implG->gGetStartBasicBlock(graph))[0];
                               auto *inst = g_implG->bbGetLastInst(start);
                               auto *constOne = g_implG->gFindOrCreateConstantI64(graph, 1U);
                               auto *constZero = g_implG->gFindOrCreateConstantI64(graph, 0U);
                               auto *instAdd = g_statG->iCreateAdd(graph, constOne, constZero);
                               g_implG->iInsertBefore(instAdd, inst);
                               ASSERT_EQ(g_implG->iGetPrev(inst), instAdd);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iInsertBefore, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IinsertBefore_2)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static_modified.abc", "test",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *bb = helpers::BBgetSuccBlocks(start)[0];
            auto *inst = g_implG->iGetNext(g_implG->bbGetFirstInst(start));
            auto *inst2 = g_implG->bbGetFirstInst(bb);
            auto *newInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
            auto *negInst = g_statG->iCreateNeg(graph, newInst);
            g_implG->iInsertBefore(negInst, inst2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{5, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}, {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_NEG, {3}},
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(inst, g_implG->iGetNext(newInst));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::iInsertBefore, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IinsertBefore_3)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "Test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = helpers::BBgetSuccBlocks(g_implG->gGetStartBasicBlock(graph))[0];
                               auto *inst = g_implG->bbGetLastInst(start);
                               auto *constOne = g_implG->gFindOrCreateConstantI64(graph, 1U);
                               auto *constZero = g_implG->gFindOrCreateConstantI64(graph, 0U);
                               auto *instAdd = g_dynG->iCreateAdd2(graph, constOne, constZero);
                               g_implG->iInsertBefore(instAdd, inst);
                               ASSERT_EQ(g_implG->iGetPrev(inst), instAdd);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iInsertBefore, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IinsertBefore_4)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_dynamic_modified.abc", "test",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *bb = helpers::BBgetSuccBlocks(start)[0];
            auto *inst = g_implG->iGetNext(g_implG->iGetNext(g_implG->iGetNext(g_implG->bbGetFirstInst(start))));
            auto *inst2 = g_implG->bbGetFirstInst(bb);
            auto *newInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
            auto *negInst = g_dynG->iCreateNeg(graph, newInst);
            g_implG->iInsertBefore(negInst, inst2);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{5, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {
                      {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEG, {3}},
                      {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(inst, g_implG->iGetNext(newInst));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::iInsertAfter, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IinsertAfter_1)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_static_modified.abc", "test",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *bb = helpers::BBgetSuccBlocks(start)[0];
            auto *inst = g_implG->iGetNext(g_implG->bbGetFirstInst(start));
            auto *newInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
            auto *negInst = g_statG->iCreateNeg(graph, newInst);
            auto *addInst = g_statG->iCreateAdd(graph, negInst, newInst);

            g_implG->bbDisconnectSuccBlock(start, 0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *empty = g_implG->bbCreateEmpty(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAppendSuccBlock(start, empty);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAppendSuccBlock(empty, bb);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->bbAddInstBack(empty, negInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->iInsertAfter(addInst, negInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{6, ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER, {}}, {3, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {{4, ABCKIT_ISA_API_STATIC_OPCODE_NEG, {3}}, {5, ABCKIT_ISA_API_STATIC_OPCODE_ADD, {4, 3}}}},
                 {{1},
                  {3},
                  {
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{2}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(inst, g_implG->iGetNext(newInst));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::iInsertAfter, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IinsertAfter_2)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/basic_blocks/basic_blocks_dynamic_modified.abc", "test",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *bb = helpers::BBgetSuccBlocks(start)[0];
            auto *inst = g_implG->iGetNext(g_implG->iGetNext(g_implG->iGetNext(g_implG->bbGetFirstInst(start))));
            auto *newInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
            auto *negInst = g_dynG->iCreateNeg(graph, newInst);
            auto *addInst = g_dynG->iCreateAdd2(graph, negInst, newInst);

            g_implG->bbDisconnectSuccBlock(start, 0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *empty = g_implG->bbCreateEmpty(graph);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAppendSuccBlock(start, empty);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->bbAppendSuccBlock(empty, bb);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->bbAddInstBack(empty, negInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->iInsertAfter(addInst, negInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(
                {{{},
                  {1},
                  {{6, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                   {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}}}},
                 {{0},
                  {2},
                  {{4, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEG, {3}}, {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2, {4, 3}}}},
                 {{1},
                  {3},
                  {
                      {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
                  }},
                 {{2}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);

            ASSERT_EQ(inst, g_implG->iGetNext(newInst));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=GraphApiImpl::iGetBasicBlock, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetBasicBlock_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);

                               ASSERT_EQ(g_implG->iGetBasicBlock(inst), start);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetBasicBlock, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetBasicBlock_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "Test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);

                               ASSERT_EQ(g_implG->iGetBasicBlock(inst), start);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetGraph, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetGraph_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "Test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);

                               ASSERT_EQ(g_implG->iGetGraph(inst), graph);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iCheckDominance, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IcheckDominance_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);
                               auto *next = g_implG->iGetNext(inst);

                               ASSERT_FALSE(g_implG->iCheckDominance(next, inst));
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iCheckDominance, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IcheckDominance_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "Test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);
                               auto *next = g_implG->iGetNext(inst);

                               ASSERT_FALSE(g_implG->iCheckDominance(next, inst));
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=ApiImpl::GetLastError, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, GetLastError)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);
                               auto *next = g_implG->iGetNext(inst);

                               ASSERT_FALSE(g_implG->iCheckDominance(next, inst));
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=ApiImpl::GetLastError, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, GetLastError_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "Test",
                           [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->bbGetFirstInst(start);
                               auto *next = g_implG->iGetNext(inst);

                               ASSERT_FALSE(g_implG->iCheckDominance(next, inst));
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetUserCount, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetUserCount_1)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->iGetNext(g_implG->bbGetFirstInst(start));

                               ASSERT_EQ(g_implG->iGetUserCount(inst), 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iGetUserCount, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetUserCount_2)
{
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "Test",
                           [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                               auto *start = g_implG->gGetStartBasicBlock(graph);
                               auto *inst = g_implG->iGetNext(g_implG->iGetNext(g_implG->bbGetFirstInst(start)));

                               ASSERT_EQ(g_implG->iGetUserCount(inst), 1);
                               ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                           });
}

// Test: test-kind=api, api=GraphApiImpl::iVisitUsers, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IvisitUsers_1)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *inst = g_implG->bbGetFirstInst(start);
        auto *inst2 = g_implG->iGetNext(g_implG->iGetNext(g_implG->iGetNext(inst)));
        uint32_t counter = 0;
        auto cbVisit = [](AbckitInst * /*input*/, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        };
        g_implG->iVisitUsers(inst2, &counter, cbVisit);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(counter, 1U);
    };

    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;", cb);
}

// Test: test-kind=api, api=GraphApiImpl::iVisitUsers, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IvisitUsers_2)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *inst = g_implG->bbGetFirstInst(start);
        auto *inst2 = g_implG->iGetNext(g_implG->iGetNext(g_implG->iGetNext(g_implG->iGetNext(inst))));
        uint32_t counter = 0;
        auto cbVisit = [](AbckitInst * /*input*/, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        };
        g_implG->iVisitUsers(inst2, &counter, cbVisit);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(counter, 2U);
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func", cb);
}

// Test: test-kind=api, api=GraphApiImpl::iVisitInputs, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IvisitInputs_1)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_IF);
        uint32_t counter = 0;
        auto cbVisit = [](AbckitInst * /*input*/, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        };
        g_implG->iVisitUsers(inst, &counter, cbVisit);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(counter, 0);
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                           "test2:inst_manipulation_static.Test;i32;i32;", cb);
}

// Test: test-kind=api, api=GraphApiImpl::iVisitInputs, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IvisitInputs_2)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_IF);
        uint32_t counter = 0;
        auto cbVisit = [](AbckitInst * /*input*/, void *data) {
            (*(reinterpret_cast<uint32_t *>(data)))++;
            return true;
        };
        g_implG->iVisitUsers(inst, &counter, cbVisit);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(counter, 0);
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func", cb);
}

// Test: test-kind=api, api=GraphApiImpl::iSetLiteralArray, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IsetLiteralArray_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test", [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *constInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
        auto arr = std::vector<AbckitLiteral *>();
        AbckitLiteral *res1 = g_implM->createLiteralString(file, "asdf", strlen("asdf"));
        AbckitLiteral *res2 = g_implM->createLiteralDouble(file, 1.0);
        arr.emplace_back(res1);
        arr.emplace_back(res2);
        auto litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());
        g_implG->iSetLiteralArray(constInst, litArr);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::iSetLiteralArray, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IsetLiteralArray_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc",
        ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc", "test",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *start = g_implG->gGetStartBasicBlock(graph);
            auto *bb = g_implG->bbGetSuccBlock(start, 0);
            auto *inst = g_implG->bbGetFirstInst(bb);
            auto arr = std::vector<AbckitLiteral *>();
            AbckitLiteral *res1 = g_implM->createLiteralString(file, "asdf", strlen("asdf"));
            AbckitLiteral *res2 = g_implM->createLiteralDouble(file, 1.0);
            arr.emplace_back(res1);
            arr.emplace_back(res2);
            auto litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());
            g_implG->iSetLiteralArray(inst, litArr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});
    g_impl->closeFile(file);
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                                   "inst_manipulation_dynamic");
    EXPECT_TRUE(helpers::Match(output, "asdf\n1\nasdf\n4 5 6 7 undefined\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iGetLiteralArray, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IgetLiteralArray_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "test",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *constInst = g_implG->gFindOrCreateConstantI64(graph, 1U);
                                 g_implG->iGetLiteralArray(constInst);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                             });
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::iGetLiteralArray, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetLiteralArray_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", &file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        auto *inst = g_implG->bbGetFirstInst(bb);
        auto *litArr = g_implG->iGetLiteralArray(inst);

        // Test that iGetLiteralArray returns same pointer for same instruction
        auto *litArr2 = g_implG->iGetLiteralArray(inst);
        ASSERT_EQ(litArr, litArr2);

        uint32_t counter = 0;
        g_implI->literalArrayEnumerateElements(
            litArr, &counter, [](AbckitFile * /*file*/, AbckitLiteral *lit, void *data) {
                if (*(reinterpret_cast<uint32_t *>(data)) == 2U) {
                    auto *str = g_implI->literalGetString(lit);
                    EXPECT_TRUE(std::string_view(helpers::AbckitStringToString(str)) == "aa");
                    return true;
                }
                // CC-OFFNXT(G.FMT.02)
                if (*(reinterpret_cast<uint32_t *>(data)) == 3U) {
                    auto val = g_implI->literalGetU32(lit);
                    EXPECT_TRUE(val == 10U);
                    return true;
                }
                // CC-OFFNXT(G.FMT.02)
                (*(reinterpret_cast<uint32_t *>(data)))++;
                return true;
            });
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };
    helpers::TransformMethod(file, "test", cb);
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=GraphApiImpl::iGetId, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetId_1)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        auto *inst = g_implG->bbGetFirstInst(bb);
        auto id = g_implG->iGetId(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(id, 3);
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test3", cb);
}

// Test: test-kind=api, api=GraphApiImpl::iGetId, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetId_2)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        auto *inst = g_implG->bbGetFirstInst(bb);
        auto id = g_implG->iGetId(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        std::vector<AbckitBasicBlock *> bbs;
        g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
            reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
            return true;
        });
        // CC-OFFNXT(G.FMT.02)
        for (auto *bb : bbs) {
            auto *curInst = g_implG->bbGetFirstInst(bb);
            while (curInst != nullptr) {
                auto idCurr = g_implG->iGetId(curInst);
                ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                ASSERT_TRUE((curInst == inst) || (idCurr != id));
                curInst = g_implG->iGetNext(curInst);
            }
        }
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test3", cb);
}

// Test: test-kind=api, api=GraphApiImpl::iGetId, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetId_3)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        auto *inst = g_implG->bbGetFirstInst(bb);
        auto id = g_implG->iGetId(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(id, 3);
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func_main_0",
                           cb);
}

// Test: test-kind=api, api=GraphApiImpl::iGetId, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetId_4)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *bb = g_implG->bbGetSuccBlock(start, 0);
        auto *inst = g_implG->bbGetFirstInst(bb);
        auto id = g_implG->iGetId(inst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        std::vector<AbckitBasicBlock *> bbs;
        g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
            reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
            return true;
        });
        for (auto *bb : bbs) {
            auto *curInst = g_implG->bbGetFirstInst(bb);
            while (curInst != nullptr) {
                auto idCurr = g_implG->iGetId(curInst);
                ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                ASSERT_TRUE((curInst == inst) || (idCurr != id));
                curInst = g_implG->iGetNext(curInst);
            }
        }
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func_main_0",
                           cb);
}

// Test: test-kind=api, api=GraphApiImpl::iGetFunction, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetFunction_1)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        AbckitInst *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL);
        // CC-OFFNXT(G.FMT.02)
        AbckitCoreFunction *currMethod = g_implG->iGetFunction(call);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto name = g_implI->functionGetName(currMethod);
        auto nameStr = helpers::AbckitStringToString(name);
        ASSERT_EQ(nameStr, "test:inst_manipulation_static.Test;void;");
        AbckitInst *call2 = g_implG->iGetNext(call);
        AbckitCoreFunction *currMethod2 = g_implG->iGetFunction(call2);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto name2 = g_implI->functionGetName(currMethod2);
        auto nameStr2 = helpers::AbckitStringToString(name2);
        ASSERT_EQ(nameStr2, "test2:inst_manipulation_static.Test;i32;i32;");
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "main", cb);
}

// Test: test-kind=api, api=GraphApiImpl::iGetFunction, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetFunction_2)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto defInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
        AbckitCoreFunction *currMethod = g_implG->iGetFunction(defInst);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto name = g_implI->functionGetName(currMethod);
        auto nameStr = helpers::AbckitStringToString(name);
        ASSERT_EQ(nameStr, "func");
        auto defInst2 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
        // CC-OFFNXT(G.FMT.02)
        AbckitCoreFunction *currMethod2 = g_implG->iGetFunction(defInst2);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto name2 = g_implI->functionGetName(currMethod2);
        auto nameStr2 = helpers::AbckitStringToString(name2);
        ASSERT_EQ(nameStr2, "Test");
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func_main_0",
                           cb);
}

// Test: test-kind=api, api=GraphApiImpl::iGetFunction, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitIrInstTest, IgetFunction_3)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *firstInst = g_implG->bbGetFirstInst(start);
        [[maybe_unused]] AbckitCoreFunction *currMethod = g_implG->iGetFunction(firstInst);
        // CC-OFFNXT(G.FMT.02)
        ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(currMethod, nullptr);
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test3", cb);
}

// Test: test-kind=api, api=GraphApiImpl::iGetFunction, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IgetFunction_4)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *start = g_implG->gGetStartBasicBlock(graph);
        auto *firstInst = g_implG->bbGetFirstInst(start);
        [[maybe_unused]] AbckitCoreFunction *currMethod = g_implG->iGetFunction(firstInst);
        ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_EQ(currMethod, nullptr);
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func_main_0",
                           cb);
}

// Test: test-kind=api, api=GraphApiImpl::iGetFunction, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IgetFunction_6)
{
    auto userInspector = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto *currMethod = g_implG->iGetFunction(call);
        auto name = g_implI->functionGetName(currMethod);
        auto nameStr = helpers::AbckitStringToString(name);
        ASSERT_EQ(nameStr, "log:std.core.Console;std.core.String;void;");
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "test",
                           userInspector);
}

// Test: test-kind=api, api=GraphApiImpl::iRemove, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, Iremove_1)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                                            "inst_manipulation_static", "main");
    EXPECT_TRUE(helpers::Match(output, "test\ntest2\n1 2 3 4 5 0\n"));
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                             "main", [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL);
                                 g_implG->iRemove(call);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });
    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                                  "inst_manipulation_static", "main");
    EXPECT_TRUE(helpers::Match(output, "test2\n1 2 3 4 5 0\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iRemove, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, Iremove_2)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "inst_manipulation_dynamic");
    EXPECT_TRUE(helpers::Match(output, "aa\n10\naa\n4 5 6 7 undefined\n"));
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                             "func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
                                 g_implG->iRemove(call);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });
    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                                   "inst_manipulation_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\naa\n4 5 6 7 undefined\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iRemove, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitIrInstTest, Iremove_3)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                                            "inst_manipulation_static", "main");
    EXPECT_TRUE(helpers::Match(output, "test\ntest2\n1 2 3 4 5 0\n"));
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                             "main", [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL);
                                 g_implG->iRemove(call);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 g_implG->iRemove(call);
                                 ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });
    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                                  "inst_manipulation_static", "main");
    EXPECT_TRUE(helpers::Match(output, "test2\n1 2 3 4 5 0\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iRemove, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, Iremove_4)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "inst_manipulation_dynamic");
    EXPECT_TRUE(helpers::Match(output, "aa\n10\naa\n4 5 6 7 undefined\n"));
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                             "func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
                                 g_implG->iRemove(call);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 g_implG->iRemove(call);
                                 ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });
    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                                   "inst_manipulation_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\naa\n4 5 6 7 undefined\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iRemove, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IcreateCallrange)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "inst_manipulation_dynamic");
    EXPECT_TRUE(helpers::Match(output, "aa\n10\naa\n4 5 6 7 undefined\n"));
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                             "func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
                                 g_implG->iRemove(call);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 g_implG->iRemove(call);
                                 ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });
    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                                   "inst_manipulation_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\naa\n4 5 6 7 undefined\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iAppendInput, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IappendInput_1)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                                            "inst_manipulation_static", "main");
    EXPECT_TRUE(helpers::Match(output, "test\ntest2\n1 2 3 4 5 0\n"));
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                             "main", [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
                                 auto *found = helpers::FindMethodByName(file, "test0:i32;void;");
                                 ASSERT_NE(found, nullptr);
                                 g_implG->iSetFunction(call, found);
                                 auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 6);
                                 g_implG->iAppendInput(call, const1);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });
    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                                  "inst_manipulation_static", "main");
    EXPECT_TRUE(helpers::Match(output, "test\ntest2\ntest0  6\n1 2 3 4 5 0\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iAppendInput, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IappendInput_2)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                                            "inst_manipulation_static", "main");
    auto cb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *call = helpers::FindLastInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL);
        auto *found = helpers::FindMethodByName(file, "test2:inst_manipulation_static.Test;i32;i32;");
        // CC-OFFNXT(G.FMT.02)
        ASSERT_NE(found, nullptr);
        g_implG->iSetFunction(call, found);
        auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 6);
        g_implG->iAppendInput(call, const1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };
    EXPECT_TRUE(helpers::Match(output, "test\ntest2\n1 2 3 4 5 0\n"));
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                             "main", cb);
    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                                  "inst_manipulation_static", "main");
    EXPECT_TRUE(helpers::Match(output, "test\ntest2\ntest2\n1 2 3 4 5 0\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iAppendInput, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IappendInput_3)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "inst_manipulation_dynamic");
    EXPECT_TRUE(helpers::Match(output, "aa\n10\naa\n4 5 6 7 undefined\n"));
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                             "func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *call = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRANGE);
                                 auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 8);
                                 g_implG->iSetImmediate(call, 1, 5);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                                 g_implG->iAppendInput(call, const1);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                             });
    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                                   "inst_manipulation_dynamic");
    EXPECT_TRUE(helpers::Match(output, "aa\n10\naa\n4 5 6 7 8\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iAppendInput, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitIrInstTest, IappendInput_4)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                             "main", [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *start = g_implG->gGetStartBasicBlock(graph);
                                 auto *inst = g_implG->bbGetFirstInst(start);
                                 auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 8);
                                 g_implG->iAppendInput(inst, const1);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                             });
}

// Test: test-kind=api, api=GraphApiImpl::iAppendInput, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitIrInstTest, IappendInput_5)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                             "main", [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING);
                                 auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 8);
                                 g_implG->iAppendInput(inst, const1);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                             });
}

// Test: test-kind=api, api=GraphApiImpl::iAppendInput, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IappendInput_6)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                             "func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *start = helpers::BBgetSuccBlocks(g_implG->gGetStartBasicBlock(graph))[0];
                                 auto *inst = g_implG->bbGetFirstInst(start);
                                 auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 8);
                                 g_implG->iAppendInput(inst, const1);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                             });
}

// Test: test-kind=api, api=GraphApiImpl::iAppendInput, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IappendInput_7)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                             "func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *start = g_implG->gGetStartBasicBlock(graph);
                                 auto *inst = g_implG->bbGetFirstInst(start);
                                 auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 8);
                                 g_implG->iAppendInput(inst, const1);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                             });
}

// Test: test-kind=api, api=GraphApiImpl::iAppendInput, abc-kind=ArkTS1, category=negative
TEST_F(LibAbcKitIrInstTest, IappendInput_8)
{
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic_modified.abc",
                             "func_main_0",
                             [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
                                 auto *call = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0);
                                 auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 8);
                                 g_implG->iAppendInput(call, const1);
                                 ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
                             });
}

// Test: test-kind=api, api=GraphApiImpl::iAppendInput, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IappendInput_9)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                                            "inst_manipulation_static", "main");
    EXPECT_TRUE(helpers::Match(output, "test\ntest2\n1 2 3 4 5 0\n"));
    auto cb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        [[maybe_unused]] auto *start = g_implG->gGetStartBasicBlock(graph);
        // CC-OFFNXT(G.FMT.02)
        auto *call = helpers::FindLastInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
        auto *found =
            helpers::FindMethodByName(file, "_ctor_:inst_manipulation_static.Test;i32;i32;i32;i32;i32;i32;void;");
        ASSERT_NE(found, nullptr);
        g_implG->iSetFunction(call, found);
        auto *const1 = g_implG->gFindOrCreateConstantI32(graph, 8);
        g_implG->iAppendInput(call, const1);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    };
    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                             "main", cb);
    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static_modified.abc",
                                  "inst_manipulation_static", "main");
    EXPECT_TRUE(helpers::Match(output, "test\ntest2\n1 2 3 4 5 8\n"));
}

// Test: test-kind=api, api=GraphApiImpl::iCheckIsCall, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IcheckIsCall_1)
{
    auto cb = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        std::vector<AbckitBasicBlock *> bbs;
        g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
            reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
            return true;
        });
        // CC-OFFNXT(G.FMT.02)
        std::set<AbckitIsaApiDynamicOpcode> callInsns = {
            ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1,
            ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRANGE,
            ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0,
        };
        for (auto *bb : bbs) {
            if (g_implG->bbIsStart(bb)) {
                continue;
            }
            // CC-OFFNXT(G.FMT.02)
            auto *curInst = g_implG->bbGetFirstInst(bb);
            while (curInst != nullptr) {
                auto isCall = g_implG->iCheckIsCall(curInst);
                ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                ASSERT_EQ(isCall, callInsns.find(g_dynG->iGetOpcode(curInst)) != callInsns.end());
                // CC-OFFNXT(G.FMT.02)
                curInst = g_implG->iGetNext(curInst);
            }
        }
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func_main_0",
                           cb);
}

// Test: test-kind=api, api=GraphApiImpl::iCheckIsCall, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, IcheckIsCall_2)
{
    helpers::InspectMethod(
        ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            std::vector<AbckitBasicBlock *> bbs;
            g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
                reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
                return true;
            });
            std::set<AbckitIsaApiStaticOpcode> callInsns = {ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT,
                                                            ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL,
                                                            ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC};
            for (auto *bb : bbs) {
                if (g_implG->bbIsStart(bb)) {
                    continue;
                }
                auto *curInst = g_implG->bbGetFirstInst(bb);
                while (curInst != nullptr) {
                    auto isCall = g_implG->iCheckIsCall(curInst);
                    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                    ASSERT_EQ(isCall, callInsns.find(g_statG->iGetOpcode(curInst)) != callInsns.end());
                    curInst = g_implG->iGetNext(curInst);
                }
            }
        });
}

static void CompareDump1(std::stringstream &ss)
{
    auto expected =
        "\\s+0.any  Parameter\\s+arg 0\\s+\n"
        "r8187 -> r8187 \\[u64\\]\n"
        "\\s+1.any  Parameter\\s+arg 1\\s+\n"
        "r8188 -> r8188 \\[u64\\]\n"
        "\\s+2.any  Parameter\\s+arg 2\\s+\n"
        "r8189 -> r8189 \\[u64\\]\n"
        "\\s+12.i32  Constant\\s+0x0 -> \\(v13\\)\\s+\n"
        "\\s+18.i32  Constant\\s+0x1 -> \\(v19\\)\\s+\n"
        "\\s+22.i32  Constant\\s+0x4 -> \\(v26\\)\\s+\n"
        "\\s+23.i32  Constant\\s+0x5 -> \\(v26\\)\\s+\n"
        "\\s+24.i32  Constant\\s+0x6 -> \\(v26\\)\\s+\n"
        "\\s+25.i32  Constant\\s+0x7 -> \\(v26\\)\\s+\n"
        "\\s+3.any  Intrinsic.definefunc\\s+0x[a-f0-9]+, 0x[a-f0-9]+, 0x1 \\(v21\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+4.any  Intrinsic.definefunc\\s+0x[a-f0-9]+, 0x[a-f0-9]+, 0x5 \\(v26\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+5.any  Intrinsic.ldhole\\s+\\(v6\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+6.any  Intrinsic.defineclasswithbuffer 0x[a-f0-9]+, 0x[a-f0-9]+, "
        // CC-OFFNXT(WordsTool.190)
        "0x[a-f0-9]+, 0x[a-f0-9]+ v5 -> \\(v8, v7\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+7.any  Intrinsic.ldobjbyname\\s+0x[a-f0-9]+, 0x[a-f0-9]+ v6\\s+bc: 0x[a-f0-9]+\n"
        "\\s+8.any  Intrinsic.newobjrange\\s+0x5, 0x[a-f0-9]+ v6 -> "
        "\\(v21, v17, v16, v11, v10\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+9.any  Intrinsic.tryldglobalbyname 0x[a-f0-9]+, 0x[a-f0-9]+ \\(v14\\)\\s+bc: 0x[a-f0-9]+\n"
        // CC-OFFNXT(WordsTool.190)
        "\\s+10.any  Intrinsic.ldobjbyname\\s+0x[a-f0-9]+, 0x[a-f0-9]+ v8 -> \\(v11\\)\\s+bc: 0x[a-f0-9]+\n"
        // CC-OFFNXT(WordsTool.190)
        "\\s+11.any  Intrinsic.callthis0\\s+0x[a-f0-9]+ v10, v8 -> \\(v13\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+13.any  Intrinsic.ldobjbyvalue\\s+0x[a-f0-9]+ v12, v11 -> \\(v14\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+14.any  Intrinsic.callarg1\\s+0x[a-f0-9]+ v9, v13\\s+bc: 0x[a-f0-9]+\n"
        "\\s+15.any  Intrinsic.tryldglobalbyname 0x[a-f0-9]+, 0x[a-f0-9]+ \\(v20\\)\\s+bc: 0x[a-f0-9]+\n"
        // CC-OFFNXT(WordsTool.190)
        "\\s+16.any  Intrinsic.ldobjbyname\\s+0x[a-f0-9]+, 0x[a-f0-9]+ v8 -> \\(v17\\)\\s+bc: 0x[a-f0-9]+\n"
        // CC-OFFNXT(WordsTool.190)
        "\\s+17.any  Intrinsic.callthis0\\s+0x[a-f0-9]+ v16, v8 -> \\(v19\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+19.any  Intrinsic.ldobjbyvalue\\s+0x[a-f0-9]+ v18, v17 -> \\(v20\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+20.any  Intrinsic.callarg1\\s+0x[a-f0-9]+ v15, v19\\s+bc: 0x[a-f0-9]+\n"
        // CC-OFFNXT(WordsTool.190)
        "\\s+21.any  Intrinsic.callarg1\\s+0x[a-f0-9]+ v3, v8\\s+bc: 0x[a-f0-9]+\n"
        "\\s+26.any  Intrinsic.callrange\\s+0x[a-f0-9]+, 0x4 v4, v22, v23, v24, v25\\s+bc: 0x[a-f0-9]+\n"
        "\\s+27.void Intrinsic.returnundefined\\s+bc: 0x[a-f0-9]+\n\n";
    EXPECT_TRUE(helpers::Match(ss.str(), expected));
}

// Test: test-kind=api, api=GraphApiImpl::iDump, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, Idump_1)
{
    auto userInspector = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto fileName = ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/tmpID.txt";
        std::FILE *fp = fopen(fileName, "we");
        ASSERT_NE(fp, nullptr);
        auto fd = fileno(fp);
        std::vector<AbckitBasicBlock *> bbs;
        g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
            reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
            return true;
        });
        for (auto *bb : bbs) {
            auto *curInst = g_implG->bbGetFirstInst(bb);
            while (curInst != nullptr) {
                g_implG->iDump(curInst, fd);
                ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                curInst = g_implG->iGetNext(curInst);
            }
        }
        ASSERT_EQ(fclose(fp), 0);
        std::ifstream tmpFile(fileName);
        std::string line;
        std::stringstream ss;
        ASSERT_TRUE(tmpFile.is_open());
        while (tmpFile.good()) {
            getline(tmpFile, line);
            ss << line << std::endl;
        }
        tmpFile.close();
        std::remove(fileName);

        CompareDump1(ss);
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_dynamic.abc", "func_main_0",
                           userInspector);
}

static void CompareDump2(std::stringstream &ss)
{
    auto expected =
        "\\s+2.i32  Constant\\s+0x1 -> \\(v12, v3\\)\\s+\n"
        "\\s+8.i32  Constant\\s+0x2 -> \\(v12\\)\\s+\n"
        "\\s+9.i32  Constant\\s+0x3 -> \\(v12\\)\\s+\n"
        "\\s+10.i32  Constant\\s+0x4 -> \\(v12\\)\\s+\n"
        "\\s+11.i32  Constant\\s+0x5 -> \\(v12\\)\\s+\n"
        "\\s+0.ref  Intrinsic.AbckitInitObjectShort 0x[a-f0-9]+ \\(v7, v3, v1\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+1.void CallVirtual [a-f0-9]+ inst_manipulation_static.Test::test v0\\s+bc: 0x[a-f0-9]+\n"
        "\\s+3.i32  CallVirtual [a-f0-9]+ inst_manipulation_static.Test::test2 v0, v2\\s+bc: 0x[a-f0-9]+\n"
        "\\s+4.void CallStatic [a-f0-9]+ inst_manipulation_static.ETSGLOBAL::test0\\s+bc: 0x[a-f0-9]+\n"
        "\\s+5.ref  Intrinsic.AbckitLoadString\\s+0x[a-f0-9]+ \\(v6\\)\\s+bc: 0x[a-f0-9]+\n"
        "\\s+6.void CallStatic [a-f0-9]+ inst_manipulation_static.ETSGLOBAL::test1 v5\\s+bc: 0x[a-f0-9]+\n"
        "\\s+7.void CallVirtual [a-f0-9]+ inst_manipulation_static.Test::test2 v0\\s+bc: 0x[a-f0-9]+\n"
        // CC-OFFNXT(WordsTool.190)
        "\\s+12.ref  Intrinsic.AbckitInitObjectRange 0x[a-f0-9]+ v2, v8, v9, v10, v11\\s+bc: 0x[a-f0-9]+\n"
        "\\s+13.void ReturnVoid\\s+bc: 0x[a-f0-9]+\n\n";
    EXPECT_TRUE(helpers::Match(ss.str(), expected));
}

// Test: test-kind=api, api=GraphApiImpl::iDump, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitIrInstTest, Idump_2)
{
    auto userInspector = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto fileName = ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/tmpIS.txt";
        std::FILE *fp = fopen(fileName, "we");
        ASSERT_NE(fp, nullptr);
        auto fd = fileno(fp);
        std::vector<AbckitBasicBlock *> bbs;
        g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
            reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
            return true;
        });
        for (auto *bb : bbs) {
            auto *curInst = g_implG->bbGetFirstInst(bb);
            while (curInst != nullptr) {
                g_implG->iDump(curInst, fd);
                ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
                curInst = g_implG->iGetNext(curInst);
            }
        }
        ASSERT_TRUE(fclose(fp) == 0);
        std::ifstream tmpFile(fileName);
        std::string line;
        std::stringstream ss;
        ASSERT_TRUE(tmpFile.is_open());
        while (tmpFile.good()) {
            getline(tmpFile, line);
            ss << line << std::endl;
        }
        tmpFile.close();
        std::remove(fileName);

        CompareDump2(ss);
    };
    helpers::InspectMethod(ABCKIT_ABC_DIR "ut/ir_core/inst_manipulation/inst_manipulation_static.abc", "main",
                           userInspector);
}
}  // namespace libabckit::test

// NOLINTEND(readability-magic-numbers)
