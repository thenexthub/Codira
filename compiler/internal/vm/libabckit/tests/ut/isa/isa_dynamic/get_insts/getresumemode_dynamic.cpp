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

auto g_icreateGetresumemode1Lambda = [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
    auto *param2 = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);

    auto *bbStart = g_implG->gGetBasicBlock(graph, 5);
    auto *bbEnd = g_implG->gGetBasicBlock(graph, 7);

    auto *bbResume = g_implG->bbCreateEmpty(graph);
    auto *bbEq = g_implG->bbCreateEmpty(graph);
    auto *bbRet = g_implG->bbCreateEmpty(graph);
    auto *bbThrow = g_implG->bbCreateEmpty(graph);

    g_implG->bbDisconnectSuccBlock(bbStart, 0);
    g_implG->bbInsertSuccBlock(bbStart, bbResume, 0);
    g_implG->bbAppendSuccBlock(bbResume, bbEq);
    g_implG->bbAppendSuccBlock(bbResume, bbRet);
    g_implG->bbAppendSuccBlock(bbEq, bbEnd);
    g_implG->bbAppendSuccBlock(bbEq, bbThrow);
    g_implG->bbAppendSuccBlock(bbRet, g_implG->gGetEndBasicBlock(graph));
    g_implG->bbAppendSuccBlock(bbThrow, g_implG->gGetEndBasicBlock(graph));

    auto *add2 = g_dynG->iCreateAdd2(graph, g_implG->gFindOrCreateConstantU64(graph, 7), param2);
    auto *ldfalse = g_dynG->iCreateLdfalse(graph);
    auto *createiterresultobj = g_dynG->iCreateCreateiterresultobj(graph, add2, ldfalse);
    auto *creategeneratorobj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEGENERATOROBJ);
    auto *suspendgenerator = g_dynG->iCreateSuspendgenerator(graph, createiterresultobj, creategeneratorobj);
    auto *resumegenerator = g_dynG->iCreateResumegenerator(graph, creategeneratorobj);
    auto *getresumemode = g_dynG->iCreateGetresumemode(graph, creategeneratorobj);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *const0 = g_implG->gFindOrCreateConstantU64(graph, 0);
    auto *eq = g_dynG->iCreateEq(graph, getresumemode, const0);
    auto *ifInst = g_dynG->iCreateIf(graph, eq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);

    g_implG->bbAddInstBack(bbResume, add2);
    g_implG->bbAddInstBack(bbResume, ldfalse);
    g_implG->bbAddInstBack(bbResume, createiterresultobj);
    g_implG->bbAddInstBack(bbResume, suspendgenerator);
    g_implG->bbAddInstBack(bbResume, resumegenerator);
    g_implG->bbAddInstBack(bbResume, getresumemode);
    g_implG->bbAddInstBack(bbResume, eq);
    g_implG->bbAddInstBack(bbResume, ifInst);

    auto *returnInst = g_dynG->iCreateReturn(graph, resumegenerator);
    g_implG->bbAddInstBack(bbRet, returnInst);

    auto *eq2 = g_dynG->iCreateEq(graph, getresumemode, g_implG->gFindOrCreateConstantU64(graph, 1));
    auto *ifInst2 = g_dynG->iCreateIf(graph, eq2, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ);
    g_implG->bbAddInstBack(bbEq, eq2);
    g_implG->bbAddInstBack(bbEq, ifInst2);

    auto *throwInst = g_dynG->iCreateThrow(graph, resumegenerator);
    g_implG->bbAddInstBack(bbThrow, throwInst);
};

}  // namespace

class LibAbcKitCreateDynGetResumeMode : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGetresumemode, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynGetResumeMode, IcreateGetresumemode_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getresumemode_dynamic.abc",
                                             "getresumemode_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\nundefined\n"));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getresumemode_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getresumemode_dynamic_modified.abc", "foo",
                             g_icreateGetresumemode1Lambda);

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getresumemode_dynamic_modified.abc", "getresumemode_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\n17\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
