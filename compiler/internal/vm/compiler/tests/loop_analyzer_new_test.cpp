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

#include "compiler/optimizer/ir/basicblock.h"
#include "graph.h"
#include "graph_test.h"
#include "mem/pool_manager.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/rpo.h"

using namespace testing::ext;

namespace panda::compiler {
class LoopAnalyzerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

/**
 * @tc.name: loop_analyzer_test_001
 * @tc.desc: Verify the RunImpl function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LoopAnalyzerTest, loop_analyzer_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<LoopAnalyzer>());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: loop_analyzer_test_002
 * @tc.desc: Verify the IsInside function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LoopAnalyzerTest, loop_analyzer_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (!bb->IsTryBegin()) {
                continue;
            }
            Loop loop(graph->GetAllocator(), bb->GetSuccessor(0), 1);
            Loop other(graph->GetAllocator(), bb->GetSuccessor(0), 1);
            EXPECT_FALSE(loop.IsInside(&other));

            loop.SetOuterLoop(&other);
            EXPECT_TRUE(loop.IsInside(&other));
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: loop_analyzer_test_003
 * @tc.desc: Verify the GetLoopOutsideSuccessor function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LoopAnalyzerTest, loop_analyzer_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (!bb->IsTryBegin()) {
                continue;
            }
            Loop loop(graph->GetAllocator(), bb->GetSuccessor(0), 1);
            loop.AppendBackEdge(bb->GetSuccessor(1));
            loop.GetBackEdges()[0]->AddSucc(bb->GetSuccessor(0));
            EXPECT_EQ(loop.GetBackEdges()[0]->GetSuccBlockIndex(loop.GetHeader()), 1);
            EXPECT_NE(GetLoopOutsideSuccessor(&loop), nullptr);
            EXPECT_FALSE(loop.IsRoot());
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: loop_analyzer_test_004
 * @tc.desc: Verify the IsLoopSingleBackEdgeExitPoint function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LoopAnalyzerTest, loop_analyzer_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (!bb->IsTryBegin()) {
                continue;
            }
            Loop loop(graph->GetAllocator(), bb->GetSuccessor(0), 1);
            EXPECT_FALSE(IsLoopSingleBackEdgeExitPoint(&loop));

            loop.SetIsIrreducible(true);
            EXPECT_TRUE(loop.IsIrreducible());
            EXPECT_FALSE(IsLoopSingleBackEdgeExitPoint(&loop));
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: loop_analyzer_test_005
 * @tc.desc: Verify the IsLoopSingleBackEdgeExitPoint function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LoopAnalyzerTest, loop_analyzer_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (!bb->IsTryBegin()) {
                continue;
            }
            Loop loop(graph->GetAllocator(), bb->GetSuccessor(0), 1);

            loop.AppendBackEdge(bb->GetSuccessor(1));
            loop.AppendBlock(bb->GetSuccessor(1));
            EXPECT_TRUE(IsLoopSingleBackEdgeExitPoint(&loop));

            loop.AppendBlock(bb->GetSuccessor(0));
            EXPECT_FALSE(IsLoopSingleBackEdgeExitPoint(&loop));
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: oop_analyzer_test_006
 * @tc.desc: Verify the MoveHeaderToSucc function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LoopAnalyzerTest, loop_analyzer_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetEndBlock();
        Loop loop(graph->GetAllocator(), bb, 0);
        loop.AppendBlock(bb);
        EXPECT_EQ(bb->GetSuccsBlocks().size(), 0);
        bb->AddSucc(bb);
        EXPECT_EQ(bb->GetSuccsBlocks().size(), 1);
        loop.MoveHeaderToSucc();
        EXPECT_NE(loop.GetHeader(), nullptr);
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler
