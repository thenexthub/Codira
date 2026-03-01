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

#include "libabckit/c/abckit.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

}  // namespace

class LibAbcKitLoadObjectStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateLoadObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitLoadObjectStaticTest, LibAbcKitTestLoadObject_I32)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test.abc",
                                            "load_object_test", "main");
    EXPECT_TRUE(helpers::Match(output, "0\\n0\\nfalse\\nload_object_test\\.C2 \\{a: test\\}\\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test_modified.abc", "loadfield1",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto newObjInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
            auto returnInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            auto strField1 =
                g_implM->createString(file, "load_object_test.C1.field1", strlen("load_object_test.C1.field1"));
            auto *loadField = g_statG->iCreateLoadObject(graph, newObjInst, strField1, ABCKIT_TYPE_ID_I32);
            g_implG->iInsertBefore(loadField, returnInst);
            ASSERT_NE(loadField, nullptr);
            g_implG->iSetInput(returnInst, loadField, 0);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{}, {1}, {}},
                 {{0},
                  {2},
                  {
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT, {3}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {2}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test_modified.abc",
                                       "load_object_test", "main");
    EXPECT_TRUE(helpers::Match(output, "1\\n0\\nfalse\\nload_object_test\\.C2 \\{a: test\\}\\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateLoadObject, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitLoadObjectStaticTest, LibAbcKitTestLoadObject_F64)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test.abc",
                                            "load_object_test", "main");
    EXPECT_TRUE(helpers::Match(output, "0\\n0\\nfalse\\nload_object_test\\.C2 \\{a: test\\}\\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test_modified.abc", "loadfield2",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto newObjInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
            auto returnInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            auto strField2 =
                g_implM->createString(file, "load_object_test.C1.field2", strlen("load_object_test.C1.field2"));
            auto *loadField = g_statG->iCreateLoadObject(graph, newObjInst, strField2, ABCKIT_TYPE_ID_F64);
            g_implG->iInsertBefore(loadField, returnInst);
            ASSERT_NE(loadField, nullptr);
            g_implG->iSetInput(returnInst, loadField, 0);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{}, {1}, {}},
                 {{0},
                  {2},
                  {
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT, {3}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {2}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test_modified.abc",
                                       "load_object_test", "main");
    EXPECT_TRUE(helpers::Match(output, "0\\n2.4\\nfalse\\nload_object_test\\.C2 \\{a: test\\}\\n"));
}

TEST_F(LibAbcKitLoadObjectStaticTest, LibAbcKitTestLoadObject_Boolean)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test.abc",
                                            "load_object_test", "main");
    EXPECT_TRUE(helpers::Match(output, "0\\n0\\nfalse\\nload_object_test\\.C2 \\{a: test\\}\\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test_modified.abc", "loadfield3",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto newObjInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
            auto returnInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            auto strField3 =
                g_implM->createString(file, "load_object_test.C1.field3", strlen("load_object_test.C1.field3"));
            auto *loadField = g_statG->iCreateLoadObject(graph, newObjInst, strField3, ABCKIT_TYPE_ID_U1);
            g_implG->iInsertBefore(loadField, returnInst);
            ASSERT_NE(loadField, nullptr);
            g_implG->iSetInput(returnInst, loadField, 0);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{}, {1}, {}},
                 {{0},
                  {2},
                  {
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT, {3}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {2}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test_modified.abc",
                                       "load_object_test", "main");
    EXPECT_TRUE(helpers::Match(output, "0\\n0\\ntrue\\nload_object_test\\.C2 \\{a: test\\}\\n"));
}

TEST_F(LibAbcKitLoadObjectStaticTest, LibAbcKitTestLoadObject_Reference)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test.abc",
                                            "load_object_test", "main");
    EXPECT_TRUE(helpers::Match(output, "0\\n0\\nfalse\\nload_object_test\\.C2 \\{a: test\\}\\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test_modified.abc", "loadfield4",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto newObjInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
            auto returnInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            auto strField4 =
                g_implM->createString(file, "load_object_test.C1.field4", strlen("load_object_test.C1.field4"));
            auto *loadField = g_statG->iCreateLoadObject(graph, newObjInst, strField4, ABCKIT_TYPE_ID_REFERENCE);
            g_implG->iInsertBefore(loadField, returnInst);
            ASSERT_NE(loadField, nullptr);
            g_implG->iSetInput(returnInst, loadField, 0);
        },
        [](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                {{{}, {1}, {}},
                 {{0},
                  {2},
                  {
                      {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                      {1, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING, {}},
                      {2, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {1}},
                      {3, ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT, {0}},
                      {4, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {3}},
                  }},
                 {{1}, {}, {}}});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/load_object/load_object_test_modified.abc",
                                       "load_object_test", "main");
    EXPECT_TRUE(helpers::Match(output, "0\\n0\\nfalse\\nload_object_test\\.C2 \\{a: c2 created by c1\\}\\n"));
}

}  // namespace libabckit::test
   // NOLINTEND(readability-magic-numbers)