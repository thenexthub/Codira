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

#include <utility>
#include <algorithm>
#include "unit_test.h"
#include "optimizer/optimizations/regalloc/interference_graph.h"

namespace panda::compiler {
class RegAllocInterferenceTest : public GraphTest {
};

namespace {
constexpr unsigned DEFAULT_CAPACITY1 = 10;
unsigned test_edges1[2][2] = {{0, 1}, {7, 4}};
auto IsInSet = [](unsigned a, unsigned b) {
    for (int i = 0; i < 2; i++) {
        if ((a == test_edges1[i][0] && b == test_edges1[i][1]) || (b == test_edges1[i][0] && a == test_edges1[i][1])) {
            return true;
        }
    }
    return false;
};
}  // namespace

TEST_F(RegAllocInterferenceTest, Basic)
{
    GraphMatrix matrix(GetLocalAllocator());
    matrix.SetCapacity(DEFAULT_CAPACITY1);
    EXPECT_FALSE(matrix.AddEdge(0, 1));
    EXPECT_FALSE(matrix.AddEdge(7, 4));
    for (unsigned i = 0; i < DEFAULT_CAPACITY1; i++) {
        for (unsigned j = 0; j < DEFAULT_CAPACITY1; j++) {
            ASSERT_EQ(matrix.HasEdge(i, j), IsInSet(i, j));
        }
    }
    EXPECT_GE(matrix.GetCapacity(), DEFAULT_CAPACITY1);
}

TEST_F(RegAllocInterferenceTest, BasicAfinity)
{
    GraphMatrix matrix(GetLocalAllocator());
    matrix.SetCapacity(DEFAULT_CAPACITY1);
    EXPECT_FALSE(matrix.AddAffinityEdge(0, 1));
    EXPECT_FALSE(matrix.AddAffinityEdge(7, 4));
    for (unsigned i = 0; i < DEFAULT_CAPACITY1; i++) {
        for (unsigned j = 0; j < DEFAULT_CAPACITY1; j++) {
            EXPECT_EQ(matrix.HasAffinityEdge(i, j), IsInSet(i, j));
        }
    }
    EXPECT_GE(matrix.GetCapacity(), DEFAULT_CAPACITY1);
}

TEST_F(RegAllocInterferenceTest, BasicGraph)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(DEFAULT_CAPACITY1);

    EXPECT_EQ(gr.Size(), 0);
    auto *node1 = gr.AllocNode();
    EXPECT_EQ(gr.Size(), 1);
    EXPECT_EQ(node1->GetNumber(), 0);

    auto *node2 = gr.AllocNode();
    EXPECT_EQ(gr.Size(), 2);
    EXPECT_EQ(node2->GetNumber(), 1);
    EXPECT_NE(node1, node2);
}

TEST_F(RegAllocInterferenceTest, GraphChordal)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(DEFAULT_CAPACITY1);

    EXPECT_TRUE(gr.IsChordal());

    gr.AllocNode();
    EXPECT_TRUE(gr.IsChordal());

    gr.AllocNode();
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
    EXPECT_FALSE(gr.IsChordal());
}

namespace {
const unsigned DEFAULT_CAPACITY2 = 5;
const unsigned DEFAULT_EDGES2 = 6;
::std::pair<unsigned, unsigned> test_edges2[DEFAULT_EDGES2] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {2, 3}, {3, 4}};

// To prevent adding "remove edge" interfaces to main code, edge removing is simulated via building new graph without
// it.
InterferenceGraph BuildSubgraph(InterferenceGraph &orig_gr, ArenaAllocator *alloc,
                                ::std::pair<unsigned, unsigned> *edges, unsigned count, ArenaVector<unsigned> &peo,
                                unsigned peo_count)
{
    InterferenceGraph gr(alloc);
    gr.Reserve(orig_gr.Size());

    for (unsigned i = 0; i < count; i++) {
        auto x = edges[i].first;
        auto y = edges[i].second;
        for (unsigned j = 0; j < peo_count; j++) {
            if (x == peo[j] || y == peo[j]) {
                continue;
            }
        }
        gr.AddEdge(x, y);
    }

    return gr;
}
}  // namespace

TEST_F(RegAllocInterferenceTest, LexBFSSimple)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(2);

    gr.AllocNode();
    gr.AllocNode();
    gr.AddEdge(0, 1);

    auto peo = gr.LexBFS();
    EXPECT_EQ(peo.size(), 2);
    EXPECT_EQ(peo[0], 0);
    EXPECT_EQ(peo[1], 1);
}

TEST_F(RegAllocInterferenceTest, LexBFS)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(DEFAULT_CAPACITY2);

    gr.AllocNode();
    gr.AllocNode();
    gr.AllocNode();
    gr.AllocNode();
    gr.AllocNode();
    for (unsigned i = 0; i < DEFAULT_EDGES2; i++) {
        auto x = test_edges2[i].first;
        auto y = test_edges2[i].second;
        gr.AddEdge(x, y);
    }

    auto peo = gr.LexBFS();
    EXPECT_EQ(peo.size(), DEFAULT_CAPACITY2);
    std::reverse(peo.begin(), peo.end());

    for (unsigned i = 0; i < (DEFAULT_CAPACITY2 - 1); i++) {
        auto gr2 = BuildSubgraph(gr, GetLocalAllocator(), test_edges2, DEFAULT_EDGES2, peo, i);
        EXPECT_TRUE(gr2.IsChordal());
    }
}

TEST_F(RegAllocInterferenceTest, AssignColorsSimple)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(DEFAULT_CAPACITY2);

    auto *nd0 = gr.AllocNode();
    auto *nd1 = gr.AllocNode();
    auto *nd2 = gr.AllocNode();
    auto *nd3 = gr.AllocNode();
    auto *nd4 = gr.AllocNode();
    for (unsigned i = 0; i < DEFAULT_EDGES2; i++) {
        auto x = test_edges2[i].first;
        auto y = test_edges2[i].second;
        gr.AddEdge(x, y);
    }

    EXPECT_EQ(gr.AssignColors<32>(3, 0), 3);
    EXPECT_NE(nd0->GetColor(), nd1->GetColor());
    EXPECT_NE(nd0->GetColor(), nd2->GetColor());
    EXPECT_NE(nd0->GetColor(), nd3->GetColor());

    EXPECT_NE(nd2->GetColor(), nd1->GetColor());
    EXPECT_NE(nd2->GetColor(), nd3->GetColor());

    EXPECT_NE(nd4->GetColor(), nd3->GetColor());
}

TEST_F(RegAllocInterferenceTest, AssignColors)
{
    const unsigned DEFAULT_CAPACITY = 11;
    const unsigned DEFAULT_EDGES = 12;
    const unsigned DEFAULT_AEDGES = 4;
    ::std::pair<unsigned, unsigned> test_edges[DEFAULT_EDGES] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {2, 3},  {3, 4},
                                                                 {6, 5}, {5, 7}, {6, 7}, {9, 8}, {9, 10}, {8, 10}};
    ::std::pair<unsigned, unsigned> test_aedges[DEFAULT_AEDGES] = {{3, 6}, {6, 9}, {2, 5}, {7, 8}};

    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(DEFAULT_CAPACITY);

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

    for (unsigned i = 0; i < DEFAULT_EDGES; i++) {
        auto x = test_edges[i].first;
        auto y = test_edges[i].second;
        gr.AddEdge(x, y);
    }
    for (unsigned i = 0; i < DEFAULT_AEDGES; i++) {
        auto x = test_aedges[i].first;
        auto y = test_aedges[i].second;
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

    // Check biases values
    EXPECT_NE(bias0.color, bias1.color);
    EXPECT_NE(bias0.color, bias2.color);
}
}  // namespace panda::compiler
