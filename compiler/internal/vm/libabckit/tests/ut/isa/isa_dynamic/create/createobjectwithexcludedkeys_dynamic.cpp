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

#include "ir_impl.h"
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

class LibAbcKitCreateDynCreateobjectwithexcludedkeys : public ::testing::Test {};

static void FillMainBb1(AbckitFile *file, AbckitGraph *graph, AbckitBasicBlock *bbMain, bool isWide)
{
    auto *createobjectwithbuffer = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
    auto *stringValue = g_implM->createString(file, "value", strlen("value"));
    auto *stringAge = g_implM->createString(file, "age", strlen("age"));
    auto *loadStringValue = g_dynG->iCreateLoadString(graph, stringValue);
    auto *loadStringAge = g_dynG->iCreateLoadString(graph, stringAge);

    auto *createobjectwithexcludedkeys =
        isWide ? g_dynG->iCreateWideCreateobjectwithexcludedkeys(graph, createobjectwithbuffer, loadStringValue, 0x1,
                                                                 loadStringAge)
               : g_dynG->iCreateCreateobjectwithexcludedkeys(graph, createobjectwithbuffer, loadStringValue, 0x1,
                                                             loadStringAge);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *ldundef2 = g_dynG->iCreateLdundefined(graph);
    auto *returnUndef = g_dynG->iCreateReturnundefined(graph);

    g_implG->bbAddInstBack(bbMain, loadStringValue);
    g_implG->bbAddInstBack(bbMain, loadStringAge);
    g_implG->bbAddInstBack(bbMain, createobjectwithexcludedkeys);

    auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
    auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
    auto *callarg11 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, loadStringValue);
    auto *callarg12 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, loadStringAge);
    g_implG->bbAddInstBack(bbMain, tryldglobalbyname);
    g_implG->bbAddInstBack(bbMain, callarg11);
    g_implG->bbAddInstBack(bbMain, callarg12);
    g_implG->bbAddInstBack(bbMain, ldundef2);
    g_implG->bbAddInstBack(bbMain, returnUndef);
}

static void FillMainBb2(AbckitFile *file, AbckitGraph *graph, AbckitBasicBlock *bbMain, bool isWide)
{
    auto *createobjectwithbuffer = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
    auto *stringValue = g_implM->createString(file, "value", strlen("value"));
    auto *stringAge = g_implM->createString(file, "age", strlen("age"));
    auto *stringName = g_implM->createString(file, "name", strlen("name"));
    auto *loadStringValue = g_dynG->iCreateLoadString(graph, stringValue);
    auto *loadStringAge = g_dynG->iCreateLoadString(graph, stringAge);
    auto *createobjectwithexcludedkeys =
        isWide ? g_dynG->iCreateWideCreateobjectwithexcludedkeys(graph, createobjectwithbuffer, loadStringValue, 0x1,
                                                                 loadStringAge)
               : g_dynG->iCreateCreateobjectwithexcludedkeys(graph, createobjectwithbuffer, loadStringValue, 0x1,
                                                             loadStringAge);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->bbAddInstBack(bbMain, loadStringValue);
    g_implG->bbAddInstBack(bbMain, loadStringAge);
    g_implG->bbAddInstBack(bbMain, createobjectwithexcludedkeys);

    auto *ldundef2 = g_dynG->iCreateLdundefined(graph);
    auto *returnUndef = g_dynG->iCreateReturnundefined(graph);
    auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
    auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
    auto *ldobjbynameValue = g_dynG->iCreateLdobjbyname(graph, createobjectwithexcludedkeys, stringValue);
    auto *ldobjbynameAge = g_dynG->iCreateLdobjbyname(graph, createobjectwithexcludedkeys, stringAge);
    auto *ldobjbynameName = g_dynG->iCreateLdobjbyname(graph, createobjectwithexcludedkeys, stringName);
    auto *callarg11 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameValue);
    auto *callarg12 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameAge);
    auto *callarg13 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameName);
    g_implG->bbAddInstBack(bbMain, tryldglobalbyname);
    g_implG->bbAddInstBack(bbMain, ldobjbynameValue);
    g_implG->bbAddInstBack(bbMain, ldobjbynameAge);
    g_implG->bbAddInstBack(bbMain, ldobjbynameName);
    g_implG->bbAddInstBack(bbMain, callarg11);
    g_implG->bbAddInstBack(bbMain, callarg12);
    g_implG->bbAddInstBack(bbMain, callarg13);
    g_implG->bbAddInstBack(bbMain, ldundef2);
    g_implG->bbAddInstBack(bbMain, returnUndef);
}

static void FillUndefBB(AbckitGraph *graph, AbckitBasicBlock *bbUndef)
{
    auto *createobjectwithbuffer = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
    auto *ldundef = g_dynG->iCreateLdundefined(graph);
    auto *strictnoteq2 = g_dynG->iCreateStrictnoteq(graph, ldundef, createobjectwithbuffer);
    auto *ifInst2 = g_dynG->iCreateIf(graph, strictnoteq2, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);

    g_implG->bbAddInstBack(bbUndef, ldundef);
    g_implG->bbAddInstBack(bbUndef, strictnoteq2);
    g_implG->bbAddInstBack(bbUndef, ifInst2);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithexcludedkeys, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynCreateobjectwithexcludedkeys, IcreateCreateobjectwithexcludedkeys_1)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic.abc",
                                   "createobjectwithexcludedkeys_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *bb = helpers::BBgetSuccBlocks(g_implG->gGetStartBasicBlock(graph))[0];
            auto *bbUndef = g_implG->bbCreateEmpty(graph);
            auto *bbThrow = g_implG->bbCreateEmpty(graph);
            auto *bbMain = g_implG->bbCreateEmpty(graph);

            g_implG->bbDisconnectSuccBlock(bb, 0);
            g_implG->bbAppendSuccBlock(bb, bbThrow);
            g_implG->bbAppendSuccBlock(bb, bbUndef);
            g_implG->bbAppendSuccBlock(bbUndef, bbMain);
            g_implG->bbAppendSuccBlock(bbUndef, bbThrow);
            g_implG->bbAppendSuccBlock(bbThrow, bbMain);
            g_implG->bbAppendSuccBlock(bbMain, g_implG->gGetEndBasicBlock(graph));

            auto *createobjectwithbuffer =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            g_implG->iRemove(g_implG->iGetNext(createobjectwithbuffer));
            g_implG->iRemove(g_implG->iGetNext(createobjectwithbuffer));

            auto *ldnull = g_dynG->iCreateLdnull(graph);
            auto *strictnoteq = g_dynG->iCreateStrictnoteq(graph, ldnull, createobjectwithbuffer);
            auto *ifInst = g_dynG->iCreateIf(graph, strictnoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
            g_implG->iInsertAfter(ldnull, createobjectwithbuffer);
            g_implG->iInsertAfter(strictnoteq, ldnull);
            g_implG->iInsertAfter(ifInst, strictnoteq);

            FillUndefBB(graph, bbUndef);

            auto *throwInst = g_dynG->iCreateThrowPatternnoncoercible(graph);
            g_implG->bbAddInstBack(bbThrow, throwInst);

            FillMainBb1(file, graph, bbMain, false);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic_modified.abc",
                                        "createobjectwithexcludedkeys_dynamic");
    EXPECT_TRUE(helpers::Match(output, "value\nage\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithexcludedkeys, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynCreateobjectwithexcludedkeys, IcreateCreateobjectwithexcludedkeys_2)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic.abc",
                                   "createobjectwithexcludedkeys_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *bb = helpers::BBgetSuccBlocks(g_implG->gGetStartBasicBlock(graph))[0];
            auto *bbUndef = g_implG->bbCreateEmpty(graph);
            auto *bbThrow = g_implG->bbCreateEmpty(graph);
            auto *bbMain = g_implG->bbCreateEmpty(graph);

            g_implG->bbDisconnectSuccBlock(bb, 0);
            g_implG->bbAppendSuccBlock(bb, bbThrow);
            g_implG->bbAppendSuccBlock(bb, bbUndef);
            g_implG->bbAppendSuccBlock(bbUndef, bbMain);
            g_implG->bbAppendSuccBlock(bbUndef, bbThrow);
            g_implG->bbAppendSuccBlock(bbThrow, bbMain);
            g_implG->bbAppendSuccBlock(bbMain, g_implG->gGetEndBasicBlock(graph));

            auto *createobjectwithbuffer =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            g_implG->iRemove(g_implG->iGetNext(createobjectwithbuffer));
            g_implG->iRemove(g_implG->iGetNext(createobjectwithbuffer));

            auto *ldnull = g_dynG->iCreateLdnull(graph);
            auto *strictnoteq = g_dynG->iCreateStrictnoteq(graph, ldnull, createobjectwithbuffer);
            auto *ifInst = g_dynG->iCreateIf(graph, strictnoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
            g_implG->iInsertAfter(ldnull, createobjectwithbuffer);
            g_implG->iInsertAfter(strictnoteq, ldnull);
            g_implG->iInsertAfter(ifInst, strictnoteq);

            FillUndefBB(graph, bbUndef);

            auto *throwInst = g_dynG->iCreateThrowPatternnoncoercible(graph);
            g_implG->bbAddInstBack(bbThrow, throwInst);

            FillMainBb2(file, graph, bbMain, false);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic_modified.abc",
                                        "createobjectwithexcludedkeys_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\nundefined\nname\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideCreateobjectwithexcludedkeys, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitCreateDynCreateobjectwithexcludedkeys, IcreateWideCreateobjectwithexcludedkeys_1)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic.abc",
                                   "createobjectwithexcludedkeys_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *bb = helpers::BBgetSuccBlocks(g_implG->gGetStartBasicBlock(graph))[0];
            auto *bbUndef = g_implG->bbCreateEmpty(graph);
            auto *bbThrow = g_implG->bbCreateEmpty(graph);
            auto *bbMain = g_implG->bbCreateEmpty(graph);

            g_implG->bbDisconnectSuccBlock(bb, 0);
            g_implG->bbAppendSuccBlock(bb, bbThrow);
            g_implG->bbAppendSuccBlock(bb, bbUndef);
            g_implG->bbAppendSuccBlock(bbUndef, bbMain);
            g_implG->bbAppendSuccBlock(bbUndef, bbThrow);
            g_implG->bbAppendSuccBlock(bbThrow, bbMain);
            g_implG->bbAppendSuccBlock(bbMain, g_implG->gGetEndBasicBlock(graph));

            auto *createobjectwithbuffer =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            g_implG->iRemove(g_implG->iGetNext(createobjectwithbuffer));
            g_implG->iRemove(g_implG->iGetNext(createobjectwithbuffer));

            auto *ldnull = g_dynG->iCreateLdnull(graph);
            auto *strictnoteq = g_dynG->iCreateStrictnoteq(graph, ldnull, createobjectwithbuffer);
            auto *ifInst = g_dynG->iCreateIf(graph, strictnoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
            g_implG->iInsertAfter(ldnull, createobjectwithbuffer);
            g_implG->iInsertAfter(strictnoteq, ldnull);
            g_implG->iInsertAfter(ifInst, strictnoteq);

            FillUndefBB(graph, bbUndef);

            auto *throwInst = g_dynG->iCreateThrowPatternnoncoercible(graph);
            g_implG->bbAddInstBack(bbThrow, throwInst);

            FillMainBb1(file, graph, bbMain, true);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic_modified.abc",
                                        "createobjectwithexcludedkeys_dynamic");
    EXPECT_TRUE(helpers::Match(output, "value\nage\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideCreateobjectwithexcludedkeys, abc-kind=ArkTS1,
// category=positive
TEST_F(LibAbcKitCreateDynCreateobjectwithexcludedkeys, IcreateWideCreateobjectwithexcludedkeys_2)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic.abc",
                                   "createobjectwithexcludedkeys_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *bb = helpers::BBgetSuccBlocks(g_implG->gGetStartBasicBlock(graph))[0];
            auto *bbUndef = g_implG->bbCreateEmpty(graph);
            auto *bbThrow = g_implG->bbCreateEmpty(graph);
            auto *bbMain = g_implG->bbCreateEmpty(graph);

            g_implG->bbDisconnectSuccBlock(bb, 0);
            g_implG->bbAppendSuccBlock(bb, bbThrow);
            g_implG->bbAppendSuccBlock(bb, bbUndef);
            g_implG->bbAppendSuccBlock(bbUndef, bbMain);
            g_implG->bbAppendSuccBlock(bbUndef, bbThrow);
            g_implG->bbAppendSuccBlock(bbThrow, bbMain);
            g_implG->bbAppendSuccBlock(bbMain, g_implG->gGetEndBasicBlock(graph));

            auto *createobjectwithbuffer =
                helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            g_implG->iRemove(g_implG->iGetNext(createobjectwithbuffer));
            g_implG->iRemove(g_implG->iGetNext(createobjectwithbuffer));

            auto *ldnull = g_dynG->iCreateLdnull(graph);
            auto *strictnoteq = g_dynG->iCreateStrictnoteq(graph, ldnull, createobjectwithbuffer);
            auto *ifInst = g_dynG->iCreateIf(graph, strictnoteq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
            g_implG->iInsertAfter(ldnull, createobjectwithbuffer);
            g_implG->iInsertAfter(strictnoteq, ldnull);
            g_implG->iInsertAfter(ifInst, strictnoteq);

            FillUndefBB(graph, bbUndef);

            auto *throwInst = g_dynG->iCreateThrowPatternnoncoercible(graph);
            g_implG->bbAddInstBack(bbThrow, throwInst);

            FillMainBb2(file, graph, bbMain, true);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithexcludedkeys_dynamic_modified.abc",
                                        "createobjectwithexcludedkeys_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\nundefined\nname\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
