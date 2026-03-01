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

class LibAbcKitCreateDynDefineGetterSetter : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinegettersetterbyvalue, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineGetterSetter, IcreateDefinegettersetterbyvalue_1)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
                                   "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *getterFunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *createObj = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *ldundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            auto *ldfalse = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE);
            auto *getterStr = g_implM->createString(file, "getter", strlen("getter"));
            auto *loadString = g_dynG->iCreateLoadString(graph, getterStr);
            auto *definegetter = g_dynG->iCreateDefinegettersetterbyvalue(graph, ldfalse, createObj, loadString,
                                                                          getterFunc, ldundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *returnundefined = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(loadString, returnundefined);
            g_implG->iInsertBefore(definegetter, returnundefined);

            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *ldglobal = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, createObj, getterStr);
            auto *callarg = g_dynG->iCreateCallarg1(graph, ldglobal, ldobjbyname);
            g_implG->iInsertBefore(ldglobal, returnundefined);
            g_implG->iInsertBefore(ldobjbyname, returnundefined);
            g_implG->iInsertBefore(callarg, returnundefined);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc",
                                        "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, "getter\n123\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinegettersetterbyvalue, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineGetterSetter, IcreateDefinegettersetterbyvalue_2)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
                                   "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *setterFunc = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *createObj = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *ldundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED);
            auto *ldfalse = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE);
            auto *setterStr = g_implM->createString(file, "setter", strlen("setter"));
            auto *loadString = g_dynG->iCreateLoadString(graph, setterStr);
            auto *definesetter = g_dynG->iCreateDefinegettersetterbyvalue(graph, ldfalse, createObj, loadString,
                                                                          ldundefined, setterFunc);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *returnundefined = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(loadString, returnundefined);
            g_implG->iInsertBefore(definesetter, returnundefined);

            auto *const1 = g_implG->gFindOrCreateConstantU64(graph, 5);
            auto *stobj = g_dynG->iCreateStobjbyname(graph, const1, setterStr, createObj);
            g_implG->iInsertBefore(stobj, returnundefined);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc",
                                        "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, "setter\n5\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefinegettersetterbyvalue, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineGetterSetter, IcreateDefinegettersetterbyvalue_3)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
                                   "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *getterFunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *createObj = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *ldfalse = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE);
            auto *getterStr = g_implM->createString(file, "getter", strlen("getter"));
            auto *loadString = g_dynG->iCreateLoadString(graph, getterStr);
            auto *definesetter =
                g_dynG->iCreateDefinegettersetterbyvalue(graph, ldfalse, createObj, loadString, getterFunc, getterFunc);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *returnundefined = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(loadString, returnundefined);
            g_implG->iInsertBefore(definesetter, returnundefined);

            auto *const1 = g_implG->gFindOrCreateConstantU64(graph, 5);
            auto *stobj = g_dynG->iCreateStobjbyname(graph, const1, getterStr, createObj);
            g_implG->iInsertBefore(stobj, returnundefined);

            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *ldglobal = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, createObj, getterStr);
            auto *callarg = g_dynG->iCreateCallarg1(graph, ldglobal, ldobjbyname);
            g_implG->iInsertBefore(ldglobal, returnundefined);
            g_implG->iInsertBefore(ldobjbyname, returnundefined);
            g_implG->iInsertBefore(callarg, returnundefined);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/definegettersetterbyvalue_dynamic_modified.abc",
                                        "definegettersetterbyvalue_dynamic");
    EXPECT_TRUE(helpers::Match(output, "getter\ngetter\n123\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
