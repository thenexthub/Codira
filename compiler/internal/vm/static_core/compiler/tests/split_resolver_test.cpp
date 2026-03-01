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
#include "compiler/optimizer/optimizations/regalloc/split_resolver.h"
#include "compiler/optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "compiler/optimizer/analysis/liveness_analyzer.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INITIALIZE_GRAPHS(G_BEFORE, G_AFTER)                        \
    for (auto __after_resolve : {true, false})                      \
        if (auto ___g = (__after_resolve ? (G_AFTER) : (G_BEFORE))) \
    GRAPH(___g)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define AFTER_SPLIT_RESOLUTION(OP) \
    if (__after_resolve)           \
    (OP)

namespace ark::compiler {
class SplitResolverTest : public GraphTest {
public:
    LivenessAnalyzer *RunLivenessAnalysis(Graph *graph)
    {
        graph->RunPass<LivenessAnalyzer>();
        return &graph->GetAnalysis<LivenessAnalyzer>();
    }

    LifeIntervals *SplitAssignReg(LifeIntervals *source, LifeNumber position, Register reg)
    {
        auto split = source->SplitAt(position - 1L, GetAllocator());
        split->SetReg(reg);
        return split;
    }

    LifeIntervals *SplitAssignSlot(LifeIntervals *source, LifeNumber position, StackSlot slot)
    {
        auto split = source->SplitAt(position - 1L, GetAllocator());
        split->SetLocation(Location::MakeStackSlot(slot));
        return split;
    }

    LifeIntervals *SplitAssignImmSlot(LifeIntervals *source, LifeNumber position, ImmTableSlot slot)
    {
        ASSERT(source->GetInst()->IsConst());
        auto split = source->SplitAt(position - 1L, GetAllocator());
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
            for (auto &sfData : sf->GetSpillFills()) {
                found |= sfData.SrcType() == src_loc && sfData.DstType() == dst_loc && sfData.SrcValue() == src &&
                         sfData.DstValue() == dst;
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

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(SplitResolverTest, ProcessIntervalsWithoutSplit)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(2U, Opcode::Return).u64().Inputs(1U);
        }
    }

    SplitResolver resolver(initialGraph, RunLivenessAnalysis(initialGraph));
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
}

TEST_F(SplitResolverTest, ConnectSiblingsWithSameBlock)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            AFTER_SPLIT_RESOLUTION(INST(4U, Opcode::SpillFill));
            INST(1U, Opcode::Add).u64().Inputs(0U, 0U);
            AFTER_SPLIT_RESOLUTION(INST(5U, Opcode::SpillFill));
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);

    auto param = la->GetInstLifeIntervals(&INS(0U));
    auto add = la->GetInstLifeIntervals(&INS(1U));
    param->SetReg(0U);

    SplitAssignReg(SplitAssignSlot(param, add->GetBegin(), 0U), add->GetEnd(), 1U);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(1U).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0U, 0U}});
    CheckSpillFills(INS(1U).GetNext(), {{LocationType::STACK, LocationType::REGISTER, 0U, 1U}});
}

SRC_GRAPH(ConnectSiblingsInDifferentBlocks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 0U);
            INST(2U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(8U, Opcode::SpillFill);
            INST(4U, Opcode::CallStatic).v0id().InputsAutoType(3U);
        }

        BASIC_BLOCK(6U, 4U)
        {
            INST(9U, Opcode::SpillFill);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(5U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(7U, Opcode::Return).u64().Inputs(0U);
        }
    }
}

OUT_GRAPH(ConnectSiblingsInDifferentBlocks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 0U);
            INST(2U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(4U, Opcode::CallStatic).v0id().InputsAutoType(3U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(5U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(7U, Opcode::Return).u64().Inputs(0U);
        }
    }
}

TEST_F(SplitResolverTest, ConnectSiblingsInDifferentBlocks)
{
    auto expectedGraph = CreateEmptyGraph();
    src_graph::ConnectSiblingsInDifferentBlocks::CREATE(expectedGraph);

    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    out_graph::ConnectSiblingsInDifferentBlocks::CREATE(initialGraph);

    auto la = RunLivenessAnalysis(initialGraph);

    auto param = la->GetInstLifeIntervals(&INS(0U));
    auto call = la->GetInstLifeIntervals(&INS(4U));
    param->SetReg(0U);

    SplitAssignSlot(param, call->GetBegin(), 0U);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(4U).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0U, 0U}});
    CheckSpillFills(BB(4U).GetPredsBlocks()[0U]->GetLastInst(),
                    {{LocationType::REGISTER, LocationType::STACK, 0U, 0U}});
}

SRC_GRAPH(ConnectSiblingsHavingCriticalEdgeBetweenBlocks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 0U);
            INST(2U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(4U, Opcode::CallStatic).v0id().InputsAutoType(3U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::Return).u64().Inputs(0U);
        }
    }
}

OUT_GRAPH(ConnectSiblingsHavingCriticalEdgeBetweenBlocks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 0U);
            INST(2U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::SpillFill);
            INST(4U, Opcode::CallStatic).v0id().InputsAutoType(3U);
        }

        BASIC_BLOCK(5U, 4U)
        {
            INST(7U, Opcode::SpillFill);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::Return).u64().Inputs(0U);
        }
    }
}

TEST_F(SplitResolverTest, ConnectSiblingsHavingCriticalEdgeBetweenBlocks)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();
    src_graph::ConnectSiblingsHavingCriticalEdgeBetweenBlocks::CREATE(initialGraph);

    auto la = RunLivenessAnalysis(initialGraph);
    auto param = la->GetInstLifeIntervals(&INS(0U));
    auto call = la->GetInstLifeIntervals(&INS(4U));
    param->SetReg(0U);

    SplitAssignSlot(param, call->GetBegin(), 0U);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    out_graph::ConnectSiblingsHavingCriticalEdgeBetweenBlocks::CREATE(expectedGraph);

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    size_t spillFills = 0;
    for (auto block : initialGraph->GetVectorBlocks()) {
        if (block == nullptr) {
            continue;
        }
        for (auto inst : block->AllInsts()) {
            if (inst->GetOpcode() == Opcode::SpillFill) {
                CheckSpillFills(inst, {{LocationType::REGISTER, LocationType::STACK, 0U, 0U}});
                spillFills++;
            }
        }
    }
    EXPECT_EQ(spillFills, 2U);
}

SRC_GRAPH(SplitAtTheEndOfBlock, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 0U);
            INST(2U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(3U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(8U, Opcode::SpillFill);
        }

        BASIC_BLOCK(7U, 4U)
        {
            INST(9U, Opcode::SpillFill);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(4U, Opcode::Mul).u64().Inputs(0U, 0U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(7U, Opcode::Add).u64().Inputs(3U, 0U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(5U, Opcode::Phi).u64().Inputs(4U, 7U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
}

OUT_GRAPH(SplitAtTheEndOfBlock, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 0U);
            INST(2U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(3U, Opcode::Add).u64().Inputs(0U, 0U);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(4U, Opcode::Mul).u64().Inputs(0U, 0U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(7U, Opcode::Add).u64().Inputs(3U, 0U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(5U, Opcode::Phi).u64().Inputs(4U, 7U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
}

TEST_F(SplitResolverTest, SplitAtTheEndOfBlock)
{
    auto expectedGraph = CreateEmptyGraph();
    src_graph::SplitAtTheEndOfBlock::CREATE(expectedGraph);

    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    out_graph::SplitAtTheEndOfBlock::CREATE(initialGraph);

    auto la = RunLivenessAnalysis(initialGraph);
    auto param = la->GetInstLifeIntervals(&INS(0U));
    auto bb3 = la->GetBlockLiveRange(&BB(3U));
    param->SetReg(0U);
    SplitAssignSlot(param, bb3.GetEnd(), 0U);
    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(4U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(5U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(7U))->SetReg(1U);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();
    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(3U).GetNext(), {{LocationType::REGISTER, LocationType::STACK, 0U, 0U}});
    CheckSpillFills(BB(4U).GetPredsBlocks()[0U]->GetLastInst(),
                    {{LocationType::REGISTER, LocationType::STACK, 0U, 0U}});
}

// If we already inserted spill fill instruction for some spill
// the we can reuse it for another one.
TEST_F(SplitResolverTest, ReuseExistingSpillFillWithinBlock)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            AFTER_SPLIT_RESOLUTION(INST(6U, Opcode::SpillFill));
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(2U);
            AFTER_SPLIT_RESOLUTION(INST(7U, Opcode::SpillFill));
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);

    auto param0 = la->GetInstLifeIntervals(&INS(0U));
    auto param1 = la->GetInstLifeIntervals(&INS(1U));
    auto call = la->GetInstLifeIntervals(&INS(3U));
    param0->SetReg(0U);
    param1->SetReg(1U);

    SplitAssignReg(SplitAssignSlot(param0, call->GetBegin(), 0U), call->GetEnd(), 0U);
    SplitAssignReg(SplitAssignSlot(param1, call->GetBegin(), 1U), call->GetEnd(), 1U);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(3U).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0},
                                        {LocationType::REGISTER, LocationType::STACK, 1U, 1U}});
    CheckSpillFills(INS(3U).GetNext(), {{LocationType::STACK, LocationType::REGISTER, 0, 0},
                                        {LocationType::STACK, LocationType::REGISTER, 1U, 1U}});
}

// If there are spill fills inserted to load instn's operand or spill instn's result
// then we can't reuse these spill fills to connect splits.
TEST_F(SplitResolverTest, DontReuseInstructionSpillFills)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            AFTER_SPLIT_RESOLUTION(INST(11U, Opcode::SpillFill));
            INST(4U, Opcode::SpillFill);  // spill fill loading
            INST(5U, Opcode::AddI).u64().Imm(42U).Inputs(2U);
            AFTER_SPLIT_RESOLUTION(INST(12U, Opcode::SpillFill));
            INST(7U, Opcode::SpillFill);  // spill fill loading
            INST(8U, Opcode::Add).u64().Inputs(0U, 5U);
            INST(9U, Opcode::Add).u64().Inputs(8U, 1U);
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);

    auto param0 = la->GetInstLifeIntervals(&INS(0U));
    auto param1 = la->GetInstLifeIntervals(&INS(1U));
    auto param2 = la->GetInstLifeIntervals(&INS(2U));
    auto addi = la->GetInstLifeIntervals(&INS(5U));
    auto add = la->GetInstLifeIntervals(&INS(8U));
    param0->SetReg(0U);
    param1->SetReg(1U);
    param2->SetLocation(Location::MakeStackSlot(4U));
    addi->SetLocation(Location::MakeStackSlot(3U));

    INS(4U).CastToSpillFill()->SetSpillFillType(SpillFillType::INPUT_FILL);
    INS(7U).CastToSpillFill()->SetSpillFillType(SpillFillType::INPUT_FILL);

    SplitAssignReg(SplitAssignSlot(param0, addi->GetBegin(), 0U), add->GetBegin(), 0U);
    SplitAssignReg(SplitAssignSlot(param1, addi->GetBegin(), 1U), add->GetBegin(), 1U);

    INS(4U).CastToSpillFill()->AddFill(param2->GetLocation().GetValue(), 11U, DataType::Type::INT64);
    INS(7U).CastToSpillFill()->AddFill(addi->GetLocation().GetValue(), 11U, DataType::Type::INT64);
    INS(5U).SetSrcReg(0U, 11U);
    INS(5U).SetDstReg(11U);
    INS(8U).SetSrcReg(0U, 0U);
    INS(8U).SetSrcReg(1U, 11U);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(4U).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0},
                                        {LocationType::REGISTER, LocationType::STACK, 1U, 1U}});
    CheckSpillFills(INS(7U).GetPrev(), {{LocationType::STACK, LocationType::REGISTER, 0, 0},
                                        {LocationType::STACK, LocationType::REGISTER, 1U, 1U}});
}

TEST_F(SplitResolverTest, DoNotReuseExistingSpillFillBeforeInstruction)
{
    auto initialGraph = CreateEmptyGraph();
    auto expectedGraph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            AFTER_SPLIT_RESOLUTION(INST(6U, Opcode::SpillFill));
            INST(5U, Opcode::SpillFill);
            INST(3U, Opcode::Mul).u64().Inputs(0U, 2U);
            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);

    auto param0 = la->GetInstLifeIntervals(&INS(0U));
    auto mul = la->GetInstLifeIntervals(&INS(3U));
    param0->SetReg(0U);
    SplitAssignSlot(param0, mul->GetBegin(), 0U);

    auto mulSf = INS(5U).CastToSpillFill();
    mulSf->AddFill(0U, 1U, DataType::Type::UINT64);
    mulSf->SetSpillFillType(SpillFillType::INPUT_FILL);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(2U).GetNext(), {{LocationType::REGISTER, LocationType::STACK, 0U, 0U}});
    CheckSpillFills(&INS(5U), {{LocationType::STACK, LocationType::REGISTER, 0U, 1U}});
}

TEST_F(SplitResolverTest, ReuseExistingSpillFillAtTheEndOfBlock)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(9U, Opcode::SpillFill);  // SF generated for PHI
        }

        BASIC_BLOCK(4U, 5U)
        {
            AFTER_SPLIT_RESOLUTION(INST(11U, Opcode::SpillFill));
            INST(5U, Opcode::Mul).u64().Inputs(1U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).u64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 0U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);

    auto phi = la->GetInstLifeIntervals(&INS(6U));
    phi->SetReg(3U);
    auto mul = la->GetInstLifeIntervals(&INS(5U));
    mul->SetReg(3U);
    auto add = la->GetInstLifeIntervals(&INS(4U));
    add->SetReg(2U);

    auto param0 = la->GetInstLifeIntervals(&INS(0U));
    param0->SetReg(1U);

    SplitAssignSlot(param0, mul->GetBegin(), 0U);

    auto phiSf = INS(9U).CastToSpillFill();
    // param0 still has r1 assigned at the end of BB3, so PHI's SF will move it from r1 to r3 assigned to PHI
    phiSf->AddMove(1U, 3U, DataType::Type::UINT64);
    phiSf->SetSpillFillType(SpillFillType::SPLIT_MOVE);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(&INS(9U), {{LocationType::REGISTER, LocationType::STACK, 1U, 0U}});
    CheckSpillFills(INS(5U).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 1U, 0U}});
}

SRC_GRAPH(ConnectSplitAtTheEndOfBlock, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            // Insert copy instruction before φ-move, because
            // inst 4 should be already copied at the end of block (where φ inserts move).
            INST(10U, Opcode::SpillFill);
            INST(9U, Opcode::SpillFill);  // SF generated for PHI
        }

        BASIC_BLOCK(6U, 4U)
        {
            INST(11U, Opcode::SpillFill);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Mul).u64().Inputs(1U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).u64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 0U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
}

OUT_GRAPH(ConnectSplitAtTheEndOfBlock, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(9U, Opcode::SpillFill);  // SF generated for PHI
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Mul).u64().Inputs(1U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).u64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 0U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
}

TEST_F(SplitResolverTest, ConnectSplitAtTheEndOfBlock)
{
    auto expectedGraph = CreateEmptyGraph();
    src_graph::ConnectSplitAtTheEndOfBlock::CREATE(expectedGraph);

    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    out_graph::ConnectSplitAtTheEndOfBlock::CREATE(initialGraph);

    auto la = RunLivenessAnalysis(initialGraph);
    auto phi = la->GetInstLifeIntervals(&INS(6U));
    phi->SetReg(3U);
    auto add = la->GetInstLifeIntervals(&INS(4U));
    add->SetReg(1U);
    auto mul = la->GetInstLifeIntervals(&INS(5U));
    mul->SetReg(3U);

    auto param0 = la->GetInstLifeIntervals(&INS(0U));
    param0->SetReg(1U);

    SplitAssignSlot(param0, add->GetBegin() + 2U, 0U);

    auto phiSf = INS(9U).CastToSpillFill();
    // param0 still has r1 assigned at the end of BB3, so PHI's SF will move it from r1 to r3 assigned to PHI
    phiSf->AddMove(1U, 3U, DataType::Type::UINT64);
    phiSf->SetSpillFillType(SpillFillType::SPLIT_MOVE);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(4U).GetNext(), {{LocationType::REGISTER, LocationType::STACK, 1U, 0U}});
}

SRC_GRAPH(GracefullyHandlePhiResolverBlocks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_LE).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(7U, Opcode::Sub).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(8U, Opcode::Phi).u64().Inputs(6U, 7U);
            INST(9U, Opcode::Add).u64().Inputs(0U, 8U);
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }
}

OUT_GRAPH(GracefullyHandlePhiResolverBlocks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_LE).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(5U, 7U)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(7U, 6U)
        {
            // Single spill fill handling both split connection and φ-move
            INST(12U, Opcode::SpillFill);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(11U, Opcode::SpillFill);  // resolve split
            INST(7U, Opcode::Sub).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(8U, Opcode::Phi).u64().Inputs(6U, 7U);
            INST(9U, Opcode::Add).u64().Inputs(0U, 8U);
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }
}

TEST_F(SplitResolverTest, GracefullyHandlePhiResolverBlocks)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    src_graph::GracefullyHandlePhiResolverBlocks::CREATE(initialGraph);

    auto la = RunLivenessAnalysis(initialGraph);
    auto pred = &BB(5U);
    auto succ = &BB(6U);
    auto phiResolver = pred->InsertNewBlockToSuccEdge(succ);
    auto sf0 = initialGraph->CreateInstSpillFill();
    sf0->SetSpillFillType(SpillFillType::SPLIT_MOVE);
    phiResolver->PrependInst(sf0);

    auto param0 = la->GetInstLifeIntervals(&INS(0U));
    param0->SetReg(0U);
    auto sub = la->GetInstLifeIntervals(&INS(7U));
    SplitAssignReg(param0, sub->GetBegin(), 4U);

    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(6U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(7U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(8U))->SetReg(1U);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    auto expectedGraph = CreateEmptyGraph();
    out_graph::GracefullyHandlePhiResolverBlocks::CREATE(expectedGraph);

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(sf0, {{LocationType::REGISTER, LocationType::REGISTER, 0U, 4U}});
    CheckSpillFills(sub->GetInst()->GetPrev(), {{LocationType::REGISTER, LocationType::REGISTER, 0U, 4U}});
}

SRC_GRAPH(ResolveSplitWithinLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 3U)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Add).u64().Inputs(1U, 4U);
            INST(12U, Opcode::SpillFill);
            INST(6U, Opcode::Add).u64().Inputs(4U, 5U);
        }

        BASIC_BLOCK(3U, 6U, 5U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(4U, 5U);
            INST(13U, Opcode::SpillFill);
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(6U, 7U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(7U);
        }

        BASIC_BLOCK(6U, 3U)
        {
            INST(14U, Opcode::SpillFill);
        }
    }
}

OUT_GRAPH(ResolveSplitWithinLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 3U)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Add).u64().Inputs(1U, 4U);
            INST(6U, Opcode::Add).u64().Inputs(4U, 5U);
        }

        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(4U, 5U);
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(6U, 7U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Return).u64().Inputs(7U);
        }
    }
}

TEST_F(SplitResolverTest, ResolveSplitWithinLoop)
{
    auto expectedGraph = CreateEmptyGraph();
    src_graph::ResolveSplitWithinLoop::CREATE(expectedGraph);

    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    out_graph::ResolveSplitWithinLoop::CREATE(initialGraph);

    auto la = RunLivenessAnalysis(initialGraph);
    auto var0 = la->GetInstLifeIntervals(&INS(4U));
    var0->SetLocation(Location::MakeStackSlot(0U));
    SplitAssignSlot(SplitAssignReg(var0, la->GetInstLifeIntervals(&INS(6U))->GetBegin(), 4U),
                    la->GetInstLifeIntervals(&INS(8U))->GetBegin(), 0);
    auto var1 = la->GetInstLifeIntervals(&INS(5U));
    var1->SetLocation(Location::MakeStackSlot(1U));
    SplitAssignSlot(SplitAssignReg(var1, la->GetInstLifeIntervals(&INS(6U))->GetBegin(), 5U),
                    la->GetInstLifeIntervals(&INS(8U))->GetBegin(), 1);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(6U).GetPrev(), {{LocationType::STACK, LocationType::REGISTER, 0, 4U},
                                        {LocationType::STACK, LocationType::REGISTER, 1U, 5U}});
    CheckSpillFills(INS(8U).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 4U, 0},
                                        {LocationType::REGISTER, LocationType::STACK, 5U, 1U}});
    auto resolverBlock = initialGraph->GetVectorBlocks().back();
    CheckSpillFills(resolverBlock->GetLastInst(), {{LocationType::STACK, LocationType::REGISTER, 0, 4U},
                                                   {LocationType::STACK, LocationType::REGISTER, 1U, 5U}});
}

TEST_F(SplitResolverTest, SkipIntervalsCoveringOnlyBlockStart)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();
    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(1U, 8U);
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(5U, 5U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }

        BASIC_BLOCK(5U, 3U)
        {
            // split (r1)
            AFTER_SPLIT_RESOLUTION(INST(11U, Opcode::SpillFill));
            INST(8U, Opcode::Sub).u64().Inputs(5U, 0U);
            // copy to location @ begging BB 3 (r0)
            AFTER_SPLIT_RESOLUTION(INST(12U, Opcode::SpillFill));
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
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs(2U, 5U);
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);

    auto param0 = la->GetInstLifeIntervals(&INS(0U));
    param0->SetReg(0U);
    SplitAssignReg(param0, la->GetInstLifeIntervals(&INS(8U))->GetBegin(), 1U);

    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(1U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(2U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(5U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(8U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(9U))->SetReg(1U);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
}

TEST_F(SplitResolverTest, ConnectIntervalsForConstantWithinBlock)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        CONSTANT(0U, 42U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::AddI).u64().Imm(1U).Inputs(0U);
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            AFTER_SPLIT_RESOLUTION(INST(6U, Opcode::SpillFill));
            INST(3U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(4U, Opcode::Add).u64().Inputs(0U, 3U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);
    auto con = la->GetInstLifeIntervals(&INS(0U));
    con->SetReg(0U);
    SplitAssignReg(SplitAssignImmSlot(SplitAssignImmSlot(con, la->GetInstLifeIntervals(&INS(1))->GetBegin(), 0),
                                      la->GetInstLifeIntervals(&INS(2U))->GetBegin(), 0),
                   la->GetInstLifeIntervals(&INS(3U))->GetBegin(), 0);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(3U).GetPrev(), {{LocationType::IMMEDIATE, LocationType::REGISTER, 0U, 0U}});
}

TEST_F(SplitResolverTest, ConnectIntervalsForConstantBetweenBlock)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        CONSTANT(0U, 42U);
        CONSTANT(1U, 64U);
        PARAMETER(2U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(2U, 2U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Add).u64().Inputs(2U, 2U);
            AFTER_SPLIT_RESOLUTION(INST(11U, Opcode::SpillFill));
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(6U, Opcode::Mul).u64().Inputs(2U, 2U);
            AFTER_SPLIT_RESOLUTION(INST(12U, Opcode::SpillFill));
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(7U, Opcode::Phi).u64().Inputs(5U, 6U);
            INST(8U, Opcode::Add).u64().Inputs(0U, 7U);
            INST(9U, Opcode::Add).u64().Inputs(1U, 8U);
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);
    auto con0 = la->GetInstLifeIntervals(&INS(0U));
    auto con1 = la->GetInstLifeIntervals(&INS(1U));
    con0->SetReg(0U);
    con1->SetReg(1U);
    SplitAssignImmSlot(con0, la->GetInstLifeIntervals(&INS(7U))->GetBegin(), 0U);
    SplitAssignReg(SplitAssignImmSlot(con1, la->GetInstLifeIntervals(&INS(3U))->GetBegin(), 0),
                   la->GetInstLifeIntervals(&INS(7U))->GetBegin(), 2U);
    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(5U))->SetReg(2U);
    la->GetInstLifeIntervals(&INS(6U))->SetReg(2U);
    la->GetInstLifeIntervals(&INS(7U))->SetReg(2U);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();
    CheckSpillFills(INS(5U).GetNext(), {{LocationType::IMMEDIATE, LocationType::REGISTER, 0U, 2U}});
    CheckSpillFills(INS(6U).GetNext(), {{LocationType::IMMEDIATE, LocationType::REGISTER, 0U, 2U}});
}

TEST_F(SplitResolverTest, DontReuseSpillFillForConstant)
{
    auto initialGraph = InitUsedRegs(CreateEmptyGraph());
    auto expectedGraph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        CONSTANT(0U, 42U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::AddI).u64().Imm(1U).Inputs(0U);
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            AFTER_SPLIT_RESOLUTION(INST(3U, Opcode::SpillFill));
            INST(4U, Opcode::SpillFill);
            INST(5U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);
    la->GetInstLifeIntervals(&INS(0U))->SetLocation(Location::MakeConstant(0U));
    la->GetInstLifeIntervals(&INS(2U))->SetReg(1U);
    SplitAssignSlot(la->GetInstLifeIntervals(&INS(2U)), la->GetInstLifeIntervals(&INS(5U))->GetBegin(), 0U);
    INS(4U).CastToSpillFill()->AddSpillFill(Location::MakeConstant(0U), Location::MakeRegister(1U),
                                            DataType::Type::INT64);
    INS(4U).CastToSpillFill()->SetSpillFillType(SpillFillType::INPUT_FILL);

    SplitResolver resolver(initialGraph, la);
    resolver.Run();

    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(2U).GetNext(), {{LocationType::REGISTER, LocationType::STACK, 1U, 0U}});
}

TEST_F(SplitResolverTest, AppendSpillFIllBeforeLoadArrayPairI)
{
    auto initialGraph = CreateEmptyGraph();
    auto expectedGraph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 42U);
        CONSTANT(2U, 24U);

        BASIC_BLOCK(2U, -1L)
        {
            AFTER_SPLIT_RESOLUTION(INST(13U, Opcode::SpillFill));
            INST(6U, Opcode::LoadArrayPairI).u64().Inputs(0U).Imm(0x0U);
            INST(7U, Opcode::LoadPairPart).u64().Inputs(6U).Imm(0x0U);
            INST(8U, Opcode::LoadPairPart).u64().Inputs(6U).Imm(0x1U);
            INST(9U, Opcode::Add).u64().Inputs(7U, 8U);
            INST(10U, Opcode::Add).u64().Inputs(1U, 2U);
            INST(11U, Opcode::Add).u64().Inputs(9U, 10U);
            INST(12U, Opcode::Return).u64().Inputs(11U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);
    la->GetInstLifeIntervals(&INS(1U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(2U))->SetReg(2U);
    // Split constants before LoadPairParts
    SplitAssignSlot(la->GetInstLifeIntervals(&INS(1U)), la->GetInstLifeIntervals(&INS(7U))->GetBegin(), 0U);
    SplitAssignSlot(la->GetInstLifeIntervals(&INS(2U)), la->GetInstLifeIntervals(&INS(8U))->GetBegin(), 0U);
    la->GetInstLifeIntervals(&INS(7U))->SetReg(1U);
    la->GetInstLifeIntervals(&INS(8U))->SetReg(1U);
    SplitResolver resolver(initialGraph, la);
    resolver.Run();
    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
}

TEST_F(SplitResolverTest, SplitAfterLastInstruction)
{
    auto initialGraph = CreateEmptyGraph();
    auto expectedGraph = CreateEmptyGraph();

    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Phi).u64().Inputs(1U, 6U);
            INST(3U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(2U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).u64().Inputs(2U);
        }

        BASIC_BLOCK(4U, 2U)
        {
            INST(6U, Opcode::AddI).u64().Imm(1U).Inputs(2U);
        }
    }

    auto la = RunLivenessAnalysis(initialGraph);
    la->GetInstLifeIntervals(&INS(0U))->SetReg(0U);
    la->GetInstLifeIntervals(&INS(1U))->SetReg(2U);
    la->GetInstLifeIntervals(&INS(2U))->SetReg(2U);
    la->GetInstLifeIntervals(&INS(3U))->SetReg(3U);
    la->GetInstLifeIntervals(&INS(6U))->SetReg(2U);
    SplitAssignSlot(la->GetInstLifeIntervals(&INS(0U)), la->GetInstLifeIntervals(&INS(6U))->GetEnd(), 0U);

    InitUsedRegs(initialGraph);
    SplitResolver resolver(initialGraph, la);
    resolver.Run();
    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
}

SRC_GRAPH(MultipleEndBlockMoves, [[maybe_unused]] Graph *unused, Graph *initialGraph, Graph *expectedGraph)
{
    INITIALIZE_GRAPHS(initialGraph, expectedGraph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            AFTER_SPLIT_RESOLUTION(INST(12U, Opcode::SpillFill));
            INST(6U, Opcode::Add).u64().Inputs(2U, 3U);
            AFTER_SPLIT_RESOLUTION(INST(13U, Opcode::SpillFill));
        }

        BASIC_BLOCK(4U, 5U)
        {
            AFTER_SPLIT_RESOLUTION(INST(15U, Opcode::SpillFill));
            INST(7U, Opcode::Mul).u64().Inputs(2U, 3U);
            AFTER_SPLIT_RESOLUTION(INST(14U, Opcode::SpillFill));
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Phi).u64().Inputs(6U, 7U);
            INST(9U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(10U, Opcode::Add).u64().Inputs(8U, 9U);
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }
}

// NOTE (a.popov) Merge equal spill-fills
TEST_F(SplitResolverTest, MultipleEndBlockMoves)
{
    auto initialGraph = CreateEmptyGraph();
    auto expectedGraph = CreateEmptyGraph();

    src_graph::MultipleEndBlockMoves::CREATE(GetGraph(), initialGraph, expectedGraph);

    auto la = RunLivenessAnalysis(initialGraph);
    auto p0 = la->GetInstLifeIntervals(&INS(0U));
    auto p1 = la->GetInstLifeIntervals(&INS(1U));
    p0->SetReg(0U);
    p1->SetReg(1U);
    SplitAssignReg(SplitAssignSlot(p0, la->GetInstLifeIntervals(&INS(5U))->GetEnd(), 0),
                   la->GetInstLifeIntervals(&INS(8U))->GetBegin(), 0);
    SplitAssignReg(SplitAssignSlot(p1, la->GetInstLifeIntervals(&INS(5U))->GetEnd(), 1),
                   la->GetInstLifeIntervals(&INS(8U))->GetBegin(), 1);
    // Assign reg to the phi and its inputs
    la->GetInstLifeIntervals(&INS(6U))->SetReg(2U);
    la->GetInstLifeIntervals(&INS(7U))->SetReg(2U);
    la->GetInstLifeIntervals(&INS(8U))->SetReg(2U);

    InitUsedRegs(initialGraph);
    SplitResolver resolver(initialGraph, la);
    resolver.Run();
    initialGraph->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(initialGraph, expectedGraph));
    CheckSpillFills(INS(6U).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0},
                                        {LocationType::REGISTER, LocationType::STACK, 1U, 1U}});
    CheckSpillFills(INS(6U).GetNext(), {{LocationType::STACK, LocationType::REGISTER, 0, 0},
                                        {LocationType::STACK, LocationType::REGISTER, 1U, 1U}});
    CheckSpillFills(INS(7U).GetPrev(), {{LocationType::REGISTER, LocationType::STACK, 0, 0},
                                        {LocationType::REGISTER, LocationType::STACK, 1U, 1U}});
    CheckSpillFills(INS(7U).GetNext(), {{LocationType::STACK, LocationType::REGISTER, 0, 0},
                                        {LocationType::STACK, LocationType::REGISTER, 1U, 1U}});
}

SRC_GRAPH(SwapCallInputs, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 10U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::CallStatic).s32().InputsAutoType(0U, 1U, 2U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
}

TEST_F(SplitResolverTest, SwapCallInputs)
{
    auto initialGraph = CreateEmptyGraph();
    if (initialGraph->GetCallingConvention() == nullptr) {
        return;
    }

    src_graph::SwapCallInputs::CREATE(initialGraph);

    InitUsedRegs(initialGraph);
    auto la = RunLivenessAnalysis(initialGraph);
    auto c0 = la->GetInstLifeIntervals(&INS(0U));
    auto c1 = la->GetInstLifeIntervals(&INS(1U));
    auto call = la->GetInstLifeIntervals(&INS(4U));
    c0->SetType(DataType::INT32);
    c1->SetType(DataType::INT32);
    call->SetType(DataType::INT32);

    c0->SetReg(11U);
    c1->SetReg(12U);
    call->SetReg(1U);
    constexpr Register SPLIT_CONST0_REG {12U};
    constexpr Register SPLIT_CONST1_REG {13U};
    SplitAssignReg(c0, call->GetBegin(), SPLIT_CONST0_REG);
    SplitAssignReg(c1, call->GetBegin(), SPLIT_CONST1_REG);

    auto regalloc = RegAllocLinearScan(initialGraph);
    regalloc.Resolve();

    // Constants were splitted before the call, these moves must come first
    // r12 -> r13, r11 -> r12
    auto splitSf = INS(2U).GetNext();
    EXPECT_TRUE(splitSf->IsSpillFill());
    auto const1Move = splitSf->CastToSpillFill()->GetSpillFill(0U);
    auto const0Move = splitSf->CastToSpillFill()->GetSpillFill(1U);
    EXPECT_EQ(const1Move.GetSrc().GetRegister(), INS(1U).GetDstReg());
    EXPECT_EQ(const1Move.GetDst().GetRegister(), SPLIT_CONST1_REG);
    EXPECT_EQ(const0Move.GetSrc().GetRegister(), INS(0U).GetDstReg());
    EXPECT_EQ(const0Move.GetDst().GetRegister(), SPLIT_CONST0_REG);

    // Then fill call inputs in the correct order
    auto fillSf = splitSf->GetNext();
    EXPECT_TRUE(fillSf->IsSpillFill());
    auto const0Fill = fillSf->CastToSpillFill()->GetSpillFill(0U);
    auto const1Fill = fillSf->CastToSpillFill()->GetSpillFill(1U);
    EXPECT_EQ(const0Fill.GetSrc().GetRegister(), SPLIT_CONST0_REG);
    EXPECT_EQ(const1Fill.GetSrc().GetRegister(), SPLIT_CONST1_REG);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
