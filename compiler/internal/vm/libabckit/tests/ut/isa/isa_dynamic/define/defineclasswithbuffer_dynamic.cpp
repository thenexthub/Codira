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

class LibAbcKitCreateDynDefineClass : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefineclasswithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineClass, IcreateDefineclasswithbuffer_1)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc", "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
            auto *litArr = g_implG->iGetLiteralArray(inst);
            auto *input = g_implG->iGetInput(inst, 0);
            auto *user = g_implG->iGetNext(inst);

            g_implG->iRemove(inst);
            g_implG->iRemove(user);

            auto *classMethod = helpers::FindMethodByName(file, "A");
            ASSERT_NE(classMethod, nullptr);

            auto *instDefine = g_dynG->iCreateDefineclasswithbuffer(graph, classMethod, litArr, 0x0, input);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringProt = g_implM->createString(file, "prototype", strlen("prototype"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, instDefine, stringProt);
            auto *returnundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            auto *newobj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE);
            g_implG->iSetInput(newobj, instDefine, 0x0);
            auto *definefunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *callarg1 = g_dynG->iCreateCallarg1(graph, definefunc, newobj);
            g_implG->iInsertBefore(instDefine, newobj);
            g_implG->iInsertBefore(ldobjbyname, newobj);
            g_implG->iInsertBefore(callarg1, returnundefined);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc",
                                        "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "\\[object Object\\]\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefineclasswithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineClass, IcreateDefineclasswithbuffer_2)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc", "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
            auto *input = g_implG->iGetInput(inst, 0);

            auto *classConstr = helpers::FindMethodByName(file, "newClassConstr");
            ASSERT_NE(classConstr, nullptr);
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralU32(file, 0));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *instDefine = g_dynG->iCreateDefineclasswithbuffer(graph, classConstr, litArr, 0x0, input);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringProt = g_implM->createString(file, "prototype", strlen("prototype"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, instDefine, stringProt);
            auto *returnundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            auto *newobj = g_dynG->iCreateNewobjrange(graph, 1, instDefine);
            g_implG->iInsertBefore(instDefine, returnundefined);
            g_implG->iInsertBefore(ldobjbyname, returnundefined);
            g_implG->iInsertBefore(newobj, returnundefined);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc",
                                        "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "constr\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefineclasswithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineClass, IcreateDefineclasswithbuffer_3)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc", "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
            auto *input = g_implG->iGetInput(inst, 0);

            auto *classConstr = helpers::FindMethodByName(file, "newClassConstr");
            ASSERT_NE(classConstr, nullptr);
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "newClassMethod", strlen("newClassMethod")));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "newClassMethod")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 0));
            arr.emplace_back(g_implM->createLiteralU32(file, 1));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *instDefine = g_dynG->iCreateDefineclasswithbuffer(graph, classConstr, litArr, 0x0, input);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringProt = g_implM->createString(file, "prototype", strlen("prototype"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, instDefine, stringProt);
            auto *returnundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            auto *newobj = g_dynG->iCreateNewobjrange(graph, 1, instDefine);
            auto *stringMethod = g_implM->createString(file, "newClassMethod", strlen("newClassMethod"));
            auto *ldobjbyname2 = g_dynG->iCreateLdobjbyname(graph, newobj, stringMethod);
            auto *callthis = g_dynG->iCreateCallthis0(graph, ldobjbyname2, newobj);
            g_implG->iInsertBefore(instDefine, returnundefined);
            g_implG->iInsertBefore(ldobjbyname, returnundefined);
            g_implG->iInsertBefore(newobj, returnundefined);
            g_implG->iInsertBefore(ldobjbyname2, returnundefined);
            g_implG->iInsertBefore(callthis, returnundefined);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc",
                                        "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "constr\nmethod\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefineclasswithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineClass, IcreateDefineclasswithbuffer_4)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc", "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
            auto *input = g_implG->iGetInput(inst, 0);

            auto *classConstr = helpers::FindMethodByName(file, "newClassConstr");
            ASSERT_NE(classConstr, nullptr);
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "newClassMethod", strlen("newClassMethod")));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "newClassMethod")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 0));
            arr.emplace_back(g_implM->createLiteralString(file, "newClassMethod2", strlen("newClassMethod2")));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "newClassMethod2")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 1));
            arr.emplace_back(g_implM->createLiteralU32(file, 2));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *instDefine = g_dynG->iCreateDefineclasswithbuffer(graph, classConstr, litArr, 0x0, input);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringProt = g_implM->createString(file, "prototype", strlen("prototype"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, instDefine, stringProt);
            auto *returnundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            auto *newobj = g_dynG->iCreateNewobjrange(graph, 1, instDefine);
            auto *stringMethod = g_implM->createString(file, "newClassMethod2", strlen("newClassMethod2"));
            auto *ldobjbyname2 = g_dynG->iCreateLdobjbyname(graph, newobj, stringMethod);
            auto *const1 = g_implG->gFindOrCreateConstantU64(graph, 5);
            auto *callthis = g_dynG->iCreateCallthis1(graph, ldobjbyname2, newobj, const1);
            g_implG->iInsertBefore(instDefine, returnundefined);
            g_implG->iInsertBefore(ldobjbyname, returnundefined);
            g_implG->iInsertBefore(newobj, returnundefined);
            g_implG->iInsertBefore(ldobjbyname2, returnundefined);
            g_implG->iInsertBefore(callthis, returnundefined);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc",
                                        "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "constr\nmethod 5\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefineclasswithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineClass, IcreateDefineclasswithbuffer_5)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc", "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
            auto *input = g_implG->iGetInput(inst, 0);

            auto *classConstr = helpers::FindMethodByName(file, "newClassConstr2");
            ASSERT_NE(classConstr, nullptr);
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "newClassMethod", strlen("newClassMethod")));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "newClassMethod")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 0));
            arr.emplace_back(g_implM->createLiteralString(file, "newClassMethod2", strlen("newClassMethod2")));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "newClassMethod2")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 1));
            arr.emplace_back(g_implM->createLiteralU32(file, 2));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *instDefine = g_dynG->iCreateDefineclasswithbuffer(graph, classConstr, litArr, 0x0, input);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringProt = g_implM->createString(file, "prototype", strlen("prototype"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, instDefine, stringProt);
            auto *returnundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            auto *string = g_implM->createString(file, "initialize", strlen("initialize"));
            auto *loadString = g_dynG->iCreateLoadString(graph, string);
            auto *newobj = g_dynG->iCreateNewobjrange(graph, 2, instDefine, loadString);
            g_implG->iInsertBefore(instDefine, returnundefined);
            g_implG->iInsertBefore(ldobjbyname, returnundefined);
            g_implG->iInsertBefore(loadString, returnundefined);
            g_implG->iInsertBefore(newobj, returnundefined);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc",
                                        "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "constr initialize\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDefineclasswithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynDefineClass, IcreateDefineclasswithbuffer_6)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc", "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER);
            auto *input = g_implG->iGetInput(inst, 0);

            auto *classConstr = helpers::FindMethodByName(file, "newClassConstr3");
            ASSERT_NE(classConstr, nullptr);
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralU32(file, 0));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *instDefine = g_dynG->iCreateDefineclasswithbuffer(graph, classConstr, litArr, 0x1, input);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringProt = g_implM->createString(file, "prototype", strlen("prototype"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, instDefine, stringProt);
            auto *returnundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(instDefine, returnundefined);
            g_implG->iInsertBefore(ldobjbyname, returnundefined);

            auto *classConstr2 = helpers::FindMethodByName(file, "newClassConstr");
            ASSERT_NE(classConstr2, nullptr);

            auto *instDefine2 = g_dynG->iCreateDefineclasswithbuffer(graph, classConstr2, litArr, 0x0, instDefine);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *ldobjbyname2 = g_dynG->iCreateLdobjbyname(graph, instDefine2, stringProt);
            auto *newobj = g_dynG->iCreateNewobjrange(graph, 1, instDefine2);
            g_implG->iInsertBefore(instDefine2, returnundefined);
            g_implG->iInsertBefore(ldobjbyname2, returnundefined);
            g_implG->iInsertBefore(newobj, returnundefined);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/define/defineclasswithbuffer_dynamic_modified.abc",
                                        "defineclasswithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "constr\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
