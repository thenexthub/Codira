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

namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformSetSetCallMethod(AbckitGraph *graph, AbckitCoreFunction *bar)
{
    auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetFunction(call, bar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

class LibAbcKitMethodStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::iSetFunction, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitMethodStaticTest, LibAbcKitTestSetCallMethod)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc", "method_static", "main");
    EXPECT_TRUE(helpers::Match(output, "foo\ntest\n"));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/method/method_static_modified.abc", "main",
                             [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
                                 auto *bar = helpers::FindMethodByName(g_implI->functionGetFile(method), "bar");
                                 ASSERT_NE(bar, nullptr);
                                 TransformSetSetCallMethod(graph, bar);
                             });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static_modified.abc", "method_static",
                                       "main");
    EXPECT_TRUE(helpers::Match(output, "bar\ntest\n"));
}

TEST_F(LibAbcKitMethodStaticTest, TestIsTrue)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc", "method_static", "main");
    EXPECT_TRUE(helpers::Match(output, "foo\ntest\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/method/method_static_modified.abc", "test",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) { g_implG->gDump(graph, 1); });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static_modified.abc", "method_static",
                                       "main");
    EXPECT_TRUE(helpers::Match(output, "foo\ntest\n"));
}

TEST_F(LibAbcKitMethodStaticTest, TestFldai)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc", &file);
    auto *method = helpers::FindMethodByName(file, "test2");
    ASSERT_NE(method, nullptr);
    auto *graph = g_implI->createGraphFromFunction(method);
    ASSERT_NE(graph, nullptr);
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

TEST_F(LibAbcKitMethodStaticTest, TestTypeOf)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc", &file);
    auto *method = helpers::FindMethodByName(file, "test3");
    ASSERT_NE(method, nullptr);
    auto *graph = g_implI->createGraphFromFunction(method);
    ASSERT_NE(graph, nullptr);
    g_impl->destroyGraph(graph);
    g_impl->closeFile(file);
}

TEST_F(LibAbcKitMethodStaticTest, TestLdObjByName)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc", "method_static", "main");
    EXPECT_TRUE(helpers::Match(output, "foo\ntest\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc",
        ABCKIT_ABC_DIR "ut/ir_core/method/method_static_modified.abc", "test4",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) { g_implG->gDump(graph, 1); });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static_modified.abc", "method_static",
                                       "main");
    EXPECT_TRUE(helpers::Match(output, "foo\ntest\n"));
}

}  // namespace libabckit::test
