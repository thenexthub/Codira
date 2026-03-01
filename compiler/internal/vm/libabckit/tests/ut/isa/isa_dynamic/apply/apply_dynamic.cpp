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

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

auto g_icreateApplyLambda0 = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
    auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
    auto *ldundefined = g_dynG->iCreateLdundefined(graph);
    auto *createemptyarray = g_dynG->iCreateCreateemptyarray(graph);
    auto *const0 = g_implG->gFindOrCreateConstantU64(graph, 0x0);
    auto *param3 = g_implG->gGetParameter(graph, 0x3);
    auto *starrayspread = g_dynG->iCreateStarrayspread(graph, param3, createemptyarray, const0);
    auto *stringLength = g_implM->createString(file, "length", strlen("length"));
    auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, createemptyarray, stringLength);
    auto *apply = g_dynG->iCreateApply(graph, ldlexvar, ldundefined, createemptyarray);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *returnundefLast = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
    g_implG->iInsertBefore(ldlexvar, returnundefLast);
    g_implG->iInsertBefore(ldundefined, returnundefLast);
    g_implG->iInsertBefore(createemptyarray, returnundefLast);
    g_implG->iInsertBefore(starrayspread, returnundefLast);
    g_implG->iInsertBefore(ldobjbyname, returnundefLast);
    g_implG->iInsertBefore(apply, returnundefLast);
};

auto g_icreateApplyLambda1 = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
    std::vector<AbckitLiteral *> arr = {g_implM->createLiteralU32(file, 1), g_implM->createLiteralU32(file, 2)};
    auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

    auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
    auto *definefuncfunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    auto *definefunctest = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    auto *stlexvar = g_dynG->iCreateStlexvar(graph, definefuncfunc, 0x0, 0x0);
    auto *createarraywithbuffer = g_dynG->iCreateCreatearraywithbuffer(graph, litArr);
    auto *callarg1 = g_dynG->iCreateCallarg1(graph, definefunctest, createarraywithbuffer);

    auto *returnundef = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
    g_implG->iInsertBefore(newlexenv, definefunctest);
    g_implG->iInsertBefore(stlexvar, returnundef);
    g_implG->iInsertBefore(createarraywithbuffer, returnundef);
    g_implG->iInsertBefore(callarg1, returnundef);
};

}  // namespace

class LibAbcKitCreateDynApply : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateApply, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynApply, IcreateApply)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/apply_dynamic.abc", "apply_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/apply_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/apply_dynamic_modified.abc", "test",
                             g_icreateApplyLambda0);

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/apply_dynamic_modified.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/apply_dynamic_modified.abc", "func_main_0",
                             g_icreateApplyLambda1);

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/apply_dynamic_modified.abc",
                                        "apply_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1 2\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateNewobjapply, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynApply, IcreateNewobjapply_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/newobjapply_dynamic.abc",
                                             "newobjapply_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/newobjapply_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/newobjapply_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *createemptyarray = g_dynG->iCreateCreateemptyarray(graph);
            auto *const0 = g_implG->gFindOrCreateConstantU64(graph, 0x0);
            auto *createarraywithbuffer =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER);
            auto *starrayspread = g_dynG->iCreateStarrayspread(graph, createarraywithbuffer, createemptyarray, const0);
            auto *stringLength = g_implM->createString(file, "length", strlen("length"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, createemptyarray, stringLength);
            auto *defineclasswithbuffer =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
            auto *newobjapply = g_dynG->iCreateNewobjapply(graph, createemptyarray, defineclasswithbuffer);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *stringA = g_implM->createString(file, "a", strlen("a"));
            auto *ldobjbyname2 = g_dynG->iCreateLdobjbyname(graph, newobjapply, stringA);
            auto *callarg1 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbyname2);

            auto *returnundef = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(createemptyarray, returnundef);
            g_implG->iInsertBefore(starrayspread, returnundef);
            g_implG->iInsertBefore(ldobjbyname, returnundef);
            g_implG->iInsertBefore(newobjapply, returnundef);
            g_implG->iInsertBefore(tryldglobalbyname, returnundef);
            g_implG->iInsertBefore(ldobjbyname2, returnundef);
            g_implG->iInsertBefore(callarg1, returnundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/newobjapply_dynamic_modified.abc",
                                        "newobjapply_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateNewobjapply, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynApply, IcreateNewobjapply_2)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/newobjapply_dynamic.abc",
                                             "newobjapply_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/newobjapply_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/newobjapply_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *createemptyarray = g_dynG->iCreateCreateemptyarray(graph);
            auto *const0 = g_implG->gFindOrCreateConstantU64(graph, 0x0);
            auto *createarraywithbuffer =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER);
            auto *starrayspread = g_dynG->iCreateStarrayspread(graph, createarraywithbuffer, createemptyarray, const0);
            auto *stringLength = g_implM->createString(file, "length", strlen("length"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, createemptyarray, stringLength);
            auto *defineclasswithbuffer =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
            auto *newobjapply = g_dynG->iCreateNewobjapply(graph, createemptyarray, defineclasswithbuffer);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *stringA = g_implM->createString(file, "a", strlen("a"));
            auto *ldobjbynameA = g_dynG->iCreateLdobjbyname(graph, newobjapply, stringA);
            auto *callarg1A = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameA);
            auto *stringB = g_implM->createString(file, "b", strlen("b"));
            auto *ldobjbynameB = g_dynG->iCreateLdobjbyname(graph, newobjapply, stringB);
            auto *callarg1B = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameB);
            auto *stringC = g_implM->createString(file, "c", strlen("c"));
            auto *ldobjbynameC = g_dynG->iCreateLdobjbyname(graph, newobjapply, stringC);
            auto *callarg1C = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameC);

            auto *returnundef = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(createemptyarray, returnundef);
            g_implG->iInsertBefore(starrayspread, returnundef);
            g_implG->iInsertBefore(ldobjbyname, returnundef);
            g_implG->iInsertBefore(newobjapply, returnundef);
            g_implG->iInsertBefore(tryldglobalbyname, returnundef);
            g_implG->iInsertBefore(ldobjbynameA, returnundef);
            g_implG->iInsertBefore(ldobjbynameB, returnundef);
            g_implG->iInsertBefore(ldobjbynameC, returnundef);
            g_implG->iInsertBefore(callarg1A, returnundef);
            g_implG->iInsertBefore(callarg1B, returnundef);
            g_implG->iInsertBefore(callarg1C, returnundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/apply/newobjapply_dynamic_modified.abc",
                                        "newobjapply_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\n2\na\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
