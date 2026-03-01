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
#include "optimizer/optimizations/move_constants.h"

namespace panda::compiler {

class MoveConstantsTest : public GraphTest {
};

TEST_F(MoveConstantsTest, MoveNullPtrToCommonImmediateDominator)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, nullptr);

        BASIC_BLOCK(2, 4, 3)
        {
            INST(2, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(3, Opcode::ReturnI).u64().Imm(0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            INST(4, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Return).ref().Inputs(1);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(8, Opcode::Return).ref().Inputs(1);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();

        BASIC_BLOCK(2, 4, 3)
        {
            INST(2, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(3, Opcode::ReturnI).u64().Imm(0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            CONSTANT(1, nullptr);
            INST(4, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Return).ref().Inputs(1);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(8, Opcode::Return).ref().Inputs(1);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, MoveToCommonImmediateDominator)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, 12345);

        BASIC_BLOCK(2, 4, 3)
        {
            INST(2, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(3, Opcode::ReturnI).u64().Imm(0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            INST(4, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(5, Opcode::Sub).u64().Inputs(1, 0);
            INST(6, Opcode::Return).u64().Inputs(5);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(7, Opcode::Add).u64().Inputs(0, 1);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();

        BASIC_BLOCK(2, 4, 3)
        {
            INST(2, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(3, Opcode::ReturnI).u64().Imm(0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            CONSTANT(1, 12345);
            INST(4, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(5, Opcode::Sub).u64().Inputs(1, 0);
            INST(6, Opcode::Return).u64().Inputs(5);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(7, Opcode::Add).u64().Inputs(0, 1);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, MoveToClosestCommonDominator)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, 12345);

        BASIC_BLOCK(2, 4, 3)
        {
            INST(4, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(30, Opcode::ReturnI).u64().Imm(0);
        }

        BASIC_BLOCK(4, 8, 5)
        {
            INST(9, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, 7, 6)
        {
            INST(12, Opcode::IfImm).CC(CC_LE).Imm(15).Inputs(0);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(17, Opcode::Add).u64().Inputs(0, 1);
            INST(18, Opcode::Return).u64().Inputs(17);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(23, Opcode::Sub).u64().Inputs(1, 0);
            INST(24, Opcode::Return).u64().Inputs(23);
        }

        BASIC_BLOCK(8, -1)
        {
            INST(27, Opcode::Div).u64().Inputs(1, 0);
            INST(28, Opcode::Return).u64().Inputs(27);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();

        BASIC_BLOCK(2, 4, 3)
        {
            INST(4, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(30, Opcode::ReturnI).u64().Imm(0);
        }

        BASIC_BLOCK(4, 8, 5)
        {
            CONSTANT(1, 12345);
            INST(9, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, 7, 6)
        {
            INST(12, Opcode::IfImm).CC(CC_LE).Imm(15).Inputs(0);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(17, Opcode::Add).u64().Inputs(0, 1);
            INST(18, Opcode::Return).u64().Inputs(17);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(23, Opcode::Sub).u64().Inputs(1, 0);
            INST(24, Opcode::Return).u64().Inputs(23);
        }

        BASIC_BLOCK(8, -1)
        {
            INST(27, Opcode::Div).u64().Inputs(1, 0);
            INST(28, Opcode::Return).u64().Inputs(27);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, MoveJustBeforeUser)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, 12345);

        BASIC_BLOCK(2, 4, 3)
        {
            INST(4, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::ReturnI).u64().Imm(0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            INST(9, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(0, 1);
            INST(11, Opcode::Return).u64().Inputs(10);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::Return).u64().Inputs(0);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();

        BASIC_BLOCK(2, 4, 3)
        {
            INST(4, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::ReturnI).u64().Imm(0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            INST(9, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            CONSTANT(1, 12345);
            INST(10, Opcode::Add).u64().Inputs(0, 1);
            INST(11, Opcode::Return).u64().Inputs(10);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::Return).u64().Inputs(0);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, MoveJustBeforeUserSingleBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, 12345);

        BASIC_BLOCK(2, 4, 3)
        {
            INST(5, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::ReturnI).Imm(0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            INST(9, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(12, Opcode::Div).u64().Inputs(1, 0);
            INST(13, Opcode::Add).u64().Inputs(12, 1);
            INST(14, Opcode::Return).u64().Inputs(13);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(15, Opcode::Return).u64().Inputs(0);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();

        BASIC_BLOCK(2, 4, 3)
        {
            INST(5, Opcode::IfImm).CC(CC_GE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::ReturnI).Imm(0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            INST(9, Opcode::IfImm).CC(CC_LE).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            CONSTANT(1, 12345);
            INST(12, Opcode::Div).u64().Inputs(1, 0);
            INST(13, Opcode::Add).u64().Inputs(12, 1);
            INST(14, Opcode::Return).u64().Inputs(13);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(15, Opcode::Return).u64().Inputs(0);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, MovePhiInput)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(2, 12345);

        BASIC_BLOCK(2, 4, 3)
        {
            INST(15, Opcode::IfImm).CC(CC_LE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(4, Opcode::Phi).u64().Inputs({{2, 0}, {3, 22}});
            INST(5, Opcode::Phi).u64().Inputs({{2, 2}, {3, 23}});
            INST(22, Opcode::AddI).u64().Imm(1).Inputs(4);
            INST(23, Opcode::AddI).u64().Imm(1).Inputs(5);
            INST(24, Opcode::If).CC(CC_GE).Inputs(23, 0);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(19, Opcode::Phi).u64().Inputs({{3, 22}, {2, 0}});
            INST(13, Opcode::Return).u64().Inputs(19);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();

        BASIC_BLOCK(2, 4, 3)
        {
            CONSTANT(2, 12345);
            INST(15, Opcode::IfImm).CC(CC_LE).Imm(5).Inputs(0);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(4, Opcode::Phi).u64().Inputs({{2, 0}, {3, 22}});
            INST(5, Opcode::Phi).u64().Inputs({{2, 2}, {3, 23}});
            INST(22, Opcode::AddI).u64().Imm(1).Inputs(4);
            INST(23, Opcode::AddI).u64().Imm(1).Inputs(5);
            INST(24, Opcode::If).CC(CC_GE).Inputs(23, 0);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(19, Opcode::Phi).u64().Inputs({{3, 22}, {2, 0}});
            INST(13, Opcode::Return).u64().Inputs(19);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, AvoidMoveToLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(2, 5);
        CONSTANT(11, 0);
        CONSTANT(22, 3);

        BASIC_BLOCK(2, 3)
        {
            INST(1, Opcode::NOP);
        }

        BASIC_BLOCK(3, 7, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs({{2, 2}, {6, 35}});
            INST(4, Opcode::Phi).u64().Inputs({{2, 0}, {6, 34}});
            INST(5, Opcode::Phi).u64().Inputs({{2, 2}, {6, 42}});
            INST(39, Opcode::If).CC(CC_GE).Inputs(5, 0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            INST(29, Opcode::IfImm).CC(CC_LE).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(5, 6, 5)
        {
            INST(14, Opcode::Phi).u64().Inputs({{4, 4}, {5, 23}});
            INST(16, Opcode::Phi).u64().Inputs({{4, 11}, {5, 40}});
            INST(23, Opcode::Mul).u64().Inputs(14, 22);
            INST(40, Opcode::AddI).u64().Imm(1).Inputs(16);
            INST(41, Opcode::If).CC(CC_GE).Inputs(40, 0);
        }

        BASIC_BLOCK(6, 3)
        {
            INST(34, Opcode::Phi).u64().Inputs({{5, 23}, {4, 4}});
            INST(35, Opcode::Phi).u64().Inputs({{5, 40}, {4, 11}});
            INST(42, Opcode::AddI).u64().Imm(1).Inputs(5);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(27, Opcode::Return).u64().Inputs(4);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();

        BASIC_BLOCK(2, 3)
        {
            CONSTANT(22, 3);
            CONSTANT(11, 0);
            CONSTANT(2, 5);
            INST(1, Opcode::NOP);
        }

        BASIC_BLOCK(3, 7, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs({{2, 2}, {6, 35}});
            INST(4, Opcode::Phi).u64().Inputs({{2, 0}, {6, 34}});
            INST(5, Opcode::Phi).u64().Inputs({{2, 2}, {6, 42}});
            INST(39, Opcode::If).CC(CC_GE).Inputs(5, 0);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            INST(29, Opcode::IfImm).CC(CC_LE).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(5, 6, 5)
        {
            INST(14, Opcode::Phi).u64().Inputs({{4, 4}, {5, 23}});
            INST(16, Opcode::Phi).u64().Inputs({{4, 11}, {5, 40}});
            INST(23, Opcode::Mul).u64().Inputs(14, 22);
            INST(40, Opcode::AddI).u64().Imm(1).Inputs(16);
            INST(41, Opcode::If).CC(CC_GE).Inputs(40, 0);
        }

        BASIC_BLOCK(6, 3)
        {
            INST(34, Opcode::Phi).u64().Inputs({{5, 23}, {4, 4}});
            INST(35, Opcode::Phi).u64().Inputs({{5, 40}, {4, 11}});
            INST(42, Opcode::AddI).u64().Imm(1).Inputs(5);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(27, Opcode::Return).u64().Inputs(4);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, MoveToClosestCommonDominator2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).u64();
        CONSTANT(4, 12345);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::If).CC(CC_LE).Inputs(0, 4);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 4);
            INST(5, Opcode::Return).u64().Inputs(3);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Mul).u64().Inputs(1, 4);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            CONSTANT(4, 12345);
            INST(2, Opcode::If).CC(CC_LE).Inputs(0, 4);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 4);
            INST(5, Opcode::Return).u64().Inputs(3);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Mul).u64().Inputs(1, 4);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, MoveToClosestCommonDominatorPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).u64();
        CONSTANT(5, 12345);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::If).CC(CC_LE).Inputs(0, 1);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(4, Opcode::Div).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs({{3, 3}, {4, 5}});
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            CONSTANT(5, 12345);
            INST(2, Opcode::If).CC(CC_LE).Inputs(0, 1);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(4, Opcode::Div).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs({{3, 3}, {4, 5}});
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_TRUE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, WasNotApplied)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Return).u64().Inputs(0);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Return).u64().Inputs(0);
        }
    }

    bool result = GetGraph()->RunPass<MoveConstants>();
    ASSERT_FALSE(result);

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(MoveConstantsTest, CatchPhiUser)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, nullptr);

        BASIC_BLOCK(2, 3, 5)
        {
            INST(1, Opcode::Try).CatchTypeIds({0x0});
        }
        BASIC_BLOCK(3, 4)
        {
            INST(2, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3, Opcode::CallStatic).ref().InputsAutoType(2);
            INST(4, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(4);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6)
        {
            // If INST(3, Opcode::CallStatic) throw exception -> use nullptr
            // If INST(5, Opcode::CallStatic) throw exception -> use INST(3, Opcode::CallStatic)
            INST(9, Opcode::CatchPhi).ref().ThrowableInsts({3, 5}).Inputs(0, 3);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(10, Opcode::Return).ref().Inputs(9);
        }
    }

    GetGraph()->RunPass<MoveConstants>();
    // CONSTANT(0, nullptr) should dominate throwable INST(3, Opcode::CallStatic)
    ASSERT_TRUE(INS(0).IsDominate(&INS(3)));
}

}  // namespace panda::compiler
