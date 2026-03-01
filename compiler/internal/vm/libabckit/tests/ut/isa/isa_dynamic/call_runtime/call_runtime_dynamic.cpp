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

auto g_icreateCallruntimeCallinit1Lambda = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
    auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
    auto *poplexenv = g_dynG->iCreatePoplexenv(graph);

    auto *classConstr = helpers::FindMethodByName(file, "innerConstr");
    ASSERT_NE(classConstr, nullptr);
    auto arr = std::vector<AbckitLiteral *>();
    arr.emplace_back(g_implM->createLiteralU32(file, 0));
    auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

    auto *ldhole = g_dynG->iCreateLdhole(graph);
    auto *defineClass = g_dynG->iCreateDefineclasswithbuffer(graph, classConstr, litArr, 0x0, ldhole);
    auto *stringProt = g_implM->createString(file, "prototype", strlen("prototype"));
    auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, defineClass, stringProt);

    auto *innerInit = helpers::FindMethodByName(file, "innerInit");
    auto *definemethod = g_dynG->iCreateDefinemethod(graph, ldobjbyname, innerInit, 0x0);
    auto *stlexvar = g_dynG->iCreateStlexvar(graph, definemethod, 0x0, 0x0);
    auto *stringD = g_implM->createString(file, "D", strlen("D"));
    auto *definepropertybyname = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEPROPERTYBYNAME);
    g_implG->iSetString(definepropertybyname, stringD);
    g_implG->iSetInput(definepropertybyname, defineClass, 0);

    g_implG->iInsertBefore(newlexenv, definepropertybyname);
    g_implG->iInsertBefore(ldhole, definepropertybyname);
    g_implG->iInsertBefore(defineClass, definepropertybyname);
    g_implG->iInsertBefore(ldobjbyname, definepropertybyname);
    g_implG->iInsertBefore(definemethod, definepropertybyname);
    g_implG->iInsertBefore(stlexvar, definepropertybyname);
    g_implG->iInsertBefore(poplexenv, definepropertybyname);
};

}  // namespace

class LibAbcKitCreateDynCallsRuntime : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeTopropertykey, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynCallsRuntime, IcreateCallruntimeTopropertykey_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/topropertykey_dynamic.abc",
                                             "topropertykey_dynamic");
    EXPECT_TRUE(helpers::Match(output, "undefined\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/topropertykey_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/topropertykey_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *ldundefFirts = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            auto *callCreate = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1);

            auto *ldhole = g_dynG->iCreateLdhole(graph);
            auto *stringX = g_implM->createString(file, "x", strlen("x"));
            auto *loadString = g_dynG->iCreateLoadString(graph, stringX);
            auto *isin = g_dynG->iCreateIsin(graph, callCreate, loadString);
            auto *topropertykey = g_dynG->iCreateCallruntimeTopropertykey(graph, isin);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralU32(file, 0));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());
            auto *classConstr = helpers::FindMethodByName(file, "classConstr");
            ASSERT_NE(classConstr, nullptr);

            auto *defineclasswithbuffer = g_dynG->iCreateDefineclasswithbuffer(graph, classConstr, litArr, 0, ldhole);
            auto *stringProt = g_implM->createString(file, "prototype", strlen("prototype"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, defineclasswithbuffer, stringProt);
            auto *funcGetter = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *ldtrue = g_dynG->iCreateLdtrue(graph);
            auto *definegetter = g_dynG->iCreateDefinegettersetterbyvalue(graph, ldtrue, ldobjbyname, topropertykey,
                                                                          funcGetter, ldundefFirts);
            auto *stringFalse = g_implM->createString(file, "false", strlen("false"));
            auto *ldobjbynameFalse = g_dynG->iCreateLdobjbyname(graph, ldobjbyname, stringFalse);
            auto *callarg1 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
            g_implG->iSetInput(callarg1, ldobjbynameFalse, 1);

            g_implG->iInsertBefore(ldhole, callarg1);
            g_implG->iInsertBefore(loadString, callarg1);
            g_implG->iInsertBefore(isin, callarg1);
            g_implG->iInsertBefore(topropertykey, callarg1);
            g_implG->iInsertBefore(defineclasswithbuffer, callarg1);
            g_implG->iInsertBefore(ldobjbyname, callarg1);
            g_implG->iInsertBefore(ldtrue, callarg1);
            g_implG->iInsertBefore(definegetter, callarg1);
            g_implG->iInsertBefore(ldobjbynameFalse, callarg1);
        });

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/topropertykey_dynamic_modified.abc", "topropertykey_dynamic");
    EXPECT_TRUE(helpers::Match(output, "via get\n"));
}

// CC-OFFNXT(huge_method, C_RULE_ID_FUNCTION_SIZE) test, solid logic
// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCallruntimeCallinit, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynCallsRuntime, IcreateCallruntimeCallinit_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/callinit_dynamic.abc",
                                             "callinit_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/callinit_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/callinit_dynamic_modified.abc",
                             "static_initializer", g_icreateCallruntimeCallinit1Lambda);

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/callinit_dynamic_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/callinit_dynamic_modified.abc", "innerConstr",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *firstBB = helpers::BBgetSuccBlocks(g_implG->gGetStartBasicBlock(graph))[0];
            auto *removeInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iRemove(removeInst);

            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
            auto *param = g_implG->gGetParameter(graph, 0x2);
            auto *callinit = g_dynG->iCreateCallruntimeCallinit(graph, ldlexvar, param);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *returnInst = g_dynG->iCreateReturn(graph, param);

            g_implG->bbAddInstBack(firstBB, ldlexvar);
            g_implG->bbAddInstBack(firstBB, callinit);
            g_implG->bbAddInstBack(firstBB, returnInst);
        });

    auto cb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *defineclasswithbuffer =
            helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
        // CC-OFFNXT(G.FMT.02)
        auto *stringD = g_implM->createString(file, "D", strlen("D"));
        auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, defineclasswithbuffer, stringD);
        auto *newobjrange = g_dynG->iCreateNewobjrange(graph, 0x1, ldobjbyname);

        auto *returnundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
        // CC-OFFNXT(G.FMT.02)
        g_implG->iInsertBefore(ldobjbyname, returnundefined);
        g_implG->iInsertBefore(newobjrange, returnundefined);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/callinit_dynamic_modified.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/callinit_dynamic_modified.abc",
                             "func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/call_runtime/callinit_dynamic_modified.abc",
                                        "callinit_dynamic");
    EXPECT_TRUE(helpers::Match(output, "innerConstr\ninnerInit\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
