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

#include <cstddef>
#include <gtest/gtest.h>
#include <sstream>

#include "graph_test.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/regalloc/interference_graph.h"

using namespace testing::ext;

namespace panda::compiler {
class RegAllocInterferenceTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    auto IsInSet(unsigned expect_edges[][2], size_t count, unsigned a, unsigned b)
    {
        ASSERT(expect_edges != nullptr);
        for (size_t i = 0; i < count; i++) {
            if ((a == expect_edges[i][0] && b == expect_edges[i][1]) ||
                (b == expect_edges[i][0] && a == expect_edges[i][1])) {
                return true;
            }
        }
        return false;
    };

    // To prevent adding "remove edge" interfaces to main code,
    // edge removing is simulated via building new graph without it.
    InterferenceGraph BuildSubgraph(InterferenceGraph &orig_ig, ArenaAllocator *alloc,
                                    std::pair<unsigned, unsigned> *edges, size_t count,
                                    ArenaVector<unsigned> &peo, size_t peo_count)
    {
        InterferenceGraph ig(alloc);
        ig.Reserve(orig_ig.Size());

        for (unsigned i = 0; i < count; i++) {
            auto x = edges[i].first;
            auto y = edges[i].second;
            for (unsigned j = 0; j < peo_count; j++) {
                if (x == peo[j] || y == peo[j]) {
                    continue;
                }
            }
            ig.AddEdge(x, y);
        }

        return ig;
    }
};

/**
 * @tc.name: reg_alloc_interference_test_001
 * @tc.desc: Verify the AddEdge function of GraphMatrix.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_001, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    GraphMatrix matrix(&allocator);
    matrix.SetCapacity(10);

    unsigned test_edges[2][2] = {{0, 1}, {7, 4}};
    EXPECT_FALSE(matrix.AddEdge(0, 1));
    EXPECT_FALSE(matrix.AddEdge(7, 4));
    for (unsigned i = 0; i < 10; i++) {
        for (unsigned j = 0; j < 10; j++) {
            EXPECT_EQ(matrix.HasEdge(i, j), IsInSet(test_edges, 2, i, j));
        }
    }
    EXPECT_GE(matrix.GetCapacity(), 10);
}

/**
 * @tc.name: reg_alloc_interference_test_002
 * @tc.desc: Verify the AddAffinityEdge function of GraphMatrix.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_002, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    GraphMatrix matrix(&allocator);
    matrix.SetCapacity(10);

    unsigned test_edges[2][2] = {{0, 1}, {7, 4}};
    EXPECT_FALSE(matrix.AddAffinityEdge(0, 1));
    EXPECT_FALSE(matrix.AddAffinityEdge(7, 4));
    for (unsigned i = 0; i < 10; i++) {
        for (unsigned j = 0; j < 10; j++) {
            EXPECT_EQ(matrix.HasAffinityEdge(i, j), IsInSet(test_edges, 2, i, j));
        }
    }
    EXPECT_GE(matrix.GetCapacity(), 10);
}

/**
 * @tc.name: reg_alloc_interference_test_003
 * @tc.desc: Verify the GetNumber function of ColorNode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_003, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ColorNode node1(0, allocator.Adapter());
    EXPECT_EQ(node1.GetNumber(), 0);
    ColorNode node2(1, allocator.Adapter());
    EXPECT_EQ(node2.GetNumber(), 1);
}

/**
 * @tc.name: reg_alloc_interference_test_004
 * @tc.desc: Verify the SetColor function of ColorNode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_004, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ColorNode node(0, allocator.Adapter());
    node.SetColor(0);
    EXPECT_EQ(node.GetColor(), 0);
    node.SetColor(1);
    EXPECT_EQ(node.GetColor(), 1);
}

/**
 * @tc.name: reg_alloc_interference_test_005
 * @tc.desc: Verify the SetFixedColor function of ColorNode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_005, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ColorNode node(0, allocator.Adapter());
    node.SetFixedColor(1, true);
    EXPECT_EQ(node.GetColor(), 1);
    EXPECT_TRUE(node.IsFixedColor());
    EXPECT_TRUE(node.IsPhysical());
    node.SetFixedColor(2, false);
    EXPECT_EQ(node.GetColor(), 2);
    EXPECT_TRUE(node.IsFixedColor());
    EXPECT_FALSE(node.IsPhysical());
}

/**
 * @tc.name: reg_alloc_interference_test_006
 * @tc.desc: Verify the SetBias function of ColorNode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_006, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ColorNode node(0, allocator.Adapter());
    node.SetBias(3);
    EXPECT_EQ(node.GetBias(), 3);
    EXPECT_TRUE(node.HasBias());
    node.SetBias(ColorNode::NO_BIAS);
    EXPECT_EQ(node.GetBias(), ColorNode::NO_BIAS);
    EXPECT_FALSE(node.HasBias());
}

/**
 * @tc.name: reg_alloc_interference_test_007
 * @tc.desc: Verify the Assign function of ColorNode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_007, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    LifeIntervals intervals(&allocator);
    ColorNode node(0, allocator.Adapter());
    node.Assign(&intervals);
    EXPECT_EQ(node.GetLifeIntervals(), &intervals);
}

/**
 * @tc.name: reg_alloc_interference_test_008
 * @tc.desc: Verify the AddCallsite function of ColorNode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_008, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ColorNode node(0, allocator.Adapter());
    EXPECT_EQ(node.GetCallsiteIntersectCount(), 0);
    node.AddCallsite(2);
    EXPECT_EQ(node.GetCallsiteIntersectCount(), 1);
    node.AddCallsite(7);
    EXPECT_EQ(node.GetCallsiteIntersectCount(), 2);
}

/**
 * @tc.name: reg_alloc_interference_test_009
 * @tc.desc: Verify the AllocNode function of InterferenceGraph.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_009, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);
    ig.Reserve(10);

    EXPECT_EQ(ig.Size(), 0);
    auto *node1 = ig.AllocNode();
    EXPECT_EQ(ig.Size(), 1);
    EXPECT_EQ(node1->GetNumber(), 0);

    auto *node2 = ig.AllocNode();
    EXPECT_EQ(ig.Size(), 2);
    EXPECT_EQ(node2->GetNumber(), 1);
    EXPECT_NE(node1, node2);
}

/**
 * @tc.name: reg_alloc_interference_test_010
 * @tc.desc: Verify the AddBias function of InterferenceGraph.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_010, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);
    EXPECT_EQ(ig.GetBiasCount(), 0);
    ig.AddBias();
    EXPECT_EQ(ig.GetBiasCount(), 1);
    ig.AddBias();
    EXPECT_EQ(ig.GetBiasCount(), 2);
}

/**
 * @tc.name: reg_alloc_interference_test_011
 * @tc.desc: Verify the LexBFS function of InterferenceGraph in simple case.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_011, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);
    ig.Reserve(2);

    ig.AllocNode();
    ig.AllocNode();
    ig.AddEdge(0, 1);

    auto peo = ig.LexBFS();
    EXPECT_EQ(peo.size(), 2);
    EXPECT_EQ(peo[0], 0);
    EXPECT_EQ(peo[1], 1);
}

/**
 * @tc.name: reg_alloc_interference_test_012
 * @tc.desc: Verify the LexBFS function of InterferenceGraph.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_012, TestSize.Level1)
{
    const size_t DEFAULT_CAPACITY = 5;
    const size_t DEFAULT_EDGES = 6;
    std::pair<unsigned, unsigned> test_edges[DEFAULT_EDGES] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {2, 3}, {3, 4}};

    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);
    ig.Reserve(DEFAULT_CAPACITY);
    
    for (size_t i = 0; i < DEFAULT_CAPACITY; i++) {
        ig.AllocNode();
    }
    
    for (size_t i = 0; i < DEFAULT_CAPACITY; i++) {
        auto x = test_edges[i].first;
        auto y = test_edges[i].second;
        ig.AddEdge(x, y);
    }

    auto peo = ig.LexBFS();
    EXPECT_EQ(peo.size(), DEFAULT_CAPACITY);
    std::reverse(peo.begin(), peo.end());

    for (size_t i = 0; i < DEFAULT_CAPACITY - 1; i++) {
        auto ig2 = BuildSubgraph(ig, &allocator, test_edges, DEFAULT_EDGES, peo, i);
        EXPECT_TRUE(ig2.IsChordal());
    }
}

/**
 * @tc.name: reg_alloc_interference_test_013
 * @tc.desc: Verify the AssignColors function of InterferenceGraph.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_013, TestSize.Level1)
{
    std::pair<unsigned, unsigned> test_edges[6] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {2, 3}, {3, 4}};

    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);
    ig.Reserve(5);

    auto *nd0 = ig.AllocNode();
    auto *nd1 = ig.AllocNode();
    auto *nd2 = ig.AllocNode();
    auto *nd3 = ig.AllocNode();
    auto *nd4 = ig.AllocNode();

    for (size_t i = 0; i < 6; i++) {
        auto x = test_edges[i].first;
        auto y = test_edges[i].second;
        ig.AddEdge(x, y);
    }

    EXPECT_EQ(ig.AssignColors<32>(3, 0), 3);
    EXPECT_NE(nd0->GetColor(), nd1->GetColor());
    EXPECT_NE(nd0->GetColor(), nd2->GetColor());
    EXPECT_NE(nd0->GetColor(), nd3->GetColor());

    EXPECT_NE(nd2->GetColor(), nd1->GetColor());
    EXPECT_NE(nd2->GetColor(), nd3->GetColor());

    EXPECT_NE(nd4->GetColor(), nd3->GetColor());
}

/**
 * @tc.name: reg_alloc_interference_test_014
 * @tc.desc: Verify the AssignColors function of InterferenceGraph.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_014, TestSize.Level1)
{
    const size_t DEFAULT_CAPACITY = 11;
    const size_t DEFAULT_EDGES = 12;
    const size_t DEFAULT_AEDGES = 4;
    std::pair<unsigned, unsigned> test_edges[DEFAULT_EDGES] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {2, 3},  {3, 4},
                                                               {6, 5}, {5, 7}, {6, 7}, {9, 8}, {9, 10}, {8, 10}};
    std::pair<unsigned, unsigned> test_aedges[DEFAULT_AEDGES] = {{3, 6}, {6, 9}, {2, 5}, {7, 8}};

    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);
    ig.Reserve(DEFAULT_CAPACITY);

    auto *nd0 = ig.AllocNode();
    auto *nd1 = ig.AllocNode();
    auto *nd2 = ig.AllocNode();
    auto *nd3 = ig.AllocNode();
    auto *nd4 = ig.AllocNode();
    auto *nd5 = ig.AllocNode();
    auto *nd6 = ig.AllocNode();
    auto *nd7 = ig.AllocNode();
    auto *nd8 = ig.AllocNode();
    auto *nd9 = ig.AllocNode();
    auto *nd10 = ig.AllocNode();

    for (size_t i = 0; i < DEFAULT_EDGES; i++) {
        auto x = test_edges[i].first;
        auto y = test_edges[i].second;
        ig.AddEdge(x, y);
    }
    for (size_t i = 0; i < DEFAULT_AEDGES; i++) {
        auto x = test_aedges[i].first;
        auto y = test_aedges[i].second;
        ig.AddAffinityEdge(x, y);
    }
    auto &bias0 = ig.AddBias();
    auto &bias1 = ig.AddBias();
    auto &bias2 = ig.AddBias();

    nd3->SetBias(0);
    nd6->SetBias(0);
    nd9->SetBias(0);
    nd2->SetBias(1);
    nd5->SetBias(1);
    nd7->SetBias(2);
    nd8->SetBias(2);

    EXPECT_EQ(ig.AssignColors<32>(3, 0), 3);

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

    // Check biases values
    EXPECT_NE(bias0.color, bias1.color);
    EXPECT_NE(bias0.color, bias2.color);
}

/**
 * @tc.name: reg_alloc_interference_test_015
 * @tc.desc: Verify the Dump function of InterferenceGraph.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_015, TestSize.Level1)
{
    const size_t DEFAULT_PHISICALS = 3;
    const size_t DEFAULT_NONPHISICALS = 11;

    const size_t DEFAULT_CAPACITY = DEFAULT_PHISICALS + DEFAULT_NONPHISICALS;
    const size_t DEFAULT_EDGES = 14;
    const size_t DEFAULT_AEDGES = 4;
    const size_t DEFAULT_BIASES = 3;
    std::pair<unsigned, unsigned> test_edges[DEFAULT_EDGES] = {{0, 1},  {1, 2},  {2, 0},   {0, 3}, {2, 3},
                                                               {3, 4},  {6, 5},  {5, 7},   {6, 7}, {9, 8},
                                                               {9, 10}, {8, 10}, {11, 12}, {12, 0}};
    std::pair<unsigned, unsigned> test_aedges[DEFAULT_AEDGES] = {{3, 6}, {6, 9}, {2, 5}, {7, 8}};

    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);
    ig.Reserve(DEFAULT_CAPACITY);

    for (size_t i = 0; i < DEFAULT_CAPACITY; i++) {
        auto node = ig.AllocNode();

        auto inst = allocator.New<ConstantInst>(Opcode::Constant, 0);
        inst->SetId(i);

        LiveRange range(i, i);
        auto intervals = allocator.New<LifeIntervals>(&allocator, inst, range);

        node->Assign(intervals);
    }

    for (size_t i = DEFAULT_NONPHISICALS; i < DEFAULT_CAPACITY; i++) {
        ig.GetNode(i).SetFixedColor(251 + i, true);
    }

    for (size_t i = 0; i < DEFAULT_EDGES; i++) {
        auto x = test_edges[i].first;
        auto y = test_edges[i].second;
        ig.AddEdge(x, y);
    }

    for (size_t i = 0; i < DEFAULT_AEDGES; i++) {
        auto x = test_aedges[i].first;
        auto y = test_aedges[i].second;
        ig.AddAffinityEdge(x, y);
    }

    for (size_t i = 0; i < DEFAULT_BIASES; i++) {
        ig.AddBias();
    }

    ig.GetNode(3).SetBias(0);
    ig.GetNode(6).SetBias(0);
    ig.GetNode(9).SetBias(0);
    ig.GetNode(2).SetBias(1);
    ig.GetNode(5).SetBias(1);
    ig.GetNode(7).SetBias(2);
    ig.GetNode(8).SetBias(2);

    EXPECT_EQ(ig.AssignColors<32>(3, 0), 3);

    std::stringstream out;
    ig.Dump("ig_with_phisical", false, out);
    std::string expect_str1 = "Nodes: 14\n"
                              "\n\n"
                              "graph ig_with_phisical {\n"
                              "node [colorscheme=spectral9]\n"
                              "0 [color=0, xlabel=\"0\", tooltip=\" {inst v0}\", shape=\"hexagon\"]\n"
                              "1 [color=1, xlabel=\"1\", tooltip=\"[1:1) {inst v1}\", shape=\"ellipse\"]\n"
                              "2 [color=2, xlabel=\"2\", tooltip=\"[2:2) {inst v2}\", shape=\"ellipse\"]\n"
                              "3 [color=1, xlabel=\"1\", tooltip=\"[3:3) {inst v3}\", shape=\"ellipse\"]\n"
                              "4 [color=0, xlabel=\"0\", tooltip=\"[4:4) {inst v4}\", shape=\"ellipse\"]\n"
                              "5 [color=2, xlabel=\"2\", tooltip=\"[5:5) {inst v5}\", shape=\"ellipse\"]\n"
                              "6 [color=1, xlabel=\"1\", tooltip=\"[6:6) {inst v6}\", shape=\"ellipse\"]\n"
                              "7 [color=0, xlabel=\"0\", tooltip=\"[7:7) {inst v7}\", shape=\"ellipse\"]\n"
                              "8 [color=0, xlabel=\"0\", tooltip=\"[8:8) {inst v8}\", shape=\"ellipse\"]\n"
                              "9 [color=1, xlabel=\"1\", tooltip=\"[9:9) {inst v9}\", shape=\"ellipse\"]\n"
                              "10 [color=2, xlabel=\"2\", tooltip=\"[10:10) {inst v10}\", shape=\"ellipse\"]\n"
                              "11 [color=3, xlabel=\"6\", tooltip=\"[11:11) {inst v11}\", shape=\"box\"]\n"
                              "12 [color=4, xlabel=\"7\", tooltip=\"[12:12) {inst v12}\", shape=\"box\"]\n"
                              "13 [color=5, xlabel=\"8\", tooltip=\"[13:13) {inst v13}\", shape=\"box\"]\n"
                              "1--0\n2--0\n2--1\n3--0\n3--2\n4--3\n6--5\n7--5\n7--6\n9--8\n10--8\n10--9\n"
                              "12--0\n12--11\n}\n";
    EXPECT_EQ(out.str(), expect_str1);

    out.str("");
    ig.Dump("ig_without_phisical", true, out);
    std::string expect_str2 = "Nodes: 14\n"
                              "\n\n"
                              "graph ig_without_phisical {\n"
                              "node [colorscheme=spectral9]\n"
                              "0 [color=0, xlabel=\"0\", tooltip=\" {inst v0}\", shape=\"hexagon\"]\n"
                              "1 [color=1, xlabel=\"1\", tooltip=\"[1:1) {inst v1}\", shape=\"ellipse\"]\n"
                              "2 [color=2, xlabel=\"2\", tooltip=\"[2:2) {inst v2}\", shape=\"ellipse\"]\n"
                              "3 [color=1, xlabel=\"1\", tooltip=\"[3:3) {inst v3}\", shape=\"ellipse\"]\n"
                              "4 [color=0, xlabel=\"0\", tooltip=\"[4:4) {inst v4}\", shape=\"ellipse\"]\n"
                              "5 [color=2, xlabel=\"2\", tooltip=\"[5:5) {inst v5}\", shape=\"ellipse\"]\n"
                              "6 [color=1, xlabel=\"1\", tooltip=\"[6:6) {inst v6}\", shape=\"ellipse\"]\n"
                              "7 [color=0, xlabel=\"0\", tooltip=\"[7:7) {inst v7}\", shape=\"ellipse\"]\n"
                              "8 [color=0, xlabel=\"0\", tooltip=\"[8:8) {inst v8}\", shape=\"ellipse\"]\n"
                              "9 [color=1, xlabel=\"1\", tooltip=\"[9:9) {inst v9}\", shape=\"ellipse\"]\n"
                              "10 [color=2, xlabel=\"2\", tooltip=\"[10:10) {inst v10}\", shape=\"ellipse\"]\n"
                              "1--0\n2--0\n2--1\n3--0\n3--2\n4--3\n6--5\n7--5\n7--6\n9--8\n10--8\n10--9\n}\n";
    EXPECT_EQ(out.str(), expect_str2);
}

/**
 * @tc.name: reg_alloc_interference_test_016
 * @tc.desc: Verify the Dump function with empty InterferenceGraph.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_016, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);

    std::stringstream out;
    ig.Dump("empty_ig", true, out);
    std::string expect_str = "Nodes: 0\n"
                             "\n\n"
                             "graph empty_ig {\n"
                             "node [colorscheme=spectral9]\n"
                             "}\n";
    EXPECT_EQ(out.str(), expect_str);
}

/**
 * @tc.name: reg_alloc_interference_test_017
 * @tc.desc: Verify the Dump function with same color error.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_017, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);
    ig.Reserve(2);

    for (size_t i = 0; i < 2; i++) {
        auto node = ig.AllocNode();

        auto inst = allocator.New<ConstantInst>(Opcode::Constant, i);
        inst->SetId(i);

        LiveRange range(i, i);
        auto intervals = allocator.New<LifeIntervals>(&allocator, inst, range);
        node->Assign(intervals);

        // set same color
        node->SetColor(3);
    }

    ig.AddEdge(0, 1);

    std::stringstream out;
    ig.Dump("same_color_error", false, out);
    std::string expect_str = "Nodes: 2\n"
                             "\n\n"
                             "graph same_color_error {\n"
                             "node [colorscheme=spectral9]\n"
                             "0 [color=0, xlabel=\"3\", tooltip=\" {inst v0}\", shape=\"ellipse\"]\n"
                             "1 [color=0, xlabel=\"3\", tooltip=\"[1:1) {inst v1}\", shape=\"ellipse\"]\n"
                             "Error: Same color\n"
                             "1--0\n"
                             "}\n";
    EXPECT_EQ(out.str(), expect_str);
}

/**
 * @tc.name: reg_alloc_interference_test_018
 * @tc.desc: Verify the IsChordal function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocInterferenceTest, reg_alloc_interference_test_018, TestSize.Level1)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    InterferenceGraph ig(&allocator);
    ig.Reserve(5);

    for (size_t i = 0; i < 5; i++) {
        ig.AllocNode();
        EXPECT_TRUE(ig.IsChordal());
    }
    ig.AddEdge(0, 1);
    EXPECT_TRUE(ig.IsChordal());
    ig.AddEdge(1, 2);
    EXPECT_TRUE(ig.IsChordal());
    ig.AddEdge(2, 0);
    EXPECT_TRUE(ig.IsChordal());

    ig.AddEdge(3, 4);
    ig.AddEdge(1, 3);
    ig.AddEdge(2, 4);
    EXPECT_FALSE(ig.IsChordal());

    ig.AddEdge(1, 4);
    EXPECT_TRUE(ig.IsChordal());
}
}  // namespace panda::compiler

