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
#include "optimizer/optimizations/licm_conditions.h"

namespace ark::compiler {
class LicmConditionsTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers,readability-function-size)

/// Graph is similar to TestUpdatePhiCorrectPredOrder but 14p prevents hoisting
TEST_F(LicmConditionsTest, TestPhiPreventsConditionHoisting)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(10U, 3U, 4U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(4U, 17U);
            INST(6U, Opcode::Phi).i64().Inputs(4U, 15U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(4U, 24U, 27U)
        {
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(28U, 24U)
        {
            // anything
        }

        BASIC_BLOCK(27U, 28U, 24U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(24U, 10U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(4U, 18U, 5U);
            INST(15U, Opcode::Add).i64().Inputs(6U, 5U);
            INST(17U, Opcode::Add).i64().Inputs(14U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).i64().Inputs(18U);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<LicmConditions>());
}

TEST_F(LicmConditionsTest, TestConditionIsNotHoistable)
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
        BASIC_BLOCK(6U, 5U, 7U)
        {
            INST(16U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 2U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(16U);
        }
        BASIC_BLOCK(5U, 8U)
        {
            INST(9U, Opcode::Add).i64().Inputs(5U, 6U);
        }
        BASIC_BLOCK(7U, 8U)
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
    ASSERT_FALSE(GetGraph()->RunPass<LicmConditions>());
}

/*
 * Test graph:
 * - Loop contains condition combination BB4 and BB6.
 * - Branches BB5 and BB7 do not contain phis.
 *     /-----[2]<----------\
 *     |      |            |
 *     |      v            |
 *     |     [4]------\    |
 *     |      |       |    |
 *     |      |       v    |
 *     |      |<-----[6]   |
 *     |      |       |    |
 *     |      v       v    |
 *     |     [5]     [7]   |
 *     |      |       |    |
 *     |      \->[8]<-/    |
 *     |          |        |
 *     |          \--------/
 *     |
 *     \----->[3]
 */
SRC_GRAPH(TestBrachWithoutPhi, Graph *graph)
{
    GRAPH(graph)
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
        BASIC_BLOCK(6U, 5U, 7U)
        {
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(5U, 8U)
        {
            INST(9U, Opcode::Add).i64().Inputs(5U, 6U);
        }
        BASIC_BLOCK(7U, 8U)
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

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestBrachWithoutPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(2U, 100U).i64();
        CONSTANT(3U, 0U).i64();
        CONSTANT(4U, 1U).i64();

        BASIC_BLOCK(14U, 4U) {}

        BASIC_BLOCK(4U, 16U, 6U)
        {
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(6U, 16U, 17U)
        {
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(17U, 16U) {}

        BASIC_BLOCK(16U, 2U)
        {
            INST(16U, Opcode::Phi).b().Inputs(4U, 4U, 3U);
        }

        BASIC_BLOCK(2U, 3U, 15U)
        {
            INST(5U, Opcode::Phi).i64().Inputs(3U, 11U);
            INST(6U, Opcode::Phi).i64().Inputs(3U, 12U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 2U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(15U, 5U, 7U)
        {
            INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(16U);
        }

        BASIC_BLOCK(5U, 8U)
        {
            INST(9U, Opcode::Add).i64().Inputs(5U, 6U);
        }

        BASIC_BLOCK(7U, 8U)
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

TEST_F(LicmConditionsTest, TestBrachWithoutPhi)
{
    src_graph::TestBrachWithoutPhi::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    out_graph::TestBrachWithoutPhi::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LicmConditionsTest, TestExtraInstPreventsHoisting)
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
        BASIC_BLOCK(6U, 5U, 7U)
        {
            INST(16U, Opcode::SaveState);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(5U, 8U)
        {
            INST(9U, Opcode::Add).i64().Inputs(5U, 6U);
        }
        BASIC_BLOCK(7U, 8U)
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
    ASSERT_FALSE(GetGraph()->RunPass<LicmConditions>());
}

/*
 * Test graph:
 * - Loop contains condition combination BB12, BB14 and BB15.
 * - Longest chain should be processed
 */
SRC_GRAPH(TestProcessLongestChain, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).b();
        PARAMETER(3U, 3U).ptr();
        CONSTANT(4U, 100U).i64();
        CONSTANT(5U, 0U).i64();
        CONSTANT(6U, 1U).i64();

        BASIC_BLOCK(20U, 3U, 12U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(5U, 17U);
            INST(7U, Opcode::Phi).i64().Inputs(5U, 15U);
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7U, 4U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(12U, 13U, 14U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(14U, 13U, 15U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(15U, 16U, 13U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(13U, 17U)
        {
            INST(20U, Opcode::Add).i64().Inputs(18U, 6U);
        }

        BASIC_BLOCK(17U, 16U)
        {
            // anything
        }

        BASIC_BLOCK(16U, 20U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(18U, 20U);
            INST(15U, Opcode::Add).i64().Inputs(7U, 6U);
            INST(17U, Opcode::Add).i64().Inputs(14U, 6U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestProcessLongestChain, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).b();
        PARAMETER(3U, 3U).ptr();
        CONSTANT(4U, 100U).i64();
        CONSTANT(5U, 0U).i64();
        CONSTANT(6U, 1U).i64();

        BASIC_BLOCK(42U, 12U) {}

        BASIC_BLOCK(12U, 44U, 14U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(14U, 44U, 15U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(15U, 45U, 44U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(45U, 44U)
        {
            // empty
        }

        BASIC_BLOCK(44U, 20U)
        {
            INST(21U, Opcode::Phi).b().Inputs(6U, 6U, 6U, 5U);
        }

        BASIC_BLOCK(20U, 3U, 43U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(5U, 17U);
            INST(7U, Opcode::Phi).i64().Inputs(5U, 15U);
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7U, 4U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(43U, 13U, 16U)
        {
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }

        BASIC_BLOCK(13U, 17U)
        {
            INST(20U, Opcode::Add).i64().Inputs(18U, 6U);
        }

        BASIC_BLOCK(17U, 16U)
        {
            // anything
        }

        BASIC_BLOCK(16U, 20U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(18U, 20U);
            INST(15U, Opcode::Add).i64().Inputs(7U, 6U);
            INST(17U, Opcode::Add).i64().Inputs(14U, 6U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

TEST_F(LicmConditionsTest, TestProcessLongestChain)
{
    src_graph::TestProcessLongestChain::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    out_graph::TestProcessLongestChain::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB12, BB14 and BB15.
 * - Longest chain should be processed but it has Phi with different inputs in multiple predecessors successor
 */
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
SRC_GRAPH(TestProcessLongestChainNotSuitable, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).b();
        PARAMETER(3U, 3U).ptr();
        CONSTANT(4U, 100U).i64();
        CONSTANT(5U, 0U).i64();
        CONSTANT(6U, 1U).i64();

        BASIC_BLOCK(20U, 3U, 12U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(5U, 17U);
            INST(7U, Opcode::Phi).i64().Inputs(5U, 15U);
            INST(23U, Opcode::Load).i64().Inputs(3U, 5U);
            INST(24U, Opcode::Load).i64().Inputs(3U, 6U);
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7U, 4U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(12U, 13U, 14U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(14U, 13U, 15U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(15U, 16U, 13U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(13U, 17U)
        {
            INST(21U, Opcode::Phi).i64().Inputs(23U, 24U, 24U);
            INST(20U, Opcode::Add).i64().Inputs(18U, 6U);
            INST(22U, Opcode::Add).i64().Inputs(20U, 21U);
        }

        BASIC_BLOCK(17U, 16U)
        {
            // anything
        }

        BASIC_BLOCK(16U, 20U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(18U, 22U);
            INST(15U, Opcode::Add).i64().Inputs(7U, 6U);
            INST(17U, Opcode::Add).i64().Inputs(14U, 6U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestProcessLongestChainNotSuitable, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).b();
        PARAMETER(3U, 3U).ptr();
        CONSTANT(4U, 100U).i64();
        CONSTANT(5U, 0U).i64();
        CONSTANT(6U, 1U).i64();

        BASIC_BLOCK(42U, 14U) {}

        BASIC_BLOCK(14U, 44U, 15U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(15U, 45U, 44U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(45U, 44U)
        {
            // empty
        }

        BASIC_BLOCK(44U, 20U)
        {
            INST(25U, Opcode::Phi).b().Inputs(6U, 6U, 5U);
        }

        BASIC_BLOCK(20U, 3U, 12U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(5U, 17U);
            INST(7U, Opcode::Phi).i64().Inputs(5U, 15U);
            INST(23U, Opcode::Load).i64().Inputs(3U, 5U);
            INST(24U, Opcode::Load).i64().Inputs(3U, 6U);
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7U, 4U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(12U, 13U, 43U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(43U, 13U, 16U)
        {
            INST(26U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(25U);
        }

        BASIC_BLOCK(13U, 17U)
        {
            INST(21U, Opcode::Phi).i64().Inputs(23U, 24U);
            INST(20U, Opcode::Add).i64().Inputs(18U, 6U);
            INST(22U, Opcode::Add).i64().Inputs(20U, 21U);
        }

        BASIC_BLOCK(17U, 16U)
        {
            // anything
        }

        BASIC_BLOCK(16U, 20U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(18U, 22U);
            INST(15U, Opcode::Add).i64().Inputs(7U, 6U);
            INST(17U, Opcode::Add).i64().Inputs(14U, 6U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

TEST_F(LicmConditionsTest, TestProcessLongestChainNotSuitable)
{
    src_graph::TestProcessLongestChainNotSuitable::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    out_graph::TestProcessLongestChainNotSuitable::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB2 and BB29.
 * - Block BB2 should be splitted
 * - Branch BB26 contains phi which requires input update
 *     /-----[6]<---------------\
 *     |      |                 |
 *     |      v                 |
 *     |     [2]----------\     |
 *     |      |           |     |
 *     |      |           v     |
 *     |      |<---------[29]   |
 *     |      |           |     |
 *     |      |           v     |
 *     |      |      /---[30]   |
 *     |      |      |     |    |
 *     |      |      v     v    |
 *     |      |    [32]   [31]  |
 *     |      |      |     |    |
 *     |      |------/-----/    |
 *     |      |                 |
 *     |      v                 |
 *     |     [26]               |
 *     |      |                 |
 *     |      \-----------------/
 *     |
 *     \----->[3]
 */
SRC_GRAPH(TestUpdatePhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(6U, 3U, 2U)
        {
            INST(14U, Opcode::Phi).i32().Inputs(4U, 15U);
            INST(6U, Opcode::Phi).i64().Inputs(4U, 13U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(2U, 26U, 29U)
        {
            INST(9U, Opcode::Load).b().Inputs(2U, 4U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(29U, 30U, 26U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(30U, 31U, 32U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }

        BASIC_BLOCK(31U, 26U)
        {
            // anything
        }

        BASIC_BLOCK(32U, 26U)
        {
            // anything
        }

        BASIC_BLOCK(26U, 6U)
        {
            INST(17U, Opcode::Phi).i32().Inputs(14U, 14U, 9U, 9U);
            INST(15U, Opcode::Add).i32().Inputs(17U, 5U);
            INST(13U, Opcode::Add).i64().Inputs(6U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(16U, Opcode::Return).i32().Inputs(14U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestUpdatePhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(60U, 61U) {}

        BASIC_BLOCK(61U, 63U, 29U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(29U, 64U, 63U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(64U, 63U)
        {
            // empty
        }

        BASIC_BLOCK(63U, 6U)
        {
            INST(18U, Opcode::Phi).b().Inputs(5U, 5U, 4U);
        }

        BASIC_BLOCK(6U, 3U, 2U)
        {
            INST(14U, Opcode::Phi).i32().Inputs(4U, 15U);
            INST(6U, Opcode::Phi).i64().Inputs(4U, 13U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(2U, 62U)
        {
            INST(9U, Opcode::Load).b().Inputs(2U, 4U);
        }

        BASIC_BLOCK(62U, 26U, 30U)
        {
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
        }

        BASIC_BLOCK(30U, 31U, 32U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }

        BASIC_BLOCK(31U, 26U)
        {
            // anything
        }

        BASIC_BLOCK(32U, 26U)
        {
            // anything
        }

        BASIC_BLOCK(26U, 6U)
        {
            INST(17U, Opcode::Phi).i32().Inputs(14U, 9U, 9U);
            INST(15U, Opcode::Add).i32().Inputs(17U, 5U);
            INST(13U, Opcode::Add).i64().Inputs(6U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(16U, Opcode::Return).i32().Inputs(14U);
        }
    }
}

TEST_F(LicmConditionsTest, TestUpdatePhi)
{
    src_graph::TestUpdatePhi::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    out_graph::TestUpdatePhi::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB50 and BB52.
 * - Branch BB51 contains phi which is hoisted
 */
SRC_GRAPH(TestHoistPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(53U, 3U, 2U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(4U, 17U);
            INST(6U, Opcode::Phi).i64().Inputs(4U, 15U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(2U, 46U, 50U)
        {
            INST(9U, Opcode::Load).b().Inputs(2U, 4U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }

        BASIC_BLOCK(50U, 51U, 52U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(52U, 46U, 51U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(51U, 46U)
        {
            INST(13U, Opcode::Phi).i64().Inputs(5U, 4U);
        }

        BASIC_BLOCK(46U, 53U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(4U, 5U, 13U);
            INST(15U, Opcode::Add).i64().Inputs(6U, 5U);
            INST(17U, Opcode::Add).i64().Inputs(14U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(16U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestHoistPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(108U, 50U) {}

        BASIC_BLOCK(50U, 110U, 52U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(52U, 111U, 110U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(111U, 110U)
        {
            // empty
        }

        BASIC_BLOCK(110U, 53U)
        {
            INST(13U, Opcode::Phi).i64().Inputs(5U, 4U, 4U);
            INST(19U, Opcode::Phi).b().Inputs(5U, 5U, 4U);
        }

        BASIC_BLOCK(53U, 3U, 2U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(4U, 17U);
            INST(6U, Opcode::Phi).i64().Inputs(4U, 15U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(2U, 46U, 109U)
        {
            INST(9U, Opcode::Load).b().Inputs(2U, 4U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }

        BASIC_BLOCK(109U, 51U, 46U)
        {
            INST(20U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(19U);
        }

        BASIC_BLOCK(51U, 46U) {}

        BASIC_BLOCK(46U, 53U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(4U, 5U, 13U);
            INST(15U, Opcode::Add).i64().Inputs(6U, 5U);
            INST(17U, Opcode::Add).i64().Inputs(14U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(16U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

TEST_F(LicmConditionsTest, TestHoistPhi)
{
    src_graph::TestHoistPhi::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    out_graph::TestHoistPhi::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB50 and BB52.
 * - Branch BB51 contains phi 13 which cannot be hoist (input is not from chain)
 *   but we can update inputs
 */
SRC_GRAPH(TestCannotHoistPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(54U, 53U)
        {
            INST(20U, Opcode::Load).b().Inputs(2U, 5U);
        }

        BASIC_BLOCK(53U, 3U, 2U)
        {
            INST(18U, Opcode::Phi).i32().Inputs(4U, 17U);
            INST(6U, Opcode::Phi).i32().Inputs(4U, 15U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(6U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(2U, 51U, 50U)
        {
            INST(9U, Opcode::Load).b().Inputs(2U, 4U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }

        BASIC_BLOCK(50U, 51U, 52U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(52U, 46U, 51U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(51U, 46U)
        {
            INST(13U, Opcode::Phi).i32().Inputs(20U, 5U, 5U);
        }

        BASIC_BLOCK(46U, 53U)
        {
            INST(14U, Opcode::Phi).i32().Inputs(5U, 13U);
            INST(15U, Opcode::Add).i32().Inputs(6U, 5U);
            INST(17U, Opcode::Add).i32().Inputs(14U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(16U, Opcode::Return).i32().Inputs(18U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestCannotHoistPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(54U, 50U)
        {
            INST(20U, Opcode::Load).b().Inputs(2U, 5U);
        }

        BASIC_BLOCK(50U, 111U, 52U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(52U, 112U, 111U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(112U, 111U)
        {
            // empty
        }

        BASIC_BLOCK(111U, 53U)
        {
            INST(21U, Opcode::Phi).b().Inputs(5U, 5U, 4U);
        }

        BASIC_BLOCK(53U, 3U, 2U)
        {
            INST(18U, Opcode::Phi).i32().Inputs(4U, 17U);
            INST(6U, Opcode::Phi).i32().Inputs(4U, 15U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(6U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(2U, 51U, 110U)
        {
            INST(9U, Opcode::Load).b().Inputs(2U, 4U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }

        BASIC_BLOCK(110U, 51U, 46U)
        {
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }

        BASIC_BLOCK(51U, 46U)
        {
            INST(13U, Opcode::Phi).i32().Inputs(20U, 5U);
        }

        BASIC_BLOCK(46U, 53U)
        {
            INST(14U, Opcode::Phi).i32().Inputs(5U, 13U);
            INST(15U, Opcode::Add).i32().Inputs(6U, 5U);
            INST(17U, Opcode::Add).i32().Inputs(14U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(16U, Opcode::Return).i32().Inputs(18U);
        }
    }
}

TEST_F(LicmConditionsTest, TestCannotHoistPhi)
{
    src_graph::TestCannotHoistPhi::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    out_graph::TestCannotHoistPhi::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB4 and BB27.
 * - Branch BB24 contains phi which requires new edge in correct predecessors order
 */
SRC_GRAPH(TestHoistPhiCorrectPredOrder, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(10U, 3U, 4U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(4U, 17U);
            INST(6U, Opcode::Phi).i64().Inputs(4U, 15U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(28U, 24U)
        {
            // anything
        }

        BASIC_BLOCK(4U, 24U, 27U)
        {
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(27U, 28U, 24U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(24U, 10U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(3U, 4U, 5U);
            INST(15U, Opcode::Add).i64().Inputs(6U, 5U);
            INST(17U, Opcode::Add).i64().Inputs(14U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestHoistPhiCorrectPredOrder, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).ptr();
        CONSTANT(3U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(58U, 4U) {}

        BASIC_BLOCK(4U, 60U, 27U)
        {
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(61U, 60U)
        {
            // empty
        }

        BASIC_BLOCK(27U, 61U, 60U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(60U, 10U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(4U, 3U, 5U);
            INST(20U, Opcode::Phi).b().Inputs(5U, 4U, 5U);
        }

        BASIC_BLOCK(10U, 3U, 59U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(4U, 17U);
            INST(6U, Opcode::Phi).i64().Inputs(4U, 15U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(59U, 24U, 28U)
        {
            INST(21U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(20U);
        }

        BASIC_BLOCK(28U, 24U)
        {
            // anything
        }

        BASIC_BLOCK(24U, 10U)
        {
            INST(15U, Opcode::Add).i64().Inputs(6U, 5U);
            INST(17U, Opcode::Add).i64().Inputs(14U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

TEST_F(LicmConditionsTest, TestHoistPhiCorrectPredOrder)
{
    src_graph::TestHoistPhiCorrectPredOrder::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    out_graph::TestHoistPhiCorrectPredOrder::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - two chains with neighbour blocks
 */
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
SRC_GRAPH(TestProcessNeigbourChains, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();
        PARAMETER(3U, 3U).i32();
        CONSTANT(20U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(4U, 16U);
            INST(6U, Opcode::Phi).i64().Inputs(4U, 17U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 20U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(9U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, 6U, 9U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(11U).Inputs(1U);
        }

        BASIC_BLOCK(6U, 7U, 8U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(12U).Inputs(2U);
        }

        BASIC_BLOCK(7U, 8U, 9U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(13U).Inputs(3U);
        }

        BASIC_BLOCK(8U, 10U)
        {
            INST(13U, Opcode::Sub).i64().Inputs(18U, 5U);
        }

        BASIC_BLOCK(9U, 10U)
        {
            INST(14U, Opcode::Add).i64().Inputs(18U, 5U);
        }

        BASIC_BLOCK(10U, 2U)
        {
            INST(15U, Opcode::Phi).i64().Inputs(13U, 14U);
            INST(16U, Opcode::Add).i64().Inputs(15U, 5U);
            INST(17U, Opcode::Add).i64().Inputs(6U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestProcessNeigbourChains, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();
        PARAMETER(3U, 3U).i32();
        CONSTANT(20U, 100U).i64();
        CONSTANT(4U, 0U).i64();
        CONSTANT(5U, 1U).i64();

        BASIC_BLOCK(14U, 6U) {}

        BASIC_BLOCK(6U, 7U, 16U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(12U).Inputs(2U);
        }

        BASIC_BLOCK(7U, 16U, 17U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(13U).Inputs(3U);
        }

        BASIC_BLOCK(17U, 16U)
        {
            // empty
        }

        BASIC_BLOCK(16U, 4U)
        {
            INST(21U, Opcode::Phi).b().Inputs(5U, 5U, 4U);
        }

        BASIC_BLOCK(4U, 19U, 5U)
        {
            INST(9U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, 19U, 20U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(11U).Inputs(1U);
        }

        BASIC_BLOCK(20U, 19U)
        {
            // empty
        }

        BASIC_BLOCK(19U, 2U)
        {
            INST(23U, Opcode::Phi).b().Inputs(5U, 5U, 4U);
        }

        BASIC_BLOCK(2U, 3U, 18U)
        {
            INST(18U, Opcode::Phi).i64().Inputs(4U, 16U);
            INST(6U, Opcode::Phi).i64().Inputs(4U, 17U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6U, 20U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(18U, 15U, 9U)
        {
            INST(24U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(23U);
        }

        BASIC_BLOCK(15U, 8U, 9U)
        {
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }

        BASIC_BLOCK(8U, 10U)
        {
            INST(13U, Opcode::Sub).i64().Inputs(18U, 5U);
        }

        BASIC_BLOCK(9U, 10U)
        {
            INST(14U, Opcode::Add).i64().Inputs(18U, 5U);
        }

        BASIC_BLOCK(10U, 2U)
        {
            INST(15U, Opcode::Phi).i64().Inputs(13U, 14U);
            INST(16U, Opcode::Add).i64().Inputs(15U, 5U);
            INST(17U, Opcode::Add).i64().Inputs(6U, 5U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(19U, Opcode::Return).i64().Inputs(18U);
        }
    }
}

TEST_F(LicmConditionsTest, TestProcessNeigbourChains)
{
    src_graph::TestProcessNeigbourChains::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    out_graph::TestProcessNeigbourChains::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - two chains with common successors
 */
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
SRC_GRAPH(TestProcessChainsWithCommonSuccessors, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).b();
        PARAMETER(3U, 3U).ptr();
        CONSTANT(4U, 100U).i64();
        CONSTANT(5U, 0U).i64();
        CONSTANT(6U, 1U).i64();

        BASIC_BLOCK(4U, 3U, 5U)
        {
            INST(19U, Opcode::Phi).i64().Inputs(5U, 18U);
            INST(7U, Opcode::Phi).i64().Inputs(5U, 17U);
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7U, 4U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(5U, 6U, 10U)
        {
            INST(23U, Opcode::Load).i64().Inputs(3U, 5U);
            INST(24U, Opcode::Load).i64().Inputs(3U, 6U);
            INST(25U, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_NE).Imm(0U).Inputs(23U);
        }
        BASIC_BLOCK(6U, 8U, 7U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(7U, 8U, 9U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(10U, 8U, 11U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(11U, 8U, 9U)
        {
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(8U, 12U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(23U, 23U, 24U, 24U);
        }
        BASIC_BLOCK(9U, 12U)
        {
            INST(15U, Opcode::Phi).i64().Inputs(24U, 23U);
        }
        BASIC_BLOCK(12U, 4U)
        {
            INST(16U, Opcode::Phi).i64().Inputs(14U, 15U);
            INST(17U, Opcode::Add).i64().Inputs(7U, 6U);
            INST(18U, Opcode::Add).i64().Inputs(16U, 6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(20U, Opcode::Return).i64().Inputs(19U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestProcessChainsWithCommonSuccessors, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).b();
        PARAMETER(3U, 3U).ptr();
        CONSTANT(4U, 100U).i64();
        CONSTANT(5U, 0U).i64();
        CONSTANT(6U, 1U).i64();

        BASIC_BLOCK(22U, 6U) {}
        BASIC_BLOCK(6U, 24U, 7U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(7U, 24U, 25U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(25U, 24U)
        {
            // empty
        }
        BASIC_BLOCK(24U, 10U)
        {
            INST(26U, Opcode::Phi).b().Inputs(6U, 6U, 5U);
        }
        BASIC_BLOCK(10U, 27U, 11U)
        {
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(11U, 27U, 28U)
        {
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(28U, 27U)
        {
            // empty
        }
        BASIC_BLOCK(27U, 4U)
        {
            INST(28U, Opcode::Phi).b().Inputs(6U, 6U, 5U);
        }
        BASIC_BLOCK(4U, 3U, 5U)
        {
            INST(19U, Opcode::Phi).i64().Inputs(5U, 18U);
            INST(7U, Opcode::Phi).i64().Inputs(5U, 17U);
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7U, 4U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(5U, 23U, 26U)
        {
            INST(23U, Opcode::Load).i64().Inputs(3U, 5U);
            INST(24U, Opcode::Load).i64().Inputs(3U, 6U);
            INST(25U, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_NE).Imm(0U).Inputs(23U);
        }
        BASIC_BLOCK(26U, 8U, 9U)
        {
            INST(29U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(28U);
        }
        BASIC_BLOCK(23U, 8U, 9U)
        {
            INST(27U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }
        BASIC_BLOCK(8U, 12U)
        {
            INST(14U, Opcode::Phi).i64().Inputs(24U, 23U);
        }
        BASIC_BLOCK(9U, 12U)
        {
            INST(15U, Opcode::Phi).i64().Inputs(23U, 24U);
        }
        BASIC_BLOCK(12U, 4U)
        {
            INST(16U, Opcode::Phi).i64().Inputs(14U, 15U);
            INST(17U, Opcode::Add).i64().Inputs(7U, 6U);
            INST(18U, Opcode::Add).i64().Inputs(16U, 6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(20U, Opcode::Return).i64().Inputs(19U);
        }
    }
}

TEST_F(LicmConditionsTest, TestProcessChainsWithCommonSuccessors)
{
    src_graph::TestProcessChainsWithCommonSuccessors::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    out_graph::TestProcessChainsWithCommonSuccessors::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - two chains can be merged
 */
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
SRC_GRAPH(TestMergeChains, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        CONSTANT(3U, 6U).i64();
        CONSTANT(4U, 5U).i64();
        CONSTANT(5U, 100U).i64();
        CONSTANT(6U, 0U).i64();
        CONSTANT(7U, 1U).i64();
        CONSTANT(34U, 2U).i64();

        BASIC_BLOCK(11U, 9U)
        {
            INST(37U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(1U, 3U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0U, 4U);
            INST(28U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(1U, 3U);
        }

        BASIC_BLOCK(9U, 10U, 2U)
        {
            INST(17U, Opcode::Phi).i32().Inputs(6U, 43U);
            INST(18U, Opcode::Phi).i32().Inputs(7U, 41U);
            INST(24U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17U, 5U);
            INST(25U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(27U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(4U, 3U, 5U)
        {
            INST(29U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(28U);
        }

        BASIC_BLOCK(5U, 3U)
        {
            INST(30U, Opcode::Add).i32().Inputs(17U, 18U);
        }

        BASIC_BLOCK(3U, 6U, 7U)
        {
            INST(31U, Opcode::Phi).i32().Inputs(18U, 18U, 30U);
            INST(33U, Opcode::Add).i32().Inputs(31U, 34U);
            INST(36U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(7U, 8U, 6U)
        {
            INST(38U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(37U);
        }

        BASIC_BLOCK(6U, 8U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(33U, 17U);
        }

        BASIC_BLOCK(8U, 9U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(33U, 40U);
            INST(43U, Opcode::Add).i32().Inputs(17U, 7U);
        }

        BASIC_BLOCK(10U, -1L)
        {
            INST(20U, Opcode::Return).i32().Inputs(18U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestMergeChains, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        CONSTANT(3U, 6U).i64();
        CONSTANT(4U, 5U).i64();
        CONSTANT(5U, 100U).i64();
        CONSTANT(6U, 0U).i64();
        CONSTANT(7U, 1U).i64();
        CONSTANT(34U, 2U).i64();

        BASIC_BLOCK(11U, 29U, 7U)
        {
            INST(37U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(1U, 3U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0U, 4U);
            INST(36U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(7U, 27U, 29U)
        {
            INST(38U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(37U);
        }

        BASIC_BLOCK(27U, 29U)
        {
            // empty
        }

        BASIC_BLOCK(29U, 9U)
        {
            INST(44U, Opcode::Phi).b().Inputs(7U, 7U, 6U);
        }

        BASIC_BLOCK(9U, 10U, 28U)
        {
            INST(17U, Opcode::Phi).i32().Inputs(6U, 43U);
            INST(18U, Opcode::Phi).i32().Inputs(7U, 41U);
            INST(24U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17U, 5U);
            INST(25U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(28U, 5U, 3U)
        {
            INST(47U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(44U);
        }

        BASIC_BLOCK(5U, 3U)
        {
            INST(30U, Opcode::Add).i32().Inputs(17U, 18U);
        }

        BASIC_BLOCK(3U, 6U, 8U)
        {
            INST(31U, Opcode::Phi).i32().Inputs(18U, 30U);
            INST(33U, Opcode::Add).i32().Inputs(31U, 34U);
            INST(45U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(44U);
        }

        BASIC_BLOCK(6U, 8U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(33U, 17U);
        }

        BASIC_BLOCK(8U, 9U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(33U, 40U);
            INST(43U, Opcode::Add).i32().Inputs(17U, 7U);
        }

        BASIC_BLOCK(10U, -1L)
        {
            INST(20U, Opcode::Return).i32().Inputs(18U);
        }
    }
}

TEST_F(LicmConditionsTest, TestMergeChains)
{
    src_graph::TestMergeChains::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    out_graph::TestMergeChains::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - two chains can be merged
 */
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
SRC_GRAPH(TestMergeChainsPhiHoisted, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        CONSTANT(3U, 6U).i64();
        CONSTANT(4U, 5U).i64();
        CONSTANT(5U, 100U).i64();
        CONSTANT(6U, 0U).i64();
        CONSTANT(7U, 1U).i64();
        CONSTANT(34U, 2U).i64();

        BASIC_BLOCK(11U, 9U)
        {
            INST(37U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(1U, 3U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0U, 4U);
            INST(28U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(1U, 3U);
        }

        BASIC_BLOCK(9U, 10U, 2U)
        {
            INST(17U, Opcode::Phi).i32().Inputs(6U, 43U);
            INST(18U, Opcode::Phi).i32().Inputs(7U, 41U);
            INST(24U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17U, 5U);
            INST(25U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(27U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(28U);
        }

        BASIC_BLOCK(4U, 3U, 5U)
        {
            INST(29U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(5U, 3U)
        {
            INST(30U, Opcode::Add).i32().Inputs(17U, 18U);
        }

        BASIC_BLOCK(3U, 6U, 7U)
        {
            INST(31U, Opcode::Phi).i32().Inputs(18U, 18U, 30U);
            INST(44U, Opcode::Phi).i32().Inputs(34U, 7U, 4U);
            INST(33U, Opcode::Add).i32().Inputs(31U, 44U);
            INST(36U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(7U, 8U, 6U)
        {
            INST(38U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(37U);
        }

        BASIC_BLOCK(6U, 8U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(33U, 17U);
        }

        BASIC_BLOCK(8U, 9U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(33U, 40U);
            INST(43U, Opcode::Add).i32().Inputs(17U, 7U);
        }

        BASIC_BLOCK(10U, -1L)
        {
            INST(20U, Opcode::Return).i32().Inputs(18U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestMergeChainsPhiHoisted, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        CONSTANT(3U, 6U).i64();
        CONSTANT(4U, 5U).i64();
        CONSTANT(5U, 100U).i64();
        CONSTANT(6U, 0U).i64();
        CONSTANT(7U, 1U).i64();
        CONSTANT(34U, 2U).i64();

        BASIC_BLOCK(11U, 2U, 7U)
        {
            INST(37U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(1U, 3U);
            INST(26U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0U, 4U);
            INST(28U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(1U, 3U);
            INST(36U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(7U, 27U, 2U)
        {
            INST(38U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(37U);
        }

        BASIC_BLOCK(27U, 2U)
        {
            // empty
        }

        BASIC_BLOCK(2U, 29U, 4U)
        {
            INST(45U, Opcode::Phi).b().Inputs(7U, 7U, 6U);
            INST(27U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(28U);
        }

        BASIC_BLOCK(4U, 29U, 30U)
        {
            INST(29U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(30U, 29U)
        {
            // empty
        }

        BASIC_BLOCK(29U, 9U)
        {
            INST(44U, Opcode::Phi).i32().Inputs(34U, 7U, 4U);
        }

        BASIC_BLOCK(9U, 10U, 28U)
        {
            INST(17U, Opcode::Phi).i32().Inputs(6U, 43U);
            INST(18U, Opcode::Phi).i32().Inputs(7U, 41U);
            INST(24U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17U, 5U);
            INST(25U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(28U, 5U, 3U)
        {
            INST(47U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(45U);
        }

        BASIC_BLOCK(5U, 3U)
        {
            INST(30U, Opcode::Add).i32().Inputs(17U, 18U);
        }

        BASIC_BLOCK(3U, 6U, 8U)
        {
            INST(31U, Opcode::Phi).i32().Inputs(18U, 30U);
            INST(33U, Opcode::Add).i32().Inputs(31U, 44U);
            INST(46U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(45U);
        }

        BASIC_BLOCK(6U, 8U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(33U, 17U);
        }

        BASIC_BLOCK(8U, 9U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(33U, 40U);
            INST(43U, Opcode::Add).i32().Inputs(17U, 7U);
        }

        BASIC_BLOCK(10U, -1L)
        {
            INST(20U, Opcode::Return).i32().Inputs(18U);
        }
    }
}

TEST_F(LicmConditionsTest, TestMergeChainsPhiHoisted)
{
    src_graph::TestMergeChainsPhiHoisted::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    out_graph::TestMergeChainsPhiHoisted::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - One of the chains is an extension of the other.
 */
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
SRC_GRAPH(TestNoMergeChains, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        CONSTANT(3U, 6U).i64();
        CONSTANT(4U, 5U).i64();
        CONSTANT(5U, 100U).i64();
        CONSTANT(6U, 0U).i64();
        CONSTANT(7U, 1U).i64();
        CONSTANT(34U, 2U).i64();

        BASIC_BLOCK(11U, 9U)
        {
            INST(26U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0U, 4U);
            INST(28U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(1U, 3U);
        }

        BASIC_BLOCK(9U, 10U, 2U)
        {
            INST(17U, Opcode::Phi).i32().Inputs(6U, 43U);
            INST(18U, Opcode::Phi).i32().Inputs(7U, 41U);
            INST(24U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17U, 5U);
            INST(25U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(27U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(28U);
        }

        BASIC_BLOCK(4U, 3U, 5U)
        {
            INST(29U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(5U, 3U)
        {
            INST(30U, Opcode::Add).i32().Inputs(17U, 18U);
        }

        BASIC_BLOCK(3U, 6U, 7U)
        {
            INST(31U, Opcode::Phi).i32().Inputs(18U, 18U, 30U);
            INST(44U, Opcode::Phi).i32().Inputs(34U, 7U, 4U);
            INST(33U, Opcode::Add).i32().Inputs(31U, 44U);
            INST(36U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(7U, 6U, 8U)
        {
            INST(38U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(6U, 8U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(33U, 17U);
        }

        BASIC_BLOCK(8U, 9U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(33U, 40U);
            INST(43U, Opcode::Add).i32().Inputs(17U, 7U);
        }

        BASIC_BLOCK(10U, -1L)
        {
            INST(20U, Opcode::Return).i32().Inputs(18U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(TestNoMergeChains, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        CONSTANT(3U, 6U).i64();
        CONSTANT(4U, 5U).i64();
        CONSTANT(5U, 100U).i64();
        CONSTANT(6U, 0U).i64();
        CONSTANT(7U, 1U).i64();
        CONSTANT(34U, 2U).i64();

        BASIC_BLOCK(11U, 2U, 7U)
        {
            INST(26U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0U, 4U);
            INST(28U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(1U, 3U);
            INST(36U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(7U, 2U, 27U)
        {
            INST(38U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(27U, 2U)
        {
            // empty
        }

        BASIC_BLOCK(2U, 29U, 4U)
        {
            INST(45U, Opcode::Phi).b().Inputs(7U, 7U, 6U);
            INST(27U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(28U);
        }

        BASIC_BLOCK(4U, 29U, 30U)
        {
            INST(29U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(26U);
        }

        BASIC_BLOCK(30U, 29U)
        {
            // empty
        }

        BASIC_BLOCK(29U, 9U)
        {
            INST(44U, Opcode::Phi).i32().Inputs(34U, 7U, 4U);
            INST(47U, Opcode::Phi).b().Inputs(7U, 7U, 6U);
        }

        BASIC_BLOCK(9U, 10U, 28U)
        {
            INST(17U, Opcode::Phi).i32().Inputs(6U, 43U);
            INST(18U, Opcode::Phi).i32().Inputs(7U, 41U);
            INST(24U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17U, 5U);
            INST(25U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(24U);
        }

        BASIC_BLOCK(28U, 5U, 3U)
        {
            INST(48U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(47U);
        }

        BASIC_BLOCK(5U, 3U)
        {
            INST(30U, Opcode::Add).i32().Inputs(17U, 18U);
        }

        BASIC_BLOCK(3U, 6U, 8U)
        {
            INST(31U, Opcode::Phi).i32().Inputs(18U, 30U);
            INST(33U, Opcode::Add).i32().Inputs(31U, 44U);
            INST(46U, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0U).Inputs(45U);
        }

        BASIC_BLOCK(6U, 8U)
        {
            INST(40U, Opcode::Sub).i32().Inputs(33U, 17U);
        }

        BASIC_BLOCK(8U, 9U)
        {
            INST(41U, Opcode::Phi).i32().Inputs(33U, 40U);
            INST(43U, Opcode::Add).i32().Inputs(17U, 7U);
        }

        BASIC_BLOCK(10U, -1L)
        {
            INST(20U, Opcode::Return).i32().Inputs(18U);
        }
    }
}

TEST_F(LicmConditionsTest, TestNoMergeChains)
{
    src_graph::TestNoMergeChains::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    out_graph::TestNoMergeChains::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
// NOLINTEND(readability-magic-numbers,readability-function-size)
}  // namespace ark::compiler
