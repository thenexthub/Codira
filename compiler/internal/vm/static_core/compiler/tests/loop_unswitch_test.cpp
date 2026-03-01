/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "optimizer/ir/analysis.h"
#include "optimizer/optimizations/loop_peeling.h"
#include "optimizer/optimizations/loop_unswitch.h"
#include "optimizer/optimizations/branch_elimination.h"

namespace ark::compiler {
// NOLINTBEGIN(readability-magic-numbers,readability-function-size)
class LoopUnswitchTest : public GraphTest {
public:
    // CC-OFFNXT(huge_method, G.FUN.01) graph creation
    void CreateIncLoopGraph(int count)
    {
        GRAPH(GetGraph())
        {
            PARAMETER(1U, 1U).ptr();
            CONSTANT(3U, 0U).i64();
            CONSTANT(4U, 1U).i64();
            CONSTANT(100U, count).i64();
            CONSTANT(8U, 15U).i64();

            BASIC_BLOCK(20U, 10U)
            {
                INST(7U, Opcode::Load).i64().Inputs(1U, 4U);
                INST(19U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(7U, 8U);
            }

            BASIC_BLOCK(10U, 9U, 6U)
            {
                INST(27U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(3U, 4U);
                INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(27U);
            }

            BASIC_BLOCK(6U, 2U)
            {
                INST(10U, Opcode::Phi).i32().Inputs(3U, 25U);
                INST(13U, Opcode::Phi).i32().Inputs(4U, 23U);
            }

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(20U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(19U);
            }

            BASIC_BLOCK(4U, 5U)
            {
                INST(21U, Opcode::Add).i32().Inputs(10U, 13U);
            }

            BASIC_BLOCK(3U, 5U)
            {
                INST(22U, Opcode::Sub).i32().Inputs(13U, 10U);
            }

            BASIC_BLOCK(5U, 11U)
            {
                INST(23U, Opcode::Phi).i32().Inputs(21U, 22U);
                INST(25U, Opcode::Add).i32().Inputs(10U, 4U);
            }

            BASIC_BLOCK(11U, 9U, 6U)
            {
                INST(17U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(25U, 100U);
                INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(17U);
            }

            BASIC_BLOCK(9U, 21U)
            {
                INST(32U, Opcode::Phi).i32().Inputs(4U, 23U);
            }

            BASIC_BLOCK(21U, -1L)
            {
                INST(26U, Opcode::Return).i32().Inputs(32U);
            }
        }
    }

    // CC-OFFNXT(huge_method, G.FUN.01) graph creation
    void CreateDecLoopGraph(int count)
    {
        GRAPH(GetGraph())
        {
            PARAMETER(1U, 1U).ptr();
            CONSTANT(3U, 1U).i64();
            CONSTANT(7U, 15U).i64();
            CONSTANT(16U, 0U).i64();
            CONSTANT(25U, -1L).i64();
            CONSTANT(100U, count).i64();

            BASIC_BLOCK(20U, 10U)
            {
                INST(6U, Opcode::Load).i64().Inputs(1U, 7U);
                INST(18U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(6U, 7U);
            }

            BASIC_BLOCK(10U, 9U, 6U)
            {
                INST(27U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LE).Inputs(3U, 16U);
                INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(27U);
            }

            BASIC_BLOCK(6U, 2U)
            {
                INST(9U, Opcode::Phi).i32().Inputs(3U, 22U);
                INST(11U, Opcode::Phi).i32().Inputs(100U, 24U);
            }

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
            }

            BASIC_BLOCK(4U, 5U)
            {
                INST(20U, Opcode::Add).i32().Inputs(11U, 9U);
            }

            BASIC_BLOCK(3U, 5U)
            {
                INST(21U, Opcode::Sub).i32().Inputs(9U, 11U);
            }

            BASIC_BLOCK(5U, 11U)
            {
                INST(22U, Opcode::Phi).i32().Inputs(20U, 21U);
                INST(24U, Opcode::Sub).i32().Inputs(11U, 3U);
            }

            BASIC_BLOCK(11U, 9U, 6U)
            {
                INST(15U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LE).Inputs(24U, 16U);
                INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(15U);
            }

            BASIC_BLOCK(9U, 21U)
            {
                INST(31U, Opcode::Phi).i32().Inputs(3U, 22U);
            }

            BASIC_BLOCK(21U, -1L)
            {
                INST(26U, Opcode::Return).i32().Inputs(31U);
            }
        }
    }

    void BuildGraphSingleCondition();
    Graph *BuildExpectedSingleCondition();

    void BuildGraphSameCondition();
    Graph *BuildExpectedSameCondition();

    void BuildGraphMultipleConditionsLimitInstructions();
    Graph *BuildExpectedMultipleConditionsLimitInstructions();

    void BuildGraphMultipleConditionsLimitLevel();
    Graph *BuildExpectedMultipleConditionsLimitLevel();

    void BuildGraphMultipleConditions();
    Graph *BuildExpectedMultipleConditions();
};

void LoopUnswitchTest::BuildGraphSingleCondition()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(2U, 100U).i64();
        CONSTANT(3U, 0U).i64();
        CONSTANT(4U, 1U).i64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).i64().Inputs(3U, 11U);
            INST(6U, Opcode::Phi).i64().Inputs(3U, 12U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(5U, 8U)
        {
            INST(9U, Opcode::Add).i64().Inputs(5U, 6U);
        }
        BASIC_BLOCK(6U, 8U)
        {
            INST(10U, Opcode::Sub).i64().Inputs(5U, 6U);
        }
        BASIC_BLOCK(8U, 2U)
        {
            INST(11U, Opcode::Phi).i64().Inputs(9U, 10U);
            INST(12U, Opcode::Add).i64().Inputs(6U, 4U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::Return).i64().Inputs(5U);
        }
    }
}

Graph *LoopUnswitchTest::BuildExpectedSingleCondition()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        CONSTANT(2U, 100U).i64();
        CONSTANT(3U, 0U).i64();
        CONSTANT(4U, 1U).i64();

        BASIC_BLOCK(27U, 16U, 18U)
        {
            INST(35U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(18U, 3U, 19U)
        {
            INST(22U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(3U, 2U);
            INST(23U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(22U);
        }
        BASIC_BLOCK(19U, 3U, 19U)
        {
            INST(24U, Opcode::Phi).i64().Inputs(3U, 32U);
            INST(25U, Opcode::Phi).i64().Inputs(3U, 28U);
            INST(32U, Opcode::Sub).i64().Inputs(24U, 25U);
            INST(28U, Opcode::Add).i64().Inputs(25U, 4U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(28U, 2U);
            INST(27U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(34U, Opcode::Phi).i64().Inputs({{2, 9U}, {19U, 32U}, {16U, 3U}, {18U, 3U}});
            INST(15U, Opcode::Return).i64().Inputs(34U);
        }
        BASIC_BLOCK(16U, 3U, 2U)
        {
            INST(16U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(3U, 2U);
            INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(16U);
        }
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(5U, Opcode::Phi).i64().Inputs(3U, 9U);
            INST(6U, Opcode::Phi).i64().Inputs(3U, 12U);
            INST(9U, Opcode::Add).i64().Inputs(5U, 6U);
            INST(12U, Opcode::Add).i64().Inputs(6U, 4U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(12U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
    }
    return graph;
}

TEST_F(LoopUnswitchTest, TestSingleCondition)
{
    BuildGraphSingleCondition();
    ASSERT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2U, 100U));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = BuildExpectedSingleCondition();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void LoopUnswitchTest::BuildGraphSameCondition()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(2U, 100U).i64();
        CONSTANT(3U, 0U).i64();
        CONSTANT(4U, 1U).i64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).i64().Inputs(3U, 16U);
            INST(6U, Opcode::Phi).i64().Inputs(3U, 15U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(5U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(6U, 5U)
        {
            INST(10U, Opcode::Add).i64().Inputs(5U, 6U);
        }
        BASIC_BLOCK(5U, 8U)
        {
            INST(11U, Opcode::Phi).i64().Inputs(5U, 10U);
            INST(12U, Opcode::Add).i64().Inputs(11U, 4U);
        }
        BASIC_BLOCK(8U, 9U, 10U)
        {
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(9U, 10U)
        {
            INST(14U, Opcode::Sub).i64().Inputs(12U, 6U);
        }
        BASIC_BLOCK(10U, 2U)
        {
            INST(15U, Opcode::Phi).i64().Inputs(12U, 14U);
            INST(16U, Opcode::Add).i64().Inputs(5U, 4U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::Return).i64().Inputs(6U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
Graph *LoopUnswitchTest::BuildExpectedSameCondition()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(2U, 100U).i64();
        CONSTANT(3U, 0U).i64();
        CONSTANT(4U, 1U).i64();

        BASIC_BLOCK(29U, 16U, 18U)
        {
            INST(40U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(18U, 27U, 19U)
        {
            INST(24U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(3U, 2U);
            INST(25U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(19U, 25U)
        {
            INST(26U, Opcode::Phi).i64().Inputs(3U, 30U);
            INST(27U, Opcode::Phi).i64().Inputs(3U, 33U);
        }

        BASIC_BLOCK(25U, 27U, 19U)
        {
            INST(36U, Opcode::Add).i64().Inputs(26U, 27U);
            INST(33U, Opcode::Add).i64().Inputs(36U, 4U);
            INST(30U, Opcode::Add).i64().Inputs(26U, 4U);

            INST(28U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(30U, 2U);
            INST(29U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(28U);
        }

        BASIC_BLOCK(27U, 28U)
        {
            INST(38U, Opcode::Phi).i64().Inputs(3U, 33U);
        }

        BASIC_BLOCK(16U, 15U, 2U)
        {
            INST(18U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(3U, 2U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
        }

        BASIC_BLOCK(2U, 15U, 2U)
        {
            INST(5U, Opcode::Phi).i64().Inputs(3U, 16U);
            INST(6U, Opcode::Phi).i64().Inputs(3U, 14U);
            INST(12U, Opcode::Add).i64().Inputs(5U, 4U);
            INST(14U, Opcode::Sub).i64().Inputs(12U, 6U);
            INST(16U, Opcode::Add).i64().Inputs(5U, 4U);

            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(16U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(15U, 28U)
        {
            INST(23U, Opcode::Phi).i64().Inputs(3U, 14U);
        }

        BASIC_BLOCK(28U, -1L)
        {
            INST(39U, Opcode::Phi).i64().Inputs(38U, 23U);
            INST(17U, Opcode::Return).i64().Inputs(39U);
        }
    }
    return graph;
}

TEST_F(LoopUnswitchTest, TestSameCondition)
{
    BuildGraphSameCondition();
    ASSERT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2U, 100U));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = BuildExpectedSameCondition();
    graph->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
void LoopUnswitchTest::BuildGraphMultipleConditions()
{
    GRAPH(GetGraph())
    {
        PARAMETER(1U, 1U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();
        CONSTANT(12U, 15U).i64();
        CONSTANT(37U, 2U).i64();

        BASIC_BLOCK(20U, 15U)
        {
            INST(8U, Opcode::Load).i64().Inputs(1U, 4U);
            INST(11U, Opcode::Load).i64().Inputs(1U, 5U);
            INST(43U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11U, 12U);
            INST(38U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8U, 12U);
            INST(31U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(11U, 12U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8U, 12U);
        }

        BASIC_BLOCK(15U, 14U, 11U)
        {
            INST(50U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(51U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(50U);
        }

        BASIC_BLOCK(11U, 2U)
        {
            INST(16U, Opcode::Phi).i32().Inputs(4U, 48U);
            INST(17U, Opcode::Phi).i32().Inputs(5U, 46U);
        }

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(27U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(28U, Opcode::Add).i32().Inputs(16U, 17U);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(29U, Opcode::Phi).i32().Inputs(17U, 28U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(31U);
        }

        BASIC_BLOCK(6U, 5U)
        {
            INST(33U, Opcode::Add).i32().Inputs(29U, 16U);
        }

        BASIC_BLOCK(5U, 7U, 8U)
        {
            INST(34U, Opcode::Phi).i32().Inputs(29U, 33U);
            INST(36U, Opcode::Add).i32().Inputs(34U, 37U);
            INST(39U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(38U);
        }

        BASIC_BLOCK(8U, 7U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(36U, 16U);
        }

        BASIC_BLOCK(7U, 9U, 10U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(36U, 40U);
            INST(44U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(43U);
        }

        BASIC_BLOCK(10U, 9U)
        {
            INST(45U, Opcode::Sub).i32().Inputs(41U, 16U);
        }

        BASIC_BLOCK(9U, 16U)
        {
            INST(46U, Opcode::Phi).i32().Inputs(41U, 45U);
            INST(48U, Opcode::Add).i32().Inputs(16U, 5U);
        }

        BASIC_BLOCK(16U, 14U, 11U)
        {
            INST(24U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48U, 3U);
            INST(25U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(24U);
        }

        BASIC_BLOCK(14U, 21U)
        {
            INST(55U, Opcode::Phi).i32().Inputs(5U, 46U);
        }

        BASIC_BLOCK(21U, -1L)
        {
            INST(49U, Opcode::Return).i32().Inputs(55U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
Graph *LoopUnswitchTest::BuildExpectedMultipleConditions()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1U, 1U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();
        CONSTANT(12U, 15U).i64();
        CONSTANT(37U, 2U).i64();

        BASIC_BLOCK(20U, 71U, 86U)
        {
            INST(8U, Opcode::Load).i64().Inputs(1U, 4U);
            INST(11U, Opcode::Load).i64().Inputs(1U, 5U);
            INST(43U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11U, 12U);
            INST(38U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8U, 12U);
            INST(78U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(43U);
        }

        BASIC_BLOCK(86U, 42U, 72U)
        {
            INST(124U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(38U);
        }

        BASIC_BLOCK(72U, 21U, 77U)
        {
            INST(102U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(103U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(102U);
        }

        BASIC_BLOCK(77U, 21U, 77U)
        {
            INST(104U, Opcode::Phi).i32().Inputs(4U, 108U);
            INST(105U, Opcode::Phi).i32().Inputs(5U, 121U);
            INST(112U, Opcode::Add).i32().Inputs(105U, 37U);
            INST(120U, Opcode::Sub).i32().Inputs(112U, 104U);
            INST(121U, Opcode::Sub).i32().Inputs(120U, 104U);
            INST(108U, Opcode::Add).i32().Inputs(104U, 5U);
            INST(106U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(108U, 3U);
            INST(107U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(106U);
        }

        BASIC_BLOCK(42U, 21U, 50U)
        {
            INST(56U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(57U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(56U);
        }

        BASIC_BLOCK(50U, 21U, 50U)
        {
            INST(58U, Opcode::Phi).i32().Inputs(4U, 62U);
            INST(59U, Opcode::Phi).i32().Inputs(5U, 75U);
            INST(72U, Opcode::Add).i32().Inputs(58U, 59U);
            INST(66U, Opcode::Add).i32().Inputs(72U, 37U);
            INST(75U, Opcode::Sub).i32().Inputs(66U, 58U);
            INST(62U, Opcode::Add).i32().Inputs(58U, 5U);
            INST(60U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(62U, 3U);
            INST(61U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(60U);
        }

        BASIC_BLOCK(71U, 15U, 57U)
        {
            INST(101U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(38U);
        }

        BASIC_BLOCK(57U, 21U, 66U)
        {
            INST(79U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(80U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(79U);
        }

        BASIC_BLOCK(66U, 21U, 66U)
        {
            INST(81U, Opcode::Phi).i32().Inputs(4U, 85U);
            INST(82U, Opcode::Phi).i32().Inputs(5U, 97U);
            INST(96U, Opcode::Add).i32().Inputs(82U, 81U);
            INST(89U, Opcode::Add).i32().Inputs(96U, 37U);
            INST(97U, Opcode::Sub).i32().Inputs(89U, 81U);
            INST(85U, Opcode::Add).i32().Inputs(81U, 5U);
            INST(83U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(85U, 3U);
            INST(84U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(83U);
        }

        BASIC_BLOCK(15U, 21U, 4U)
        {
            INST(50U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(51U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(50U);
        }

        BASIC_BLOCK(4U, 21U, 4U)
        {
            INST(16U, Opcode::Phi).i32().Inputs(4U, 48U);
            INST(17U, Opcode::Phi).i32().Inputs(5U, 36U);
            INST(28U, Opcode::Add).i32().Inputs(16U, 17U);
            INST(33U, Opcode::Add).i32().Inputs(28U, 16U);
            INST(36U, Opcode::Add).i32().Inputs(33U, 37U);
            INST(48U, Opcode::Add).i32().Inputs(16U, 5U);
            INST(24U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48U, 3U);
            INST(25U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(21U, -1L)
        {
            INST(77U, Opcode::Phi)
                .i32()
                .Inputs({{15U, 5U}, {42U, 5U}, {57U, 5U}, {4U, 36U}, {66U, 97U}, {72U, 5U}, {50U, 75U}, {77U, 121U}});
            INST(49U, Opcode::Return).i32().Inputs(77U);
        }
    }
    return graph;
}

TEST_F(LoopUnswitchTest, TestMultipleConditions)
{
    BuildGraphMultipleConditions();
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2U, 100U));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = BuildExpectedMultipleConditions();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
void LoopUnswitchTest::BuildGraphMultipleConditionsLimitLevel()
{
    GRAPH(GetGraph())
    {
        PARAMETER(1U, 1U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();
        CONSTANT(12U, 15U).i64();
        CONSTANT(37U, 2U).i64();

        BASIC_BLOCK(20U, 15U)
        {
            INST(8U, Opcode::Load).i64().Inputs(1U, 4U);
            INST(11U, Opcode::Load).i64().Inputs(1U, 5U);
            INST(43U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11U, 12U);
            INST(38U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8U, 12U);
            INST(31U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(11U, 12U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8U, 12U);
        }

        BASIC_BLOCK(15U, 14U, 11U)
        {
            INST(50U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(51U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(50U);
        }

        BASIC_BLOCK(11U, 2U)
        {
            INST(16U, Opcode::Phi).i32().Inputs(4U, 48U);
            INST(17U, Opcode::Phi).i32().Inputs(5U, 46U);
        }

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(27U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(28U, Opcode::Add).i32().Inputs(16U, 17U);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(29U, Opcode::Phi).i32().Inputs(17U, 28U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(31U);
        }

        BASIC_BLOCK(6U, 5U)
        {
            INST(33U, Opcode::Add).i32().Inputs(29U, 16U);
        }

        BASIC_BLOCK(5U, 7U, 8U)
        {
            INST(34U, Opcode::Phi).i32().Inputs(29U, 33U);
            INST(36U, Opcode::Add).i32().Inputs(34U, 37U);
            INST(39U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(38U);
        }

        BASIC_BLOCK(8U, 7U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(36U, 16U);
        }

        BASIC_BLOCK(7U, 9U, 10U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(36U, 40U);
            INST(44U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(43U);
        }

        BASIC_BLOCK(10U, 9U)
        {
            INST(45U, Opcode::Sub).i32().Inputs(41U, 16U);
        }

        BASIC_BLOCK(9U, 16U)
        {
            INST(46U, Opcode::Phi).i32().Inputs(41U, 45U);
            INST(48U, Opcode::Add).i32().Inputs(16U, 5U);
        }

        BASIC_BLOCK(16U, 14U, 11U)
        {
            INST(24U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48U, 3U);
            INST(25U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(24U);
        }

        BASIC_BLOCK(14U, 21U)
        {
            INST(55U, Opcode::Phi).i32().Inputs(5U, 46U);
        }

        BASIC_BLOCK(21U, -1L)
        {
            INST(49U, Opcode::Return).i32().Inputs(55U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
Graph *LoopUnswitchTest::BuildExpectedMultipleConditionsLimitLevel()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1U, 1U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();
        CONSTANT(12U, 15U).i64();
        CONSTANT(37U, 2U).i64();

        BASIC_BLOCK(20U, 15U, 42U)
        {
            INST(8U, Opcode::Load).i64().Inputs(1U, 4U);
            INST(11U, Opcode::Load).i64().Inputs(1U, 5U);
            INST(43U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11U, 12U);
            INST(38U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8U, 12U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8U, 12U);
            INST(78U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(43U);
        }

        BASIC_BLOCK(42U, 21U, 49U)
        {
            INST(56U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(57U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(56U);
        }

        BASIC_BLOCK(49U, 47U, 50U)
        {
            INST(58U, Opcode::Phi).i32().Inputs(4U, 62U);
            INST(59U, Opcode::Phi).i32().Inputs(5U, 75U);
            INST(71U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(50U, 47U)
        {
            INST(72U, Opcode::Add).i32().Inputs(58U, 59U);
        }

        BASIC_BLOCK(47U, 53U, 52U)
        {
            INST(70U, Opcode::Phi).i32().Inputs(59U, 72U);
            INST(66U, Opcode::Add).i32().Inputs(70U, 37U);
            INST(67U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(38U);
        }

        BASIC_BLOCK(52U, 53U)
        {
            INST(74U, Opcode::Sub).i32().Inputs(66U, 58U);
        }

        BASIC_BLOCK(53U, 21U, 49U)
        {
            INST(65U, Opcode::Phi).i32().Inputs(66U, 74U);
            INST(75U, Opcode::Sub).i32().Inputs(65U, 58U);
            INST(62U, Opcode::Add).i32().Inputs(58U, 5U);
            INST(60U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(62U, 3U);
            INST(61U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(60U);
        }

        BASIC_BLOCK(15U, 21U, 2U)
        {
            INST(50U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(51U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(50U);
        }

        BASIC_BLOCK(2U, 6U, 4U)
        {
            INST(16U, Opcode::Phi).i32().Inputs(4U, 48U);
            INST(17U, Opcode::Phi).i32().Inputs(5U, 41U);
            INST(27U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(26U);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(28U, Opcode::Add).i32().Inputs(16U, 17U);
        }

        BASIC_BLOCK(6U, 9U, 8U)
        {
            INST(29U, Opcode::Phi).i32().Inputs(17U, 28U);
            INST(33U, Opcode::Add).i32().Inputs(29U, 16U);
            INST(36U, Opcode::Add).i32().Inputs(33U, 37U);
            INST(39U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(38U);
        }

        BASIC_BLOCK(8U, 9U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(36U, 16U);
        }

        BASIC_BLOCK(9U, 21U, 2U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(36U, 40U);
            INST(48U, Opcode::Add).i32().Inputs(16U, 5U);
            INST(24U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48U, 3U);
            INST(25U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(21U, -1L)
        {
            INST(77U, Opcode::Phi).i32().Inputs({{15U, 5U}, {42U, 5U}, {9U, 41U}, {53U, 75U}});
            INST(49U, Opcode::Return).i32().Inputs(77U);
        }
    }
    return graph;
}

TEST_F(LoopUnswitchTest, TestMultipleConditionsLimitLevel)
{
    BuildGraphMultipleConditionsLimitLevel();
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(1U, 100U));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = BuildExpectedMultipleConditionsLimitLevel();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
void LoopUnswitchTest::BuildGraphMultipleConditionsLimitInstructions()
{
    GRAPH(GetGraph())
    {
        PARAMETER(1U, 1U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();
        CONSTANT(12U, 15U).i64();
        CONSTANT(37U, 2U).i64();

        BASIC_BLOCK(20U, 15U)
        {
            INST(8U, Opcode::Load).i64().Inputs(1U, 4U);
            INST(11U, Opcode::Load).i64().Inputs(1U, 5U);
            INST(43U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11U, 12U);
            INST(38U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8U, 12U);
            INST(31U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(11U, 12U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8U, 12U);
        }

        BASIC_BLOCK(15U, 14U, 11U)
        {
            INST(50U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(51U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(50U);
        }

        BASIC_BLOCK(11U, 2U)
        {
            INST(16U, Opcode::Phi).i32().Inputs(4U, 48U);
            INST(17U, Opcode::Phi).i32().Inputs(5U, 46U);
        }

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(27U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(28U, Opcode::Add).i32().Inputs(16U, 17U);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(29U, Opcode::Phi).i32().Inputs(17U, 28U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(31U);
        }

        BASIC_BLOCK(6U, 5U)
        {
            INST(33U, Opcode::Add).i32().Inputs(29U, 16U);
        }

        BASIC_BLOCK(5U, 7U, 8U)
        {
            INST(34U, Opcode::Phi).i32().Inputs(29U, 33U);
            INST(36U, Opcode::Add).i32().Inputs(34U, 37U);
            INST(39U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(38U);
        }

        BASIC_BLOCK(8U, 7U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(36U, 16U);
        }

        BASIC_BLOCK(7U, 9U, 10U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(36U, 40U);
            INST(44U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(43U);
        }

        BASIC_BLOCK(10U, 9U)
        {
            INST(45U, Opcode::Sub).i32().Inputs(41U, 16U);
        }

        BASIC_BLOCK(9U, 16U)
        {
            INST(46U, Opcode::Phi).i32().Inputs(41U, 45U);
            INST(48U, Opcode::Add).i32().Inputs(16U, 5U);
        }

        BASIC_BLOCK(16U, 14U, 11U)
        {
            INST(24U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48U, 3U);
            INST(25U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(24U);
        }

        BASIC_BLOCK(14U, 21U)
        {
            INST(55U, Opcode::Phi).i32().Inputs(5U, 46U);
        }

        BASIC_BLOCK(21U, -1L)
        {
            INST(49U, Opcode::Return).i32().Inputs(55U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
Graph *LoopUnswitchTest::BuildExpectedMultipleConditionsLimitInstructions()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1U, 1U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();
        CONSTANT(12U, 15U).i64();
        CONSTANT(37U, 2U).i64();

        BASIC_BLOCK(20U, 71U, 42U)
        {
            INST(8U, Opcode::Load).i64().Inputs(1U, 4U);
            INST(11U, Opcode::Load).i64().Inputs(1U, 5U);
            INST(43U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11U, 12U);
            INST(38U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8U, 12U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8U, 12U);
            INST(78U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(43U);
        }

        BASIC_BLOCK(42U, 21U, 49U)
        {
            INST(56U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(57U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(56U);
        }

        BASIC_BLOCK(49U, 47U, 50U)
        {
            INST(58U, Opcode::Phi).i32().Inputs(4U, 62U);
            INST(59U, Opcode::Phi).i32().Inputs(5U, 75U);
            INST(71U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(50U, 47U)
        {
            INST(72U, Opcode::Add).i32().Inputs(58U, 59U);
        }

        BASIC_BLOCK(47U, 53U, 52U)
        {
            INST(70U, Opcode::Phi).i32().Inputs(59U, 72U);
            INST(66U, Opcode::Add).i32().Inputs(70U, 37U);
            INST(67U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(38U);
        }

        BASIC_BLOCK(52U, 53U)
        {
            INST(74U, Opcode::Sub).i32().Inputs(66U, 58U);
        }

        BASIC_BLOCK(53U, 21U, 49U)
        {
            INST(65U, Opcode::Phi).i32().Inputs(66U, 74U);
            INST(75U, Opcode::Sub).i32().Inputs(65U, 58U);
            INST(62U, Opcode::Add).i32().Inputs(58U, 5U);
            INST(60U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(62U, 3U);
            INST(61U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(60U);
        }

        BASIC_BLOCK(71U, 15U, 57U)
        {
            INST(101U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(38U);
        }

        BASIC_BLOCK(57U, 21U, 66U)
        {
            INST(79U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(80U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(79U);
        }

        BASIC_BLOCK(66U, 21U, 66U)
        {
            INST(81U, Opcode::Phi).i32().Inputs(4U, 85U);
            INST(82U, Opcode::Phi).i32().Inputs(5U, 97U);
            INST(96U, Opcode::Add).i32().Inputs(82U, 81U);
            INST(89U, Opcode::Add).i32().Inputs(96U, 37U);
            INST(97U, Opcode::Sub).i32().Inputs(89U, 81U);
            INST(85U, Opcode::Add).i32().Inputs(81U, 5U);
            INST(83U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(85U, 3U);
            INST(84U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(83U);
        }

        BASIC_BLOCK(15U, 21U, 4U)
        {
            INST(50U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4U, 3U);
            INST(51U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(50U);
        }

        BASIC_BLOCK(4U, 21U, 4U)
        {
            INST(16U, Opcode::Phi).i32().Inputs(4U, 48U);
            INST(17U, Opcode::Phi).i32().Inputs(5U, 36U);
            INST(28U, Opcode::Add).i32().Inputs(16U, 17U);
            INST(33U, Opcode::Add).i32().Inputs(28U, 16U);
            INST(36U, Opcode::Add).i32().Inputs(33U, 37U);
            INST(48U, Opcode::Add).i32().Inputs(16U, 5U);
            INST(24U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48U, 3U);
            INST(25U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(21U, -1L)
        {
            INST(77U, Opcode::Phi).i32().Inputs({{15U, 5U}, {42U, 5U}, {53U, 75U}, {57U, 5U}, {4U, 36U}, {66U, 97U}});
            INST(49U, Opcode::Return).i32().Inputs(77U);
        }
    }
    return graph;
}

TEST_F(LoopUnswitchTest, TestMultipleConditionsLimitInstructions)
{
    BuildGraphMultipleConditionsLimitInstructions();
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2U, 40U));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = BuildExpectedMultipleConditionsLimitInstructions();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopInc0)
{
    CreateIncLoopGraph(0U);
    ASSERT_FALSE(GetGraph()->RunPass<LoopUnswitch>(2U, 100U));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopInc1)
{
    CreateIncLoopGraph(1U);
    ASSERT_FALSE(GetGraph()->RunPass<LoopUnswitch>(2U, 100U));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopInc2)
{
    CreateIncLoopGraph(2U);
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2U, 100U));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopDec0)
{
    CreateDecLoopGraph(0U);
    ASSERT_FALSE(GetGraph()->RunPass<LoopUnswitch>(2U, 100U));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopDec1)
{
    CreateDecLoopGraph(1U);
    ASSERT_FALSE(GetGraph()->RunPass<LoopUnswitch>(2U, 100U));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopDec2)
{
    CreateDecLoopGraph(2U);
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2U, 100U));
}

class ConditionComparatorTest : public GraphTest {
public:
    ConditionComparatorTest()
    {
        GetGraph()->CreateStartBlock();
        firstInput = GetGraph()->FindOrCreateConstant(10U);
        secondInput = GetGraph()->FindOrCreateConstant(20U);
        thirdInput = GetGraph()->FindOrCreateConstant(30U);
    }
    Inst *CreateInstIfImm(Inst *input, uint64_t imm, ConditionCode cc)
    {
        auto inst = GetGraph()->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, input, imm, input->GetType(), cc);
        return inst;
    }
    Inst *CreateInstIfImm(Inst *input0, Inst *input1, ConditionCode compareCc, uint64_t imm, ConditionCode ifCc)
    {
        auto compareInst =
            GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, input0, input1, input0->GetType(), compareCc);
        auto inst =
            GetGraph()->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, compareInst, imm, compareInst->GetType(), ifCc);
        return inst;
    }

    Inst *firstInput;   // NOLINT(misc-non-private-member-variables-in-classes)
    Inst *secondInput;  // NOLINT(misc-non-private-member-variables-in-classes)
    Inst *thirdInput;   // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ConditionComparatorTest, TestNoCompareInput)
{
    {
        auto ifImm0 = CreateInstIfImm(firstInput, 10U, ConditionCode::CC_EQ);
        auto ifImm1 = CreateInstIfImm(firstInput, 10U, ConditionCode::CC_EQ);
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, true));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, 1U, ConditionCode::CC_EQ);
        auto ifImm1 = CreateInstIfImm(secondInput, 1U, ConditionCode::CC_EQ);
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, true));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, 1U, ConditionCode::CC_EQ);
        auto ifImm1 = CreateInstIfImm(firstInput, 1U, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(ifImm0, ifImm1, false));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, true));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, 0U, ConditionCode::CC_EQ);
        auto ifImm1 = CreateInstIfImm(firstInput, 1U, ConditionCode::CC_EQ);
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
        ASSERT_TRUE(IsConditionEqual(ifImm0, ifImm1, true));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, 1U, ConditionCode::CC_NE);
        auto ifImm1 = CreateInstIfImm(firstInput, 1U, ConditionCode::CC_EQ);
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
        ASSERT_TRUE(IsConditionEqual(ifImm0, ifImm1, true));
    }
}

TEST_F(ConditionComparatorTest, TestCompareInput)
{
    {
        auto ifImm0 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 10U, ConditionCode::CC_NE);
        auto ifImm1 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 10U, ConditionCode::CC_NE);
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, true));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 1U, ConditionCode::CC_NE);
        auto ifImm1 = CreateInstIfImm(firstInput, thirdInput, ConditionCode::CC_LT, 1U, ConditionCode::CC_NE);
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, true));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 0U, ConditionCode::CC_NE);
        auto ifImm1 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 0U, ConditionCode::CC_NE);
        ASSERT_TRUE(IsConditionEqual(ifImm0, ifImm1, false));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, true));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 0U, ConditionCode::CC_NE);
        auto ifImm1 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_GE, 0U, ConditionCode::CC_NE);
        ASSERT_TRUE(IsConditionEqual(ifImm0, ifImm1, true));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 0U, ConditionCode::CC_NE);
        auto ifImm1 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 0U, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(ifImm0, ifImm1, true));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_GE, 0U, ConditionCode::CC_NE);
        auto ifImm1 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 0U, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(ifImm0, ifImm1, false));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, true));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_GE, 0U, ConditionCode::CC_NE);
        auto ifImm1 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_LT, 1U, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(ifImm0, ifImm1, true));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
    }
    {
        auto ifImm0 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_GE, 0U, ConditionCode::CC_EQ);
        auto ifImm1 = CreateInstIfImm(firstInput, secondInput, ConditionCode::CC_GE, 1U, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(ifImm0, ifImm1, true));
        ASSERT_FALSE(IsConditionEqual(ifImm0, ifImm1, false));
    }
}
// NOLINTEND(readability-magic-numbers,readability-function-size)
}  // namespace ark::compiler
