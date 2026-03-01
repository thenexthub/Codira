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

#include <vector>
#include "unit_test.h"
#include "optimizer/analysis/dominators_tree.h"

namespace panda::compiler {
class DomTreeTest : public GraphTest {
public:
    void CheckImmediateDominators(BasicBlock *dominator, const std::set<BasicBlock *> &&expected)
    {
        ASSERT_EQ(dominator->GetDominatedBlocks().size(), expected.size());

        for (auto block : dominator->GetDominatedBlocks()) {
            EXPECT_EQ(block->GetDominator(), dominator);
            EXPECT_TRUE(expected.find(block) != expected.end());
        }
    }

    void CheckImmediateDominatorsIdSet(int id_dom, std::vector<int> &&bb_ids)
    {
        std::set<BasicBlock *> bb_set;
        for (auto id : bb_ids) {
            bb_set.insert(&BB(id));
        }
        CheckImmediateDominators(&BB(id_dom), std::move(bb_set));
    }

    template <const bool Condition>
    void CheckListDominators(BasicBlock *dominator, const std::vector<BasicBlock *> &&expected)
    {
        for (auto dom : expected) {
            EXPECT_EQ(dominator->IsDominate(dom), Condition);
        }
    }
};

TEST_F(DomTreeTest, OneBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
        };
    }

    auto block = GetGraph()->GetStartBlock();
    GetGraph()->RunPass<DominatorsTree>();

    EXPECT_TRUE(GetGraph()->IsAnalysisValid<DominatorsTree>());
    EXPECT_TRUE(block->IsDominate(block));
}
/*
 *                      [entry]
 *                         |
 *                         v
 *                /-------[2]-------\
 *                |                 |
 *                v                 v
 *               [3]               [4]
 *                |                 |
 *                |                 v
 *                |        /-------[5]-------\
 *                |        |                 |
 *                |        v                 v
 *                |       [6]               [7]
 *                |        |                 |
 *                |        v	             |
 *                \----->[exit]<-------------/
 *
 *  Dominators tree:
 *
 *                      [entry]
 *                         |
 *                         v
 *                        [2]
 *                  /      |      \
 *                 v       v       v
 *                [3]   [exit]    [4]
 *                                 |
 *                                 v
 *                                [5]
 *                             /       \
 *                            v         v
 *                           [6]       [7]
 *
 *
 */
TEST_F(DomTreeTest, GraphNoCycles)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(4, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6, 7)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(8, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(9, Opcode::ReturnVoid);
        }
    }

    auto entry = GetGraph()->GetStartBlock();
    auto A = &BB(2);
    auto B = &BB(3);
    auto C = &BB(4);
    auto D = &BB(5);
    auto E = &BB(6);
    auto F = &BB(7);
    auto exit = GetGraph()->GetEndBlock();

    // Check if DomTree is valid after building Dom tree
    GetGraph()->RunPass<DominatorsTree>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<DominatorsTree>());

    // Check if DomTree is not valid after adding new block
    auto G = GetGraph()->CreateEmptyBlock();
    auto returnVoid = GetGraph()->CreateInstReturnVoid();
    G->AppendInst(returnVoid);
    auto cmp = GetGraph()->CreateInstCompare();
    cmp->SetType(DataType::BOOL);
    cmp->SetInput(0, &INS(0));
    cmp->SetInput(1, &INS(1));
    cmp->SetOperandsType(DataType::Type::INT64);
    C->AppendInst(cmp);
    auto if_inst = GetGraph()->CreateInstIfImm();
    if_inst->SetOperandsType(DataType::BOOL);
    if_inst->SetCc(CC_NE);
    if_inst->SetImm(0);
    if_inst->SetInput(0, cmp);
    C->AppendInst(if_inst);
    C->AddSucc(G);
    G->AddSucc(exit);
    GetGraph()->GetRootLoop()->AppendBlock(G);

    EXPECT_FALSE(GetGraph()->IsAnalysisValid<DominatorsTree>());
    GraphChecker(GetGraph()).Check();

    // Rebuild DomTree and checks dominators
    GetGraph()->RunPass<DominatorsTree>();
    CheckImmediateDominators(entry, {A});
    CheckImmediateDominators(A, {B, exit, C});
    CheckImmediateDominators(B, {});
    CheckImmediateDominators(C, {D, G});
    CheckImmediateDominators(D, {E, F});
    CheckImmediateDominators(E, {});
    CheckImmediateDominators(F, {});
    CheckImmediateDominators(G, {});
    CheckImmediateDominators(exit, {});

    CheckListDominators<true>(entry, {entry, A, B, C, D, E, F, G, exit});
    CheckListDominators<true>(A, {A, B, C, D, E, F, G, exit});
    CheckListDominators<true>(C, {C, D, E, F, G});
    CheckListDominators<true>(D, {D, E, F});

    CheckListDominators<false>(B, {entry, A, C, D, E, F, G, exit});
    CheckListDominators<false>(E, {entry, A, B, C, D, F, G, exit});
    CheckListDominators<false>(F, {entry, A, B, C, D, E, G, exit});
    CheckListDominators<false>(G, {entry, A, B, C, D, E, F, exit});
    CheckListDominators<false>(exit, {entry, A, B, C, D, E, F, G});
}

/*
 *                           [entry]
 *                              |
 *                              v
 *       /-------------------->[2]-------------\
 *       |                      |               |
 *       |                      |               v
 *       |                      |              [3]
 *       |                      v               |
 *       |                     [4]<-------------/
 *       |                      ^
 *       |                      |
 *       |                      v
 *       |                     [5]
 *       |                   /  ^
 *       |                  v   |
 *       |               [6]    |
 *       |                  \   |
 *       |                   \  |
 *       |                    v |
 *       |                     [8]
 *       |                      |
 *       |                      v
 *       |                     [9]
 *       |                   /     \
 *       |                  v       v
 *       \----------------[10]     [11]
 *                                  |
 *                                  |
 *                                  v
 *                                [exit]
 *
 *  Dominators tree:
 *                          [entry]
 *                             |
 *                             v
 *                            [2]
 *                         /       \
 *                        v         v
 *                       [3]       [4]
 *                                  |
 *                                 [5]
 *                                  |
 *                                  v
 *                                 [6]
 *                                  |
 *                                  v
 *                                 [9]
 *                              /       \
 *                             v         v
 *                            [10]      [11]
 *                                       |
 *                                       v
 *                                     [exit]
 */
TEST_F(DomTreeTest, GraphWithCycles)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6, 4)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(6, 8) {}
        BASIC_BLOCK(8, 5, 9)
        {
            INST(9, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(9, 10, 11)
        {
            INST(11, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(10, 2) {}
        BASIC_BLOCK(11, -1)
        {
            INST(14, Opcode::ReturnVoid);
        }
    }
    auto entry = GetGraph()->GetStartBlock();
    auto K = &BB(2);
    auto A = &BB(3);
    auto B = &BB(4);
    auto C = &BB(5);
    auto D = &BB(6);
    auto F = &BB(8);
    auto G = &BB(9);
    auto H = &BB(10);
    auto I = &BB(11);
    auto exit = GetGraph()->GetEndBlock();

    GraphChecker(GetGraph()).Check();

    // Check if DomTree is valid after building Dom tree
    GetGraph()->GetAnalysis<DominatorsTree>().SetValid(false);
    GetGraph()->RunPass<DominatorsTree>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<DominatorsTree>());

    CheckImmediateDominators(GetGraph()->GetStartBlock(), {BB(2).GetLoop()->GetPreHeader()});
    CheckImmediateDominators(&BB(2), {&BB(3), BB(4).GetLoop()->GetPreHeader()});
    CheckImmediateDominatorsIdSet(3, {});
    CheckImmediateDominatorsIdSet(4, {5});
    CheckImmediateDominatorsIdSet(5, {6});
    CheckImmediateDominatorsIdSet(6, {8});
    CheckImmediateDominatorsIdSet(8, {9});
    CheckImmediateDominatorsIdSet(9, {10, 11});
    CheckImmediateDominatorsIdSet(10, {});
    CheckImmediateDominatorsIdSet(11, {IrConstructor::ID_EXIT_BB});

    CheckListDominators<true>(entry, {entry, K, A, B, C, D, F, G, H, I, exit});
    CheckListDominators<true>(K, {K, A, B, C, D, F, G, H, I, exit});
    CheckListDominators<true>(B, {B, C, D, F, G, H, I, exit});
    CheckListDominators<true>(C, {C, D, F, G, H, I, exit});
    CheckListDominators<true>(F, {F, G, H, I, exit});
    CheckListDominators<true>(G, {H, I, exit});

    CheckListDominators<false>(A, {entry, B, C, D, F, G, H, I, exit});
    CheckListDominators<false>(D, {entry, A, B, C});
    CheckListDominators<false>(H, {entry, A, B, C, D, F, G, I, exit});
    CheckListDominators<false>(I, {entry, A, B, C, D, F, G, H});
}
}  // namespace panda::compiler
