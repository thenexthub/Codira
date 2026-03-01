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

#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/abckit.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/statuses.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitCreateDynLexEnvVar : public ::testing::Test {};

static void TransformIrMain(AbckitFile *file, AbckitGraph *graph, bool isWideMode, bool isWithName)
{
    auto *mainBB = g_implG->gGetBasicBlock(graph, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *firstConst = g_implG->gFindOrCreateConstantI32(graph, 0x1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *secondConst = g_implG->gFindOrCreateConstantI32(graph, 0x3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *newlexenv = nullptr;
    if (isWithName) {
        std::array<AbckitLiteral *, 5U> statlitarr = {
            g_implM->createLiteralU32(file, 2), g_implM->createLiteralString(file, "x", strlen("x")),
            g_implM->createLiteralU32(file, 0), g_implM->createLiteralString(file, "y", strlen("x")),
            g_implM->createLiteralU32(file, 1)};
        AbckitLiteralArray *litarr = g_implM->createLiteralArray(file, statlitarr.data(), 5U);
        newlexenv = isWideMode ? g_dynG->iCreateWideNewlexenvwithname(graph, 0x2, litarr)
                               : g_dynG->iCreateNewlexenvwithname(graph, 0x2, litarr);
    } else {
        newlexenv = isWideMode ? g_dynG->iCreateWideNewlexenv(graph, 0x2) : g_dynG->iCreateNewlexenv(graph, 0x2);
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *firstStlexvar = nullptr;
    if (isWideMode) {
        firstStlexvar = g_dynG->iCreateWideStlexvar(graph, firstConst, 0x0, 0x0);
    } else {
        firstStlexvar = g_dynG->iCreateStlexvar(graph, firstConst, 0x0, 0x0);
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitInst *secondStlexvar = nullptr;
    if (isWideMode) {
        secondStlexvar = g_dynG->iCreateWideStlexvar(graph, secondConst, 0x0, 0x1);
    } else {
        secondStlexvar = g_dynG->iCreateStlexvar(graph, secondConst, 0x0, 0x1);
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *firstInstToInsert = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(secondStlexvar, firstInstToInsert);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(firstStlexvar, secondStlexvar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->bbAddInstFront(mainBB, newlexenv);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *poplexenv = g_dynG->iCreatePoplexenv(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(poplexenv, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrFuncs(AbckitFile *file, AbckitGraph *graph, uint32_t slot, bool isWideMode)
{
    auto *firstConst = g_implG->gFindOrCreateConstantI32(graph, 0x1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *ldlexvar = nullptr;
    if (isWideMode) {
        ldlexvar = g_dynG->iCreateWideLdlexvar(graph, 0x0, slot);
    } else {
        ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, slot);
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    AbckitString *name = nullptr;
    if (slot == 0x0) {
        name = g_implM->createString(file, "x", strlen("x"));
    } else if (slot == 0x1) {
        name = g_implM->createString(file, "y", strlen("y"));
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *throwundef = g_dynG->iCreateThrowUndefinedifholewithname(graph, ldlexvar, name);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *add2 = g_dynG->iCreateAdd2(graph, firstConst, ldlexvar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(add2, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(throwundef, add2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(ldlexvar, throwundef);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetInput(ret, add2, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void ValidateIrMain(AbckitGraph *graph, bool isWideMode, bool isWithName)
{
    AbckitIsaApiDynamicOpcode newlexenv;
    if (isWithName) {
        newlexenv = isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_NEWLEXENVWITHNAME
                               : ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWLEXENVWITHNAME;
    } else {
        newlexenv = isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_NEWLEXENV : ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWLEXENV;
    }
    AbckitIsaApiDynamicOpcode stlexvar =
        isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STLEXVAR : ABCKIT_ISA_API_DYNAMIC_OPCODE_STLEXVAR;
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema = {
        {{},
         {1},
         {
             {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
             {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
         }},
        {{0},
         {2},
         {
             {3, newlexenv, {}},
             {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
             {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
             {9, stlexvar, {8}},
             {11, stlexvar, {10}},
             {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
             {13, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {7}},
             {14, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {12, 13}},
             {15, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME, {}},
             {16, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {6}},
             {17, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {15, 16}},
             {18, ABCKIT_ISA_API_DYNAMIC_OPCODE_POPLEXENV, {}},
             {19, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void ValidateIrFuncs(AbckitGraph *graph, bool isWideMode)
{
    AbckitIsaApiDynamicOpcode ldlexvar =
        isWideMode ? ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDLEXVAR : ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLEXVAR;
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema = {
        {{},
         {1},
         {
             {0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
             {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT, {}},
         }},
        {{0},
         {2},
         {
             {3, ldlexvar, {}},
             {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME, {3}},
             {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2, {5, 3}},
             {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {6}},
         }},
        {{1}, {}, {}}};

    helpers::VerifyGraph(graph, bbSchema);
}

static void TestLexEnvVar(bool isWideMode, bool isWithName = false)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/lexenv/lexenv_dynamic.abc", "lexenv_dynamic");
    EXPECT_TRUE(helpers::Match(output, "0\n0\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/lexenv/lexenv_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/lexenv/lexenv_dynamic_modified.abc", "lexenv_dynamic.func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*func*/, AbckitGraph *graph) {
            TransformIrMain(file, graph, isWideMode, isWithName);
        },
        [&](AbckitGraph *graph) { ValidateIrMain(graph, isWideMode, isWithName); });

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/lexenv/lexenv_dynamic_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/lexenv/lexenv_dynamic_modified.abc", "lexenv_dynamic.foo",
        [&](AbckitFile *file, AbckitCoreFunction * /*func*/, AbckitGraph *graph) {
            TransformIrFuncs(file, graph, 0x0, isWideMode);
        },
        [&](AbckitGraph *graph) { ValidateIrFuncs(graph, isWideMode); });

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/lexenv/lexenv_dynamic_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/lexenv/lexenv_dynamic_modified.abc", "lexenv_dynamic.bar",
        [&](AbckitFile *file, AbckitCoreFunction * /*func*/, AbckitGraph *graph) {
            TransformIrFuncs(file, graph, 0x1, isWideMode);
        },
        [&](AbckitGraph *graph) { ValidateIrFuncs(graph, isWideMode); });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/lexenv/lexenv_dynamic_modified.abc",
                                        "lexenv_dynamic");
    EXPECT_TRUE(helpers::Match(output, "2\n4\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateNewlexenv, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynLexEnvVar, NewLexenv)
{
    TestLexEnvVar(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStlexvar, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynLexEnvVar, StLexvar)
{
    TestLexEnvVar(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdlexvar, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynLexEnvVar, LdLexvar)
{
    TestLexEnvVar(false);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideNewlexenv, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynLexEnvVar, WideNewLexenv)
{
    TestLexEnvVar(true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideStlexvar, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynLexEnvVar, WideStLexvar)
{
    TestLexEnvVar(true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideLdlexvar, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynLexEnvVar, WideLdLexvar)
{
    TestLexEnvVar(true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateNewlexenvwithname, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynLexEnvVar, NewLexenvWithName)
{
    TestLexEnvVar(false, true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideNewlexenvwithname, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynLexEnvVar, WideNewLexenvWithName)
{
    TestLexEnvVar(true, true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreatePoplexenv, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynLexEnvVar, PopLexenv)
{
    TestLexEnvVar(false);
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
