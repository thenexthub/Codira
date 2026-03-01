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

#include <gtest/gtest.h>
#include <cstring>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/js/metadata_js.h"
#include "ir_impl.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/statuses.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implJsI = AbckitGetJsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implJsM = AbckitGetJsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitJSModifyApiModulesTest : public ::testing::Test {};

static constexpr int MAX_ITER = 256;

static void StByNameTest(AbckitFile *file, AbckitGraph *graph, bool isSuper)
{
    AbckitInst *param;
    if (isSuper) {
        param = g_implG->iGetNext(
            g_implG->iGetNext(helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER)));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(param, nullptr);
    }

    AbckitInst *newAge = g_implG->gFindOrCreateConstantI32(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newAge, nullptr);

    AbckitString *fieldName = g_implM->createString(file, "age", strlen("age"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(fieldName, nullptr);

    AbckitInst *store;
    if (isSuper) {
        store = g_dynG->iCreateStsuperbyname(graph, newAge, fieldName, param);
    } else {
        store = g_dynG->iCreateStthisbyname(graph, newAge, fieldName);
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(store, nullptr);

    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ret, nullptr);

    g_implG->iInsertBefore(store, ret);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStsuperbyname, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStsuperbyname)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super.abc",
                                             "store_super");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "setAge",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) { StByNameTest(file, graph, true); },
        [&]([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "store_super");
    expected = "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStsuperbyname, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStsuperbynameLarge)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super.abc",
                                             "store_super");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "setAge",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            StByNameTest(file, graph, true);
            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < MAX_ITER; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "store_super");
    expected = "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStthisbyname, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStthisbyname)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this.abc",
                                             "store_this");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc", "setAge",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) { StByNameTest(file, graph, false); },
        [&]([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc",
                                        "store_this");
    expected = "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStthisbyname, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStthisbynameLarge)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this.abc",
                                             "store_this");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc", "setAge",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            StByNameTest(file, graph, false);
            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x1, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < MAX_ITER; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc",
                                        "store_this");
    expected = "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

static void StByValueTest(AbckitFile *file, AbckitGraph *graph, bool isSuper)
{
    AbckitInst *newAge = g_implG->gFindOrCreateConstantI32(graph, 2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newAge, nullptr);

    AbckitInst *param;
    if (isSuper) {
        param = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(param, nullptr);
    }
    AbckitString *fieldName = g_implM->createString(file, "age", strlen("age"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(fieldName, nullptr);

    AbckitInst *stringObj = g_dynG->iCreateLoadString(graph, fieldName);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(stringObj, nullptr);

    AbckitInst *store;
    if (isSuper) {
        store = g_dynG->iCreateStsuperbyvalue(graph, newAge, param, stringObj);
    } else {
        store = g_dynG->iCreateStthisbyvalue(graph, newAge, stringObj);
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(store, nullptr);

    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ret, nullptr);

    g_implG->iInsertBefore(stringObj, ret);
    g_implG->iInsertBefore(store, ret);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStsuperbyvalue, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStsuperbyvalue)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super.abc",
                                             "store_super");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "setAge",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) { StByValueTest(file, graph, true); },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "store_super");
    expected = "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStsuperbyvalue, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStsuperbyvalueLarge)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super.abc",
                                             "store_super");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "setAge",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            StByValueTest(file, graph, true);
            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < MAX_ITER; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_super_modified.abc", "store_super");
    expected = "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStthisbyvalue, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStthisbyvalue)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this.abc",
                                             "store_this");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc", "setAge",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) { StByValueTest(file, graph, false); },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc",
                                        "store_this");
    expected = "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateStthisbyvalue, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateStthisbyvalueLarge)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this.abc",
                                             "store_this");
    auto expected = "1\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc", "setAge",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            StByValueTest(file, graph, false);
            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x1, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < MAX_ITER; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/store_this_modified.abc",
                                        "store_this");
    expected = "2\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

static void LdByNameTest(AbckitFile *file, AbckitGraph *graph, bool isSuper)
{
    AbckitInst *param;
    if (isSuper) {
        param = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(param, nullptr);
    }
    AbckitString *funcName = g_implM->createString(file, "getName", strlen("getName"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(funcName, nullptr);

    AbckitInst *load;
    if (isSuper) {
        load = g_dynG->iCreateLdsuperbyname(graph, param, funcName);
    } else {
        load = g_dynG->iCreateLdthisbyname(graph, funcName);
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(load, nullptr);

    AbckitInst *call0 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(call0, nullptr);

    g_implG->iInsertBefore(load, call0);
    g_implG->iSetInput(call0, load, 0);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdsuperbyname, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateLdsuperbyname)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super.abc",
                                             "load_super");
    auto expected = "3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super_modified.abc", "getInfo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) { LdByNameTest(file, graph, true); },
        [&]([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super_modified.abc",
                                        "load_super");
    expected = "doggo\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdthisbyname, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateLdthisbyname)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this.abc", "load_this");
    auto expected = "3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc", "getInfo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) { LdByNameTest(file, graph, false); },
        [&]([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc",
                                        "load_this");
    expected = "doggo\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdthisbyname, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateLdthisbynameLarge)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this.abc", "load_this");
    auto expected = "3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc", "getInfo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            LdByNameTest(file, graph, false);
            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < MAX_ITER; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc",
                                        "load_this");
    expected = "doggo\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

static void LdByValueTest(AbckitFile *file, AbckitGraph *graph, bool isSuper)
{
    AbckitInst *param;

    if (isSuper) {
        param = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(param, nullptr);
    }

    AbckitString *funcName = g_implM->createString(file, "getName", strlen("getName"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(funcName, nullptr);

    AbckitInst *stringObj = g_dynG->iCreateLoadString(graph, funcName);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(stringObj, nullptr);

    AbckitInst *load;
    if (isSuper) {
        load = g_dynG->iCreateLdsuperbyvalue(graph, stringObj, param);
    } else {
        load = g_dynG->iCreateLdthisbyvalue(graph, stringObj);
    }
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(load, nullptr);

    AbckitInst *call0 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(call0, nullptr);

    g_implG->iInsertBefore(stringObj, call0);
    g_implG->iInsertBefore(load, call0);
    g_implG->iSetInput(call0, load, 0);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdsuperbyvalue, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateLdsuperbyvalue)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super.abc",
                                             "load_super");
    auto expected = "3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super_modified.abc", "getInfo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) { LdByValueTest(file, graph, true); },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super_modified.abc",
                                        "load_super");
    expected = "doggo\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdsuperbyvalue, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateLdsuperbyvalueLarge)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super.abc",
                                             "load_super");
    auto expected = "3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super_modified.abc", "getInfo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            LdByValueTest(file, graph, true);
            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < MAX_ITER; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super_modified.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_super_modified.abc",
                                        "load_super");
    expected = "doggo\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdthisbyvalue, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateLdthisbyvalue)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this.abc", "load_this");
    auto expected = "3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc", "getInfo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) { LdByValueTest(file, graph, false); },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc",
                                        "load_this");
    expected = "doggo\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdthisbyvalue, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSModifyApiModulesTest, DynamicIcreateLdthisbyvalueLarge)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this.abc", "load_this");
    auto expected = "3\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc", "getInfo",
        [](AbckitFile *file, AbckitCoreFunction *, AbckitGraph *graph) {
            LdByValueTest(file, graph, false);
            auto *ldlexvar = g_dynG->iCreateLdlexvar(graph, 0x0, 0x0);
            g_implG->bbAddInstFront(g_implG->gGetBasicBlock(graph, 0), ldlexvar);
            for (auto idx = 0; idx < MAX_ITER; idx++) {
                auto *call = g_dynG->iCreateCallarg0(graph, ldlexvar);
                g_implG->iInsertAfter(call, ldlexvar);
            }
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc",
        ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc", "func_main_0",
        [](AbckitFile *, AbckitCoreFunction *, AbckitGraph *graph) {
            auto *newlexenv = g_dynG->iCreateNewlexenv(graph, 0x1);
            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *stlexvar = g_dynG->iCreateStlexvar(graph, deffunc, 0x0, 0x0);
            g_implG->iInsertBefore(newlexenv, deffunc);
            g_implG->iInsertAfter(stlexvar, deffunc);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/extensions/js/modify_api/super_this/load_this_modified.abc",
                                        "load_this");
    expected = "doggo\n";
    EXPECT_TRUE(helpers::Match(output, expected));
}

}  // namespace libabckit::test
