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

class LibAbcKitCreateDynSuperCall : public ::testing::Test {};

static void TransformSuperCallThisRangeIr(AbckitGraph *graph, bool isWideMode)
{
    auto *par = g_implG->iGetPrev(helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER));
    ASSERT_NE(par, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *obj = g_implG->iGetNext(par);
    ASSERT_NE(obj, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *supercall = isWideMode ? g_dynG->iCreateWideSupercallthisrange(graph, 1, obj)
                                       : g_dynG->iCreateSupercallthisrange(graph, 1, obj);
    ASSERT_NE(supercall, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *throwcall1 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL);
    ASSERT_NE(throwcall1, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetInput(throwcall1, supercall, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetInput(ret, supercall, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *throwcall2 = g_dynG->iCreateThrowIfsupernotcorrectcall(graph, par, 0x1);
    ASSERT_NE(throwcall2, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *mainBB = g_implG->gGetBasicBlock(graph, 0);
    ASSERT_NE(mainBB, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->bbAddInstFront(mainBB, throwcall2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstFront(mainBB, supercall);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformSuperCallThisRangeIrLarge(AbckitGraph *graph, bool isWideMode)
{
    auto *par1 = g_implG->iGetNext(helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER));
    par1 = g_implG->iGetNext(g_implG->iGetNext(par1));
    ASSERT_NE(par1, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *par2 = g_implG->iGetNext(par1);
    ASSERT_NE(par2, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *par3 = g_implG->iGetNext(par2);
    auto *par4 = g_implG->iGetNext(par3);
    auto *par5 = g_implG->iGetNext(par4);
    auto *par6 = g_implG->iGetNext(par5);
    auto *par7 = g_implG->iGetNext(par6);
    auto *par8 = g_implG->iGetNext(par7);
    auto *par9 = g_implG->iGetNext(par8);
    auto *par10 = g_implG->iGetNext(par9);
    auto *par11 = g_implG->iGetNext(par10);
    auto *par12 = g_implG->iGetNext(par11);
    auto *par13 = g_implG->iGetNext(par12);
    auto *par14 = g_implG->iGetNext(par13);
    auto *par15 = g_implG->iGetNext(par14);
    auto *par16 = g_implG->iGetNext(par15);

    AbckitInst *supercall =
        isWideMode ? g_dynG->iCreateWideSupercallthisrange(graph, 0x10, par1, par2, par3, par4, par5, par6, par7, par8,
                                                           par9, par10, par11, par12, par13, par14, par15, par16)
                   : g_dynG->iCreateSupercallthisrange(graph, 0x10, par1, par2, par3, par4, par5, par6, par7, par8,
                                                       par9, par10, par11, par12, par13, par14, par15, par16);
    ASSERT_NE(supercall, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *throwcall1 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL);
    ASSERT_NE(throwcall1, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetInput(throwcall1, supercall, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetInput(ret, supercall, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *throwcall2 = g_dynG->iCreateThrowIfsupernotcorrectcall(graph, par1, 0x10);
    ASSERT_NE(throwcall2, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *mainBB = g_implG->gGetBasicBlock(graph, 0);
    ASSERT_NE(mainBB, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->bbAddInstFront(mainBB, throwcall2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstFront(mainBB, supercall);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void VerifySuperCallThisRangeIr(AbckitGraph *graph, bool isWideMode)
{
    AbckitIsaApiDynamicOpcode supercall = isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_SUPERCALLTHISRANGE
                                                     : ABCKIT_ISA_API_DYNAMIC_OPCODE_SUPERCALLTHISRANGE;
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
             {4, supercall, {3}},
             {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL, {2}},
             {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
             {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {6, 3}},
             {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL, {4}},
             {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void TestSuperCallThisRangeImpl(bool isWideMode)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic.abc",
                                             "supercall_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic_modified.abc", "supercall_dynamic.B",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformSuperCallThisRangeIr(graph, isWideMode);
        },
        [&](AbckitGraph *graph) { VerifySuperCallThisRangeIr(graph, isWideMode); });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic_modified.abc",
                                        "supercall_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\n1\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16\n"));
}

static void TestSuperCallThisRangeImplLarge(bool isWideMode)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic.abc",
                                             "supercall_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic_modified.abc", "supercall_dynamic.B",
        [&](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            TransformSuperCallThisRangeIr(graph, isWideMode);
        },
        [&](AbckitGraph *graph) { VerifySuperCallThisRangeIr(graph, isWideMode); });

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic_modified.abc", "supercall_dynamic.D",
        [&](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            TransformSuperCallThisRangeIrLarge(graph, isWideMode);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercall_dynamic_modified.abc",
                                        "supercall_dynamic");
    EXPECT_TRUE(helpers::Match(
        output, "1\n1\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16\n"));
}

static void TransformSuperCallSpreadIr(AbckitGraph *graph)
{
    auto *par0 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
    ASSERT_NE(par0, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *par3 = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
    ASSERT_NE(par3, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *constant = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT);
    ASSERT_NE(constant, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *supercallthis = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_SUPERCALLTHISRANGE);
    ASSERT_NE(supercallthis, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *createemptyarr = g_dynG->iCreateCreateemptyarray(graph);
    ASSERT_NE(createemptyarr, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *starrayspread = g_dynG->iCreateStarrayspread(graph, par3, createemptyarr, constant);
    ASSERT_NE(starrayspread, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *supercallspread = g_dynG->iCreateSupercallspread(graph, par0, createemptyarr);
    ASSERT_NE(supercallspread, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *mainBB = g_implG->gGetBasicBlock(graph, 0x0);
    ASSERT_NE(mainBB, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->bbAddInstFront(mainBB, starrayspread);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->bbAddInstFront(mainBB, createemptyarr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::ReplaceInst(supercallthis, supercallspread);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void VerifySuperCallSpreadIr(AbckitGraph *graph)
{
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema = {
        {{},
         {1},
         {
             {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
         }},
        {{0},
         {2},
         {
             {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYARRAY, {}},
             {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_STARRAYSPREAD, {3, 4, 5}},
             {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_SUPERCALLSPREAD, {0, 4}},
             {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL, {2}},
             {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
             {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {9, 3}},
             {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL, {7}},
             {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {7}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void TestSuperCallSpreadImpl()
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallspread_dynamic.abc",
                                             "supercallspread_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n1,2\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallspread_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallspread_dynamic_modified.abc", "supercallspread_dynamic.B",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformSuperCallSpreadIr(graph);
        },
        [&](AbckitGraph *graph) { VerifySuperCallSpreadIr(graph); });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallspread_dynamic_modified.abc", "supercallspread_dynamic");
    EXPECT_TRUE(helpers::Match(output, "3\n1,2\n"));
}

static void VerifySuperCallArrowRangeIr(AbckitGraph *graph, bool isWideMode)
{
    AbckitIsaApiDynamicOpcode superCall = isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_SUPERCALLARROWRANGE
                                                     : ABCKIT_ISA_API_DYNAMIC_OPCODE_SUPERCALLARROWRANGE;
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
             {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED, {}},
             {4, superCall, {0, 3}},
             {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL, {2}},
             {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL, {4}},
             {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void TransformSuperCallArrowRangeIr(AbckitGraph *graph, bool isWideMode)
{
    auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);

    auto *undef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);

    auto *newsupercall = isWideMode ? g_dynG->iCreateWideSupercallarrowrange(graph, obj, 0x1, undef)
                                    : g_dynG->iCreateSupercallarrowrange(graph, obj, 0x1, undef);
    ASSERT_NE(newsupercall, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *supercall = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_SUPERCALLTHISRANGE);

    helpers::ReplaceInst(supercall, newsupercall);
}

static void TransformSuperCallArrowRangeIrLarge(AbckitGraph *graph, bool isWideMode)
{
    auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);

    auto *par1 = g_implG->iGetNext(obj);
    par1 = g_implG->iGetNext(g_implG->iGetNext(par1));
    ASSERT_NE(par1, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *par2 = g_implG->iGetNext(par1);
    ASSERT_NE(par2, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *par3 = g_implG->iGetNext(par2);
    auto *par4 = g_implG->iGetNext(par3);
    auto *par5 = g_implG->iGetNext(par4);

    auto *newsupercall = isWideMode
                             ? g_dynG->iCreateWideSupercallarrowrange(graph, obj, 0x5, par1, par2, par3, par4, par5)
                             : g_dynG->iCreateSupercallarrowrange(graph, obj, 0x5, par1, par2, par3, par4, par5);
    ASSERT_NE(newsupercall, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *supercall = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_SUPERCALLTHISRANGE);

    helpers::ReplaceInst(supercall, newsupercall);
}

static void TestSuperCallArrowRangeImpl(bool isWideMode)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallarrow_dynamic.abc",
                                             "supercallarrow_dynamic");
    EXPECT_TRUE(helpers::Match(output, "Hello\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallarrow_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallarrow_dynamic_modified.abc", "supercallarrow_dynamic.B",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformSuperCallArrowRangeIr(graph, isWideMode);
        },
        [&](AbckitGraph *graph) { VerifySuperCallArrowRangeIr(graph, isWideMode); });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallarrow_dynamic_modified.abc", "supercallarrow_dynamic");
    EXPECT_TRUE(helpers::Match(output, "Hello\n"));
}

static void TestSuperCallArrowRangeImplLarge(bool isWideMode)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallarrow_dynamic.abc",
                                             "supercallarrow_dynamic");
    EXPECT_TRUE(helpers::Match(output, "Hello\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallarrow_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallarrow_dynamic_modified.abc", "supercallarrow_dynamic.D",
        [&](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            TransformSuperCallArrowRangeIrLarge(graph, isWideMode);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/supercall/supercallarrow_dynamic_modified.abc", "supercallarrow_dynamic");
    EXPECT_TRUE(helpers::Match(output, "Hello\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSupercallthisrange, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSuperCall, CreateSuperCallThisRange)
{
    TestSuperCallThisRangeImpl(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSupercallthisrange, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSuperCall, CreateSuperCallThisRange_large)
{
    TestSuperCallThisRangeImplLarge(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideSupercallthisrange, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSuperCall, CreateWideSuperCallThisRange)
{
    TestSuperCallThisRangeImpl(true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideSupercallthisrange, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSuperCall, CreateWideSuperCallThisRange_large)
{
    TestSuperCallThisRangeImplLarge(true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSupercallspread, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynSuperCall, CreateSuperCallSpread)
{
    TestSuperCallSpreadImpl();
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSupercallarrowrange, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSuperCall, CreateSuperCallArrowRange)
{
    TestSuperCallArrowRangeImpl(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateSupercallarrowrange, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSuperCall, CreateSuperCallArrowRange_large)
{
    TestSuperCallArrowRangeImplLarge(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideSupercallarrowrange, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSuperCall, CreateWideSuperCallArrowRange)
{
    TestSuperCallArrowRangeImpl(true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideSupercallarrowrange, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynSuperCall, CreateWideSuperCallArrowRange_large)
{
    TestSuperCallArrowRangeImplLarge(true);
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
