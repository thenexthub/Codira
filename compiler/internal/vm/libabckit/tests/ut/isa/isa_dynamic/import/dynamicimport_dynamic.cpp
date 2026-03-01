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

class LibAbcKitCreateDynamicImport : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDynamicimport, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynamicImport, IcreateDynamicimport_1)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic.abc",
                                             "dynamicimport_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic_modified.abc",
        "dynamicimport_dynamic.func_main_0",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *dynamicimport = g_dynG->iCreateDynamicimport(graph, inst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *returnundefined = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(dynamicimport, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas {
                {{},
                 {1},
                 {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
                {{0},
                 {2},
                 {{3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER, {}},
                  {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                  {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_DYNAMICIMPORT, {3}},
                  {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}}}},
                {{1}, {}, {}}};
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic_modified.abc",
                                        "dynamicimport_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDynamicimport, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynamicImport, IcreateDynamicimport_2)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic.abc",
                                             "dynamicimport_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic_modified.abc",
        "dynamicimport_dynamic.func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *moduleStr = g_implM->createString(file, "./modules/moduleB.js", strlen("./modules/moduleB.js"));
            auto *loadString = g_dynG->iCreateLoadString(graph, moduleStr);
            auto *dynamicimport = g_dynG->iCreateDynamicimport(graph, loadString);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *returnundefined = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(loadString, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(dynamicimport, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas {
                {{},
                 {1},
                 {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
                {{0},
                 {2},
                 {{3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER, {}},
                  {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                  {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                  {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_DYNAMICIMPORT, {5}},
                  {17, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}}}},
                {{1}, {}, {}}};
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic_modified.abc",
                                        "dynamicimport_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDynamicimport, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynamicImport, IcreateDynamicimport_3)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic.abc",
                                             "dynamicimport_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic_modified.abc",
        "dynamicimport_dynamic.func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *moduleStr = g_implM->createString(file, "module", strlen("module"));
            auto *loadString = g_dynG->iCreateLoadString(graph, moduleStr);
            auto *dynamicimport = g_dynG->iCreateDynamicimport(graph, loadString);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *returnundefined = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(loadString, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(dynamicimport, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas {
                {{},
                 {1},
                 {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
                {{0},
                 {2},
                 {{3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER, {}},
                  {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                  {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
                  {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_DYNAMICIMPORT, {5}},
                  {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}}}},
                {{1}, {}, {}}};
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic_modified.abc",
                                        "dynamicimport_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));
}

static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> GetSchema()
{
    return {{{},
             {1},
             {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
            {{0},
             {2},
             {{3, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER, {}},
              {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
              {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_DYNAMICIMPORT, {3}},
              {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {5}},
              {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1, {6, 5, 4}},
              {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {7}},
              {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {8, 7}},
              {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME, {9}},
              {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0, {10, 9}},
              {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}}}},
            {{1}, {}, {}}};
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateDynamicimport, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitCreateDynamicImport, IcreateDynamicimport_4)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic.abc",
                                             "dynamicimport_dynamic");
    EXPECT_TRUE(helpers::Match(output, ""));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic_modified.abc",
        "dynamicimport_dynamic.func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto *inst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER);
            auto *dynamicimport = g_dynG->iCreateDynamicimport(graph, inst);

            auto *deffunc = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC);
            auto *thenStr = g_implM->createString(file, "then", strlen("then"));
            auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, dynamicimport, thenStr);
            auto *callthis1 = g_dynG->iCreateCallthis1(graph, ldobjbyname, dynamicimport, deffunc);
            auto *ldobjbyname2 = g_dynG->iCreateLdobjbyname(graph, callthis1, thenStr);
            auto *callthis0 = g_dynG->iCreateCallthis0(graph, ldobjbyname2, callthis1);
            auto *catchStr = g_implM->createString(file, "catch", strlen("catch"));
            auto *ldobjbyname3 = g_dynG->iCreateLdobjbyname(graph, callthis0, catchStr);
            auto *callthis03 = g_dynG->iCreateCallthis0(graph, ldobjbyname3, callthis0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *returnundefined = helpers::FindLastInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
            g_implG->iInsertBefore(dynamicimport, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(ldobjbyname, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(callthis1, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(ldobjbyname2, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(callthis0, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(ldobjbyname3, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(callthis03, returnundefined);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, GetSchema()); });

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/import/dynamicimport_dynamic_modified.abc",
                                        "dynamicimport_dynamic");
    EXPECT_TRUE(helpers::Match(output, "from moduleA: moduleB::a 6 36.6\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
