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

class LibAbcKitCreateDynCreateobjectwithbuffer : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynCreateobjectwithbuffer, IcreateCreateobjectwithbuffer_1)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
                                   "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "a", strlen("a")));
            arr.emplace_back(g_implM->createLiteralU32(file, 1));
            arr.emplace_back(g_implM->createLiteralString(file, "b", strlen("b")));
            arr.emplace_back(g_implM->createLiteralString(file, "str", strlen("str")));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *createobjectwithbuffer = g_dynG->iCreateCreateobjectwithbuffer(graph, litArr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *stringA = g_implM->createString(file, "a", strlen("a"));
            auto *ldobjbynameA = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringA);
            auto *stringB = g_implM->createString(file, "b", strlen("b"));
            auto *ldobjbynameB = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringB);
            auto *callarg10 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameA);
            auto *callarg11 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameB);

            auto *returnundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(createobjectwithbuffer, returnundef);
            g_implG->iInsertBefore(tryldglobalbyname, returnundef);
            g_implG->iInsertBefore(ldobjbynameA, returnundef);
            g_implG->iInsertBefore(ldobjbynameB, returnundef);
            g_implG->iInsertBefore(callarg10, returnundef);
            g_implG->iInsertBefore(callarg11, returnundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc",
                                        "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "1\nstr\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynCreateobjectwithbuffer, IcreateCreateobjectwithbuffer_2)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
                                   "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "toString", strlen("toString")));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "toString")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 0));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *createobjectwithbuffer = g_dynG->iCreateCreateobjectwithbuffer(graph, litArr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *callarg1 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, createobjectwithbuffer);

            auto *returnundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(createobjectwithbuffer, returnundef);
            g_implG->iInsertBefore(tryldglobalbyname, returnundef);
            g_implG->iInsertBefore(callarg1, returnundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc",
                                        "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "objA\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynCreateobjectwithbuffer, IcreateCreateobjectwithbuffer_3)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
                                   "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "foo", strlen("foo")));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "foo")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 0));
            arr.emplace_back(g_implM->createLiteralString(file, "bar", strlen("bar")));
            arr.emplace_back(g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "bar")));
            arr.emplace_back(g_implM->createLiteralMethodAffiliate(file, 1));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *createobjectwithbuffer = g_dynG->iCreateCreateobjectwithbuffer(graph, litArr);
            auto *stringFoo = g_implM->createString(file, "foo", strlen("foo"));
            auto *stringBar = g_implM->createString(file, "bar", strlen("bar"));
            auto *ldobjbynameFoo = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringFoo);
            auto *ldobjbynameBar = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringBar);
            auto *callthisFoo = g_dynG->iCreateCallthis0(graph, ldobjbynameFoo, createobjectwithbuffer);
            auto *callthisBar = g_dynG->iCreateCallthis1(graph, ldobjbynameBar, createobjectwithbuffer,
                                                         g_implG->gFindOrCreateConstantU64(graph, 777));

            auto *returnundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(createobjectwithbuffer, returnundef);
            g_implG->iInsertBefore(ldobjbynameFoo, returnundef);
            g_implG->iInsertBefore(ldobjbynameBar, returnundef);
            g_implG->iInsertBefore(callthisFoo, returnundef);
            g_implG->iInsertBefore(callthisBar, returnundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc",
                                        "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "foo\nbar 777\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithbuffer, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitCreateDynCreateobjectwithbuffer, IcreateCreateobjectwithbuffer_4)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
                                   "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "a", strlen("a")));
            arr.emplace_back(g_implM->createLiteralNullValue(file));
            arr.emplace_back(g_implM->createLiteralString(file, "b", strlen("b")));
            arr.emplace_back(g_implM->createLiteralString(file, "str", strlen("str")));
            arr.emplace_back(g_implM->createLiteralString(file, "c", strlen("c")));
            arr.emplace_back(g_implM->createLiteralNullValue(file));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *createobjectwithbuffer = g_dynG->iCreateCreateobjectwithbuffer(graph, litArr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *stringA = g_implM->createString(file, "a", strlen("a"));
            auto *ldobjbynameA = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringA);
            auto *stringB = g_implM->createString(file, "b", strlen("b"));
            auto *ldobjbynameB = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringB);
            auto *stringC = g_implM->createString(file, "c", strlen("c"));
            auto *ldobjbynameC = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringC);
            auto *callarg10 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameA);
            auto *callarg11 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameB);
            auto *callarg12 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbynameC);

            auto *returnundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(createobjectwithbuffer, returnundef);
            g_implG->iInsertBefore(tryldglobalbyname, returnundef);
            g_implG->iInsertBefore(ldobjbynameA, returnundef);
            g_implG->iInsertBefore(ldobjbynameB, returnundef);
            g_implG->iInsertBefore(ldobjbynameC, returnundef);
            g_implG->iInsertBefore(callarg10, returnundef);
            g_implG->iInsertBefore(callarg11, returnundef);
            g_implG->iInsertBefore(callarg12, returnundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc",
                                        "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "null\nstr\nnull\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateCreateobjectwithbuffer with createLiteralNullValue and
// iCreateDefinepropertybyname, abc-kind=ArkTS1, category=positive, extension=c
// mock scenario: function addUser(name) { const jointPoint = { path: "ppath", param0: name }; }
TEST_F(LibAbcKitCreateDynCreateobjectwithbuffer, IcreateAddUserLike)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
                                   "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc",
        "createobjectwithbuffer_dynamic.addUser",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            // mock: const jointPoint = { path: "ppath", param0: null };
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralString(file, "path", strlen("path")));
            arr.emplace_back(g_implM->createLiteralString(file, "ppath", strlen("ppath")));
            arr.emplace_back(g_implM->createLiteralString(file, "param0", strlen("param0")));
            arr.emplace_back(g_implM->createLiteralNullValue(file));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *createobjectwithbuffer = g_dynG->iCreateCreateobjectwithbuffer(graph, litArr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *nameParam = g_implG->gGetParameter(graph, 3);
            ASSERT_NE(nameParam, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            // use definepropertybyname to update param0 property
            auto *param0Name = g_implM->createString(file, "param0", strlen("param0"));
            ASSERT_NE(param0Name, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *defineProp =
                g_dynG->iCreateDefinepropertybyname(graph, nameParam, param0Name, createobjectwithbuffer);
            ASSERT_NE(defineProp, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
            auto *tryldglobalbyname1 = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
            auto *tryldglobalbyname2 = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);

            auto *stringPath = g_implM->createString(file, "path", strlen("path"));
            auto *ldobjbynamePath = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringPath);

            auto *stringParam0 = g_implM->createString(file, "param0", strlen("param0"));
            auto *ldobjbynameParam0 = g_dynG->iCreateLdobjbyname(graph, createobjectwithbuffer, stringParam0);

            auto *callarg1Path = g_dynG->iCreateCallarg1(graph, tryldglobalbyname1, ldobjbynamePath);
            auto *callarg1Param0 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname2, ldobjbynameParam0);

            auto *returnundef = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(createobjectwithbuffer, returnundef);
            g_implG->iInsertBefore(defineProp, returnundef);
            g_implG->iInsertBefore(tryldglobalbyname1, returnundef);
            g_implG->iInsertBefore(ldobjbynamePath, returnundef);
            g_implG->iInsertBefore(callarg1Path, returnundef);
            g_implG->iInsertBefore(tryldglobalbyname2, returnundef);
            g_implG->iInsertBefore(ldobjbynameParam0, returnundef);
            g_implG->iInsertBefore(callarg1Param0, returnundef);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR
                                        "ut/isa/isa_dynamic/create/createobjectwithbuffer_dynamic_modified.abc",
                                        "createobjectwithbuffer_dynamic");
    EXPECT_TRUE(helpers::Match(output, "ppath\nmy_param_value\n"));
}
}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
