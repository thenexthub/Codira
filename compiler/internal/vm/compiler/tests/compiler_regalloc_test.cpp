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

#include "bytecode_optimizer/reg_acc_alloc.h"
#include "bytecode_optimizer/reg_encoder.h"
#include "compiler_options.h"
#include "graph_test.h"
#include "mem/arena_allocator.h"
#include "mem/pool_manager.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/regalloc/interference_graph.h"
#include "optimizer/optimizations/regalloc/location_mask.h"
#include "optimizer/optimizations/regalloc/reg_alloc_base.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"
#include "optimizer/optimizations/regalloc/reg_alloc_stat.h"
#include "optimizer/optimizations/regalloc/reg_type.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "optimizer/optimizations/regalloc/split_resolver.h"
#include "utils/arena_containers.h"

using namespace testing::ext;

namespace panda::compiler {
class RegallocTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    GraphTest graph_test_;
};

namespace {
constexpr unsigned DEFAULT_CAPACITY1 = 10;
unsigned TEST_EDGES1[2][2] = {{0, 1}, {7, 4}};
auto is_in_set = [](unsigned a, unsigned b) {
    for (int i = 0; i < 2; i++) {
        if ((a == TEST_EDGES1[i][0] && b == TEST_EDGES1[i][1]) || (b == TEST_EDGES1[i][0] && a == TEST_EDGES1[i][1])) {
            return true;
        }
    }
    return false;
};

const unsigned DEFAULT_CAPACITY2 = 5;
const unsigned DEFAULT_EDGES2 = 6;
::std::pair<unsigned, unsigned> TEST_EDGES2[DEFAULT_EDGES2] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {2, 3}, {3, 4}};
}

/**
 * @tc.name: regalloc_test_001
 * @tc.desc: Verify the GetCallsiteIntersectCount function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        InterferenceGraph inter_graph(graph->GetAllocator());
        inter_graph.Reserve(1);
        auto cnode = inter_graph.AllocNode();

        cnode->SetBias(1);
        EXPECT_EQ(cnode->GetBias(), 1);
        EXPECT_FALSE(cnode->IsPhysical());

        Register color = 1;
        bool is_physical = true;
        cnode->SetFixedColor(color, is_physical);
        EXPECT_TRUE(cnode->IsPhysical());
        is_physical = false;
        cnode->SetFixedColor(color, is_physical);
        EXPECT_FALSE(cnode->IsPhysical());
        LifeNumber point = 0;
        cnode->AddCallsite(point);
        EXPECT_EQ(cnode->GetCallsiteIntersectCount(), 1U);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_002
 * @tc.desc: Verify the HasAffinityEdge function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        GraphMatrix graph_matrix(graph->GetAllocator());
        unsigned capacity = 8;
        graph_matrix.SetCapacity(capacity);
        EXPECT_EQ(graph_matrix.GetCapacity(), 8);
        graph_matrix.AddAffinityEdge(1, 1);
        EXPECT_TRUE(graph_matrix.HasAffinityEdge(1, 1));
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_003
 * @tc.desc: Verify the UpdateBiasData function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        InterferenceGraph inter_graph(graph->GetAllocator());
        inter_graph.Reserve(1);
        auto cnode = inter_graph.AllocNode();
        EXPECT_FALSE(inter_graph.GetNode(0).HasBias());
        inter_graph.AddBias();
        InterferenceGraph::Bias bias;
        bias.callsites = 1;
        bias.color = INVALID_REG;
        inter_graph.UpdateBiasData(&bias, *cnode);

        cnode->SetColor(ACC_REG_ID);
        inter_graph.UpdateBiasData(&bias, *cnode);
        EXPECT_EQ(inter_graph.GetBiasCount(), 1);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_004
 * @tc.desc: Verify the HasEdge function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        GraphMatrix matrix(graph->GetAllocator());
        matrix.SetCapacity(DEFAULT_CAPACITY1);
        EXPECT_FALSE(matrix.AddEdge(0, 1));
        unsigned a = 7;
        unsigned b = 4;
        EXPECT_FALSE(matrix.AddEdge(a, b));
        for (unsigned i = 0; i < DEFAULT_CAPACITY1; i++) {
            for (unsigned j = 0; j < DEFAULT_CAPACITY1; j++) {
                EXPECT_EQ(matrix.HasEdge(i, j), is_in_set(i, j));
            }
        }
        EXPECT_GE(matrix.GetCapacity(), DEFAULT_CAPACITY1);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_005
 * @tc.desc: Verify the SetPhysicalReg function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        status = true;
        EXPECT_TRUE(graph->RunPass<compiler::Cleanup>());
        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_TRUE(RegAlloc(graph));
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_006
 * @tc.desc: Verify the HasAffinityEdge function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        InterferenceGraph gr(graph->GetAllocator());
        gr.Reserve(DEFAULT_CAPACITY1);

        EXPECT_EQ(gr.Size(), 0);
        auto *node1 = gr.AllocNode();
        EXPECT_EQ(gr.Size(), 1);
        EXPECT_EQ(node1->GetNumber(), 0);

        auto *node2 = gr.AllocNode();
        EXPECT_EQ(gr.Size(), 2);
        EXPECT_EQ(node2->GetNumber(), 1);
        EXPECT_NE(node1, node2);
        EXPECT_FALSE(gr.HasAffinityEdge(1, 1));
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_007
 * @tc.desc: Verify the AddEdge function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_007, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        InterferenceGraph gr(graph->GetAllocator());
        gr.Reserve(DEFAULT_CAPACITY1);

        EXPECT_TRUE(gr.IsChordal());

        gr.AllocNode();
        EXPECT_TRUE(gr.IsChordal());

        gr.AddEdge(0, 1);
        EXPECT_TRUE(gr.IsChordal());

        gr.AddEdge(1, 2);
        EXPECT_TRUE(gr.IsChordal());

        gr.AddEdge(0, 2);
        EXPECT_TRUE(gr.IsChordal());

        // Make nonchordal
        gr.AllocNode();
        gr.AllocNode();
        gr.AddEdge(3, 2);
        gr.AddEdge(3, 4);
        gr.AddEdge(0, 4);
        EXPECT_TRUE(gr.IsChordal());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_008
 * @tc.desc: Verify the LexBFS function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_008, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        InterferenceGraph gr(graph->GetAllocator());
        gr.Reserve(2);

        gr.AllocNode();
        gr.AllocNode();
        gr.AddEdge(0, 1);

        auto peo = gr.LexBFS();
        EXPECT_EQ(peo.size(), 2);
        EXPECT_EQ(peo[0], 0);
        EXPECT_EQ(peo[1], 1);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_009
 * @tc.desc: Verify the IsChordal function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_009, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        InterferenceGraph gr(graph->GetAllocator());
        gr.Reserve(DEFAULT_CAPACITY2);
        for (auto i = 0; i < DEFAULT_CAPACITY2; i++) {
            gr.AllocNode();
        }

        for (unsigned i = 0; i < DEFAULT_EDGES2; i++) {
            auto x = TEST_EDGES2[i].first;
            auto y = TEST_EDGES2[i].second;
            gr.AddEdge(x, y);
        }

        auto peo = gr.LexBFS();
        EXPECT_EQ(peo.size(), DEFAULT_CAPACITY2);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_010
 * @tc.desc: Verify the SetColor function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_010, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        InterferenceGraph gr(graph->GetAllocator());
        gr.Reserve(DEFAULT_CAPACITY2);

        auto *nd0 = gr.AllocNode();
        auto *nd1 = gr.AllocNode();
        auto *nd2 = gr.AllocNode();
        auto *nd3 = gr.AllocNode();
        auto *nd4 = gr.AllocNode();
        for (unsigned i = 0; i < DEFAULT_EDGES2; i++) {
            auto x = TEST_EDGES2[i].first;
            auto y = TEST_EDGES2[i].second;
            gr.AddEdge(x, y);
        }
        // 32: Max color value
        // 3: Set max size
        EXPECT_EQ(gr.AssignColors<32>(3, 0), 3);
        EXPECT_NE(nd0->GetColor(), nd1->GetColor());
        EXPECT_NE(nd0->GetColor(), nd2->GetColor());
        EXPECT_NE(nd0->GetColor(), nd3->GetColor());

        EXPECT_NE(nd2->GetColor(), nd1->GetColor());
        EXPECT_NE(nd2->GetColor(), nd3->GetColor());

        nd4->SetColor(ACC_REG_ID);
        EXPECT_NE(nd4->GetColor(), nd3->GetColor());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_011
 * @tc.desc: Verify the GetColor function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_011, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        const unsigned DEFAULT_CAPACITY = 11;
        const unsigned DEFAULT_EDGES1 = 12;
        const unsigned DEFAULT_EDGES2 = 4;
        ::std::pair<unsigned, unsigned> test_edges1[DEFAULT_EDGES1] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {2, 3},  {3, 4},
                                                                      {6, 5}, {5, 7}, {6, 7}, {9, 8}, {9, 10}, {8, 10}};
        ::std::pair<unsigned, unsigned> test_edges2[DEFAULT_EDGES2] = {{3, 6}, {6, 9}, {2, 5}, {7, 8}};
        InterferenceGraph gr(graph->GetAllocator());
        gr.Reserve(DEFAULT_CAPACITY);

        std::string name = "test:main" ;
        std::string test_data = "Nodes: 0\n\n\n"
                                "graph test_main {\n"
                                "node [colorscheme=spectral9]\n}\n";
        std::stringstream data;
        gr.Dump(name, false, data);
        EXPECT_EQ(data.str(), test_data);
        auto *nd0 = gr.AllocNode();
        auto *nd1 = gr.AllocNode();
        auto *nd2 = gr.AllocNode();
        auto *nd3 = gr.AllocNode();
        auto *nd4 = gr.AllocNode();
        auto *nd5 = gr.AllocNode();
        auto *nd6 = gr.AllocNode();
        auto *nd7 = gr.AllocNode();
        auto *nd8 = gr.AllocNode();
        auto *nd9 = gr.AllocNode();
        auto *nd10 = gr.AllocNode();

        for (unsigned i = 0; i < DEFAULT_EDGES1; i++) {
            auto x = test_edges1[i].first;
            auto y = test_edges1[i].second;
            gr.AddEdge(x, y);
        }
        for (unsigned i = 0; i < DEFAULT_EDGES2; i++) {
            auto x = test_edges2[i].first;
            auto y = test_edges2[i].second;
            gr.AddAffinityEdge(x, y);
        }
        auto &bias0 = gr.AddBias();
        auto &bias1 = gr.AddBias();
        auto &bias2 = gr.AddBias();

        nd3->SetBias(0);
        nd6->SetBias(0);
        nd9->SetBias(0);
        nd2->SetBias(1);
        nd5->SetBias(1);
        nd7->SetBias(2);
        nd8->SetBias(2);

        // 32: Max color value
        // 3: Set max size
        EXPECT_EQ(gr.AssignColors<32>(3, 0), 3);

        // Check nodes inequality
        EXPECT_NE(nd0->GetColor(), nd1->GetColor());
        EXPECT_NE(nd0->GetColor(), nd2->GetColor());
        EXPECT_NE(nd0->GetColor(), nd3->GetColor());

        EXPECT_NE(nd2->GetColor(), nd1->GetColor());
        EXPECT_NE(nd2->GetColor(), nd3->GetColor());

        EXPECT_NE(nd4->GetColor(), nd3->GetColor());

        EXPECT_NE(nd5->GetColor(), nd6->GetColor());
        EXPECT_NE(nd7->GetColor(), nd6->GetColor());
        EXPECT_NE(nd5->GetColor(), nd7->GetColor());

        EXPECT_NE(nd8->GetColor(), nd9->GetColor());
        EXPECT_NE(nd8->GetColor(), nd10->GetColor());
        EXPECT_NE(nd9->GetColor(), nd10->GetColor());

        // Check biases work
        EXPECT_EQ(nd3->GetColor(), nd6->GetColor());
        EXPECT_EQ(nd9->GetColor(), nd6->GetColor());

        EXPECT_EQ(nd2->GetColor(), nd5->GetColor());
        EXPECT_EQ(nd7->GetColor(), nd8->GetColor());

        Register color = INVALID_REG;
        nd7->SetColor(color);
        EXPECT_NE(nd7->GetColor(), nd8->GetColor());

        // Check biases values
        EXPECT_NE(bias0.color, bias1.color);
        EXPECT_NE(bias0.color, bias2.color);

        auto inst = graph->CreateInst(Opcode::SaveState);
        LifeIntervals lifeInter(graph->GetAllocator(), inst);
        lifeInter.AppendRange({90, 100});
        lifeInter.AppendRange({80, 90});
        lifeInter.AppendRange({40, 50});
        lifeInter.AppendRange({35, 40});
        lifeInter.AppendRange({20, 34});
        lifeInter.StartFrom(30);
        lifeInter.AppendRange({10, 20});
        lifeInter.AppendGroupRange({10, 25});
        lifeInter.AppendGroupRange({10, 79});
        lifeInter.AppendGroupRange({10, 95});

        nd0->Assign(&lifeInter);
        nd1->Assign(&lifeInter);
        nd2->Assign(&lifeInter);
        nd3->Assign(&lifeInter);
        nd4->Assign(&lifeInter);
        nd5->Assign(&lifeInter);
        nd6->Assign(&lifeInter);
        nd7->Assign(&lifeInter);
        nd8->Assign(&lifeInter);
        nd9->Assign(&lifeInter);
        nd10->Assign(&lifeInter);

        test_data = "Nodes: 0\n\n\n"
                    "graph test_main {\n"
                        "node [colorscheme=spectral9]\n"
                    "}\n"
                    "Nodes: 11\n\n\n"
                    "graph test_main {\n"
                        "node [colorscheme=spectral9]\n"
                        "0 [color=0, xlabel=\"0\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "1 [color=1, xlabel=\"1\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "2 [color=2, xlabel=\"2\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "3 [color=1, xlabel=\"1\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "4 [color=0, xlabel=\"0\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "5 [color=2, xlabel=\"2\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "6 [color=1, xlabel=\"1\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "7 [color=0, xlabel=\"255\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "8 [color=0, xlabel=\"0\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "9 [color=1, xlabel=\"1\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "10 [color=2, xlabel=\"2\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "1--0\n2--0\n2--1\n3--0\n3--2\n4--3\n6--5\n7--5\n7--6\n9--8\n10--8\n10--9\n}\n";
        data.clear();
        gr.Dump(name, false, data);
        EXPECT_EQ(data.str(), test_data);
        data.str("");
        nd10->SetFixedColor(1, true);
        test_data = "Nodes: 11\n\n\n"
                    "graph test_main {\n"
                        "node [colorscheme=spectral9]\n"
                        "0 [color=0, xlabel=\"0\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "1 [color=1, xlabel=\"1\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "2 [color=2, xlabel=\"2\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "3 [color=1, xlabel=\"1\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "4 [color=0, xlabel=\"0\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "5 [color=2, xlabel=\"2\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "6 [color=1, xlabel=\"1\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "7 [color=0, xlabel=\"255\", tooltip=\"[10:100) {inst v61}\", shape=\"ellipse\"]\n"
                        "8 [color=0, xlabel=\"0\", tooltip=\"[10:100) {inst v61}\", shape=\"hexagon\"]\n"
                        "9 [color=1, xlabel=\"1\", tooltip=\"[10:100) {inst v61}\", shape=\"hexagon\"]\n"
                        "1--0\n2--0\n2--1\n3--0\n3--2\n4--3\n6--5\n7--5\n7--6\n9--8\n}\n";
        data.clear();
        gr.Dump(name, true, data);
        EXPECT_EQ(data.str(), test_data);

        // Check biases work
        EXPECT_EQ(nd3->GetColor(), nd6->GetColor());
        EXPECT_EQ(nd9->GetColor(), nd6->GetColor());
        EXPECT_EQ(nd2->GetColor(), nd5->GetColor());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_012
 * @tc.desc: Verify the ConvertRegType function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_012, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_EQ(ConvertRegType(graph, DataType::Type::FLOAT32), DataType::Type::UINT64);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_013
 * @tc.desc: Verify the AbortIfFailed function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_013, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        RegMask reg_mask(0U);
        std::shared_ptr<RegAllocBase> base = std::make_shared<RegAllocGraphColoring>(graph);
        base->SetRegMask(reg_mask);
        EXPECT_EQ(base->GetRegMask().GetSize(), 32);  // 32: It's registers number
        base->SetVRegMask(reg_mask);
        EXPECT_EQ(base->GetVRegMask().GetSize(), 32);  // 32: It's registers number
        base->SetSlotsCount(0);
        EXPECT_EQ(base->GetStackMask().GetSize(), 0);
        EXPECT_NE(base->GetPassName(), nullptr);
        EXPECT_TRUE(base->AbortIfFailed());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_014
 * @tc.desc: Verify the SetCompilerDumpLifeIntervals function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_014, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        RegisterMap map(graph->GetLocalAllocator());
        RegMask reg_mask(0U);
        RegAllocGraphColoring base(graph);
        base.SetRegMask(reg_mask);

        LocationMask reg_mask1 = base.GetRegMask();
        size_t priority_reg = 1;
        map.SetMask(reg_mask1, priority_reg);
        map.SetCallerFirstMask(reg_mask1, 0, 1);
        EXPECT_EQ(map.RegallocToCodegenReg(0), 2);
        map.SetCallerFirstMask(reg_mask1, 1, 1);
        EXPECT_EQ(map.RegallocToCodegenReg(1), 2);
        reg_mask1.Set(0);
        map.SetCallerFirstMask(reg_mask1, 1, 1);
        EXPECT_EQ(map.RegallocToCodegenReg(2), 4);

        EXPECT_EQ(map.Size(), 32);
        EXPECT_TRUE(map.IsRegAvailable(0, Arch::AARCH32));
        EXPECT_TRUE(map.IsRegAvailable(0, Arch::AARCH64));

        Register reg = 65; // 65: Random Numbers
        EXPECT_FALSE(map.IsRegAvailable(reg, Arch::AARCH32));

        std::string test_data = "Regalloc -> Codegen\n"
                                "r0 -> r2\n"
                                "r1 -> r3\n"
                                "r2 -> r4\n"
                                "r3 -> r5\n"
                                "r4 -> r6\n"
                                "r5 -> r7\n"
                                "r6 -> r8\n"
                                "r7 -> r9\n"
                                "r8 -> r10\n"
                                "r9 -> r11\n"
                                "r10 -> r12\n"
                                "r11 -> r13\n"
                                "r12 -> r14\n"
                                "r13 -> r15\n"
                                "r14 -> r16\n"
                                "r15 -> r17\n"
                                "r16 -> r18\n"
                                "r17 -> r19\n"
                                "r18 -> r20\n"
                                "r19 -> r21\n"
                                "r20 -> r22\n"
                                "r21 -> r23\n"
                                "r22 -> r24\n"
                                "r23 -> r25\n"
                                "r24 -> r26\n"
                                "r25 -> r27\n"
                                "r26 -> r28\n"
                                "r27 -> r29\n"
                                "r28 -> r30\n"
                                "r29 -> r31\n"
                                "r30 -> r1\n"
                                "Unavailable for RA:\n"
                                "r31 -> r0\n";
        std::stringstream data;
        EXPECT_EQ(map.CodegenToRegallocReg(0), 31);
        map.Dump(&data);
        EXPECT_EQ(data.str(), test_data);
        EXPECT_EQ(map.Size(), 32);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_015
 * @tc.desc: Verify the Resolve function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_015, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        for (auto block : graph->GetBlocksRPO()) {
            for (auto inst : block->AllInstsSafe()) {
                if (inst->IsSaveState()) {
                    continue;
                }
                inst->SetOpcode(Opcode::Phi);
                EXPECT_EQ(inst->GetOpcode(), Opcode::Phi);
            }
        }
        EXPECT_TRUE(graph->RunPass<LivenessAnalyzer>());
        RegAllocResolver regalloc(graph);
        regalloc.Resolve();
        EXPECT_TRUE(regalloc.ResolveCatchPhis());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_016
 * @tc.desc: Verify the ResolveIfRequired function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_016, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_FALSE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_FALSE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegEncoder>());
        for (const auto &block : graph->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            for (auto inst : block->AllInsts()) {
                if (inst->GetOpcode() != Opcode::SpillFill) {
                    continue;
                }
                status = true;
                Location src(Location::Kind::IMMEDIATE, 0);
                Location dst(Location::Kind::REGISTER, 0);
                auto spill_fill_inst = inst->CastToSpillFill();
                spill_fill_inst->AddSpillFill(src, dst, DataType::UINT64);
                EXPECT_EQ(spill_fill_inst->GetSpillFills().size(), 6);  // It's spillfille size

                SpillFillsResolver resolver(graph);
                resolver.Resolve(spill_fill_inst);

                Location src1(Location::Kind::REGISTER, 0);
                Location dst1(Location::Kind::STACK_ARGUMENT, 0);
                auto spill_fill_inst1 = inst->CastToSpillFill();
                spill_fill_inst1->AddSpillFill(src1, dst1, DataType::UINT64);
                EXPECT_EQ(spill_fill_inst->GetSpillFills().size(), 7);  // It's spillfille size
                resolver.Resolve(spill_fill_inst1);

                SpillFillsResolver sResolver1(graph);
                sResolver1.Resolve(spill_fill_inst);
                sResolver1.ResolveIfRequired(spill_fill_inst);
                sResolver1.Run();

                auto spill_fill_inst2 = inst->CastToSpillFill();
                spill_fill_inst2->AddSpillFill(src1, dst, DataType::UINT64);
                EXPECT_EQ(spill_fill_inst->GetSpillFills().size(), 8);  // It's spillfille size
                resolver.Resolve(spill_fill_inst2);

                Location src2(Location::Kind::STACK, 30);  // It's random number
                Location dst3(Location::Kind::STACK, 25);  // It's random number
                spill_fill_inst->AddSpillFill(src2, dst3, DataType::UINT64);
                EXPECT_EQ(spill_fill_inst->GetSpillFills().size(), 8);  // It's spillfille size
                Location dst4(Location::Kind::FP_REGISTER, 0);
                spill_fill_inst->AddSpillFill(src1, dst4, DataType::UINT64);
                EXPECT_EQ(spill_fill_inst->GetSpillFills().size(), 9);  // It's spillfille size
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_017
 * @tc.desc: Verify the Run function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_017, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "cleanUpTest.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph,
        std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<LivenessAnalyzer>());
        auto liven = &graph->GetAnalysis<LivenessAnalyzer>();
        int32_t reg = 0;
        int32_t index = 0; // index: mark opcode
        int32_t index1 = 2;
        int32_t index2 = 3;
        for (const auto &block : graph->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            for (auto inst : block->AllInsts()) {
                if (inst->GetOpcode() != Opcode::Phi) {
                    continue;
                }
                if (index == index1 || index == index2) {
                    status = true;
                    auto param = liven->GetInstLifeIntervals(inst);
                    param->SetReg(reg++);
                    param->AppendRange({0, 10});
                    param->StartFrom(0);
                    EXPECT_EQ(param->GetRanges().size(), 2);
                    EXPECT_EQ(param->GetReg(), reg - 1);
                    EXPECT_NE(param->SplitAt(1, graph->GetAllocator()), nullptr);
                    param->SetLocation(Location::MakeConstant(0));
                    EXPECT_EQ(param->GetLocation().GetValue(), 0);
                }
                index++;
            }
        }
        for (auto interval : liven->GetLifeIntervals()) {
            interval->StartFrom(0);
            EXPECT_EQ(interval->GetRanges().size(), 1);
        }
        SplitResolver resolver(graph, liven);
        resolver.Run();
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_018
 * @tc.desc: Verify the RegAllocStat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_018, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<LivenessAnalyzer>());
        auto interval = graph->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals();
        RegAllocStat sts(interval);
        Location rhs(Location::Kind::REGISTER, 0);
        for (const auto &interv : interval) {
            interv->SetLocation(rhs);
        }
        RegAllocStat st1(interval);
        EXPECT_EQ(st1.GetVSlotCount(), 0);

        Location rhs1(Location::Kind::FP_REGISTER, 1);
        for (const auto &interv : interval) {
            interv->SetLocation(rhs1);
        }
        RegAllocStat st2(interval);
        EXPECT_EQ(st2.GetSlotCount(), 0);

        Location rhs2(Location::Kind::STACK, 2);
        for (const auto &interv : interval) {
            interv->SetLocation(rhs2);
        }
        RegAllocStat st3(interval);
        EXPECT_EQ(st3.GetVRegCount(), 0);

        for (const auto &interv : interval) {
            interv->SetPhysicalReg(1, DataType::UINT64);
        }
        RegAllocStat st4(interval);
        EXPECT_EQ(st4.GetRegCount(), 0);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_019
 * @tc.desc: Verify the SetCatchBegin function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_019, TestSize.Level1)
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
        for (auto block : graph->GetBlocksRPO()) {
            for (auto inst : block->AllInstsSafe()) {
                if (inst->IsSaveState()) {
                    continue;
                }
                inst->SetOpcode(Opcode::Phi);
                EXPECT_EQ(inst->GetOpcode(), Opcode::Phi);
            }
        }
        graph->RunPass<LivenessAnalyzer>();
        RegAllocResolver regalloc(graph);
        regalloc.Resolve();
        for (auto block : graph->GetBlocksRPO()) {
            block->SetCatchBegin(true);
            EXPECT_TRUE(block->IsCatchBegin());
        }
        EXPECT_TRUE(regalloc.ResolveCatchPhis());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_020
 * @tc.desc: Verify the ResolveCatchPhis function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_020, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        EXPECT_NE(graph, nullptr);
        graph->RunPass<LivenessAnalyzer>();
        ArenaVector<bool> reg_write(graph->GetLocalAllocator()->Adapter());
        size_t offset = 32;
        reg_write.resize(offset);
        graph->InitUsedRegs<DataType::INT64>(&reg_write);

        for (auto block : graph->GetBlocksRPO()) {
            if (block == nullptr) {
                continue;
            }
            for (auto inst : block->AllInstsSafe()) {
                if (inst->GetOpcode() == Opcode::Phi || inst->GetOpcode() == Opcode::CatchPhi ||
                    inst->GetOpcode() == Opcode::SaveState) {
                    continue;
                }
                for (size_t i = 0; i < inst->GetInputsCount(); i++) {
                    if (inst->GetLocation(i).IsUnallocatedRegister()) {
                        status = true;
                        Location src(Location::Kind::IMMEDIATE, i);
                        inst->SetLocation(i, src);
                        EXPECT_FALSE(inst->GetLocation(i).IsConstant());
                        inst->SetType(DataType::UINT64);
                        EXPECT_EQ(inst->GetType(), DataType::UINT64);
                    }
                }
            }
        }
        for (auto block : graph->GetBlocksRPO()) {
            for (auto inst : block->AllInstsSafe()) {
                if (inst->IsSaveState()) {
                    continue;
                }
                inst->SetOpcode(Opcode::Phi);
                EXPECT_EQ(inst->GetOpcode(), Opcode::Phi);
            }
        }
        RegAllocResolver regalloc(graph);
        regalloc.Resolve();
        EXPECT_TRUE(regalloc.ResolveCatchPhis());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: regalloc_test_021
 * @tc.desc: Verify the Arch::AARCH32 function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RegallocTest, regalloc_test_021, TestSize.Level1)
{
    std::string pfile_name = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "#*#func4";
    bool status = false;
    auto pfile = panda_file::OpenPandaFile(pfile_name);
    for (uint32_t id : pfile->GetClasses()) {
        panda_file::File::EntityId record_id {id};

        if (pfile->IsExternal(record_id)) {
            continue;
        }

        panda_file::ClassDataAccessor cda {*pfile, record_id};
        cda.EnumerateMethods([&pfile, test_method_name, &status](panda_file::MethodDataAccessor &mda) {
            if (mda.IsExternal()) {
                return;
            }
            ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
            ArenaAllocator local_allocator {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};

            auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(
                mda.GetMethodId().GetOffset());
            panda::BytecodeOptimizerRuntimeAdapter adapter(mda.GetPandaFile());
            auto *graph = allocator.New<Graph>(&allocator, &local_allocator, Arch::AARCH32, method_ptr, &adapter,
                                               false, nullptr, true, true);
            graph->RunPass<panda::compiler::IrBuilder>();

            auto method_name = std::string(utf::Mutf8AsCString(pfile->GetStringData(mda.GetNameId()).data));
            if (test_method_name != method_name) {
                return;
            }
            status = true;
            EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
            EXPECT_TRUE(RegAlloc(graph));
        });
    }
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler
