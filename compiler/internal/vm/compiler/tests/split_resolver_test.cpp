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
#include "compiler/optimizer/optimizations/regalloc/split_resolver.h"
#include "compiler/optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "compiler/optimizer/analysis/liveness_analyzer.h"

#define INITIALIZE_GRAPHS(G_BEFORE, G_AFTER)                    \
    for (auto __after_resolve : {true, false})                  \
        if (auto ___g = (__after_resolve ? G_AFTER : G_BEFORE)) \
    GRAPH(___g)

#define AFTER_SPLIT_RESOLUTION(OP) \
    if (__after_resolve)           \
    OP

namespace panda::compiler {
class SplitResolverTest : public GraphTest {
public:
    LivenessAnalyzer *RunLivenessAnalysis(Graph *graph)
    {
        graph->RunPass<LivenessAnalyzer>();
        return &graph->GetAnalysis<LivenessAnalyzer>();
    }

    LifeIntervals *SplitAssignReg(LifeIntervals *source, LifeNumber position, Register reg)
    {
        auto split = source->SplitAt(position - 1, GetAllocator());
        split->SetReg(reg);
        return split;
    }

    LifeIntervals *SplitAssignSlot(LifeIntervals *source, LifeNumber position, StackSlot slot)
    {
        auto split = source->SplitAt(position - 1, GetAllocator());
        split->SetLocation(Location::MakeStackSlot(slot));
        return split;
    }

    LifeIntervals *SplitAssignImmSlot(LifeIntervals *source, LifeNumber position, ImmTableSlot slot)
    {
        ASSERT(source->GetInst()->IsConst());
        auto split = source->SplitAt(position - 1, GetAllocator());
        split->SetLocation(Location::MakeConstant(slot));
        return split;
    }

    void CheckSpillFills(Inst *inst,
                         std::initializer_list<std::tuple<LocationType, LocationType, Register, Register>> data)
    {
        ASSERT_EQ(inst->GetOpcode(), Opcode::SpillFill);
        auto sf = inst->CastToSpillFill();

        for (auto &[src_loc, dst_loc, src, dst] : data) {
            bool found = false;
            for (auto &sf_data : sf->GetSpillFills()) {
                found |= sf_data.SrcType() == src_loc && sf_data.DstType() == dst_loc && sf_data.SrcValue() == src &&
                         sf_data.DstValue() == dst;
            }
            EXPECT_TRUE(found) << "SpillFillData {move, src=" << static_cast<int>(src)
                               << ", dest=" << static_cast<int>(dst) << "} was not found in inst " << inst->GetId();
        }
    }

    Graph *InitUsedRegs(Graph *graph)
    {
        ArenaVector<bool> regs =
            ArenaVector<bool>(std::max(MAX_NUM_REGS, MAX_NUM_VREGS), false, GetAllocator()->Adapter());
        graph->InitUsedRegs<DataType::INT64>(&regs);
        graph->InitUsedRegs<DataType::FLOAT64>(&regs);
        return graph;
    }
};

TEST_F(SplitResolverTest, ProcessIntervalsWithoutSplit)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Add).u64().Inputs(0, 0);
            INST(2, Opcode::Return).u64().Inputs(1);
        }
    }

    SplitResolver resolver(initial_graph, RunLivenessAnalysis(initial_graph));
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
}

TEST_F(SplitResolverTest, ConnectSiblingsWithSameBlock)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            AFTER_SPLIT_RESOLUTION(INST(4, Opcode::SpillFill));
            INST(1, Opcode::Add).u64().Inputs(0, 0);
            AFTER_SPLIT_RESOLUTION(INST(5, Opcode::SpillFill));
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);

    auto param = la->GetInstLifeIntervals(&INS(0));
    auto add = la->GetInstLifeIntervals(&INS(1));
    param->SetReg(0);

    SplitAssignReg(SplitAssignSlot(param, add->GetBegin(), 0), add->GetEnd(), 1);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(1).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0}});
    CheckSpillFills(INS(1).GetNext(), {{LocationType::STACK, LocationType::REGISTER, 0, 1}});
}

TEST_F(SplitResolverTest, ConnectSiblingsInDifferentBlocks)
{
    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 6)
        {
            INST(1, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 0);
            INST(2, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::SpillFill);
            INST(4, Opcode::CallStatic).v0id().InputsAutoType(3);
        }

        BASIC_BLOCK(6, 4)
        {
            INST(9, Opcode::SpillFill);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(5);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(7, Opcode::Return).u64().Inputs(0);
        }
    }

    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    GRAPH(initial_graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 0);
            INST(2, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(4, Opcode::CallStatic).v0id().InputsAutoType(3);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(5);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(7, Opcode::Return).u64().Inputs(0);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);

    auto param = la->GetInstLifeIntervals(&INS(0));
    auto call = la->GetInstLifeIntervals(&INS(4));
    param->SetReg(0);

    SplitAssignSlot(param, call->GetBegin(), 0);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(4).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0}});
    CheckSpillFills(BB(4).GetPredsBlocks()[0]->GetLastInst(), {{LocationType::REGISTER, LocationType::STACK, 0, 0}});
}

TEST_F(SplitResolverTest, ConnectSiblingsHavingCriticalEdgeBetweenBlocks)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();
    GRAPH(initial_graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 0);
            INST(2, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(4, Opcode::CallStatic).v0id().InputsAutoType(3);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(5, Opcode::Return).u64().Inputs(0);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);

    auto param = la->GetInstLifeIntervals(&INS(0));
    auto call = la->GetInstLifeIntervals(&INS(4));
    param->SetReg(0);

    SplitAssignSlot(param, call->GetBegin(), 0);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 5)
        {
            INST(1, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 0);
            INST(2, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::SpillFill);
            INST(4, Opcode::CallStatic).v0id().InputsAutoType(3);
        }

        BASIC_BLOCK(5, 4)
        {
            INST(7, Opcode::SpillFill);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(5, Opcode::Return).u64().Inputs(0);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    size_t spill_fills = 0;
    for (auto block : initial_graph->GetVectorBlocks()) {
        if (block == nullptr) {
            continue;
        }
        for (auto inst : block->AllInsts()) {
            if (inst->GetOpcode() == Opcode::SpillFill) {
                CheckSpillFills(inst, {{LocationType::REGISTER, LocationType::STACK, 0, 0}});
                spill_fills++;
            }
        }
    }
    EXPECT_EQ(spill_fills, 2);
}

TEST_F(SplitResolverTest, SplitAtTheEndOfBlock)
{
    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 7)
        {
            INST(1, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 0);
            INST(2, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(3, Opcode::Add).u64().Inputs(0, 0);
            INST(8, Opcode::SpillFill);
        }

        BASIC_BLOCK(7, 4)
        {
            INST(9, Opcode::SpillFill);
        }

        BASIC_BLOCK(4, 6)
        {
            INST(4, Opcode::Mul).u64().Inputs(0, 0);
        }

        BASIC_BLOCK(5, 6)
        {
            INST(7, Opcode::Add).u64().Inputs(3, 0);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(5, Opcode::Phi).u64().Inputs(4, 7);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }

    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    GRAPH(initial_graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 0);
            INST(2, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(3, Opcode::Add).u64().Inputs(0, 0);
        }

        BASIC_BLOCK(4, 6)
        {
            INST(4, Opcode::Mul).u64().Inputs(0, 0);
        }

        BASIC_BLOCK(5, 6)
        {
            INST(7, Opcode::Add).u64().Inputs(3, 0);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(5, Opcode::Phi).u64().Inputs(4, 7);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);
    auto param = la->GetInstLifeIntervals(&INS(0));
    auto bb3 = la->GetBlockLiveRange(&BB(3));
    param->SetReg(0);
    SplitAssignSlot(param, bb3.GetEnd(), 0);
    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(4))->SetReg(1);
    la->GetInstLifeIntervals(&INS(5))->SetReg(1);
    la->GetInstLifeIntervals(&INS(7))->SetReg(1);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();
    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(3).GetNext(), {{LocationType::REGISTER, LocationType::STACK, 0, 0}});
    CheckSpillFills(BB(4).GetPredsBlocks()[0]->GetLastInst(), {{LocationType::REGISTER, LocationType::STACK, 0, 0}});
}

// If we already inserted spill fill instruction for some spill
// the we can reuse it for another one.
TEST_F(SplitResolverTest, ReuseExistingSpillFillWithinBlock)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            AFTER_SPLIT_RESOLUTION(INST(6, Opcode::SpillFill));
            INST(3, Opcode::CallStatic).v0id().InputsAutoType(2);
            AFTER_SPLIT_RESOLUTION(INST(7, Opcode::SpillFill));
            INST(4, Opcode::Add).u64().Inputs(0, 1);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);

    auto param0 = la->GetInstLifeIntervals(&INS(0));
    auto param1 = la->GetInstLifeIntervals(&INS(1));
    auto call = la->GetInstLifeIntervals(&INS(3));
    param0->SetReg(0);
    param1->SetReg(1);

    SplitAssignReg(SplitAssignSlot(param0, call->GetBegin(), 0), call->GetEnd(), 0);
    SplitAssignReg(SplitAssignSlot(param1, call->GetBegin(), 1), call->GetEnd(), 1);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(3).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0},
                                       {LocationType::REGISTER, LocationType::STACK, 1, 1}});
    CheckSpillFills(INS(3).GetNext(), {{LocationType::STACK, LocationType::REGISTER, 0, 0},
                                       {LocationType::STACK, LocationType::REGISTER, 1, 1}});
}

// If there are spill fills inserted to load instn's operand or spill instn's result
// then we can't reuse these spill fills to connect splits.
TEST_F(SplitResolverTest, DontReuseInstructionSpillFills)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            AFTER_SPLIT_RESOLUTION(INST(11, Opcode::SpillFill));
            INST(4, Opcode::SpillFill);  // spill fill loading
            INST(5, Opcode::AddI).u64().Imm(42).Inputs(2);
            AFTER_SPLIT_RESOLUTION(INST(12, Opcode::SpillFill));
            INST(7, Opcode::SpillFill);  // spill fill loading
            INST(8, Opcode::Add).u64().Inputs(0, 5);
            INST(9, Opcode::Add).u64().Inputs(8, 1);
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);

    auto param0 = la->GetInstLifeIntervals(&INS(0));
    auto param1 = la->GetInstLifeIntervals(&INS(1));
    auto param2 = la->GetInstLifeIntervals(&INS(2));
    auto addi = la->GetInstLifeIntervals(&INS(5));
    auto add = la->GetInstLifeIntervals(&INS(8));
    param0->SetReg(0);
    param1->SetReg(1);
    param2->SetLocation(Location::MakeStackSlot(4));
    addi->SetLocation(Location::MakeStackSlot(3));

    INS(4).CastToSpillFill()->SetSpillFillType(SpillFillType::INPUT_FILL);
    INS(7).CastToSpillFill()->SetSpillFillType(SpillFillType::INPUT_FILL);

    SplitAssignReg(SplitAssignSlot(param0, addi->GetBegin(), 0), add->GetBegin(), 0);
    SplitAssignReg(SplitAssignSlot(param1, addi->GetBegin(), 1), add->GetBegin(), 1);

    INS(4).CastToSpillFill()->AddFill(param2->GetLocation().GetValue(), 11, DataType::Type::INT64);
    INS(7).CastToSpillFill()->AddFill(addi->GetLocation().GetValue(), 11, DataType::Type::INT64);
    INS(5).SetSrcReg(0, 11);
    INS(5).SetDstReg(11);
    INS(8).SetSrcReg(0, 0);
    INS(8).SetSrcReg(1, 11);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(4).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0},
                                       {LocationType::REGISTER, LocationType::STACK, 1, 1}});
    CheckSpillFills(INS(7).GetPrev(), {{LocationType::STACK, LocationType::REGISTER, 0, 0},
                                       {LocationType::STACK, LocationType::REGISTER, 1, 1}});
}

TEST_F(SplitResolverTest, DoNotReuseExistingSpillFillBeforeInstruction)
{
    auto initial_graph = CreateEmptyGraph();
    auto expected_graph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            AFTER_SPLIT_RESOLUTION(INST(6, Opcode::SpillFill));
            INST(5, Opcode::SpillFill);
            INST(3, Opcode::Mul).u64().Inputs(0, 2);
            INST(4, Opcode::Return).u64().Inputs(3);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);

    auto param0 = la->GetInstLifeIntervals(&INS(0));
    auto mul = la->GetInstLifeIntervals(&INS(3));
    param0->SetReg(0);
    SplitAssignSlot(param0, mul->GetBegin(), 0);

    auto mul_sf = INS(5).CastToSpillFill();
    mul_sf->AddFill(0, 1, DataType::Type::UINT64);
    mul_sf->SetSpillFillType(SpillFillType::INPUT_FILL);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(2).GetNext(), {{LocationType::REGISTER, LocationType::STACK, 0, 0}});
    CheckSpillFills(&INS(5), {{LocationType::STACK, LocationType::REGISTER, 0, 1}});
}

TEST_F(SplitResolverTest, ReuseExistingSpillFillAtTheEndOfBlock)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initial_graph, expected_graph)
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
            INST(9, Opcode::SpillFill);  // SF generated for PHI
        }

        BASIC_BLOCK(4, 5)
        {
            AFTER_SPLIT_RESOLUTION(INST(11, Opcode::SpillFill));
            INST(5, Opcode::Mul).u64().Inputs(1, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs(4, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 0);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);

    auto phi = la->GetInstLifeIntervals(&INS(6));
    phi->SetReg(3);
    auto mul = la->GetInstLifeIntervals(&INS(5));
    mul->SetReg(3);
    auto add = la->GetInstLifeIntervals(&INS(4));
    add->SetReg(2);

    auto param0 = la->GetInstLifeIntervals(&INS(0));
    param0->SetReg(1);

    SplitAssignSlot(param0, mul->GetBegin(), 0);

    auto phi_sf = INS(9).CastToSpillFill();
    // param0 still has r1 assigned at the end of BB3, so PHI's SF will move it from r1 to r3 assigned to PHI
    phi_sf->AddMove(1, 3, DataType::Type::UINT64);
    phi_sf->SetSpillFillType(SpillFillType::SPLIT_MOVE);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(&INS(9), {{LocationType::REGISTER, LocationType::STACK, 1, 0}});
    CheckSpillFills(INS(5).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 1, 0}});
}

TEST_F(SplitResolverTest, ConnectSplitAtTheEndOfBlock)
{
    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Add).u64().Inputs(0, 1);
            // Insert copy instruction before φ-move, because
            // inst 4 should be already copied at the end of block (where φ inserts move).
            INST(10, Opcode::SpillFill);
            INST(9, Opcode::SpillFill);  // SF generated for PHI
        }

        BASIC_BLOCK(6, 4)
        {
            INST(11, Opcode::SpillFill);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Mul).u64().Inputs(1, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs(4, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 0);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    GRAPH(initial_graph)
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
            INST(9, Opcode::SpillFill);  // SF generated for PHI
        }

        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Mul).u64().Inputs(1, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs(4, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 0);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);

    auto phi = la->GetInstLifeIntervals(&INS(6));
    phi->SetReg(3);
    auto add = la->GetInstLifeIntervals(&INS(4));
    add->SetReg(1);
    auto mul = la->GetInstLifeIntervals(&INS(5));
    mul->SetReg(3);

    auto param0 = la->GetInstLifeIntervals(&INS(0));
    param0->SetReg(1);

    SplitAssignSlot(param0, add->GetBegin() + 2, 0);

    auto phi_sf = INS(9).CastToSpillFill();
    // param0 still has r1 assigned at the end of BB3, so PHI's SF will move it from r1 to r3 assigned to PHI
    phi_sf->AddMove(1, 3, DataType::Type::UINT64);
    phi_sf->SetSpillFillType(SpillFillType::SPLIT_MOVE);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(4).GetNext(), {{LocationType::REGISTER, LocationType::STACK, 1, 0}});
}

TEST_F(SplitResolverTest, GracefullyHandlePhiResolverBlocks)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    GRAPH(initial_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, 5, 3)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5, 4)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_LE).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(5, 6)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(4, 6)
        {
            INST(7, Opcode::Sub).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(8, Opcode::Phi).u64().Inputs(6, 7);
            INST(9, Opcode::Add).u64().Inputs(0, 8);
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);
    auto pred = &BB(5);
    auto succ = &BB(6);
    auto phi_resolver = pred->InsertNewBlockToSuccEdge(succ);
    auto sf0 = initial_graph->CreateInstSpillFill();
    sf0->SetSpillFillType(SpillFillType::SPLIT_MOVE);
    phi_resolver->PrependInst(sf0);

    auto param0 = la->GetInstLifeIntervals(&INS(0));
    param0->SetReg(0);
    auto sub = la->GetInstLifeIntervals(&INS(7));
    SplitAssignReg(param0, sub->GetBegin(), 4);

    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(6))->SetReg(1);
    la->GetInstLifeIntervals(&INS(7))->SetReg(1);
    la->GetInstLifeIntervals(&INS(8))->SetReg(1);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, 5, 3)
        {
            INST(2, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5, 4)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_LE).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(5, 7)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(7, 6)
        {
            // Single spill fill handling both split connection and φ-move
            INST(12, Opcode::SpillFill);
        }

        BASIC_BLOCK(4, 6)
        {
            INST(11, Opcode::SpillFill);  // resolve split
            INST(7, Opcode::Sub).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(8, Opcode::Phi).u64().Inputs(6, 7);
            INST(9, Opcode::Add).u64().Inputs(0, 8);
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(sf0, {{LocationType::REGISTER, LocationType::REGISTER, 0, 4}});
    CheckSpillFills(sub->GetInst()->GetPrev(), {{LocationType::REGISTER, LocationType::REGISTER, 0, 4}});
}

TEST_F(SplitResolverTest, ResolveSplitWithinLoop)
{
    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, 3)
        {
            INST(4, Opcode::Add).u64().Inputs(0, 1);
            INST(5, Opcode::Add).u64().Inputs(1, 4);
            INST(12, Opcode::SpillFill);
            INST(6, Opcode::Add).u64().Inputs(4, 5);
        }

        BASIC_BLOCK(3, 6, 5)
        {
            INST(7, Opcode::Mul).u64().Inputs(4, 5);
            INST(13, Opcode::SpillFill);
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(6, 7);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(7);
        }

        BASIC_BLOCK(6, 3)
        {
            INST(14, Opcode::SpillFill);
        }
    }

    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    GRAPH(initial_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, 3)
        {
            INST(4, Opcode::Add).u64().Inputs(0, 1);
            INST(5, Opcode::Add).u64().Inputs(1, 4);
            INST(6, Opcode::Add).u64().Inputs(4, 5);
        }

        BASIC_BLOCK(3, 3, 5)
        {
            INST(7, Opcode::Mul).u64().Inputs(4, 5);
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(6, 7);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(7);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);
    auto var0 = la->GetInstLifeIntervals(&INS(4));
    var0->SetLocation(Location::MakeStackSlot(0));
    SplitAssignSlot(SplitAssignReg(var0, la->GetInstLifeIntervals(&INS(6))->GetBegin(), 4),
                    la->GetInstLifeIntervals(&INS(8))->GetBegin(), 0);
    auto var1 = la->GetInstLifeIntervals(&INS(5));
    var1->SetLocation(Location::MakeStackSlot(1));
    SplitAssignSlot(SplitAssignReg(var1, la->GetInstLifeIntervals(&INS(6))->GetBegin(), 5),
                    la->GetInstLifeIntervals(&INS(8))->GetBegin(), 1);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(6).GetPrev(), {{LocationType::STACK, LocationType::REGISTER, 0, 4},
                                       {LocationType::STACK, LocationType::REGISTER, 1, 5}});
    CheckSpillFills(INS(8).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 4, 0},
                                       {LocationType::REGISTER, LocationType::STACK, 5, 1}});
    auto resolver_block = initial_graph->GetVectorBlocks().back();
    CheckSpillFills(resolver_block->GetLastInst(), {{LocationType::STACK, LocationType::REGISTER, 0, 4},
                                                    {LocationType::STACK, LocationType::REGISTER, 1, 5}});
}

TEST_F(SplitResolverTest, SkipIntervalsCoveringOnlyBlockStart)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 2);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 0);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, 5, 4)
        {
            INST(5, Opcode::Phi).u64().Inputs(1, 8);
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(5, 5);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(5, 3)
        {
            // split (r1)
            AFTER_SPLIT_RESOLUTION(INST(11, Opcode::SpillFill));
            INST(8, Opcode::Sub).u64().Inputs(5, 0);
            // copy to location @ begging BB 3 (r0)
            AFTER_SPLIT_RESOLUTION(INST(12, Opcode::SpillFill));
        }

        // Interval for parameter 0 is live in loop's header (BB 3), so its live range
        // will be prolonged until the end of loop (i.e. until the end of BB 5). As a result
        // parameter 0 will be covering start of the BB 4 and the location of its life interval
        // at the beginning of BB 4 will differ from location at the end of BB 2 (reg1 vs reg0).
        // However, we should not insert spill fill in this case.
        // param0 is not actually alive at BB 4 and the same register may be assigned to a phi (inst 9),
        // so the move will corrupt Phi's value. If param0 is the Phi's input then it'll be copied
        // by the phi-move inserted during Phi resolution and the spill-fill connecting sibling intervals
        // is not required too.
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs(2, 5);
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);

    auto param0 = la->GetInstLifeIntervals(&INS(0));
    param0->SetReg(0);
    SplitAssignReg(param0, la->GetInstLifeIntervals(&INS(8))->GetBegin(), 1);

    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(1))->SetReg(1);
    la->GetInstLifeIntervals(&INS(2))->SetReg(1);
    la->GetInstLifeIntervals(&INS(5))->SetReg(1);
    la->GetInstLifeIntervals(&INS(8))->SetReg(1);
    la->GetInstLifeIntervals(&INS(9))->SetReg(1);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
}

TEST_F(SplitResolverTest, ConnectIntervalsForConstantWithinBlock)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        CONSTANT(0, 42);

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::AddI).u64().Imm(1).Inputs(0);
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            AFTER_SPLIT_RESOLUTION(INST(6, Opcode::SpillFill));
            INST(3, Opcode::Add).u64().Inputs(0, 2);
            INST(4, Opcode::Add).u64().Inputs(0, 3);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);
    auto con = la->GetInstLifeIntervals(&INS(0));
    con->SetReg(0);
    SplitAssignReg(SplitAssignImmSlot(SplitAssignImmSlot(con, la->GetInstLifeIntervals(&INS(1))->GetBegin(), 0),
                                      la->GetInstLifeIntervals(&INS(2))->GetBegin(), 0),
                   la->GetInstLifeIntervals(&INS(3))->GetBegin(), 0);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(3).GetPrev(), {{LocationType::IMMEDIATE, LocationType::REGISTER, 0, 0}});
}

TEST_F(SplitResolverTest, ConnectIntervalsForConstantBetweenBlock)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        CONSTANT(0, 42);
        CONSTANT(1, 64);
        PARAMETER(2, 0).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(2, 2);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Add).u64().Inputs(2, 2);
            AFTER_SPLIT_RESOLUTION(INST(11, Opcode::SpillFill));
        }

        BASIC_BLOCK(4, 5)
        {
            INST(6, Opcode::Mul).u64().Inputs(2, 2);
            AFTER_SPLIT_RESOLUTION(INST(12, Opcode::SpillFill));
        }

        BASIC_BLOCK(5, -1)
        {
            INST(7, Opcode::Phi).u64().Inputs(5, 6);
            INST(8, Opcode::Add).u64().Inputs(0, 7);
            INST(9, Opcode::Add).u64().Inputs(1, 8);
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);
    auto con0 = la->GetInstLifeIntervals(&INS(0));
    auto con1 = la->GetInstLifeIntervals(&INS(1));
    con0->SetReg(0);
    con1->SetReg(1);
    SplitAssignImmSlot(con0, la->GetInstLifeIntervals(&INS(7))->GetBegin(), 0);
    SplitAssignReg(SplitAssignImmSlot(con1, la->GetInstLifeIntervals(&INS(3))->GetBegin(), 0),
                   la->GetInstLifeIntervals(&INS(7))->GetBegin(), 2);
    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(5))->SetReg(2);
    la->GetInstLifeIntervals(&INS(6))->SetReg(2);
    la->GetInstLifeIntervals(&INS(7))->SetReg(2);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();
    CheckSpillFills(INS(5).GetNext(), {{LocationType::IMMEDIATE, LocationType::REGISTER, 0, 2}});
    CheckSpillFills(INS(6).GetNext(), {{LocationType::IMMEDIATE, LocationType::REGISTER, 0, 2}});
}

TEST_F(SplitResolverTest, DontReuseSpillFillForConstant)
{
    auto initial_graph = InitUsedRegs(CreateEmptyGraph());
    auto expected_graph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        CONSTANT(0, 42);

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::AddI).u64().Imm(1).Inputs(0);
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            AFTER_SPLIT_RESOLUTION(INST(3, Opcode::SpillFill));
            INST(4, Opcode::SpillFill);
            INST(5, Opcode::Add).u64().Inputs(0, 2);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);
    la->GetInstLifeIntervals(&INS(0))->SetLocation(Location::MakeConstant(0));
    la->GetInstLifeIntervals(&INS(2))->SetReg(1);
    SplitAssignSlot(la->GetInstLifeIntervals(&INS(2)), la->GetInstLifeIntervals(&INS(5))->GetBegin(), 0);
    INS(4).CastToSpillFill()->AddSpillFill(Location::MakeConstant(0), Location::MakeRegister(1), DataType::Type::INT64);
    INS(4).CastToSpillFill()->SetSpillFillType(SpillFillType::INPUT_FILL);

    SplitResolver resolver(initial_graph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(2).GetNext(), {{LocationType::REGISTER, LocationType::STACK, 1, 0}});
}

TEST_F(SplitResolverTest, AppendSpillFIllBeforeLoadArrayPairI)
{
    auto initial_graph = CreateEmptyGraph();
    auto expected_graph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 42);
        CONSTANT(2, 24);

        BASIC_BLOCK(2, -1)
        {
            AFTER_SPLIT_RESOLUTION(INST(13, Opcode::SpillFill));
            INST(6, Opcode::LoadArrayPairI).u64().Inputs(0).Imm(0x0);
            INST(7, Opcode::LoadPairPart).u64().Inputs(6).Imm(0x0);
            INST(8, Opcode::LoadPairPart).u64().Inputs(6).Imm(0x1);
            INST(9, Opcode::Add).u64().Inputs(7, 8);
            INST(10, Opcode::Add).u64().Inputs(1, 2);
            INST(11, Opcode::Add).u64().Inputs(9, 10);
            INST(12, Opcode::Return).u64().Inputs(11);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);
    la->GetInstLifeIntervals(&INS(1))->SetReg(1);
    la->GetInstLifeIntervals(&INS(2))->SetReg(2);
    // Split constants before LoadPairParts
    SplitAssignSlot(la->GetInstLifeIntervals(&INS(1)), la->GetInstLifeIntervals(&INS(7))->GetBegin(), 0);
    SplitAssignSlot(la->GetInstLifeIntervals(&INS(2)), la->GetInstLifeIntervals(&INS(8))->GetBegin(), 0);
    la->GetInstLifeIntervals(&INS(7))->SetReg(1);
    la->GetInstLifeIntervals(&INS(8))->SetReg(1);
    SplitResolver resolver(initial_graph, la);
    resolver.Run();
    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
}

TEST_F(SplitResolverTest, SplitAfterLastInstruction)
{
    auto initial_graph = CreateEmptyGraph();
    auto expected_graph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Phi).u64().Inputs(1, 6);
            INST(3, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(2, 0);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Return).u64().Inputs(2);
        }

        BASIC_BLOCK(4, 2)
        {
            INST(6, Opcode::AddI).u64().Imm(1).Inputs(2);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);
    la->GetInstLifeIntervals(&INS(0))->SetReg(0);
    la->GetInstLifeIntervals(&INS(1))->SetReg(2);
    la->GetInstLifeIntervals(&INS(2))->SetReg(2);
    la->GetInstLifeIntervals(&INS(3))->SetReg(3);
    la->GetInstLifeIntervals(&INS(6))->SetReg(2);
    SplitAssignSlot(la->GetInstLifeIntervals(&INS(0)), la->GetInstLifeIntervals(&INS(6))->GetEnd(), 0);

    InitUsedRegs(initial_graph);
    SplitResolver resolver(initial_graph, la);
    resolver.Run();
    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
}

// TODO (a.popov) Merge equal spill-fills
TEST_F(SplitResolverTest, MultipleEndBlockMoves)
{
    auto initial_graph = CreateEmptyGraph();
    auto expected_graph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initial_graph, expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(3, 5)
        {
            AFTER_SPLIT_RESOLUTION(INST(12, Opcode::SpillFill));
            INST(6, Opcode::Add).u64().Inputs(2, 3);
            AFTER_SPLIT_RESOLUTION(INST(13, Opcode::SpillFill));
        }

        BASIC_BLOCK(4, 5)
        {
            AFTER_SPLIT_RESOLUTION(INST(15, Opcode::SpillFill));
            INST(7, Opcode::Mul).u64().Inputs(2, 3);
            AFTER_SPLIT_RESOLUTION(INST(14, Opcode::SpillFill));
        }

        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Phi).u64().Inputs(6, 7);
            INST(9, Opcode::Add).u64().Inputs(0, 1);
            INST(10, Opcode::Add).u64().Inputs(8, 9);
            INST(11, Opcode::Return).u64().Inputs(10);
        }
    }

    auto la = RunLivenessAnalysis(initial_graph);
    auto p0 = la->GetInstLifeIntervals(&INS(0));
    auto p1 = la->GetInstLifeIntervals(&INS(1));
    p0->SetReg(0);
    p1->SetReg(1);
    SplitAssignReg(SplitAssignSlot(p0, la->GetInstLifeIntervals(&INS(5))->GetEnd(), 0),
                   la->GetInstLifeIntervals(&INS(8))->GetBegin(), 0);
    SplitAssignReg(SplitAssignSlot(p1, la->GetInstLifeIntervals(&INS(5))->GetEnd(), 1),
                   la->GetInstLifeIntervals(&INS(8))->GetBegin(), 1);
    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(6))->SetReg(2);
    la->GetInstLifeIntervals(&INS(7))->SetReg(2);
    la->GetInstLifeIntervals(&INS(8))->SetReg(2);

    InitUsedRegs(initial_graph);
    SplitResolver resolver(initial_graph, la);
    resolver.Run();
    initial_graph->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(initial_graph, expected_graph));
    CheckSpillFills(INS(6).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0},
                                       {LocationType::REGISTER, LocationType::STACK, 1, 1}});
    CheckSpillFills(INS(6).GetNext(), {{LocationType::STACK, LocationType::REGISTER, 0, 0},
                                       {LocationType::STACK, LocationType::REGISTER, 1, 1}});
    CheckSpillFills(INS(7).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0},
                                       {LocationType::REGISTER, LocationType::STACK, 1, 1}});
    CheckSpillFills(INS(7).GetNext(), {{LocationType::STACK, LocationType::REGISTER, 0, 0},
                                       {LocationType::STACK, LocationType::REGISTER, 1, 1}});
}
}  // namespace panda::compiler
