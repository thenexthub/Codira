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
#include "optimizer/analysis/rpo.h"

namespace ark::compiler {
class RpoTest : public GraphTest {
public:
    void CheckSubsequence(const std::vector<BasicBlock *> &&subsequence)
    {
        auto subseqIter = subsequence.begin();
        for (auto block : GetGraph()->GetBlocksRPO()) {
            if (block == *subseqIter) {
                if (++subseqIter == subsequence.end()) {
                    break;
                }
            }
        }
        EXPECT_TRUE(subseqIter == subsequence.end());
    }
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(RpoTest, OneBlock)
{
    auto block = GetGraph()->CreateStartBlock();
    CheckSubsequence({block});
}

/*
 *                 [entry]
 *                    |
 *                    v
 *                   [A]
 *                    |       \
 *                    v       v
 *                   [B]  <- [C]
 *                    |       |
 *                    v       v
 *                   [D]  <- [E]
 *                    |
 *                    v
 *                  [exit]
 *
 * Add [M], [N], [K]:
 *                 [entry]
 *                    |
 *                    v
 *                   [A]
 *            /       |       \
 *            v       v       v
 *           [T]  -> [B]  <- [C]
 *                    |       |
 *                    v       v
 *                   [D]  <- [E] -> [N]
 *                    |              |
 *                    v              |
 *                   [M]             |
 *                    |              |
 *                    v              |
 *                   [K]             |
 *                    |              |
 *                    v              |
 *                  [exit] <---------/
 */
SRC_GRAPH(GraphNoCyclesRPO, Graph *graph)
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
        BASIC_BLOCK(4U, 3U, 6U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 5U) {}
        BASIC_BLOCK(6U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(RpoTest, GraphNoCyclesRPO)
{
    src_graph::GraphNoCyclesRPO::CREATE(GetGraph());
    auto a = &BB(2U);
    auto b = &BB(3U);
    auto c = &BB(4U);
    auto d = &BB(5U);
    auto e = &BB(6U);
    auto exit = GetGraph()->GetEndBlock();

    CheckSubsequence({a, b, d});
    CheckSubsequence({a, c, e, d});
    CheckSubsequence({a, c, b, d});

    auto m = GetGraph()->CreateEmptyBlock();
    auto n = GetGraph()->CreateEmptyBlock();
    auto ret1 = GetGraph()->CreateInstReturnVoid();
    n->AppendInst(ret1);
    auto k = GetGraph()->CreateEmptyBlock();
    auto ret2 = GetGraph()->CreateInstReturnVoid();
    k->AppendInst(ret2);
    d->AddSucc(m);
    d->RemoveSucc(exit);
    d->RemoveInst(&INS(9U));
    exit->RemovePred(d);
    auto cmp =
        GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, &INS(0U), &INS(1U), DataType::Type::INT64, CC_NE);
    e->AppendInst(cmp);
    auto ifInst = GetGraph()->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, cmp, 0U, DataType::BOOL, CC_NE);
    e->AppendInst(ifInst);
    e->AddSucc(n);
    m->AddSucc(k);
    k->AddSucc(exit);
    n->AddSucc(exit);
    // Check handle tree update
    EXPECT_FALSE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(true);
    ArenaVector<BasicBlock *> addedBlocks({m, k}, GetGraph()->GetAllocator()->Adapter());
    GetGraph()->GetAnalysis<Rpo>().AddVectorAfter(d, addedBlocks);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockAfter(e, n);

    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GraphChecker(GetGraph()).Check();
    CheckSubsequence({a, b, d, m, k});
    CheckSubsequence({a, c, b, d, m, k});
    CheckSubsequence({a, c, e, d, m, k});
    CheckSubsequence({a, c, e, n});

    // Check tree rebuilding
    EXPECT_TRUE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(false);
    CheckSubsequence({a, b, d, m, k});
    CheckSubsequence({a, c, b, d, m, k});
    CheckSubsequence({a, c, e, d, m, k});
    CheckSubsequence({a, c, e, n});
}

/*
 *                 [entry]
 *                    |
 *                    v
 *                   [A]
 *                    |       \
 *                    v       v
 *                   [B]     [C] <- [M]
 *                    |       |      ^
 *                    V       v     /
 *           [G]<--> [D]  <- [E] --/
 *                    |
 *                    v
 *                  [exit]
 *
 *
 * Add [N], [K]
 *                 [entry]
 *                    |
 *                    v
 *                   [A]
 *                    |       \
 *                    v       v
 *                   [B]     [C] <- [M]
 *                    |       |      ^
 *                    V       v     /
 *    [N] <- [G]<--> [D]  <- [E] --/
 *     |      ^       |
 *     |     /        v
 *     |    /        [L]
 *     |   /          |
 *     v  /           v
 *    [K]/          [exit]
 */
SRC_GRAPH(GraphWithCyclesRPO, Graph *graph)
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
        BASIC_BLOCK(4U, 6U) {}
        BASIC_BLOCK(6U, 5U, 7U)
        {
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(7U, 4U) {}
        BASIC_BLOCK(3U, 5U) {}
        BASIC_BLOCK(5U, 9U, 8U)
        {
            INST(9U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(9U, -1L)
        {
            INST(11U, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(8U, 5U) {}
    }
}

TEST_F(RpoTest, GraphWithCyclesRPO)
{
    src_graph::GraphWithCyclesRPO::CREATE(GetGraph());
    auto a = &BB(2U);
    auto b = &BB(3U);
    auto c = &BB(4U);
    auto d = &BB(5U);
    auto e = &BB(6U);
    auto g = &BB(7U);
    auto m = &BB(8U);
    auto l = &BB(9U);
    auto exit = GetGraph()->GetEndBlock();

    // NOTE {A, B, T, D, exit} doesn't work
    CheckSubsequence({a, b, d, l, exit});
    CheckSubsequence({a, c, e, d, l, exit});
    CheckSubsequence({a, c, e, m, l});

    auto n = GetGraph()->CreateEmptyBlock();
    auto cmp =
        GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, &INS(0U), &INS(1U), DataType::Type::INT64, CC_NE);
    g->AppendInst(cmp);
    auto ifInst = GetGraph()->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, cmp, 0U, DataType::BOOL, CC_NE);
    g->AppendInst(ifInst);
    auto k = GetGraph()->CreateEmptyBlock();
    g->AddSucc(n);
    n->AddSucc(k);
    k->AddSucc(g);

    // Check handle tree update
    EXPECT_FALSE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(true);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockAfter(g, n);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockAfter(n, k);
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GraphChecker(GetGraph()).Check();

    CheckSubsequence({a, b, d, l, exit});
    CheckSubsequence({a, c, e, d, l, exit});
    CheckSubsequence({a, c, e, m});

    // Check tree rebuilding
    EXPECT_TRUE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(false);
    CheckSubsequence({a, b, d, l, exit});
    CheckSubsequence({a, c, e, d, l, exit});
    CheckSubsequence({a, c, e, m});
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
