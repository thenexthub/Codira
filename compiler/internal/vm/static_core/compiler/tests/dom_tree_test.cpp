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
#include <vector>
#include "optimizer/analysis/dominators_tree.h"

namespace ark::compiler {
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

    void CheckImmediateDominatorsIdSet(int idDom, std::vector<int> &&bbIds)
    {
        std::set<BasicBlock *> bbSet;
        for (auto id : bbIds) {
            bbSet.insert(&BB(id));
        }
        CheckImmediateDominators(&BB(idDom), std::move(bbSet));
    }

    template <const bool CONDITION>
    void CheckListDominators(BasicBlock *dominator, const std::vector<BasicBlock *> &&expected)
    {
        for (auto dom : expected) {
            EXPECT_EQ(dominator->IsDominate(dom), CONDITION);
        }
    }
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(DomTreeTest, OneBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
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
SRC_GRAPH(GraphNoCycles, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(4U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(8U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(DomTreeTest, GraphNoCycles)
{
    src_graph::GraphNoCycles::CREATE(GetGraph());

    auto entry = GetGraph()->GetStartBlock();
    auto a = &BB(2U);
    auto b = &BB(3U);
    auto c = &BB(4U);
    auto d = &BB(5U);
    auto e = &BB(6U);
    auto f = &BB(7U);
    auto exit = GetGraph()->GetEndBlock();

    // Check if DomTree is valid after building Dom tree
    GetGraph()->RunPass<DominatorsTree>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<DominatorsTree>());

    // Check if DomTree is not valid after adding new block
    auto g = GetGraph()->CreateEmptyBlock();
    auto returnVoid = GetGraph()->CreateInstReturnVoid();
    g->AppendInst(returnVoid);
    auto cmp =
        GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, &INS(0U), &INS(1U), DataType::Type::INT64, CC_NE);
    c->AppendInst(cmp);
    auto ifInst = GetGraph()->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, cmp, 0U, DataType::BOOL, CC_NE);
    c->AppendInst(ifInst);
    c->AddSucc(g);
    g->AddSucc(exit);
    GetGraph()->GetRootLoop()->AppendBlock(g);

    EXPECT_FALSE(GetGraph()->IsAnalysisValid<DominatorsTree>());
    GraphChecker(GetGraph()).Check();

    // Rebuild DomTree and checks dominators
    GetGraph()->RunPass<DominatorsTree>();
    CheckImmediateDominators(entry, {a});
    CheckImmediateDominators(a, {b, exit, c});
    CheckImmediateDominators(b, {});
    CheckImmediateDominators(c, {d, g});
    CheckImmediateDominators(d, {e, f});
    CheckImmediateDominators(e, {});
    CheckImmediateDominators(f, {});
    CheckImmediateDominators(g, {});
    CheckImmediateDominators(exit, {});

    CheckListDominators<true>(entry, {entry, a, b, c, d, e, f, g, exit});
    CheckListDominators<true>(a, {a, b, c, d, e, f, g, exit});
    CheckListDominators<true>(c, {c, d, e, f, g});
    CheckListDominators<true>(d, {d, e, f});

    CheckListDominators<false>(b, {entry, a, c, d, e, f, g, exit});
    CheckListDominators<false>(e, {entry, a, b, c, d, f, g, exit});
    CheckListDominators<false>(f, {entry, a, b, c, d, e, g, exit});
    CheckListDominators<false>(g, {entry, a, b, c, d, e, f, exit});
    CheckListDominators<false>(exit, {entry, a, b, c, d, e, f, g});
}

/*
 *                           [entry]
 *                              |
 *                              v
 *       /-------------------->[2]--------------\
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
SRC_GRAPH(GraphWithCycles, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U, 4U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(6U, 8U) {}
        BASIC_BLOCK(8U, 5U, 9U)
        {
            INST(9U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(9U, 10U, 11U)
        {
            INST(11U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(10U, 2U) {}
        BASIC_BLOCK(11U, -1L)
        {
            INST(14U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(DomTreeTest, GraphWithCycles)
{
    src_graph::GraphWithCycles::CREATE(GetGraph());
    auto entry = GetGraph()->GetStartBlock();
    auto k = &BB(2U);
    auto a = &BB(3U);
    auto b = &BB(4U);
    auto c = &BB(5U);
    auto d = &BB(6U);
    auto f = &BB(8U);
    auto g = &BB(9U);
    auto h = &BB(10U);
    auto i = &BB(11U);
    auto exit = GetGraph()->GetEndBlock();

    GraphChecker(GetGraph()).Check();

    // Check if DomTree is valid after building Dom tree
    GetGraph()->GetAnalysis<DominatorsTree>().SetValid(false);
    GetGraph()->RunPass<DominatorsTree>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<DominatorsTree>());

    CheckImmediateDominators(GetGraph()->GetStartBlock(), {BB(2U).GetLoop()->GetPreHeader()});
    CheckImmediateDominators(&BB(2U), {&BB(3U), BB(4U).GetLoop()->GetPreHeader()});
    CheckImmediateDominatorsIdSet(3U, {});
    CheckImmediateDominatorsIdSet(4U, {5U});
    CheckImmediateDominatorsIdSet(5U, {6U});
    CheckImmediateDominatorsIdSet(6U, {8U});
    CheckImmediateDominatorsIdSet(8U, {9U});
    CheckImmediateDominatorsIdSet(9U, {10U, 11U});
    CheckImmediateDominatorsIdSet(10U, {});
    CheckImmediateDominatorsIdSet(11U, {IrConstructor::ID_EXIT_BB});

    CheckListDominators<true>(entry, {entry, k, a, b, c, d, f, g, h, i, exit});
    CheckListDominators<true>(k, {k, a, b, c, d, f, g, h, i, exit});
    CheckListDominators<true>(b, {b, c, d, f, g, h, i, exit});
    CheckListDominators<true>(c, {c, d, f, g, h, i, exit});
    CheckListDominators<true>(f, {f, g, h, i, exit});
    CheckListDominators<true>(g, {h, i, exit});

    CheckListDominators<false>(a, {entry, b, c, d, f, g, h, i, exit});
    CheckListDominators<false>(d, {entry, a, b, c});
    CheckListDominators<false>(h, {entry, a, b, c, d, f, g, i, exit});
    CheckListDominators<false>(i, {entry, a, b, c, d, f, g, h});
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
