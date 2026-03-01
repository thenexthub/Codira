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
#include "optimizer/optimizations/move_constants.h"

namespace ark::compiler {

class MoveConstantsTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(MoveNullPtrToCommonImmediateDominator, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, nullptr);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(2U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::ReturnI).u64().Imm(0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(4U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Return).ref().Inputs(1U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(8U, Opcode::Return).ref().Inputs(1U);
        }
    }
}

OUT_GRAPH(MoveNullPtrToCommonImmediateDominator, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(2U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::ReturnI).u64().Imm(0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            CONSTANT(1U, nullptr);
            INST(4U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Return).ref().Inputs(1U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(8U, Opcode::Return).ref().Inputs(1U);
        }
    }
}

TEST_F(MoveConstantsTest, MoveNullPtrToCommonImmediateDominator)
{
    src_graph::MoveNullPtrToCommonImmediateDominator::CREATE(GetGraph());
    Graph *graphEt = CreateEmptyGraph();
    out_graph::MoveNullPtrToCommonImmediateDominator::CREATE(graphEt);

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(MoveToCommonImmediateDominator, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, 12345U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(2U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::ReturnI).u64().Imm(0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(4U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(5U, Opcode::Sub).u64().Inputs(1U, 0U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(7U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
}

OUT_GRAPH(MoveToCommonImmediateDominator, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(2U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::ReturnI).u64().Imm(0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            CONSTANT(1U, 12345U);
            INST(4U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(5U, Opcode::Sub).u64().Inputs(1U, 0U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(7U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
}

TEST_F(MoveConstantsTest, MoveToCommonImmediateDominator)
{
    src_graph::MoveToCommonImmediateDominator::CREATE(GetGraph());
    Graph *graphEt = CreateEmptyGraph();
    out_graph::MoveToCommonImmediateDominator::CREATE(graphEt);

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(MoveToClosestCommonDominator, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, 12345U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(4U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(30U, Opcode::ReturnI).u64().Imm(0U);
        }

        BASIC_BLOCK(4U, 8U, 5U)
        {
            INST(9U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, 7U, 6U)
        {
            INST(12U, Opcode::IfImm).CC(CC_LE).Imm(15U).Inputs(0U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(17U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(18U, Opcode::Return).u64().Inputs(17U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(23U, Opcode::Sub).u64().Inputs(1U, 0U);
            INST(24U, Opcode::Return).u64().Inputs(23U);
        }

        BASIC_BLOCK(8U, -1L)
        {
            INST(27U, Opcode::Div).u64().Inputs(1U, 0U);
            INST(28U, Opcode::Return).u64().Inputs(27U);
        }
    }
}

OUT_GRAPH(MoveToClosestCommonDominator, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(4U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(30U, Opcode::ReturnI).u64().Imm(0U);
        }

        BASIC_BLOCK(4U, 8U, 5U)
        {
            CONSTANT(1U, 12345U);
            INST(9U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, 7U, 6U)
        {
            INST(12U, Opcode::IfImm).CC(CC_LE).Imm(15U).Inputs(0U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(17U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(18U, Opcode::Return).u64().Inputs(17U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(23U, Opcode::Sub).u64().Inputs(1U, 0U);
            INST(24U, Opcode::Return).u64().Inputs(23U);
        }

        BASIC_BLOCK(8U, -1L)
        {
            INST(27U, Opcode::Div).u64().Inputs(1U, 0U);
            INST(28U, Opcode::Return).u64().Inputs(27U);
        }
    }
}

TEST_F(MoveConstantsTest, MoveToClosestCommonDominator)
{
    src_graph::MoveToClosestCommonDominator::CREATE(GetGraph());
    Graph *graphEt = CreateEmptyGraph();
    out_graph::MoveToClosestCommonDominator::CREATE(graphEt);

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(MoveJustBeforeUser, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, 12345U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(4U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::ReturnI).u64().Imm(0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(9U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::Return).u64().Inputs(0U);
        }
    }
}

OUT_GRAPH(MoveJustBeforeUser, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(4U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(13U, Opcode::ReturnI).u64().Imm(0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(9U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            CONSTANT(1U, 12345U);
            INST(10U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::Return).u64().Inputs(0U);
        }
    }
}

TEST_F(MoveConstantsTest, MoveJustBeforeUser)
{
    src_graph::MoveJustBeforeUser::CREATE(GetGraph());
    Graph *graphEt = CreateEmptyGraph();
    out_graph::MoveJustBeforeUser::CREATE(graphEt);

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(MoveJustBeforeUserSingleBlock, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, 12345U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(5U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::ReturnI).Imm(0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(9U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(12U, Opcode::Div).u64().Inputs(1U, 0U);
            INST(13U, Opcode::Add).u64().Inputs(12U, 1U);
            INST(14U, Opcode::Return).u64().Inputs(13U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(15U, Opcode::Return).u64().Inputs(0U);
        }
    }
}

OUT_GRAPH(MoveJustBeforeUserSingleBlock, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(5U, Opcode::IfImm).CC(CC_GE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::ReturnI).Imm(0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(9U, Opcode::IfImm).CC(CC_LE).Imm(10U).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            CONSTANT(1U, 12345U);
            INST(12U, Opcode::Div).u64().Inputs(1U, 0U);
            INST(13U, Opcode::Add).u64().Inputs(12U, 1U);
            INST(14U, Opcode::Return).u64().Inputs(13U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(15U, Opcode::Return).u64().Inputs(0U);
        }
    }
}

TEST_F(MoveConstantsTest, MoveJustBeforeUserSingleBlock)
{
    src_graph::MoveJustBeforeUserSingleBlock::CREATE(GetGraph());
    Graph *graphEt = CreateEmptyGraph();
    out_graph::MoveJustBeforeUserSingleBlock::CREATE(graphEt);

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(MovePhiInput, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(2U, 12345U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(15U, Opcode::IfImm).CC(CC_LE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 22U}});
            INST(5U, Opcode::Phi).u64().Inputs({{2U, 2U}, {3U, 23U}});
            INST(22U, Opcode::AddI).u64().Imm(1U).Inputs(4U);
            INST(23U, Opcode::AddI).u64().Imm(1U).Inputs(5U);
            INST(24U, Opcode::If).CC(CC_GE).Inputs(23U, 0U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::Phi).u64().Inputs({{3U, 22U}, {2U, 0U}});
            INST(13U, Opcode::Return).u64().Inputs(19U);
        }
    }
}

OUT_GRAPH(MovePhiInput, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();

        BASIC_BLOCK(2U, 4U, 3U)
        {
            CONSTANT(2U, 12345U);
            INST(15U, Opcode::IfImm).CC(CC_LE).Imm(5U).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(4U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 22U}});
            INST(5U, Opcode::Phi).u64().Inputs({{2U, 2U}, {3U, 23U}});
            INST(22U, Opcode::AddI).u64().Imm(1U).Inputs(4U);
            INST(23U, Opcode::AddI).u64().Imm(1U).Inputs(5U);
            INST(24U, Opcode::If).CC(CC_GE).Inputs(23U, 0U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::Phi).u64().Inputs({{3U, 22U}, {2U, 0U}});
            INST(13U, Opcode::Return).u64().Inputs(19U);
        }
    }
}

TEST_F(MoveConstantsTest, MovePhiInput)
{
    src_graph::MovePhiInput::CREATE(GetGraph());
    Graph *graphEt = CreateEmptyGraph();
    out_graph::MovePhiInput::CREATE(graphEt);

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(AvoidMoveToLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(2U, 5U);
        CONSTANT(11U, 0U);
        CONSTANT(22U, 3U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::NOP);
        }

        BASIC_BLOCK(3U, 7U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{2U, 2U}, {6U, 35U}});
            INST(4U, Opcode::Phi).u64().Inputs({{2U, 0U}, {6U, 34U}});
            INST(5U, Opcode::Phi).u64().Inputs({{2U, 2U}, {6U, 42U}});
            INST(39U, Opcode::If).CC(CC_GE).Inputs(5U, 0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(29U, Opcode::IfImm).CC(CC_LE).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(5U, 6U, 5U)
        {
            INST(14U, Opcode::Phi).u64().Inputs({{4U, 4U}, {5U, 23U}});
            INST(16U, Opcode::Phi).u64().Inputs({{4U, 11U}, {5U, 40U}});
            INST(23U, Opcode::Mul).u64().Inputs(14U, 22U);
            INST(40U, Opcode::AddI).u64().Imm(1U).Inputs(16U);
            INST(41U, Opcode::If).CC(CC_GE).Inputs(40U, 0U);
        }

        BASIC_BLOCK(6U, 3U)
        {
            INST(34U, Opcode::Phi).u64().Inputs({{5U, 23U}, {4U, 4U}});
            INST(35U, Opcode::Phi).u64().Inputs({{5U, 40U}, {4U, 11U}});
            INST(42U, Opcode::AddI).u64().Imm(1U).Inputs(5U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(27U, Opcode::Return).u64().Inputs(4U);
        }
    }
}

OUT_GRAPH(AvoidMoveToLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();

        BASIC_BLOCK(2U, 3U)
        {
            CONSTANT(22U, 3U);
            CONSTANT(11U, 0U);
            CONSTANT(2U, 5U);
            INST(1U, Opcode::NOP);
        }

        BASIC_BLOCK(3U, 7U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{2U, 2U}, {6U, 35U}});
            INST(4U, Opcode::Phi).u64().Inputs({{2U, 0U}, {6U, 34U}});
            INST(5U, Opcode::Phi).u64().Inputs({{2U, 2U}, {6U, 42U}});
            INST(39U, Opcode::If).CC(CC_GE).Inputs(5U, 0U);
        }

        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(29U, Opcode::IfImm).CC(CC_LE).Imm(0U).Inputs(0U);
        }

        BASIC_BLOCK(5U, 6U, 5U)
        {
            INST(14U, Opcode::Phi).u64().Inputs({{4U, 4U}, {5U, 23U}});
            INST(16U, Opcode::Phi).u64().Inputs({{4U, 11U}, {5U, 40U}});
            INST(23U, Opcode::Mul).u64().Inputs(14U, 22U);
            INST(40U, Opcode::AddI).u64().Imm(1U).Inputs(16U);
            INST(41U, Opcode::If).CC(CC_GE).Inputs(40U, 0U);
        }

        BASIC_BLOCK(6U, 3U)
        {
            INST(34U, Opcode::Phi).u64().Inputs({{5U, 23U}, {4U, 4U}});
            INST(35U, Opcode::Phi).u64().Inputs({{5U, 40U}, {4U, 11U}});
            INST(42U, Opcode::AddI).u64().Imm(1U).Inputs(5U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(27U, Opcode::Return).u64().Inputs(4U);
        }
    }
}

TEST_F(MoveConstantsTest, AvoidMoveToLoop)
{
    src_graph::AvoidMoveToLoop::CREATE(GetGraph());
    Graph *graphEt = CreateEmptyGraph();
    out_graph::AvoidMoveToLoop::CREATE(graphEt);

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(MoveToClosestCommonDominator2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).u64();
        CONSTANT(4U, 12345U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::If).CC(CC_LE).Inputs(0U, 4U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::Mul).u64().Inputs(0U, 4U);
            INST(5U, Opcode::Return).u64().Inputs(3U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Mul).u64().Inputs(1U, 4U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
}

OUT_GRAPH(MoveToClosestCommonDominator2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            CONSTANT(4U, 12345U);
            INST(2U, Opcode::If).CC(CC_LE).Inputs(0U, 4U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::Mul).u64().Inputs(0U, 4U);
            INST(5U, Opcode::Return).u64().Inputs(3U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Mul).u64().Inputs(1U, 4U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
}

TEST_F(MoveConstantsTest, MoveToClosestCommonDominator2)
{
    src_graph::MoveToClosestCommonDominator2::CREATE(GetGraph());
    Graph *graphEt = CreateEmptyGraph();
    out_graph::MoveToClosestCommonDominator2::CREATE(graphEt);

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(MoveToClosestCommonDominatorPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).u64();
        CONSTANT(5U, 12345U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::If).CC(CC_LE).Inputs(0U, 1U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(3U, Opcode::Mul).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(4U, Opcode::Div).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).u64().Inputs({{3U, 3U}, {4U, 5U}});
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
}

OUT_GRAPH(MoveToClosestCommonDominatorPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            CONSTANT(5U, 12345U);
            INST(2U, Opcode::If).CC(CC_LE).Inputs(0U, 1U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(3U, Opcode::Mul).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(4U, Opcode::Div).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).u64().Inputs({{3U, 3U}, {4U, 5U}});
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
}

TEST_F(MoveConstantsTest, MoveToClosestCommonDominatorPhi)
{
    src_graph::MoveToClosestCommonDominatorPhi::CREATE(GetGraph());
    Graph *graphEt = CreateEmptyGraph();
    out_graph::MoveToClosestCommonDominatorPhi::CREATE(graphEt);

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

TEST_F(MoveConstantsTest, WasNotApplied)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Return).u64().Inputs(0U);
        }
    }
    Graph *graphEt = CreateEmptyGraph();
    GRAPH(graphEt)
    {
        PARAMETER(0U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Return).u64().Inputs(0U);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_FALSE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

TEST_F(MoveConstantsTest, CatchPhiUser)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, nullptr);

        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(1U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3U, Opcode::CallStatic).ref().InputsAutoType(2U);
            INST(4U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(4U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U)
        {
            // If INST(3, Opcode::CallStatic) throw exception -> use nullptr
            // If INST(5, Opcode::CallStatic) throw exception -> use INST(3, Opcode::CallStatic)
            INST(9U, Opcode::CatchPhi).ref().ThrowableInsts({3U, 5U}).Inputs(0U, 3U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(10U, Opcode::Return).ref().Inputs(9U);
        }
    }

    GetGraph()->RunPass<MoveConstants>();
    // CONSTANT(0, nullptr) should dominate throwable INST(3, Opcode::CallStatic)
    ASSERT_TRUE(INS(0U).IsDominate(&INS(3U)));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
