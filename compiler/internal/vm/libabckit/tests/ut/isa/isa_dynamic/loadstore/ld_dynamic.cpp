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

static void TransformIrDynLdInstValid(AbckitGraph *graph, AbckitInst *(*ldInstToCheck)(AbckitGraph *graph))
{
    AbckitInst *retOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN);
    auto *mainInst = ldInstToCheck(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iInsertBefore(mainInst, retOp);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implG->iSetInput(retOp, mainInst, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynLd(
    AbckitIsaApiDynamicOpcode opcode)
{
    return {{{},
             {1},
             {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
              {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
            {{0},
             {2},
             {
                 {4, opcode, {}},
                 {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN, {4}},
             }},
            {{1}, {}, {}}};
}

static void ValidTest(AbckitInst *(*ldInstToCheck)(AbckitGraph *graph), AbckitIsaApiDynamicOpcode expectedOpcode,
                      const std::string &expectedOutput, bool skipExecute = false)
{
    auto output =
        helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ld_dynamic.abc", "ld_dynamic");
    EXPECT_TRUE(helpers::Match(output, "10\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ld_dynamic.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ld_dynamic_modified.abc", "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrDynLdInstValid(graph, ldInstToCheck);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas(CreateBBSchemaForDynLd(expectedOpcode));
            helpers::VerifyGraph(graph, bbSchemas);
        });

    if (!skipExecute) {
        output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/ld_dynamic_modified.abc",
                                            "ld_dynamic");
        EXPECT_TRUE(helpers::Match(output, expectedOutput));
    }
}

class LibAbcKitDynLdInstTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdnan, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdnanValid)
{
    ValidTest(g_dynG->iCreateLdnan, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDNAN, "NaN\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdinfinity, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdinfinityValid)
{
    ValidTest(g_dynG->iCreateLdinfinity, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDINFINITY, "Infinity\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdundefined, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdundefinedValid)
{
    ValidTest(g_dynG->iCreateLdundefined, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED, "undefined\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdnull, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdnullValid)
{
    ValidTest(g_dynG->iCreateLdnull, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDNULL, "null\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdsymbol, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdsymbolValid)
{
    ValidTest(g_dynG->iCreateLdsymbol, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDSYMBOL,
              "function Symbol\\(\\) \\{ \\[native code\\] \\}\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdglobal, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdglobalValid)
{
    ValidTest(g_dynG->iCreateLdglobal, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDGLOBAL, "\\[object Object\\]\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdtrue, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdtrueValid)
{
    ValidTest(g_dynG->iCreateLdtrue, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE, "true\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdfalse, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdfalseValid)
{
    ValidTest(g_dynG->iCreateLdfalse, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE, "false\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdhole, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdholeValid)
{
    ValidTest(g_dynG->iCreateLdhole, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDHOLE, "\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdfunction, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdfunctionValid)
{
    ValidTest(g_dynG->iCreateLdfunction, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFUNCTION, "", true);
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdnewtarget, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdnewtargetValid)
{
    ValidTest(g_dynG->iCreateLdnewtarget, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDNEWTARGET, "\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateLdthis, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynLdthisValid)
{
    ValidTest(g_dynG->iCreateLdthis, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTHIS, "\\[object Object\\]\n");
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateTryldglobalbyname, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynTryldglobalbynameValid)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/tryldglobalbyname.abc",
                                             "tryldglobalbyname");
    EXPECT_TRUE(helpers::Match(output, ""));

    auto cb = [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
        auto *stringPrint = g_implM->createString(file, "print", strlen("print"));
        auto *tryldglobalbyname = g_dynG->iCreateTryldglobalbyname(graph, stringPrint);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        auto *newobjrange = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE);
        // CC-OFFNXT(G.FMT.02)
        auto *stringf = g_implM->createString(file, "f", strlen("f"));
        auto *ldobjbyname = g_dynG->iCreateLdobjbyname(graph, newobjrange, stringf);
        auto *callarg1 = g_dynG->iCreateCallarg1(graph, tryldglobalbyname, ldobjbyname);
        auto *returnundefined = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED);
        // CC-OFFNXT(G.FMT.02)
        g_implG->iInsertBefore(tryldglobalbyname, returnundefined);
        g_implG->iInsertBefore(ldobjbyname, returnundefined);
        g_implG->iInsertBefore(callarg1, returnundefined);
    };

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/tryldglobalbyname.abc",
                             ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/tryldglobalbyname_modified.abc",
                             "func_main_0", cb);

    output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/tryldglobalbyname_modified.abc",
                                        "tryldglobalbyname");
    EXPECT_TRUE(helpers::Match(output, "123\n"));
}

// Test: test-kind=api, api=IsaApiDynamicImpl::iCreateTrystglobalbyname, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitDynLdInstTest, CreateDynTrystglobalbynameValid)
{
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/trystglobalbyname.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_dynamic/loadstore/trystglobalbyname_modified.abc", "func_main_0",
        [&](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto arr = std::vector<AbckitLiteral *>();
            arr.emplace_back(g_implM->createLiteralU32(file, 0));
            arr.emplace_back(g_implM->createLiteralU32(file, 111));
            arr.emplace_back(g_implM->createLiteralString(file, "hello", strlen("hello")));
            auto *litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());

            auto *createarraywithbuffer = g_dynG->iCreateCreatearraywithbuffer(graph, litArr);
            auto *stringO = g_implM->createString(file, "o", strlen("o"));
            auto *trystglobalbyname = g_dynG->iCreateTrystglobalbyname(graph, createarraywithbuffer, stringO);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            auto *callarg0 = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0);

            g_implG->iInsertBefore(createarraywithbuffer, callarg0);
            g_implG->iInsertBefore(trystglobalbyname, callarg0);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas = {
                {{},
                 {1},
                 {{0, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
                  {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}}},
                {{0},
                 {2},
                 {
                     {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC, {}},
                     {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER, {}},
                     {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYSTGLOBALBYNAME, {4}},
                     {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0, {3}},
                     {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
                 }},
                {{1}, {}, {}}};
            helpers::VerifyGraph(graph, bbSchemas);
        });
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
