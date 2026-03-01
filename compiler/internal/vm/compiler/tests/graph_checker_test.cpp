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

#include <cstring>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>

#include "compiler_options.h"
#include "constants.h"
#include "graph_checker.h"
#include "graph_visitor.h"
#include "graph_test.h"
#include "inst.h"
#include "locations.h"
#include "mem/pool_manager.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/move_constants.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "reg_acc_alloc.h"

using namespace testing::ext;

namespace panda::compiler {

class GraphCheckerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    GraphTest graphTest_;
};

/**
 * @tc.name: graph_checker_test_001
 * @tc.desc: Verify the Check function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(GraphCheckerTest, graph_checker_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func6";
    bool status = false;
    graphTest_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        graph->InvalidateAnalysis<LoopAnalyzer>();
        EXPECT_TRUE(graph->RunPass<LoopAnalyzer>());
        GraphChecker gChecker(graph);
        gChecker.Check();
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_checker_test_002
 * @tc.desc: Verify the IsTryCatchDomination function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(GraphCheckerTest, graph_checker_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "testTryCatch.abc";
    const char *test_method_name = "func6";
    bool status = false;
    graphTest_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_TRUE(graph->RunPass<MoveConstants>());
        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_TRUE(RegAlloc(graph));

        GraphChecker gChecker(graph);
        gChecker.Check();

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_checker_test_003
 * @tc.desc: Verify the Check function with infinite loop.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(GraphCheckerTest, graph_checker_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    const char *test_method_name = "loop2";
    bool status = false;
    graphTest_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        GraphChecker gChecker(graph);
        gChecker.Check();
        status = true;
    });
    EXPECT_TRUE(status);
}
}
