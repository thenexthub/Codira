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

#include "unit_test.h"

namespace panda::compiler {
class GraphComparatorTest : public CommonTest {
public:
    GraphComparatorTest() {}

    Graph *CreateGraph(std::initializer_list<std::pair<int, int>> inputs)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0, 10);
            PARAMETER(1, 0).s32();
            PARAMETER(2, 1).b();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
            }
            BASIC_BLOCK(3, 7)
            {
                INST(4, Opcode::Add).s32().Inputs(0, 1);
            }
            BASIC_BLOCK(4, 5, 6)
            {
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
            }
            BASIC_BLOCK(5, 7)
            {
                INST(6, Opcode::Sub).s32().Inputs(0, 1);
            }
            BASIC_BLOCK(6, 7)
            {
                INST(7, Opcode::Mul).s32().Inputs(0, 1);
            }
            BASIC_BLOCK(7, 1)
            {
                INST(8, Opcode::Phi).s32().Inputs(inputs);
                INST(9, Opcode::Return).s32().Inputs(8);
            }
        }
        return graph;
    }
};

TEST_F(GraphComparatorTest, CompareIDs)
{
    // graph1 and graph2 is equal but have different ids
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 10);
        PARAMETER(1, 0).s32();
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::Add).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(3, 1)
        {
            INST(4, Opcode::Return).s32().Inputs(2);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        CONSTANT(23, 10);
        PARAMETER(3, 0).s32();
        BASIC_BLOCK(15, 25)
        {
            INST(6, Opcode::Add).s32().Inputs(23, 3);
        }
        BASIC_BLOCK(25, 1)
        {
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(GraphComparatorTest, ComparePhi1)
{
    // graph1 and graph2 is equal but have different ids
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 10);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).b();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Add).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(6, Opcode::Phi).s32().Inputs({{3, 4}, {4, 5}});
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        CONSTANT(0, 10);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).b();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Add).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(6, Opcode::Phi).s32().Inputs({{4, 5}, {3, 4}});
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(GraphComparatorTest, ComparePhi2)
{
    auto graph1 = CreateGraph({{3, 4}, {5, 6}, {6, 7}});
    auto graph2 = CreateGraph({{3, 4}, {6, 7}, {5, 6}});
    auto graph3 = CreateGraph({{5, 6}, {3, 4}, {6, 7}});
    auto graph4 = CreateGraph({{5, 6}, {6, 7}, {3, 4}});
    auto graph5 = CreateGraph({{6, 7}, {3, 4}, {5, 6}});
    auto graph6 = CreateGraph({{6, 7}, {5, 6}, {3, 4}});
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

TEST_F(GraphComparatorTest, CompareDifferentInstCount)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0, 10);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).b();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Add).s32().Inputs(0, 1);
            INST(20, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(6, Opcode::Phi).s32().Inputs({{3, 4}, {4, 5}});
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }

    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        CONSTANT(0, 10);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).b();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Add).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
            INST(20, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(6, Opcode::Phi).s32().Inputs({{3, 4}, {4, 5}});
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }
    ASSERT_FALSE(GraphComparator().Compare(graph1, graph2));
}
}  // namespace panda::compiler
