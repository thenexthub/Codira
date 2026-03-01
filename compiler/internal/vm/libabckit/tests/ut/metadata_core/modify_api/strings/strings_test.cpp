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
#include "libabckit/c/isa/isa_dynamic.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/metadata_core.h"

namespace libabckit::test {

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

auto g_dynamicCreateString2Lambda = [](AbckitGraph *graph) {
    // NOLINTBEGIN(readability-magic-numbers)
    std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> insts1(
        {{1, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
         {2, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}},
         {14, ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER, {}}});
    helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb1({{}, {1}, insts1});
    std::vector<helpers::InstSchema<AbckitIsaApiDynamicOpcode>> insts2({
        {3, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
        {4, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLEXVAR, {}},
        {5, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {4, 3}},
        {6, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
        {7, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLEXVAR, {}},
        {8, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {7, 6}},
        {9, ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING, {}},
        {10, ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLEXVAR, {}},
        {11, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1, {10, 9}},
        {12, ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED, {}},
    });
    helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb2({{0}, {2}, insts2});
    helpers::BBSchema<AbckitIsaApiDynamicOpcode> bb3({{1}, {}, {}});
    std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> bbSchemas({bb1, bb2, bb3});
    // NOLINTEND(readability-magic-numbers)
    helpers::VerifyGraph(graph, bbSchemas);
};

}  // namespace

class LibAbcKitModifyApiStringsTest : public ::testing::Test {};

struct UserData {
    AbckitFile *file = nullptr;
};

// Test: test-kind=api, api=ModifyApiImpl::createString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiStringsTest, StaticCreateString)
{
    UserData userData = {};
    auto testImpl = [&userData]() {
        auto abckitstr = g_implM->createString(userData.file, "newString", strlen("newString"));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(abckitstr, nullptr);

        auto str = helpers::AbckitStringToString(abckitstr);
        ASSERT_EQ(std::strncmp(str.data(), "newString", sizeof("newString")), 0);
    };

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/metadata_core/modify_api/strings/strings_static.abc",
        ABCKIT_ABC_DIR "ut/metadata_core/modify_api/strings/strings_static_modified.abc", "main",
        [&](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *) {
            userData.file = g_implI->functionGetFile(method);
            testImpl();
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=ModifyApiImpl::createString, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiStringsTest, StaticCreateString2)
{
    UserData userData = {};
    auto testImpl = [&userData](AbckitGraph *graph) {
        AbckitInst *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
        auto consoleLogStr = helpers::FindMethodByName(userData.file, "ConsoleLogStr");

        auto str1 = g_implM->createString(userData.file, "string1", strlen("string1"));
        auto loadString1 = g_statG->iCreateLoadString(graph, str1);
        g_implG->iInsertAfter(loadString1, callOp);
        AbckitInst *log1 = g_statG->iCreateCallStatic(graph, consoleLogStr, 1, loadString1);
        g_implG->iInsertAfter(log1, loadString1);

        auto str2 = g_implM->createString(userData.file, "string2", strlen("string2"));
        auto loadString2 = g_statG->iCreateLoadString(graph, str2);
        g_implG->iInsertAfter(loadString2, log1);
        AbckitInst *log2 = g_statG->iCreateCallStatic(graph, consoleLogStr, 1, loadString2);
        g_implG->iInsertAfter(log2, loadString2);
    };

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/metadata_core/modify_api/strings/strings_static.abc",
        ABCKIT_ABC_DIR "ut/metadata_core/modify_api/strings/strings_static_modify.abc", "main",
        [&](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            userData.file = g_implI->functionGetFile(method);

            testImpl(graph);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{}, {1}, {}},
                 {{0},
                  {2},
                  {
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_CHECKCAST, {0}},
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {0}},
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {3}},
                      {5, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                      {6, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {5}},
                      {7, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });
}

// Test: test-kind=api, api=ModifyApiImpl::createString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiStringsTest, DynamicCreateString)
{
    UserData userData = {};
    auto testImpl = [&userData]() {
        auto abckitstr = g_implM->createString(userData.file, "newString", strlen("newString"));
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(abckitstr, nullptr);

        auto str = helpers::AbckitStringToString(abckitstr);
        ASSERT_EQ(std::strncmp(str.data(), "newString", sizeof("newString")), 0);
    };

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/metadata_core/modify_api/strings/strings_dynamic.abc",
        ABCKIT_ABC_DIR "ut/metadata_core/modify_api/strings/strings_dynamic.abc", "main",
        [&](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *) {
            userData.file = g_implI->functionGetFile(method);

            testImpl();
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// CC-OFFNXT(huge_method, C_RULE_ID_FUNCTION_SIZE) test, solid logic
// Test: test-kind=api, api=ModifyApiImpl::createString, abc-kind=ArkTS1, category=positive, extension=c
TEST_F(LibAbcKitModifyApiStringsTest, DynamicCreateString2)
{
    auto output = helpers::ExecuteDynamicAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/strings/strings_dynamic.abc",
                                             "strings_dynamic");
    EXPECT_TRUE(helpers::Match(output, "testString\n"));

    UserData userData = {};
    auto testImpl = [&userData](AbckitGraph *graph) {
        AbckitInst *callOp = helpers::FindFirstInst(graph, ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1);

        auto str1 = g_implM->createString(userData.file, "string1", strlen("string1"));
        auto str2 = g_implM->createString(userData.file, "string2", strlen("string2"));

        auto loadString1 = g_dynG->iCreateLoadString(graph, str1);
        g_implG->iInsertAfter(loadString1, callOp);
        auto *consoleLog1 = g_dynG->iCreateLdlexvar(graph, 0, 0);
        g_implG->iInsertAfter(consoleLog1, loadString1);
        AbckitInst *log1 = g_dynG->iCreateCallarg1(graph, consoleLog1, loadString1);
        g_implG->iInsertAfter(log1, consoleLog1);

        auto loadString2 = g_dynG->iCreateLoadString(graph, str2);
        g_implG->iInsertAfter(loadString2, log1);
        auto *consoleLog2 = g_dynG->iCreateLdlexvar(graph, 0, 0);
        g_implG->iInsertAfter(consoleLog2, loadString2);
        AbckitInst *log2 = g_dynG->iCreateCallarg1(graph, consoleLog2, loadString2);
        g_implG->iInsertAfter(log2, consoleLog2);
    };

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/metadata_core/modify_api/strings/strings_dynamic.abc",
        ABCKIT_ABC_DIR "ut/metadata_core/modify_api/strings/strings_dynamic_modified.abc", "main",
        [&](AbckitFile *, AbckitCoreFunction *method, AbckitGraph *graph) {
            userData.file = g_implI->functionGetFile(method);

            testImpl(graph);
        },
        g_dynamicCreateString2Lambda);

    // NOTE: ExecuteDynamicAbc
}

}  // namespace libabckit::test
