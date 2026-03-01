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

void TransformIrNegativeInst(AbckitGraph *graph, AbckitFile *file, AbckitCoreClass *classA)
{
    AbckitInst *initObj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
    ASSERT_NE(initObj, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *type = g_implM->createReferenceType(file, classA);
    ASSERT_NE(type, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *isInstance = g_statG->iCreateIsInstance(graph, initObj, type);
    ASSERT_NE(isInstance, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *idx = g_implG->gFindOrCreateConstantI64(graph, 1);
    g_implG->iSetInput(isInstance, idx, 0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(isInstance, initObj);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::ReplaceInst(ret, g_statG->iCreateReturn(graph, isInstance));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

void TransformIrNegativeBB(AbckitGraph *graph)
{
    AbckitBasicBlock *bbEnd = g_implG->gGetEndBasicBlock(graph);
    AbckitBasicBlock *bb2 = g_implG->gGetBasicBlock(graph, 2);
    g_implG->bbAppendSuccBlock(bbEnd, bb2);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

void TransformIrNegativeBinInst(AbckitGraph *graph)
{
    AbckitInst *loadString = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING);
    ASSERT_NE(loadString, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *idx = g_implG->gFindOrCreateConstantI64(graph, 5);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *addInst = g_statG->iCreateAdd(graph, loadString, idx);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(addInst, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace

class LibAbcKitGraphVerifierTest : public ::testing::Test {};

// Test: test-kind=internal, abc-kind=ArkTS2, category=internal
TEST_F(LibAbcKitGraphVerifierTest, LibAbcKitTestGraphVerifierInst)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/graph_verifier/graph_verifier.abc";
    auto output = helpers::ExecuteStaticAbc(INPUT_PATH, "graph_verifier", "main");
    EXPECT_TRUE(helpers::Match(output, "123\nfalse\n"));

    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    auto *method = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(method, nullptr);
    auto *module = g_implI->functionGetModule(method);
    helpers::ClassByNameContext classCtxFinder = {nullptr, "A"};
    g_implI->moduleEnumerateClasses(module, &classCtxFinder, helpers::ClassByNameFinder);
    ASSERT_NE(classCtxFinder.klass, nullptr);

    AbckitGraph *graph = g_implI->createGraphFromFunction(method);
    TransformIrNegativeInst(graph, g_implI->functionGetFile(method), classCtxFinder.klass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implM->functionSetGraph(method, graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->destroyGraph(graph);

    g_impl->closeFile(file);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=internal
TEST_F(LibAbcKitGraphVerifierTest, LibAbcKitTestGraphVerifierBB)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/graph_verifier/graph_verifier.abc";
    auto output = helpers::ExecuteStaticAbc(INPUT_PATH, "graph_verifier", "main");
    EXPECT_TRUE(helpers::Match(output, "123\nfalse\n"));

    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    auto *method = helpers::FindMethodByName(file, "bar");
    ASSERT_NE(method, nullptr);

    AbckitGraph *graph = g_implI->createGraphFromFunction(method);
    TransformIrNegativeBB(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implM->functionSetGraph(method, graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->destroyGraph(graph);

    g_impl->closeFile(file);
}

// Test: test-kind=internal, abc-kind=ArkTS2, category=internal
TEST_F(LibAbcKitGraphVerifierTest, LibAbcKitTestGraphVerifierBinInst)
{
    constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/ir_core/graph_verifier/graph_verifier.abc";
    auto output = helpers::ExecuteStaticAbc(INPUT_PATH, "graph_verifier", "main");
    EXPECT_TRUE(helpers::Match(output, "123\nfalse\n"));

    AbckitFile *file = g_impl->openAbc(INPUT_PATH, strlen(INPUT_PATH));
    auto *method = helpers::FindMethodByName(file, "foo");
    ASSERT_NE(method, nullptr);

    AbckitGraph *graph = g_implI->createGraphFromFunction(method);
    TransformIrNegativeBinInst(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_implM->functionSetGraph(method, graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
    g_impl->destroyGraph(graph);

    g_impl->closeFile(file);
}

}  // namespace libabckit::test
