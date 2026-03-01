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

#include <cstdint>
#include <set>
#include "bytecode_optimizer/constant_propagation/constant_propagation.h"
#include "bytecode_optimizer/ir_interface.h"
#include "compiler/tests/graph_test.h"
#include "gtest/gtest.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/cleanup.h"

using namespace testing::ext;

namespace panda::compiler {
class MismergeTryCatch : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

/**
 * @tc.name: mismerge_try_catch_test_001
 * @tc.desc: Verify basic block size.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MismergeTryCatch, mismerge_try_catch_test_001, TestSize.Level1)
{
    std::string pfile_unopt = GRAPH_TEST_ABC_DIR "mismergeTryCatch.abc";
    options.SetCompilerUseSafepoint(false);
    graph_test_.TestBuildGraphFromFile(pfile_unopt, [](Graph *graph, std::string &method_name) {
        if (method_name != "test01") {
            return;
        }
        EXPECT_NE(graph, nullptr);
        int EXPECT_BB_SIZE = 16;
        EXPECT_TRUE(graph->GetVectorBlocks().size() == EXPECT_BB_SIZE);
    });
}

/**
 * @tc.name: mismerge_try_catch_test_002
 * @tc.desc: Verify basic block size.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MismergeTryCatch, mismerge_try_catch_test_002, TestSize.Level1)
{
    std::string pfile_unopt = GRAPH_TEST_ABC_DIR "mismergeTryCatch.abc";
    options.SetCompilerUseSafepoint(false);
    graph_test_.TestBuildGraphFromFile(pfile_unopt, [](Graph *graph, std::string &method_name) {
        if (method_name != "test02") {
            return;
        }
        EXPECT_NE(graph, nullptr);
        int EXPECT_BB_SIZE = 16;
        EXPECT_TRUE(graph->GetVectorBlocks().size() == EXPECT_BB_SIZE);
    });
}

}  // namespace panda::compiler

