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

class LibAbcKitCreateDynGetNextProp : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGetnextpropname, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynGetNextProp, IcreateGetnextpropname_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getnextpropname_dynamic.abc",
                                             "getnextpropname_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getnextpropname_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getnextpropname_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *bb = helpers::BBgetSuccBlocks(g_implG->gGetStartBasicBlock(graph))[0];
            auto *createArr = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER);
            auto *newBB = g_implG->bbCreateEmpty(graph);
            auto *newFalseBB = g_implG->bbCreateEmpty(graph);
            auto *newEndBB = g_implG->bbCreateEmpty(graph);
            g_implG->bbDisconnectSuccBlock(bb, 0);
            g_implG->bbAppendSuccBlock(bb, newBB);
            g_implG->bbAppendSuccBlock(newBB, newEndBB);  // true branch -> end block
            g_implG->bbAppendSuccBlock(newBB, newFalseBB);
            g_implG->bbAppendSuccBlock(newFalseBB, newBB);
            g_implG->bbAppendSuccBlock(newEndBB, g_implG->gGetEndBasicBlock(graph));

            auto *returnundef = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iRemove(returnundef);

            auto *iterator = g_dynG->iCreateGetpropiterator(graph, createArr);
            g_implG->bbAddInstBack(bb, iterator);

            auto *getnextpropname = g_dynG->iCreateGetnextpropname(graph, iterator);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *ldundef = g_dynG->iCreateLdundefined(graph);
            auto *eq = g_dynG->iCreateEq(graph, ldundef, getnextpropname);
            auto *ifInst = g_dynG->iCreateIf(graph, eq, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE);
            g_implG->bbAddInstBack(newBB, getnextpropname);
            g_implG->bbAddInstBack(newBB, ldundef);
            g_implG->bbAddInstBack(newBB, eq);
            g_implG->bbAddInstBack(newBB, ifInst);

            auto *ldundef2 = g_dynG->iCreateLdundefined(graph);
            auto *returnundef2 = g_dynG->iCreateReturnundefined(graph);
            g_implG->bbAddInstBack(newEndBB, ldundef2);
            g_implG->bbAddInstBack(newEndBB, returnundef2);

            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *ldobjbyvalue = g_dynG->iCreateLdobjbyvalue(graph, getnextpropname, createArr);
            auto *callarg1 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbyvalue);
            g_implG->bbAddInstBack(newFalseBB, tryldglobalbyname);
            g_implG->bbAddInstBack(newFalseBB, ldobjbyvalue);
            g_implG->bbAddInstBack(newFalseBB, callarg1);
        });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/getnextpropname_dynamic_modified.abc", "getnextpropname_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\n2\nsuccess\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
