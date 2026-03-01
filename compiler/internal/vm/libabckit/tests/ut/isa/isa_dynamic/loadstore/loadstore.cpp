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

// CC-OFFNXT(huge_method, C_RULE_ID_FUNCTION_SIZE) test helper
static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchema(AbckitIsaApiDynamicOpcode opcode)
{
    helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb0 {{},
                                                      {1},
                                                      {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                                                       {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                                                       {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}};

    helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb2 {{1}, {}, {}};

    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchema;

    if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAME) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {7, opcode, {5, 4}},
                         {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYVALUE) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {7, opcode, {6, 4, 5}},
                         {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYINDEX ||
               opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STOWNBYINDEX) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER, {}},
                         {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYINDEX, {5, 4}},
                         {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {8, opcode, {4, 7}},
                         {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYVALUEWITHNAMESET) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                         {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {7, opcode, {5, 4, 6}},
                         {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAMEWITHNAMESET) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                         {6, opcode, {5, 4}},
                         {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYNAME) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {6, opcode, {5, 4}},
                         {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYVALUE) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {7, opcode, {5, 4, 6}},
                         {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYINDEX ||
               opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STOBJBYINDEX) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER, {}},
                         {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {5, opcode, {4, 3}},
                         {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {3}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYVALUE) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {6, opcode, {5, 3}},
                         {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {6}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER, {}},
                         {5, opcode, {3}},
                         {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {5}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYINDEX ||
               opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDOBJBYINDEX) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER, {}},
                         {5, opcode, {3}},
                         {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {5}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_LDGLOBALVAR) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {4, opcode, {}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
                     }},
                    bb2};
    } else if (opcode == ABCKIT_ISA_API_DYNAMIC_OPCODE_STGLOBALVAR) {
        bbSchema = {bb0,
                    {{0},
                     {2},
                     {
                         {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                         {4, opcode, {3}},
                         {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDGLOBAL, {}},
                         {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {5}},
                         {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {6}},
                     }},
                    bb2};
    }
    return bbSchema;
}

class LibAbcKitCreateDynOwnInstTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStownbyname, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynStownbynameValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "initial\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "stown",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *newKeyString = g_implM->createString(file, "abckit_test_key", strlen("abckit_test_key"));
            AbckitString *newValueString =
                g_implM->createString(file, "abckit_test_value", strlen("abckit_test_value"));
            auto *newValue = g_dynG->iCreateLoadString(graph, newValueString);
            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT);
            AbckitInst *newStOwn = g_dynG->iCreateStownbyname(graph, newValue, newKeyString, obj);

            g_implG->iInsertAfter(newValue, obj);
            g_implG->iInsertAfter(newStOwn, newValue);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAME);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "abckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStownbyvalue, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynStownbyvalueValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "initial\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "stown",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *newKeyString = g_implM->createString(file, "abckit_test_key", strlen("abckit_test_key"));
            AbckitString *newValueString =
                g_implM->createString(file, "abckit_test_value", strlen("abckit_test_value"));
            auto *newKey = g_dynG->iCreateLoadString(graph, newKeyString);
            auto *newValue = g_dynG->iCreateLoadString(graph, newValueString);
            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT);
            AbckitInst *newStOwn = g_dynG->iCreateStownbyvalue(graph, newValue, obj, newKey);

            g_implG->iInsertAfter(newKey, obj);
            g_implG->iInsertAfter(newValue, newKey);
            g_implG->iInsertAfter(newStOwn, newValue);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYVALUE);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "abckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStownbyindex, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynStownbyindexValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stownbyindex.abc", "stownbyindex");
    EXPECT_TRUE(helpers::Match(output, "6,3\n4\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stownbyindex.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stownbyindex_modified.abc", "stownbyindex.foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *newVar = g_dynG->iCreateLdlexvar(graph, 0, 2);

            AbckitInst *call1 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
            AbckitInst *oldStown = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYINDEX);
            ASSERT_NE(oldStown, nullptr);

            AbckitInst *stown = g_dynG->iCreateStownbyindex(graph, newVar, g_implG->iGetInput(oldStown, 1), 2);
            g_implG->iInsertBefore(newVar, call1);
            g_implG->iInsertBefore(stown, call1);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stownbyindex_modified.abc",
                                        "stownbyindex");
    EXPECT_TRUE(helpers::Match(output, "6,3,4\n4\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideStownbyindex, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynWideStownbyindexValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stownbyindex.abc", "stownbyindex");
    EXPECT_TRUE(helpers::Match(output, "6,3\n4\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stownbyindex.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stownbyindex_modified.abc", "stownbyindex.foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *newVar = g_dynG->iCreateLdlexvar(graph, 0, 2);

            AbckitInst *call1 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);
            AbckitInst *oldStown = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYINDEX);
            ASSERT_NE(oldStown, nullptr);

            AbckitInst *stown = g_dynG->iCreateWideStownbyindex(graph, newVar, g_implG->iGetInput(oldStown, 1), 2);
            g_implG->iInsertBefore(newVar, call1);
            g_implG->iInsertBefore(stown, call1);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stownbyindex_modified.abc",
                                        "stownbyindex");
    EXPECT_TRUE(helpers::Match(output, "6,3,4\n4\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStownbyvaluewithnameset, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynWideStownbyvaluewithnamesetValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "initial\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "stown",
        []([[maybe_unused]] AbckitFile *file, [[maybe_unused]] AbckitCoreFunction *method,
           [[maybe_unused]] AbckitGraph *graph) {
            AbckitString *newKeyString = g_implM->createString(file, "abckit_test_key", strlen("abckit_test_key"));
            AbckitCoreFunction *funcB = helpers::FindMethodByName(g_implI->functionGetFile(method), "a");
            auto *obj = g_dynG->iCreateCreateemptyobject(graph);
            auto *newKey = g_dynG->iCreateLoadString(graph, newKeyString);
            auto *defineB = g_dynG->iCreateDefinefunc(graph, funcB, 0);
            AbckitInst *newStOwn = g_dynG->iCreateStownbyvaluewithnameset(graph, defineB, obj, newKey);

            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, obj, 0);

            g_implG->iInsertBefore(obj, ret);
            g_implG->iInsertAfter(defineB, obj);
            g_implG->iInsertAfter(newKey, defineB);
            g_implG->iInsertAfter(newStOwn, newKey);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYVALUEWITHNAMESET);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "abckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStownbynamewithnameset, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynStownbynamewithnamesetValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "initial\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "stown",
        []([[maybe_unused]] AbckitFile *file, [[maybe_unused]] AbckitCoreFunction *method,
           [[maybe_unused]] AbckitGraph *graph) {
            AbckitString *newKeyString = g_implM->createString(file, "abckit_test_key", strlen("abckit_test_key"));
            AbckitCoreFunction *funcB = helpers::FindMethodByName(g_implI->functionGetFile(method), "a");
            auto *obj = g_dynG->iCreateCreateemptyobject(graph);
            auto *defineB = g_dynG->iCreateDefinefunc(graph, funcB, 0);
            AbckitInst *newStOwn = g_dynG->iCreateStownbynamewithnameset(graph, defineB, newKeyString, obj);

            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, obj, 0);

            g_implG->iInsertBefore(obj, ret);
            g_implG->iInsertAfter(defineB, obj);
            g_implG->iInsertAfter(newStOwn, defineB);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAMEWITHNAMESET);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "abckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStownbynamewithnameset, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynStownbynamewithnamesetLargeValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "initial\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "stown",
        []([[maybe_unused]] AbckitFile *file, [[maybe_unused]] AbckitCoreFunction *method,
           [[maybe_unused]] AbckitGraph *graph) {
            AbckitString *newKeyString = g_implM->createString(file, "abckit_test_key", strlen("abckit_test_key"));
            AbckitCoreFunction *funcB = helpers::FindMethodByName(g_implI->functionGetFile(method), "a");
            auto *obj = g_dynG->iCreateCreateemptyobject(graph);
            auto *defineB = g_dynG->iCreateDefinefunc(graph, funcB, 0);
            AbckitInst *newStOwn = g_dynG->iCreateStownbynamewithnameset(graph, defineB, newKeyString, obj);
            g_implG->iSetImmediate(newStOwn, 0, 0xfff);

            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, obj, 0);

            g_implG->iInsertBefore(obj, ret);
            g_implG->iInsertAfter(defineB, obj);
            g_implG->iInsertAfter(newStOwn, defineB);

            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < 256; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "abckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStobjbyname, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynStobjbynameValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "initial\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "stown",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *newKeyString = g_implM->createString(file, "abckit_test_key", strlen("abckit_test_key"));
            AbckitString *newValueString =
                g_implM->createString(file, "abckit_test_value", strlen("abckit_test_value"));
            auto *obj = g_dynG->iCreateCreateemptyobject(graph);
            auto *newValue = g_dynG->iCreateLoadString(graph, newValueString);
            AbckitInst *newStOwn = g_dynG->iCreateStobjbyname(graph, newValue, newKeyString, obj);

            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, obj, 0);

            g_implG->iInsertBefore(obj, ret);
            g_implG->iInsertAfter(newValue, obj);
            g_implG->iInsertAfter(newStOwn, newValue);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYNAME);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "abckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStobjbyvalue, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynStobjbyvalueValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "initial\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "stown",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *newKeyString = g_implM->createString(file, "abckit_test_key", strlen("abckit_test_key"));
            AbckitString *newValueString =
                g_implM->createString(file, "abckit_test_value", strlen("abckit_test_value"));

            auto *obj = g_dynG->iCreateCreateemptyobject(graph);
            auto *newKey = g_dynG->iCreateLoadString(graph, newKeyString);
            auto *newValue = g_dynG->iCreateLoadString(graph, newValueString);
            AbckitInst *newStOwn = g_dynG->iCreateStobjbyvalue(graph, newValue, obj, newKey);

            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, obj, 0);

            g_implG->iInsertBefore(obj, ret);
            g_implG->iInsertAfter(newValue, obj);
            g_implG->iInsertAfter(newKey, newValue);
            g_implG->iInsertAfter(newStOwn, newKey);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYVALUE);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/emptyobj_modified.abc", "emptyobj");
    EXPECT_TRUE(helpers::Match(output, "abckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStobjbyindex, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynStobjbyindexValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex.abc", "stobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "a\nundefined\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex_modified.abc", "stown",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *newValueString =
                g_implM->createString(file, "abckit_test_value", strlen("abckit_test_value"));

            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *newValue = g_dynG->iCreateLoadString(graph, newValueString);
            AbckitInst *newStOwn = g_dynG->iCreateStobjbyindex(graph, newValue, obj, 1);

            g_implG->iInsertAfter(newValue, obj);
            g_implG->iInsertAfter(newStOwn, newValue);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYINDEX);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex_modified.abc",
                                        "stobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "a\nabckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStobjbyindex, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynStobjbyindexLargeValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex.abc", "stobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "a\nundefined\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex_modified.abc", "stown",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            AbckitString *newValueString =
                g_implM->createString(file, "abckit_test_value", strlen("abckit_test_value"));

            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *newValue = g_dynG->iCreateLoadString(graph, newValueString);
            AbckitInst *newStOwn = g_dynG->iCreateStobjbyindex(graph, newValue, obj, 1);

            g_implG->iInsertAfter(newValue, obj);
            g_implG->iInsertAfter(newStOwn, newValue);

            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < 256; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex_modified.abc",
                                        "stobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "a\nabckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideStobjbyindex, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, CreateDynWideStobjbyindexValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex.abc", "stobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "a\nundefined\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex_modified.abc", "stown",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *newValueString =
                g_implM->createString(file, "abckit_test_value", strlen("abckit_test_value"));

            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *newValue = g_dynG->iCreateLoadString(graph, newValueString);
            AbckitInst *newStOwn = g_dynG->iCreateWideStobjbyindex(graph, newValue, obj, 1);

            g_implG->iInsertAfter(newValue, obj);
            g_implG->iInsertAfter(newStOwn, newValue);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STOBJBYINDEX);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stobjbyindex_modified.abc",
                                        "stobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "a\nabckit_test_value\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdobjbyvalue, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, IcreateDynLdobjbyvalueValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyvalue.abc", "ldobjbyvalue");
    EXPECT_TRUE(helpers::Match(output, "just_data\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyvalue.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyvalue_modified.abc", "stown",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *fieldName = g_implM->createString(file, "field", strlen("field"));
            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *stringObj = g_dynG->iCreateLoadString(graph, fieldName);
            auto *fieldValue = g_dynG->iCreateLdobjbyvalue(graph, stringObj, obj);
            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, fieldValue, 0);
            g_implG->iInsertBefore(fieldValue, ret);
            g_implG->iInsertBefore(stringObj, fieldValue);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYVALUE);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyvalue_modified.abc",
                                        "ldobjbyvalue");
    EXPECT_TRUE(helpers::Match(output, "field_data\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdobjbyname, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, IcreateDynLdobjbynameValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyname.abc", "ldobjbyname");
    EXPECT_TRUE(helpers::Match(output, "just_data\n123\nresult: 43\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyname.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyname_modified.abc", "foo",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *fieldName = g_implM->createString(file, "field", strlen("field"));
            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *fieldValue = g_dynG->iCreateLdobjbyname(graph, obj, fieldName);
            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, fieldValue, 0);
            g_implG->iInsertBefore(fieldValue, ret);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyname_modified.abc",
                                        "ldobjbyname");
    EXPECT_TRUE(helpers::Match(output, "field_data\n123\nresult: 43\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateWideLdobjbyindex, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, IcreateDynWideLdobjbyindexValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex.abc", "ldobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "just_data\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *fieldValue = g_dynG->iCreateWideLdobjbyindex(graph, obj, 0);
            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, fieldValue, 0);
            g_implG->iInsertBefore(fieldValue, ret);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDOBJBYINDEX);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex_modified.abc",
                                        "ldobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "field_data\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdobjbyindex, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, IcreateDynLdobjbyindexValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex.abc", "ldobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "just_data\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *fieldValue = g_dynG->iCreateLdobjbyindex(graph, obj, 0);
            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, fieldValue, 0);
            g_implG->iInsertBefore(fieldValue, ret);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYINDEX);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex_modified.abc",
                                        "ldobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "field_data\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdobjbyindex, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, IcreateDynLdobjbyindexLargeValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex.abc", "ldobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "just_data\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex_modified.abc", "foo",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *obj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *fieldValue = g_dynG->iCreateLdobjbyindex(graph, obj, 0);
            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, fieldValue, 0);
            g_implG->iInsertBefore(fieldValue, ret);

            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < 256; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyindex_modified.abc",
                                        "ldobjbyindex");
    EXPECT_TRUE(helpers::Match(output, "field_data\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdglobalvar, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, IcreateDynLdglobalvarValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldglobalvar.abc", "ldglobalvar");
    EXPECT_TRUE(helpers::Match(output, "funcfoo_data\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldglobalvar.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldglobalvar_modified.abc", "foo",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *varName = g_implM->createString(file, "str", strlen("str"));
            auto *globalValue = g_dynG->iCreateLdglobalvar(graph, varName);
            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
            g_implG->iSetInput(ret, globalValue, 0);
            g_implG->iInsertBefore(globalValue, ret);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_LDGLOBALVAR);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldglobalvar_modified.abc",
                                        "ldglobalvar");
    EXPECT_TRUE(helpers::Match(output, "globalvar_data\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStglobalvar, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynOwnInstTest, IcreateDynStglobalvarValid)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stglobalvar.abc", "stglobalvar");
    EXPECT_TRUE(helpers::Match(output, "globalvar_data\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stglobalvar.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stglobalvar_modified.abc", "foo",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitString *varName = g_implM->createString(file, "str", strlen("str"));
            AbckitString *funcfooData = g_implM->createString(file, "funcfoo_data", strlen("funcfoo_data"));
            auto *newStr = g_dynG->iCreateLoadString(graph, funcfooData);
            auto *stGlobalVar = g_dynG->iCreateStglobalvar(graph, newStr, varName);
            auto *ldGlobal = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDGLOBAL);
            g_implG->iInsertBefore(stGlobalVar, ldGlobal);
            g_implG->iInsertBefore(newStr, stGlobalVar);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas =
                CreateBBSchema(ABCKIT_ISA_API_DYNAMIC_OPCODE_STGLOBALVAR);
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/stglobalvar_modified.abc",
                                        "stglobalvar");
    EXPECT_TRUE(helpers::Match(output, "funcfoo_data\n"));
}

TEST_F(LibAbcKitCreateDynOwnInstTest, ReplaceCallBack)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyname.abc", "ldobjbyname");
    EXPECT_TRUE(helpers::Match(output, "just_data\n123\nresult: 43\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyname.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyname_modified.abc", "runNamedThen",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            // find definefunc instruction (reference anonymous function)
            auto *defineFuncInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            ASSERT_NE(defineFuncInst, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            // create ldglobalvar instruction to load then_cb
            AbckitString *thenCbName = g_implM->createString(file, "then_cb", strlen("then_cb"));
            ASSERT_NE(thenCbName, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *ldglobalvarInst = g_dynG->iCreateLdglobalvar(graph, thenCbName);
            ASSERT_NE(ldglobalvarInst, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            helpers::ReplaceInst(defineFuncInst, ldglobalvarInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) { g_implG->gDump(graph, 1); });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ldobjbyname_modified.abc",
                                        "ldobjbyname");
    EXPECT_TRUE(helpers::Match(output, "just_data\nthen_cb\nresult: 40\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
