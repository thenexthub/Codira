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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "unit_test.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/liveness_analyzer.h"

namespace panda::compiler {
using LiveRanges = ArenaDeque<LiveRange>;

#define LIVE_RANGES_DEQUE(...) LiveRanges({__VA_ARGS__}, GetAllocator()->Adapter())

class LivenessAnalyzerTest : public GraphTest {
public:
    void Check_Subsequence(const ArenaVector<BasicBlock *> &blocks, const ArenaVector<BasicBlock *> &&subsequence)
    {
        auto subseq_iter = subsequence.begin();
        for (auto block : blocks) {
            if (block == *subseq_iter) {
                if (++subseq_iter == subsequence.end()) {
                    break;
                }
            }
        }
        EXPECT_TRUE(subseq_iter == subsequence.end());
    }
};

/*
 * Test Graph:
 *                                    [0]
 *                                     |
 *                                     v
 *                                 /--[2]--\
 *                                /         \
 *                               /           \
 *                              v             V
 *                /----------->[4]---------->[3]<---------[15]-------\
 *                |             |             |                      |
 *                |             v             V                      |
 *               [12]          [5]           [6]<-----------\        |
 *                |             |             |             |        |
 *                |             v             v             |        |
 *                \------------[11]          [7]          [10]       |
 *                              |             |             ^        |
 *                              v             v             |        |
 *                             [13]          [8]---------->[9]       |
 *                              |             |                      |
 *                              v             v                      |
 *                             [1]          [14]---------------------/
 *
 * Linear order:
 * [0, 2, 4, 5, 11, 12, 13, 1, 3, 6, 7, 8, 9, 10, 14, 15, 1]
 */
TEST_F(LivenessAnalyzerTest, LinearizeGraph)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, 4, 3)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 6) {}
        BASIC_BLOCK(4, 5, 3)
        {
            INST(5, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(5, 11) {}
        BASIC_BLOCK(6, 7) {}
        BASIC_BLOCK(7, 8) {}
        BASIC_BLOCK(8, 14, 9)
        {
            INST(10, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(9, 10) {}
        BASIC_BLOCK(10, 6) {}
        BASIC_BLOCK(11, 13, 12)
        {
            INST(14, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(12, 4) {}
        BASIC_BLOCK(13, -1)
        {
            INST(17, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(14, 15) {}
        BASIC_BLOCK(15, 3) {}
    }
    EXPECT_TRUE(GetGraph()->RunPass<LivenessAnalyzer>());

    const auto &blocks = GetGraph()->GetAnalysis<LivenessAnalyzer>().GetLinearizedBlocks();
    Check_Subsequence(blocks, GetBlocksById(GetGraph(), {0, 2, 4, 5, 11, 12, 13, 1, 3, 6, 7, 8, 9, 10, 14, 15}));
}

TEST_F(LivenessAnalyzerTest, LinearizeGraph2)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);

        BASIC_BLOCK(100, 6) {}
        BASIC_BLOCK(6, 2, 7)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(2, 5) {}
        BASIC_BLOCK(5, 4, 3)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(4, 21) {}
        BASIC_BLOCK(21, 17, 14)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(14, 5) {}
        BASIC_BLOCK(3, 6) {}
        BASIC_BLOCK(7, -1)
        {
            INST(14, Opcode::ReturnVoid);
        }

        BASIC_BLOCK(17, 18, 19)
        {
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(18, 24, 31)
        {
            INST(10, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }

        BASIC_BLOCK(19, 36, 43)
        {
            INST(12, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(12);
        }

        BASIC_BLOCK(24, 20) {}
        BASIC_BLOCK(36, 20) {}
        BASIC_BLOCK(20, 21) {}
        BASIC_BLOCK(31, -1)
        {
            INST(15, Opcode::ReturnVoid);
        }
        BASIC_BLOCK(43, -1)
        {
            INST(16, Opcode::ReturnVoid);
        }
    }
    EXPECT_TRUE(GetGraph()->RunPass<LivenessAnalyzer>());

    const auto &blocks = GetGraph()->GetAnalysis<LivenessAnalyzer>().GetLinearizedBlocks();
    Check_Subsequence(blocks,
                      GetBlocksById(GetGraph(), {0, 100, 6, 2, 5, 4, 21, 17, 18, 24, 19, 36, 20, 14, 3, 43, 31, 7, 1}));
}

/*
 *          [0]
 *           |
 *           v
 *          [2]
 *         /   \
 *        v     v
 *      [3]<----[4]
 *       |
 *       v
 *      [1]
 */
TEST_F(LivenessAnalyzerTest, LinearizeGraphWithoutLoops)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(4, 3) {}
        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::ReturnVoid);
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LivenessAnalyzer>());
    Check_Subsequence(GetGraph()->GetAnalysis<LivenessAnalyzer>().GetLinearizedBlocks(),
                      GetBlocksById(GetGraph(), {0, 2, 4, 3, 1}));

    BB(2).SwapTrueFalseSuccessors();
    EXPECT_TRUE(GetGraph()->RunPass<LivenessAnalyzer>());
    Check_Subsequence(GetGraph()->GetAnalysis<LivenessAnalyzer>().GetLinearizedBlocks(),
                      GetBlocksById(GetGraph(), {0, 2, 4, 3, 1}));
}

/*
 * Сheck LifeIntervals updating correctness
 */
TEST_F(LivenessAnalyzerTest, LifeIntervals)
{
    LifeIntervals life_inter(GetAllocator());
    life_inter.AppendRange({90, 100});
    life_inter.AppendRange({80, 90});
    life_inter.AppendRange({40, 50});
    life_inter.AppendRange({35, 40});
    EXPECT_EQ(life_inter.GetRanges(), LIVE_RANGES_DEQUE({35, 50}, {80, 100}));

    life_inter.AppendRange({20, 34});
    life_inter.StartFrom(30);
    EXPECT_EQ(life_inter.GetRanges(), LIVE_RANGES_DEQUE({30, 34}, {35, 50}, {80, 100}));

    life_inter.AppendRange({10, 20});
    life_inter.AppendGroupRange({10, 25});
    EXPECT_EQ(life_inter.GetRanges(), LIVE_RANGES_DEQUE({10, 25}, {30, 34}, {35, 50}, {80, 100}));

    life_inter.AppendGroupRange({10, 79});
    EXPECT_EQ(life_inter.GetRanges(), LIVE_RANGES_DEQUE({10, 79}, {80, 100}));

    life_inter.AppendGroupRange({10, 95});
    EXPECT_EQ(life_inter.GetRanges(), LIVE_RANGES_DEQUE({10, 100}));
}

/*
 * Test Graph:
 *              [0]
 *               |
 *               v
 *        /-----[2]<----\
 *        |      |      |
 *        |      v      |
 *        |     [3]-----/
 *        |
 *        \---->[4]
 *               |
 *               v
 *             [exit]
 *
 *
 * Blocks linear order:
 * ID   LIFE RANGE
 * ---------------
 * 0    [0, 8]
 * 2    [8, 14]
 * 3    [14, 22]
 * 4    [22, 26]
 *
 *
 * Insts linear order:
 * ID   INST(INPUTS)    LIFE NUMBER LIFE INTERVALS
 * ------------------------------------------
 * 0.   Constant        2           [2-22]
 * 1.   Constant        4           [4-8]
 * 2.   Constant        6           [6-24]
 *
 * 3.   Phi (0,7)       8           [8-16][22-24]
 * 4.   Phi (1,8)       8           [8-18]
 * 5.   Cmp (4,0)       10          [10-12]
 * 6.   If    (5)       12          -
 *
 * 7.   Mul (3,4)       16          [16-2?]
 * 8.   Sub (4,0)       18          [18-2?]
 *
 * 9.   Add (2,3)       2?          [2?-2?]
 */
TEST_F(LivenessAnalyzerTest, InstructionsLifetime)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        CONSTANT(1, 10);
        CONSTANT(2, 20);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs({{0, 0}, {3, 7}});
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {3, 8}});
            INST(5, Opcode::Compare).b().Inputs(4, 0);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }

        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::Mul).u64().Inputs(3, 4);
            INST(8, Opcode::Sub).u64().Inputs(4, 0);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Add).u64().Inputs(2, 3);
            INST(10, Opcode::ReturnVoid);
        }
    }
    auto liveness_analyzer = &GetGraph()->GetValidAnalysis<LivenessAnalyzer>();

    auto const0 = liveness_analyzer->GetInstLifeIntervals(&INS(0));
    auto const1 = liveness_analyzer->GetInstLifeIntervals(&INS(1));
    auto const2 = liveness_analyzer->GetInstLifeIntervals(&INS(2));
    auto phi0 = liveness_analyzer->GetInstLifeIntervals(&INS(3));
    auto phi1 = liveness_analyzer->GetInstLifeIntervals(&INS(4));
    auto cmp = liveness_analyzer->GetInstLifeIntervals(&INS(5));
    auto mul = liveness_analyzer->GetInstLifeIntervals(&INS(7));
    auto sub = liveness_analyzer->GetInstLifeIntervals(&INS(8));
    auto add = liveness_analyzer->GetInstLifeIntervals(&INS(9));

    auto b0_lifetime = liveness_analyzer->GetBlockLiveRange(&BB(0));
    auto b2_lifetime = liveness_analyzer->GetBlockLiveRange(&BB(2));
    auto b3_lifetime = liveness_analyzer->GetBlockLiveRange(&BB(3));
    auto b4_lifetime = liveness_analyzer->GetBlockLiveRange(&BB(4));

    EXPECT_EQ(const0->GetRanges()[0], LiveRange(b0_lifetime.GetBegin() + 2, b3_lifetime.GetEnd()));
    EXPECT_EQ(const1->GetRanges()[0], LiveRange(b0_lifetime.GetBegin() + 4, phi0->GetRanges()[0].GetBegin()));
    EXPECT_EQ(const2->GetRanges()[0], LiveRange(b0_lifetime.GetBegin() + 6, add->GetRanges()[0].GetBegin()));
    EXPECT_EQ(phi0->GetRanges()[0], LiveRange(b2_lifetime.GetBegin(), mul->GetRanges()[0].GetBegin()));
    EXPECT_EQ(phi0->GetRanges()[1], LiveRange(b4_lifetime.GetBegin(), add->GetRanges()[0].GetBegin()));
    EXPECT_EQ(phi1->GetRanges()[0], LiveRange(b2_lifetime.GetBegin(), sub->GetRanges()[0].GetBegin()));
    EXPECT_EQ(cmp->GetRanges()[0], LiveRange(b2_lifetime.GetBegin() + 2, b2_lifetime.GetBegin() + 4));
    EXPECT_EQ(mul->GetRanges()[0], LiveRange(b3_lifetime.GetBegin() + 2, b3_lifetime.GetEnd()));
    EXPECT_EQ(sub->GetRanges()[0], LiveRange(b3_lifetime.GetBegin() + 4, b3_lifetime.GetEnd()));
    EXPECT_EQ(add->GetRanges()[0], LiveRange(b4_lifetime.GetBegin() + 2, b4_lifetime.GetBegin() + 4));
}

TEST_F(LivenessAnalyzerTest, LoadStoreArrayDataFlow)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // array
        PARAMETER(1, 1).s64();  // index
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 1, 2);
            INST(6, Opcode::LoadArray).u64().Inputs(3, 5);
            INST(7, Opcode::Add).s64().Inputs(6, 6);
            INST(8, Opcode::StoreArray).u64().Inputs(3, 5, 7);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(7);  // Some return value
        }
    }

    auto liveness_analyzer = &GetGraph()->GetAnalysis<LivenessAnalyzer>();
    liveness_analyzer->Run();

    auto array = liveness_analyzer->GetInstLifeIntervals(&INS(0));
    auto index = liveness_analyzer->GetInstLifeIntervals(&INS(1));
    auto null_check = liveness_analyzer->GetInstLifeIntervals(&INS(3));
    auto len_array = liveness_analyzer->GetInstLifeIntervals(&INS(4));
    auto bounds_check = liveness_analyzer->GetInstLifeIntervals(&INS(5));
    auto st_array = liveness_analyzer->GetInstLifeIntervals(&INS(8));

    auto b0_lifetime = liveness_analyzer->GetBlockLiveRange(&BB(0));

    EXPECT_EQ(array->GetRanges()[0], LiveRange(b0_lifetime.GetBegin() + 2, st_array->GetRanges()[0].GetBegin()));
    EXPECT_EQ(index->GetRanges()[0], LiveRange(b0_lifetime.GetBegin() + 4, st_array->GetRanges()[0].GetBegin()));

    EXPECT_EQ(null_check->GetRanges()[0].GetEnd() - null_check->GetRanges()[0].GetBegin(), 2U);
    EXPECT_EQ(bounds_check->GetRanges()[0].GetEnd() - bounds_check->GetRanges()[0].GetBegin(), 2U);
    EXPECT_EQ(len_array->GetRanges()[0].GetEnd() - len_array->GetRanges()[0].GetBegin(), 2U);
}

TEST_F(LivenessAnalyzerTest, SaveStateInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).u32();
        PARAMETER(2, 2).u32();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::StoreObject).u32().Inputs(4, 1);
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }
    auto liveness_analyzer = &GetGraph()->GetAnalysis<LivenessAnalyzer>();
    liveness_analyzer->Run();

    auto par0_lifetime = liveness_analyzer->GetInstLifeIntervals(&INS(0));
    auto par1_lifetime = liveness_analyzer->GetInstLifeIntervals(&INS(1));
    auto par2_lifetime = liveness_analyzer->GetInstLifeIntervals(&INS(2));
    auto null_check_lifetime = liveness_analyzer->GetInstLifeIntervals(&INS(4));
    EXPECT_TRUE(par0_lifetime->GetEnd() == null_check_lifetime->GetEnd());
    EXPECT_TRUE(par1_lifetime->GetEnd() == null_check_lifetime->GetEnd());
    EXPECT_TRUE(par2_lifetime->GetEnd() == null_check_lifetime->GetBegin());
}

/**
 *        [begin]
 *           |
 *          [2]
 *           |
 *          [3]<-------\
 *         /    \      |
 *      [end]   [4]    |
 *               |     |
 *        /---->[5]----/
 *        |      |
 *        \-----[6]
 *
 */
TEST_F(LivenessAnalyzerTest, InnerLoops)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();  // a
        PARAMETER(1, 1).u32();  // b
        PARAMETER(2, 2).u32();  // c
        CONSTANT(3, 1);         // d

        BASIC_BLOCK(2, 3)
        {
            INST(5, Opcode::Mul).u32().Inputs(0, 1);  // a * b
            INST(6, Opcode::Add).u32().Inputs(0, 1);  // a + b
        }
        BASIC_BLOCK(3, 4, 7)
        {
            INST(9, Opcode::Phi).u32().Inputs(2, 12);
            INST(10, Opcode::If).SrcType(DataType::UINT32).CC(CC_LT).Inputs(9, 5);  // if c < a * b
        }
        BASIC_BLOCK(4, 5)
        {
            INST(11, Opcode::Add).u32().Inputs(9, 3);  // c++
        }
        BASIC_BLOCK(5, 6, 3)
        {
            INST(12, Opcode::Phi).u32().Inputs(11, 14);
            INST(13, Opcode::If).SrcType(DataType::UINT32).CC(CC_LT).Inputs(12, 6);  // if c < a + b
        }
        BASIC_BLOCK(6, 5)
        {
            INST(14, Opcode::Add).u32().Inputs(12, 3);  // c++
        }
        BASIC_BLOCK(7, -1)
        {
            INST(15, Opcode::ReturnVoid);
        }
    }

    auto liveness_analyzer = &GetGraph()->GetAnalysis<LivenessAnalyzer>();
    liveness_analyzer->Run();
    auto mul = liveness_analyzer->GetInstLifeIntervals(&INS(5));
    auto add = liveness_analyzer->GetInstLifeIntervals(&INS(6));
    auto inner_loop_back = liveness_analyzer->GetBlockLiveRange(&BB(6));
    EXPECT_EQ(mul->GetEnd(), inner_loop_back.GetEnd());
    EXPECT_EQ(add->GetEnd(), inner_loop_back.GetEnd());
}

TEST_F(LivenessAnalyzerTest, UpdateExistingRanges)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();

        BASIC_BLOCK(2, 4, 3)
        {
            INST(2, Opcode::Add).u32().Inputs(0, 1);
            INST(3, Opcode::Compare).b().Inputs(0, 2);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Cast).u32().SrcType(DataType::BOOL).Inputs(3);
            INST(6, Opcode::Return).u32().Inputs(5);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::ReturnI).u32().Imm(1);
        }
    }

    auto la = &GetGraph()->GetAnalysis<LivenessAnalyzer>();
    la->Run();

    auto cmp = la->GetInstLifeIntervals(&INS(3));
    EXPECT_EQ(cmp->GetRanges().size(), 2);

    auto first_interval = cmp->GetRanges().front();
    auto add = la->GetInstLifeIntervals(&INS(2));
    EXPECT_EQ(first_interval.GetBegin(), add->GetEnd());
    EXPECT_EQ(first_interval.GetEnd(), la->GetBlockLiveRange(&BB(2)).GetEnd());

    auto second_interval = cmp->GetRanges().back();
    auto cast = la->GetInstLifeIntervals(&INS(5));
    EXPECT_EQ(second_interval.GetBegin(), la->GetBlockLiveRange(&BB(3)).GetBegin());
    EXPECT_EQ(second_interval.GetEnd(), cast->GetBegin());
}

TEST_F(LivenessAnalyzerTest, ReturnInlinedLiveness)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(20, 2).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(20).SrcVregs({0});
            INST(3, Opcode::CallStatic).v0id().Inlined().InputsAutoType(2);
            INST(4, Opcode::ReturnInlined).s32().Inputs(2);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::CallStatic).v0id().Inlined().InputsAutoType(5);
            INST(7, Opcode::CallStatic).v0id().Inlined().InputsAutoType(5);
            INST(8, Opcode::SaveStateDeoptimize).Inputs(6, 7).SrcVregs({0, 1});
            INST(9, Opcode::ReturnInlined).s32().Inputs(5);
            INST(10, Opcode::ReturnInlined).s32().Inputs(5);
            INST(13, Opcode::NOP);
            INST(14, Opcode::NOP);
            INST(11, Opcode::Deoptimize).Inputs(8);
            INST(12, Opcode::ReturnVoid).v0id();
        }
    }
    INS(10).CastToReturnInlined()->SetExtendedLiveness();

    auto la = &GetGraph()->GetAnalysis<LivenessAnalyzer>();
    la->Run();
    auto par0_lifetime = la->GetInstLifeIntervals(&INS(0));
    auto par1_lifetime = la->GetInstLifeIntervals(&INS(1));
    auto par2_lifetime = la->GetInstLifeIntervals(&INS(20));
    auto deopt_lifetime = la->GetInstLifeIntervals(&INS(11));
    // 5.SaveState's inputs' liveness should be propagated up to 11.Deoptimize
    EXPECT_GE(par0_lifetime->GetEnd(), deopt_lifetime->GetBegin());
    EXPECT_GE(par1_lifetime->GetEnd(), deopt_lifetime->GetBegin());
    // 2.SaveState's input's liveness should not be propagated
    EXPECT_LT(par2_lifetime->GetEnd(), deopt_lifetime->GetBegin());
}

TEST_F(LivenessAnalyzerTest, LookupInstByLifeNumber)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Add).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Sub).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs(4, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    la.Run();

    EXPECT_EQ(la.GetInstByLifeNumber(la.GetInstLifeIntervals(&INS(4))->GetBegin()), &INS(4));
    EXPECT_EQ(la.GetInstByLifeNumber(la.GetInstLifeIntervals(&INS(4))->GetBegin() + 1), &INS(4));
    EXPECT_EQ(la.GetInstByLifeNumber(la.GetInstLifeIntervals(&INS(6))->GetBegin()), nullptr);
}

TEST_F(LivenessAnalyzerTest, PhiDataFlowInput)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, nullptr).ref();
        PARAMETER(1, 0).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::NullCheck).ref().Inputs(1, 5);
            INST(7, Opcode::LoadObject).s32().Inputs(6);
            INST(8, Opcode::IfImm).SrcType(compiler::DataType::INT32).CC(compiler::CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 1)
        {
            INST(9, Opcode::Phi).ref().Inputs(0, 6);
            INST(10, Opcode::Return).ref().Inputs(9);
        }
    }

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    la.Run();
    auto par0_lifetime = la.GetInstLifeIntervals(&INS(1));
    auto phi_lifetime = la.GetInstLifeIntervals(&INS(9));
    EXPECT_EQ(par0_lifetime->GetEnd(), phi_lifetime->GetBegin());
}

// TODO (a.popov) Enable
TEST_F(LivenessAnalyzerTest, DISABLED_CatchProcessing)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::Try).CatchTypeIds({0x0});
        }

        BASIC_BLOCK(3, 5)
        {
            INST(2, Opcode::Mul).u64().Inputs(0, 0);
            INST(3, Opcode::Mul).u64().Inputs(0, 2);
        }

        BASIC_BLOCK(5, 6, 4) {}  // try-end

        BASIC_BLOCK(4, -1)
        {
            INST(5, Opcode::Return).u64().Inputs(2);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(4, Opcode::Return).u64().Inputs(3);
        }
    }

    GetGraph()->AppendThrowableInst(&INS(3), &BB(5));
    INS(1).CastToTry()->SetTryEndBlock(&BB(5));

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    la.Run();

    EXPECT_EQ(la.GetInstByLifeNumber(la.GetInstLifeIntervals(&INS(2))->GetBegin()), &INS(2));
    EXPECT_EQ(la.GetInstByLifeNumber(la.GetInstLifeIntervals(&INS(3))->GetBegin()), &INS(3));
}

TEST_F(LivenessAnalyzerTest, FirstIntersection)
{
    // li:      [10-20]         [30-40]
    // other:           [21-25]         [45-100]
    // intersection: INVALID_LIFE_NUMBER
    LifeIntervals li(GetAllocator());
    li.AppendRange({30, 40});
    li.AppendRange({10, 20});
    LifeIntervals other_li(GetAllocator());
    other_li.AppendRange({45, 100});
    other_li.AppendRange({21, 25});
    EXPECT_EQ(li.GetFirstIntersectionWith(&other_li), INVALID_LIFE_NUMBER);

    // li:             [21-25] [30-40]
    // other:   [10-20]                 [45-100]
    // intersection: INVALID_LIFE_NUMBER
    li.Clear();
    li.AppendRange({30, 40});
    li.AppendRange({21, 25});
    other_li.Clear();
    other_li.AppendRange({45, 100});
    other_li.AppendRange({10, 20});
    EXPECT_EQ(li.GetFirstIntersectionWith(&other_li), INVALID_LIFE_NUMBER);

    // li:      [10-20]         [30-40]
    // other:       [15-25]         [35-100]
    // intersection: INVALID_LIFE_NUMBER
    li.Clear();
    li.AppendRange({30, 40});
    li.AppendRange({10, 20});
    other_li.Clear();
    other_li.AppendRange({35, 100});
    other_li.AppendRange({15, 25});
    EXPECT_EQ(li.GetFirstIntersectionWith(&other_li), LifeNumber(15));

    // li:          [15-25]         [35-100]
    // other:   [10-20]         [30-40]
    // intersection: INVALID_LIFE_NUMBER
    li.Clear();
    li.AppendRange({35, 100});
    li.AppendRange({15, 25});
    other_li.Clear();
    other_li.AppendRange({30, 40});
    other_li.AppendRange({10, 20});
    EXPECT_EQ(li.GetFirstIntersectionWith(&other_li), LifeNumber(15));

    // li:               [25-35] [45    -    100]
    // other:   [10-20]              [50-60]
    // intersection: INVALID_LIFE_NUMBER
    li.Clear();
    li.AppendRange({45, 100});
    li.AppendRange({25, 35});
    other_li.Clear();
    other_li.AppendRange({50, 60});
    other_li.AppendRange({10, 20});
    EXPECT_EQ(li.GetFirstIntersectionWith(&other_li), LifeNumber(50));

    // li:      [0-10]
    // other:     [6-12]
    // seach_from:   8
    // intersection: 8
    li.Clear();
    li.AppendRange({0, 10});
    other_li.Clear();
    other_li.AppendRange({6, 12});
    EXPECT_EQ(li.GetFirstIntersectionWith(&other_li, 8), LifeNumber(8));

    // li:      [0-10]     [20-30]
    // other:   [0-2]   [18-24]
    // search_from: 2
    // intersection: 20
    li.Clear();
    li.AppendRange({20, 30});
    li.AppendRange({0, 10});
    other_li.Clear();
    other_li.AppendRange({18, 24});
    other_li.AppendRange({0, 2});
    EXPECT_EQ(li.GetFirstIntersectionWith(&other_li, 2), LifeNumber(20));

    // li:         [10-20]
    // other:    [8-30]
    // search_from: 18
    // intersection: 18
    li.Clear();
    li.AppendRange({10, 20});
    other_li.Clear();
    other_li.AppendRange({8, 30});
    EXPECT_EQ(li.GetFirstIntersectionWith(&other_li, 18), LifeNumber(18));

    // li:      [0-10]  [20-22]       [24-26]
    // other:   [0-2]          [22-24]
    // search_from: 12
    // intersection: INVALID_LIFE_NUMBER
    li.Clear();
    li.AppendRange({24, 26});
    li.AppendRange({20, 22});
    li.AppendRange({0, 10});
    other_li.Clear();
    other_li.AppendRange({22, 24});
    other_li.AppendRange({0, 2});
    EXPECT_EQ(li.GetFirstIntersectionWith(&other_li, 12), INVALID_LIFE_NUMBER);
}

TEST_F(LivenessAnalyzerTest, NextUsePositions)
{
    LifeIntervals li(GetAllocator());
    li.AppendRange({0, 100});
    li.AddUsePosition(20);
    li.AddUsePosition(50);
    li.AddUsePosition(75);
    li.AddUsePosition(100);

    EXPECT_EQ(li.GetNextUsage(0), 20);
    EXPECT_EQ(li.GetNextUsage(20), 20);
    EXPECT_EQ(li.GetNextUsage(30), 50);
    EXPECT_EQ(li.GetNextUsage(50), 50);
    EXPECT_EQ(li.GetNextUsage(70), 75);
    EXPECT_EQ(li.GetNextUsage(75), 75);
    EXPECT_EQ(li.GetNextUsage(90), 100);
    EXPECT_EQ(li.GetNextUsage(100), 100);
    EXPECT_EQ(li.GetNextUsage(150), INVALID_LIFE_NUMBER);
}

TEST_F(LivenessAnalyzerTest, NoUsageUntil)
{
    LifeIntervals li(GetAllocator());
    li.AppendRange({0, 100});
    li.AddUsePosition(20);
    li.AddUsePosition(50);
    EXPECT_TRUE(li.NoUsageUntil(0));
    EXPECT_TRUE(li.NoUsageUntil(19));
    EXPECT_FALSE(li.NoUsageUntil(20));
    EXPECT_FALSE(li.NoUsageUntil(21));
    EXPECT_FALSE(li.NoUsageUntil(100));
}

TEST_F(LivenessAnalyzerTest, SplitUsePositions)
{
    LifeIntervals li(GetAllocator());
    li.AppendRange({0, 200});
    li.AddUsePosition(20);
    li.AddUsePosition(50);
    li.AddUsePosition(75);
    li.AddUsePosition(100);

    auto split = li.SplitAt(50, GetAllocator());
    EXPECT_THAT(li.GetUsePositions(), ::testing::ElementsAre(20));
    EXPECT_THAT(split->GetUsePositions(), ::testing::ElementsAre(50, 75, 100));

    auto next_spit = split->SplitAt(150, GetAllocator());
    EXPECT_TRUE(next_spit->GetUsePositions().empty());
    EXPECT_THAT(split->GetUsePositions(), ::testing::ElementsAre(50, 75, 100));
}

TEST_F(LivenessAnalyzerTest, PropagateLivenessForImplicitNullCheckSaveStateInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::AddI).s32().Imm(1U).Inputs(1);
            INST(5, Opcode::LenArray).s32().Inputs(3);
            INST(6, Opcode::Add).s32().Inputs(4, 5);
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }
    INS(3).CastToNullCheck()->SetImplicit(true);

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    ASSERT_TRUE(la.Run());

    auto param = la.GetInstLifeIntervals(&INS(1));
    auto len = la.GetInstLifeIntervals(&INS(5));
    ASSERT_GE(param->GetEnd(), len->GetBegin());
}

TEST_F(LivenessAnalyzerTest, PropagateLivenessForExplicitNullCheckSaveStateInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::AddI).s32().Imm(1U).Inputs(1);
            INST(5, Opcode::LenArray).s32().Inputs(3);
            INST(6, Opcode::Add).s32().Inputs(4, 5);
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }
    INS(3).CastToNullCheck()->SetImplicit(false);

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    ASSERT_TRUE(la.Run());

    auto param = la.GetInstLifeIntervals(&INS(1));
    auto addi = la.GetInstLifeIntervals(&INS(4));
    ASSERT_EQ(param->GetEnd(), addi->GetBegin());
}

TEST_F(LivenessAnalyzerTest, NullCheckWithoutUsers)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(2, Opcode::CallStatic).ref().InputsAutoType(0, 20);
            INST(3, Opcode::SaveState).Inputs(2).SrcVregs({0});
            INST(4, Opcode::NullCheck).ref().Inputs(2, 3);
            INST(5, Opcode::Return).s32().Inputs(1);
        }
    }
    BB(2).SetTry(true);
    INS(4).CastToNullCheck()->SetImplicit(true);

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    ASSERT_TRUE(la.Run());

    auto call = la.GetInstLifeIntervals(&INS(2));
    auto null_check = la.GetInstLifeIntervals(&INS(4));
    ASSERT_EQ(call->GetEnd(), null_check->GetBegin() + 1U);
}
}  // namespace panda::compiler
