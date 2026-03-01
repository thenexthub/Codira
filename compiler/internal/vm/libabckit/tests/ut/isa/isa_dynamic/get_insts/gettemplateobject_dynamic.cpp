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

auto g_icreateGettemplateobject1Lambda = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
    auto *worldStr = g_implM->createString(file, "world", strlen("world"));
    auto *loadStringWorld = g_dynG->iCreateLoadString(graph, worldStr);
    auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
    auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
    auto *createemptyarray1 = g_dynG->iCreateCreateemptyarray(graph);
    auto *createemptyarray2 = g_dynG->iCreateCreateemptyarray(graph);
    auto *helloStr = g_implM->createString(file, "Hello ", strlen("Hello "));
    auto *loadStringHello = g_dynG->iCreateLoadString(graph, helloStr);
    auto *const0 = g_implG->gFindOrCreateConstantU64(graph, 0);
    auto *definefieldbyvalue1 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, loadStringHello, const0, createemptyarray1);
    auto *definefieldbyvalue2 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, loadStringHello, const0, createemptyarray2);
    auto *str2 = g_implM->createString(file, "!", strlen("!"));
    auto *loadString2 = g_dynG->iCreateLoadString(graph, str2);
    auto *const1 = g_implG->gFindOrCreateConstantU64(graph, 1);
    auto *definefieldbyvalue3 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, loadString2, const1, createemptyarray1);
    auto *definefieldbyvalue4 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, loadString2, const1, createemptyarray2);
    auto *createemptyarray3 = g_dynG->iCreateCreateemptyarray(graph);
    auto *definefieldbyvalue5 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, createemptyarray1, const0, createemptyarray3);
    auto *definefieldbyvalue6 =
        g_dynG->iCreateCallruntimeDefinefieldbyvalue(graph, createemptyarray2, const1, createemptyarray3);
    auto *gettemplateobject = g_dynG->iCreateGettemplateobject(graph, createemptyarray3);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
    auto *callargs2 = g_dynG->iCreateCallargs2(graph, deffunc, gettemplateobject, loadStringWorld);
    auto *callarg1 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, callargs2);

    auto *returnundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
    g_implG->iInsertBefore(loadStringWorld, returnundef);
    g_implG->iInsertBefore(tryldglobalbyname, returnundef);
    g_implG->iInsertBefore(createemptyarray1, returnundef);
    g_implG->iInsertBefore(createemptyarray2, returnundef);
    g_implG->iInsertBefore(loadStringHello, returnundef);
    g_implG->iInsertBefore(definefieldbyvalue1, returnundef);
    g_implG->iInsertBefore(definefieldbyvalue2, returnundef);
    g_implG->iInsertBefore(loadString2, returnundef);
    g_implG->iInsertBefore(definefieldbyvalue3, returnundef);
    g_implG->iInsertBefore(definefieldbyvalue4, returnundef);
    g_implG->iInsertBefore(createemptyarray3, returnundef);
    g_implG->iInsertBefore(definefieldbyvalue5, returnundef);
    g_implG->iInsertBefore(definefieldbyvalue6, returnundef);
    g_implG->iInsertBefore(gettemplateobject, returnundef);
    g_implG->iInsertBefore(callargs2, returnundef);
    g_implG->iInsertBefore(callarg1, returnundef);
};

}  // namespace

class LibAbcKitCreateDynGettemplateobject : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateGettemplateobject, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynGettemplateobject, IcreateGettemplateobject_1)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/gettemplateobject_dynamic.abc", "gettemplateobject_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/gettemplateobject_dynamic.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/gettemplateobject_dynamic_modified.abc",
                             "func_main_0", g_icreateGettemplateobject1Lambda);

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/get_insts/gettemplateobject_dynamic_modified.abc",
                                   "gettemplateobject_dynamic");
    EXPECT_TRUE(helpers::Match(output, "Hello world!\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
