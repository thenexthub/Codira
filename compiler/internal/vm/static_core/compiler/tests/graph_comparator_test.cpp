/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "unit_test.h"

namespace ark::compiler {
class GraphComparatorTest : public CommonTest {
public:
    GraphComparatorTest() = default;

    Graph *CreateGraph(std::initializer_list<std::pair<int, int>> inputs)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0U, 10U);
            PARAMETER(1U, 0U).s32();
            PARAMETER(2U, 1U).b();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
            }
            BASIC_BLOCK(3U, 7U)
            {
                INST(4U, Opcode::Add).s32().Inputs(0U, 1U);
            }
            BASIC_BLOCK(4U, 5U, 6U)
            {
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
            }
            BASIC_BLOCK(5U, 7U)
            {
                INST(6U, Opcode::Sub).s32().Inputs(0U, 1U);
            }
            BASIC_BLOCK(6U, 7U)
            {
                INST(7U, Opcode::Mul).s32().Inputs(0U, 1U);
            }
            BASIC_BLOCK(7U, 1U)
            {
                INST(8U, Opcode::Phi).s32().Inputs(inputs);
                INST(9U, Opcode::Return).s32().Inputs(8U);
            }
        }
        return graph;
    }

    Graph *CreateDifferentInstCountGraph1();
    Graph *CreateComparePhi1Graph1();
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(GraphComparatorTest, CompareIDs)
{
    // graph1 and graph2 is equal but have different ids
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 10U);
        PARAMETER(1U, 0U).s32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 1U)
        {
            INST(4U, Opcode::Return).s32().Inputs(2U);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        CONSTANT(23U, 10U);
        PARAMETER(3U, 0U).s32();
        BASIC_BLOCK(15U, 25U)
        {
            INST(6U, Opcode::Add).s32().Inputs(23U, 3U);
        }
        BASIC_BLOCK(25U, 1U)
        {
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

Graph *GraphComparatorTest::CreateComparePhi1Graph1()
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 10U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(6U, Opcode::Phi).s32().Inputs({{3U, 4U}, {4U, 5U}});
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    return graph1;
}

TEST_F(GraphComparatorTest, ComparePhi1)
{
    // graph1 and graph2 is equal but have different ids
    auto graph1 = CreateComparePhi1Graph1();
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        CONSTANT(0U, 10U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(6U, Opcode::Phi).s32().Inputs({{4U, 5U}, {3U, 4U}});
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(GraphComparatorTest, ComparePhi2)
{
    auto graph1 = CreateGraph({{3U, 4U}, {5U, 6U}, {6U, 7U}});
    auto graph2 = CreateGraph({{3U, 4U}, {6U, 7U}, {5U, 6U}});
    auto graph3 = CreateGraph({{5U, 6U}, {3U, 4U}, {6U, 7U}});
    auto graph4 = CreateGraph({{5U, 6U}, {6U, 7U}, {3U, 4U}});
    auto graph5 = CreateGraph({{6U, 7U}, {3U, 4U}, {5U, 6U}});
    auto graph6 = CreateGraph({{6U, 7U}, {5U, 6U}, {3U, 4U}});
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph3));
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph4));
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph5));
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph6));
    ASSERT_TRUE(GraphComparator().Compare(graph2, graph3));
    ASSERT_TRUE(GraphComparator().Compare(graph2, graph4));
    ASSERT_TRUE(GraphComparator().Compare(graph2, graph5));
    ASSERT_TRUE(GraphComparator().Compare(graph2, graph6));
    ASSERT_TRUE(GraphComparator().Compare(graph3, graph4));
    ASSERT_TRUE(GraphComparator().Compare(graph3, graph5));
    ASSERT_TRUE(GraphComparator().Compare(graph3, graph6));
    ASSERT_TRUE(GraphComparator().Compare(graph4, graph5));
    ASSERT_TRUE(GraphComparator().Compare(graph4, graph6));
    ASSERT_TRUE(GraphComparator().Compare(graph5, graph6));
}

Graph *GraphComparatorTest::CreateDifferentInstCountGraph1()
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 10U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(6U, Opcode::Phi).s32().Inputs({{3U, 4U}, {4U, 5U}});
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    return graph1;
}

TEST_F(GraphComparatorTest, CompareDifferentInstCount)
{
    auto graph1 = CreateDifferentInstCountGraph1();
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        CONSTANT(0U, 10U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(6U, Opcode::Phi).s32().Inputs({{3U, 4U}, {4U, 5U}});
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_FALSE(GraphComparator().Compare(graph1, graph2));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
