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
#include "optimizer/analysis/rpo.h"

namespace panda::compiler {
class RpoTest : public GraphTest {
public:
    void Check_Subsequence(const std::vector<BasicBlock *> &&subsequence)
    {
        auto subseq_iter = subsequence.begin();
        for (auto block : GetGraph()->GetBlocksRPO()) {
            if (block == *subseq_iter) {
                if (++subseq_iter == subsequence.end()) {
                    break;
                }
            }
        }
        EXPECT_TRUE(subseq_iter == subsequence.end());
    }
};

TEST_F(RpoTest, OneBlock)
{
    auto block = GetGraph()->CreateStartBlock();
    Check_Subsequence({block});
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
TEST_F(RpoTest, GraphNoCycles)
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
        BASIC_BLOCK(4, 3, 6)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(6, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::ReturnVoid);
        }
    }
    auto A = &BB(2);
    auto B = &BB(3);
    auto C = &BB(4);
    auto D = &BB(5);
    auto E = &BB(6);
    auto exit = GetGraph()->GetEndBlock();

    Check_Subsequence({A, B, D});
    Check_Subsequence({A, C, E, D});
    Check_Subsequence({A, C, B, D});

    auto M = GetGraph()->CreateEmptyBlock();
    auto N = GetGraph()->CreateEmptyBlock();
    auto ret1 = GetGraph()->CreateInstReturnVoid();
    N->AppendInst(ret1);
    auto K = GetGraph()->CreateEmptyBlock();
    auto ret2 = GetGraph()->CreateInstReturnVoid();
    K->AppendInst(ret2);
    D->AddSucc(M);
    D->RemoveSucc(exit);
    D->RemoveInst(&INS(9));
    exit->RemovePred(D);
    auto cmp = GetGraph()->CreateInstCompare();
    cmp->SetType(DataType::BOOL);
    cmp->SetInput(0, &INS(0));
    cmp->SetInput(1, &INS(1));
    cmp->SetOperandsType(DataType::Type::INT64);
    E->AppendInst(cmp);
    auto if_inst = GetGraph()->CreateInstIfImm();
    if_inst->SetOperandsType(DataType::BOOL);
    if_inst->SetCc(CC_NE);
    if_inst->SetImm(0);
    if_inst->SetInput(0, cmp);
    E->AppendInst(if_inst);
    E->AddSucc(N);
    M->AddSucc(K);
    K->AddSucc(exit);
    N->AddSucc(exit);
    // Check handle tree update
    EXPECT_FALSE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(true);
    ArenaVector<BasicBlock *> added_blocks({M, K}, GetGraph()->GetAllocator()->Adapter());
    GetGraph()->GetAnalysis<Rpo>().AddVectorAfter(D, added_blocks);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockAfter(E, N);

    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GraphChecker(GetGraph()).Check();
    Check_Subsequence({A, B, D, M, K});
    Check_Subsequence({A, C, B, D, M, K});
    Check_Subsequence({A, C, E, D, M, K});
    Check_Subsequence({A, C, E, N});

    // Check tree rebuilding
    EXPECT_TRUE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(false);
    Check_Subsequence({A, B, D, M, K});
    Check_Subsequence({A, C, B, D, M, K});
    Check_Subsequence({A, C, E, D, M, K});
    Check_Subsequence({A, C, E, N});
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
TEST_F(RpoTest, GraphWithCycles)
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
        BASIC_BLOCK(4, 6) {}
        BASIC_BLOCK(6, 5, 7)
        {
            INST(5, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(7, 4) {}
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(5, 9, 8)
        {
            INST(9, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(11, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(8, 5) {}
    }
    auto A = &BB(2);
    auto B = &BB(3);
    auto C = &BB(4);
    auto D = &BB(5);
    auto E = &BB(6);
    auto G = &BB(7);
    auto M = &BB(8);
    auto L = &BB(9);
    auto exit = GetGraph()->GetEndBlock();

    // FIXME {A, B, T, D, exit} doesn't work
    Check_Subsequence({A, B, D, L, exit});
    Check_Subsequence({A, C, E, D, L, exit});
    Check_Subsequence({A, C, E, M, L});

    auto N = GetGraph()->CreateEmptyBlock();
    auto cmp = GetGraph()->CreateInstCompare();
    cmp->SetType(DataType::BOOL);
    cmp->SetInput(0, &INS(0));
    cmp->SetInput(1, &INS(1));
    cmp->SetOperandsType(DataType::Type::INT64);
    G->AppendInst(cmp);
    auto if_inst = GetGraph()->CreateInstIfImm();
    if_inst->SetOperandsType(DataType::BOOL);
    if_inst->SetCc(CC_NE);
    if_inst->SetImm(0);
    if_inst->SetInput(0, cmp);
    G->AppendInst(if_inst);
    auto K = GetGraph()->CreateEmptyBlock();
    G->AddSucc(N);
    N->AddSucc(K);
    K->AddSucc(G);

    // Check handle tree update
    EXPECT_FALSE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(true);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockAfter(G, N);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockAfter(N, K);
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GraphChecker(GetGraph()).Check();

    Check_Subsequence({A, B, D, L, exit});
    Check_Subsequence({A, C, E, D, L, exit});
    Check_Subsequence({A, C, E, M});

    // Check tree rebuilding
    EXPECT_TRUE(GetGraph()->GetAnalysis<Rpo>().IsValid());
    GetGraph()->GetAnalysis<Rpo>().SetValid(false);
    Check_Subsequence({A, B, D, L, exit});
    Check_Subsequence({A, C, E, D, L, exit});
    Check_Subsequence({A, C, E, M});
}
}  // namespace panda::compiler
