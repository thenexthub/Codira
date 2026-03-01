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
#include <numeric>
#include <unordered_map>

#include "compiler_options.h"
#include "graph_test.h"
#include "mem/arena_allocator.h"
#include "mem/pool_manager.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/move_constants.h"
#include "optimizer/optimizations/vn.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "utils/arena_containers.h"

using namespace testing::ext;

namespace panda::compiler {
class OptimizationsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

/**
 * @tc.name: optimizations_test_001
 * @tc.desc: Verify the Add function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizationsTest, optimizations_test_001, TestSize.Level1)
{
    panda::compiler::VnObject vn_obj;
    uint32_t obj1 = 1U;
    uint64_t obj2 = 1U;
    vn_obj.Add(obj1);
    vn_obj.Add(obj2);
    auto size = vn_obj.GetSize();
    EXPECT_NE(vn_obj.GetArray(), nullptr);
    EXPECT_EQ(vn_obj.GetElement(0), 1);
    EXPECT_EQ(size, 3);
}

/**
 * @tc.name: optimizations_test_002
 * @tc.desc: Verify the VnObject function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizationsTest, optimizations_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    bool status1 = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status, &status1](Graph* graph,
        std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        EXPECT_NE(graph, nullptr);
        Inst *inst1 = nullptr;
        Inst *inst2 = nullptr;
        for (const auto &block : graph->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            for (auto inst : block->AllInsts()) {
                if (inst->GetOpcode() == Opcode::SaveState) {
                    inst->SetType(DataType::UINT64);
                    EXPECT_EQ(inst->GetType(), DataType::UINT64);
                    inst1 = inst;
                    status = true;
                }

                if (inst->GetOpcode() == Opcode::Intrinsic) {
                    inst->SetType(DataType::UINT64);
                    EXPECT_EQ(inst->GetType(), DataType::UINT64);
                    inst->SetFlag(inst_flags::Flags::NONE);
                    EXPECT_FALSE(inst->GetFlag(inst_flags::Flags::NONE));
                    inst2 = inst;
                    status1 = true;
                }
            }
        }
        panda::compiler::VnObject vn_obj;
        inst1->SetVN(0);
        EXPECT_EQ(inst1->GetVN(), 0);
        vn_obj.Add(inst1);
        inst2->SetVN(1);
        EXPECT_EQ(inst2->GetVN(), 1);
        vn_obj.Add(inst2);
        auto size = vn_obj.GetSize();
        EXPECT_EQ(size, 5);
    });
    EXPECT_TRUE(status);
    EXPECT_TRUE(status1);
}

/**
 * @tc.name: optimizations_test_003
 * @tc.desc: Verify the Compare function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizationsTest, optimizations_test_003, TestSize.Level1)
{
    panda::compiler::VnObject vn_obj;
    uint32_t obj1 = 1U;
    uint64_t obj2 = 1U;
    vn_obj.Add(obj1);
    vn_obj.Add(obj2);
    auto size = vn_obj.GetSize();
    EXPECT_EQ(size, 3);

    EXPECT_TRUE(vn_obj.Compare(&vn_obj));

    panda::compiler::VnObject vn_obj1;
    EXPECT_FALSE(vn_obj.Compare(&vn_obj1));
    EXPECT_TRUE(vn_obj1.Compare(&vn_obj1));

    obj1 = 2U;
    obj2 = 3U;
    vn_obj1.Add(obj1);
    vn_obj1.Add(obj2);
    VnObjEqual vnEqual;
    vnEqual(&vn_obj, &vn_obj1);
    EXPECT_FALSE(vn_obj.Compare(&vn_obj1));
}

/**
 * @tc.name: optimizations_test_004
 * @tc.desc: Verify the FindEqualVnOrCreateNew function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizationsTest, optimizations_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTryTest.abc";
    const char *test_method_name = "func2";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        EXPECT_FALSE(graph->RunPass<ValNum>());
        ValNum vnum(graph);
        for (const auto &block : graph->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            for (auto inst : block->AllInsts()) {
                if (inst->GetOpcode() == Opcode::Compare) {
                    vnum.FindEqualVnOrCreateNew(inst);
                    status = true;
                }
            }
        }
        EXPECT_EQ(vnum.GetPassName(), "GVN");
        EXPECT_TRUE(vnum.IsEnable());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: optimizations_test_005
 * @tc.desc: Verify the MoveConstants function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizationsTest, optimizations_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func1";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<MoveConstants>());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: optimizations_test_006
 * @tc.desc: Verify the Lowering function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizationsTest, optimizations_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTryTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: optimizations_test_007
 * @tc.desc: Verify the Cleanup function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(OptimizationsTest, optimizations_test_007, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTryTest.abc";
    const char *test_method_name = "func1";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
    });
    EXPECT_TRUE(status);
}
}

