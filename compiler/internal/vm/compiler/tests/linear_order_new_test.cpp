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
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/rpo.h"

using namespace testing::ext;

namespace panda::compiler {
class LinearOrderTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

/**
 * @tc.name: linear_order_test_001
 * @tc.desc: Verify the RunImpl function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LinearOrderTest, linear_order_test_001, TestSize.Level1)
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
        EXPECT_TRUE(graph->RunPass<LinearOrder>());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: linear_order_test_002
 * @tc.desc: Verify the RunImpl function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LinearOrderTest, linear_order_test_002, TestSize.Level1)
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
            if (bb->IsTryBegin()) {
                graph->SetStartBlock(bb->GetSuccessor(1));
            }
        }
        EXPECT_TRUE(graph->RunPass<LinearOrder>());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: linear_order_test_003
 * @tc.desc: Verify the GetBlocks function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LinearOrderTest, linear_order_test_003, TestSize.Level1)
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
        LinearOrder linear_order(graph);
        EXPECT_EQ(linear_order.GetBlocks().size(), 0);
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler
