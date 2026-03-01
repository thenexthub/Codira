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
#include "optimizer/analysis/dominators_tree.h"

using namespace testing::ext;

namespace panda::compiler {
class DominatorsTreeTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

/**
 * @tc.name: dominators_tree_test_001
 * @tc.desc: Verify the UpdateAfterResolverInsertion function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DominatorsTreeTest, dominators_tree_test_001, TestSize.Level1)
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
            if (!bb->IsCatchBegin()) {
                continue;
            }
            DominatorsTree dominators_tree(graph);
            bb->GetPredecessor(1)->SetDominator(bb->GetPredecessor(0));
            EXPECT_NE(bb->GetPredecessor(1)->GetDominator(), nullptr);
            bb->GetPredecessor(0)->AddDominatedBlock(bb->GetPredecessor(1));
            dominators_tree.UpdateAfterResolverInsertion(bb->GetPredecessor(0), bb->GetPredecessor(1),
                bb->GetPredecessor(0)->GetSuccsBlocks()[0]);
            EXPECT_TRUE(dominators_tree.IsValid());
        }
    });
    EXPECT_TRUE(status);
}
    
/**
 * @tc.name: dominators_tree_test_002
 * @tc.desc: Verify the UpdateAfterResolverInsertion function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DominatorsTreeTest, dominators_tree_test_002, TestSize.Level1)
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
            if (!bb->IsCatchBegin()) {
                continue;
            }
            DominatorsTree dominators_tree(graph);
            bb->GetPredecessor(1)->SetDominator(bb->GetPredecessor(0));
            EXPECT_NE(bb->GetPredecessor(1)->GetDominator(), nullptr);
            dominators_tree.UpdateAfterResolverInsertion(bb->GetPredecessor(0), bb->GetPredecessor(1),
                bb->GetSuccessor(0));
            EXPECT_TRUE(dominators_tree.IsValid());
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dominators_tree_test_003
 * @tc.desc: Verify the RunImpl function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DominatorsTreeTest, dominators_tree_test_003, TestSize.Level1)
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
        EXPECT_TRUE(graph->RunPass<DominatorsTree>());
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler