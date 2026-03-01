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
#include "optimizer/optimizations/select_optimization.h"

namespace ark::compiler {
class SelectOptimizationTest : public GraphTest {
public:
    SelectOptimizationTest()
    {
        SetGraphArch(ark::RUNTIME_ARCH);
    }
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(SelectOptimizationTest, SelectEqCostantBoolOperands01X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).i32();
        CONSTANT(1U, 1U).i32();
        PARAMETER(2U, 0U).b();
        CONSTANT(3U, 1U).i32();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Select).b().SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U, 1U, 2U, 3U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::XorI).b().Inputs(2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(8U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectImmEqCostantBoolOperands01X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).i32();
        CONSTANT(1U, 1U).i32();
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U, 1U, 2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::XorI).b().Inputs(2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(8U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectEqCostantBoolOperands10X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).i32();
        CONSTANT(1U, 0U).i32();
        PARAMETER(2U, 0U).b();
        CONSTANT(3U, 1U).i32();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Select).b().SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U, 1U, 2U, 3U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectImmEqCostantBoolOperands10X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).i32();
        CONSTANT(1U, 0U).i32();
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U, 1U, 2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectNeCostantBoolOperands01X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).i32();
        CONSTANT(1U, 1U).i32();
        PARAMETER(2U, 0U).b();
        CONSTANT(3U, 1U).i32();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Select).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U, 3U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectImmNeCostantBoolOperands01X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).i32();
        CONSTANT(1U, 1U).i32();
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectNeCostantBoolOperands10X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).i32();
        CONSTANT(1U, 0U).i32();
        PARAMETER(2U, 0U).b();
        CONSTANT(3U, 1U).i32();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Select).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U, 3U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::XorI).b().Inputs(2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(8U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectImmNeCostantBoolOperands10X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).i32();
        CONSTANT(1U, 0U).i32();
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::XorI).b().Inputs(2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(8U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
