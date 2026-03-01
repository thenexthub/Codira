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
#include "libarkbase/utils/utils.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/live_registers.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/registers_description.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace ark::compiler {
static bool operator==(const SpillFillData &lhs, const SpillFillData &rhs)
{
    return std::forward_as_tuple(lhs.SrcType(), lhs.DstType(), lhs.GetType(), lhs.SrcValue(), lhs.DstValue()) ==
           std::forward_as_tuple(rhs.SrcType(), rhs.DstType(), rhs.GetType(), rhs.SrcValue(), rhs.DstValue());
}

class RegAllocLinearScanTest : public GraphTest {
public:
    void CheckInstRegNotEqualOthersInstRegs(int checkId, std::vector<int> &&instIds)
    {
        for (auto id : instIds) {
            ASSERT_NE(INS(checkId).GetDstReg(), INS(id).GetDstReg());
        }
    }

    void CompareSpillFillInsts(SpillFillInst *lhs, SpillFillInst *rhs)
    {
        const auto &lhsSfs = lhs->GetSpillFills();
        const auto &rhsSfs = rhs->GetSpillFills();
        ASSERT_EQ(lhsSfs.size(), rhsSfs.size());

        for (size_t i = 0; i < lhsSfs.size(); i++) {
            auto res = lhsSfs[i] == rhsSfs[i];
            ASSERT_TRUE(res);
        }
    }

    bool CheckImmediateSpillFill(Inst *inst, size_t inputNum)
    {
        auto sfInst = inst->GetPrev();
        auto constInput = inst->GetInput(inputNum).GetInst();
        if (sfInst->GetOpcode() != Opcode::SpillFill || !constInput->IsConst()) {
            return false;
        }
        auto graph = inst->GetBasicBlock()->GetGraph();
        auto srcReg = inst->GetSrcReg(inputNum);
        for (auto sf : sfInst->CastToSpillFill()->GetSpillFills()) {
            if (sf.SrcType() == LocationType::IMMEDIATE && (graph->GetSpilledConstant(sf.SrcValue()) == constInput) &&
                sf.DstValue() == srcReg) {
                return true;
            }
        }
        return false;
    }

    template <DataType::Type REG_TYPE>
    void TestPhiMovesOverwriting(Graph *graph, SpillFillInst *sf, SpillFillInst *expectedSf);
    template <DataType::Type REG_TYPE>
    void TestPhiMovesOverwritingCyclic(Graph *graph, SpillFillsResolver &resolver, SpillFillInst *sf,
                                       SpillFillInst *expectedSf);
    template <DataType::Type REG_TYPE>
    void TestPhiMovesOverwritingNotApplied(SpillFillsResolver &resolver, SpillFillInst *sf, SpillFillInst *expectedSf);
    template <DataType::Type REG_TYPE>
    void TestPhiMovesOverwritingComplex(SpillFillsResolver &resolver, SpillFillInst *sf, SpillFillInst *expectedSf);
    void PhiMovesOverwritingMixed(SpillFillsResolver &resolver, SpillFillInst *sf, SpillFillInst *expectedSf);
    void PhiMovesOverwriting2(SpillFillsResolver &resolver, SpillFillInst *sf, SpillFillInst *expectedSf);
    template <unsigned int CONSTANTS_NUM>
    bool FillGraphWithConstants(Graph *graph);

protected:
    void InitUsedRegs(Graph *graph)
    {
        ArenaVector<bool> regs =
            ArenaVector<bool>(std::max(MAX_NUM_REGS, MAX_NUM_VREGS), false, GetAllocator()->Adapter());
        graph->InitUsedRegs<DataType::INT64>(&regs);
        graph->InitUsedRegs<DataType::FLOAT64>(&regs);
        graph->SetStackSlotsCount(MAX_NUM_STACK_SLOTS);
    }
};

// NOLINTBEGIN(readability-magic-numbers)
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
 * Insts linear order:
 * ID   INST(INPUTS)    LIFE NUMBER LIFE INTERVALS      ARM64 REGS
 * ------------------------------------------
 * 0.   Constant        2           [2-22]              R4
 * 1.   Constant        4           [4-8]               R5
 * 2.   Constant        6           [6-24]              R6
 *
 * 3.   Phi (0,7)       8           [8-16][22-24]       R7
 * 4.   Phi (1,8)       8           [8-18]              R8
 * 5.   Cmp (4,0)       10          [10-12]             R9
 * 6.   If    (5)       12          -
 *
 * 7.   Mul (3,4)       16          [16-22]             R5
 * 8.   Sub (4,0)       18          [18-22]             R9
 *
 * 9.   Add (2,3)       24          [24-26]             R4
 */
TEST_F(RegAllocLinearScanTest, ARM64Regs)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 10U);
        CONSTANT(2U, 20U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{0U, 0U}, {3U, 7U}});
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {3U, 8U}});
            INST(5U, Opcode::Compare).b().Inputs(4U, 0U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(3U, 4U);
            INST(8U, Opcode::Sub).u64().Inputs(4U, 0U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();
    CheckInstRegNotEqualOthersInstRegs(0U, {1U, 2U, 5U, 7U, 8U});
    CheckInstRegNotEqualOthersInstRegs(1U, {2U});
    CheckInstRegNotEqualOthersInstRegs(2U, {5U, 7U, 8U});
    CheckInstRegNotEqualOthersInstRegs(3U, {5U});
    CheckInstRegNotEqualOthersInstRegs(4U, {5U, 7U});
    CheckInstRegNotEqualOthersInstRegs(7U, {8U});
}

/**
 * NOTE(a.popov) Support test-case
 *
 * The way to allocate registers in this test is to spill onе of the source registers on the stack:
 * A -> r1
 * B -> r2
 * spill r1 -> STACK
 * ADD(A, B) -> r1
 *
 * Currently, sources on the stack are supported for calls only
 */
TEST_F(RegAllocLinearScanTest, DISABLED_TwoFreeRegs)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 10U);
        CONSTANT(2U, 20U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{0U, 0U}, {3U, 7U}});
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {3U, 8U}});
            INST(5U, Opcode::Compare).b().Inputs(4U, 0U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(3U, 4U);
            INST(8U, Opcode::Sub).u64().Inputs(4U, 0U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(11U, Opcode::ReturnVoid);
        }
    }
    // Create reg_mask with 2 available general registers
    RegAllocLinearScan ra(GetGraph());
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xF3FFFFFFU : 0xFABFFFFFU;
    ra.SetRegMask(RegMask {regMask});
    ra.SetVRegMask(VRegMask {0U});
    auto result = ra.Run();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        EXPECT_FALSE(result);
        return;
    }
    EXPECT_TRUE(result);
    GraphChecker(GetGraph()).Check();
}

/*
 *           [start]
 *              |
 *              v
 *             [2]
 *            /   \
 *           v     v
 *          [3]   [4]---\
 *         /   \        |
 *        v     v       |
 *       [5]<--[6]<-----/
 *        |
 *        v
 *      [exit]
 *
 *  SpillFill instructions will be inserted between BB3 and BB5, then between BB3 and BB6
 *  Check if these instructions are unique
 *
 */
TEST_F(RegAllocLinearScanTest, InsertSpillFillsAfterSameBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 10U).u64();
        PARAMETER(2U, 20U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(5U, Opcode::Mul).u64().Inputs(0U, 1U);
            INST(6U, Opcode::Compare).b().CC(CC_EQ).Inputs(1U, 2U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(8U, Opcode::Mul).u64().Inputs(0U, 2U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Phi).u64().Inputs({{3U, 5U}, {6U, 12U}});
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }

        BASIC_BLOCK(6U, 5U)
        {
            INST(12U, Opcode::Phi).u64().Inputs({{3U, 5U}, {4U, 8U}});
            INST(13U, Opcode::Mul).u64().Inputs(12U, 2U);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->AllInsts()) {
            ASSERT_EQ(inst->GetBasicBlock(), block);
        }
    }
}

TEST_F(RegAllocLinearScanTest, RzeroAssigment)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 10U);
        CONSTANT(2U, 20U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs(0U, 7U);
            INST(4U, Opcode::Phi).u64().Inputs(1U, 8U);
            INST(5U, Opcode::Compare).b().Inputs(4U, 0U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(3U, 4U);
            INST(8U, Opcode::Sub).u64().Inputs(4U, 0U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(13U, Opcode::SaveState).Inputs(10U).SrcVregs({0U});
            INST(11U, Opcode::CallStatic).u64().InputsAutoType(0U, 13U);
            INST(12U, Opcode::Return).u64().Inputs(11U);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    auto zeroReg = GetGraph()->GetZeroReg();
    // check for target arch, which has a zero register
    if (zeroReg != INVALID_REG) {
        ASSERT_EQ(INS(0U).GetDstReg(), zeroReg);
        ASSERT_EQ(INS(5U).GetSrcReg(1U), zeroReg);
        ASSERT_EQ(INS(8U).GetSrcReg(1U), zeroReg);

        // Find spill-fill with moving zero constant -> phi destination register
        auto phiResolver = BB(2U).GetPredBlockByIndex(0U);
        auto spillFillInst = phiResolver->GetLastInst()->IsControlFlow() ? phiResolver->GetLastInst()->GetPrev()
                                                                         : phiResolver->GetLastInst();
        ASSERT_TRUE(spillFillInst->GetOpcode() == Opcode::SpillFill);
        auto spillFills = spillFillInst->CastToSpillFill()->GetSpillFills();
        auto phiReg = INS(3U).GetDstReg();
        ASSERT_TRUE(phiReg != INVALID_REG);
        auto iter = std::find_if(spillFills.begin(), spillFills.end(), [zeroReg, phiReg](const SpillFillData &sf) {
            return (sf.SrcValue() == zeroReg && sf.DstValue() == phiReg);
        });
        ASSERT_NE(iter, spillFills.end());
    }
}

template <DataType::Type REG_TYPE>
void RegAllocLinearScanTest::TestPhiMovesOverwriting(Graph *graph, SpillFillInst *sf, SpillFillInst *expectedSf)
{
    auto resolver = SpillFillsResolver(graph);

    // simple cyclical sf
    sf->ClearSpillFills();
    sf->AddMove(4U, 5U, REG_TYPE);
    sf->AddMove(5U, 4U, REG_TYPE);
    expectedSf->ClearSpillFills();
    if (graph->GetArch() != Arch::AARCH32) {  // temp is register
        Register tempReg = DataType::IsFloatType(REG_TYPE) ? graph->GetArchTempVReg() : graph->GetArchTempReg();
        expectedSf->AddMove(4U, tempReg, REG_TYPE);
        expectedSf->AddMove(5U, 4U, REG_TYPE);
        expectedSf->AddMove(tempReg, 5U, REG_TYPE);
    } else {  // temp is stack slot
        auto tempSlot = StackSlot(0U);
        expectedSf->AddSpill(4U, tempSlot, REG_TYPE);
        expectedSf->AddMove(5U, 4U, REG_TYPE);
        expectedSf->AddFill(tempSlot, 5U, REG_TYPE);
    }
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);

    TestPhiMovesOverwritingCyclic<REG_TYPE>(graph, resolver, sf, expectedSf);
    TestPhiMovesOverwritingNotApplied<REG_TYPE>(resolver, sf, expectedSf);
    TestPhiMovesOverwritingComplex<REG_TYPE>(resolver, sf, expectedSf);
}

template <DataType::Type REG_TYPE>
void RegAllocLinearScanTest::TestPhiMovesOverwritingCyclic(Graph *graph, SpillFillsResolver &resolver,
                                                           SpillFillInst *sf, SpillFillInst *expectedSf)
{
    // cyclical sf with memcopy
    sf->ClearSpillFills();
    sf->AddMemCopy(4U, 5U, REG_TYPE);
    sf->AddMemCopy(5U, 4U, REG_TYPE);
    expectedSf->ClearSpillFills();
    if (graph->GetArch() != Arch::AARCH32) {  // temp is register
        Register tempReg = DataType::IsFloatType(REG_TYPE) ? graph->GetArchTempVReg() : graph->GetArchTempReg();
        expectedSf->AddFill(4U, tempReg, REG_TYPE);
        expectedSf->AddMemCopy(5U, 4U, REG_TYPE);
        expectedSf->AddSpill(tempReg, 5U, REG_TYPE);
    } else {  // temp is stack slot
        auto tempSlot = StackSlot(0U);
        expectedSf->AddMemCopy(4U, tempSlot, REG_TYPE);
        expectedSf->AddMemCopy(5U, 4U, REG_TYPE);
        expectedSf->AddMemCopy(tempSlot, 5U, REG_TYPE);
    }
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);

    // cyclic sf with all move-types
    sf->ClearSpillFills();
    sf->AddMove(4U, 5U, REG_TYPE);
    sf->AddSpill(5U, 10U, REG_TYPE);
    sf->AddMemCopy(10U, 11U, REG_TYPE);
    sf->AddFill(11U, 4U, REG_TYPE);
    expectedSf->ClearSpillFills();
    expectedSf->ClearSpillFills();
    if (graph->GetArch() != Arch::AARCH32) {  // temp is register
        Register tempReg = DataType::IsFloatType(REG_TYPE) ? graph->GetArchTempVReg() : graph->GetArchTempReg();
        expectedSf->AddMove(4U, tempReg, REG_TYPE);
        expectedSf->AddFill(11U, 4U, REG_TYPE);
        expectedSf->AddMemCopy(10U, 11U, REG_TYPE);
        expectedSf->AddSpill(5U, 10U, REG_TYPE);
        expectedSf->AddMove(tempReg, 5U, REG_TYPE);
    } else {  // temp is stack slot
        auto tempSlot = StackSlot(0U);
        expectedSf->AddSpill(4U, tempSlot, REG_TYPE);
        expectedSf->AddFill(11U, 4U, REG_TYPE);
        expectedSf->AddMemCopy(10U, 11U, REG_TYPE);
        expectedSf->AddSpill(5U, 10U, REG_TYPE);
        expectedSf->AddFill(tempSlot, 5U, REG_TYPE);
    }
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);
}

template <DataType::Type REG_TYPE>
void RegAllocLinearScanTest::TestPhiMovesOverwritingNotApplied(SpillFillsResolver &resolver, SpillFillInst *sf,
                                                               SpillFillInst *expectedSf)
{
    // not applied
    sf->ClearSpillFills();
    sf->AddMove(4U, 5U, REG_TYPE);
    sf->AddMove(6U, 7U, REG_TYPE);
    expectedSf->ClearSpillFills();
    expectedSf->AddMove(4U, 5U, REG_TYPE);
    expectedSf->AddMove(6U, 7U, REG_TYPE);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);
}

template <DataType::Type REG_TYPE>
void RegAllocLinearScanTest::TestPhiMovesOverwritingComplex(SpillFillsResolver &resolver, SpillFillInst *sf,
                                                            SpillFillInst *expectedSf)
{
    // comlex sf
    sf->ClearSpillFills();
    sf->AddFill(1U, 15U, REG_TYPE);
    sf->AddMove(4U, 5U, REG_TYPE);
    sf->AddFill(2U, 16U, REG_TYPE);
    sf->AddMove(5U, 6U, REG_TYPE);
    sf->AddMove(6U, 7U, REG_TYPE);
    sf->AddMove(7U, 9U, REG_TYPE);
    sf->AddMove(6U, 4U, REG_TYPE);
    sf->AddMove(11U, 12U, REG_TYPE);
    sf->AddMove(20U, 19U, REG_TYPE);
    sf->AddMove(21U, 20U, REG_TYPE);
    sf->AddSpill(15U, 3U, REG_TYPE);
    sf->AddMove(10U, 11U, REG_TYPE);
    expectedSf->ClearSpillFills();
    expectedSf->AddMove(7U, 9U, REG_TYPE);
    expectedSf->AddMove(6U, 7U, REG_TYPE);
    expectedSf->AddMove(5U, 6U, REG_TYPE);
    expectedSf->AddMove(4U, 5U, REG_TYPE);
    expectedSf->AddMove(7U, 4U, REG_TYPE);

    expectedSf->AddMove(11U, 12U, REG_TYPE);
    expectedSf->AddMove(10U, 11U, REG_TYPE);

    expectedSf->AddFill(2U, 16U, REG_TYPE);

    expectedSf->AddMove(20U, 19U, REG_TYPE);
    expectedSf->AddMove(21U, 20U, REG_TYPE);

    expectedSf->AddSpill(15U, 3U, REG_TYPE);
    expectedSf->AddFill(1U, 15U, REG_TYPE);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);
}

void RegAllocLinearScanTest::PhiMovesOverwritingMixed(SpillFillsResolver &resolver, SpillFillInst *sf,
                                                      SpillFillInst *expectedSf)
{
    // mixed reg-types sf
    sf->ClearSpillFills();
    sf->AddFill(1U, 5U, DataType::FLOAT64);
    sf->AddFill(2U, 7U, DataType::UINT32);
    sf->AddMove(4U, 5U, DataType::UINT64);
    sf->AddMove(5U, 6U, DataType::UINT64);
    sf->AddSpill(2U, 3U, DataType::UINT64);
    sf->AddSpill(7U, 4U, DataType::UINT64);
    sf->AddMove(6U, 4U, DataType::UINT64);
    sf->AddMove(6U, 7U, DataType::FLOAT64);
    sf->AddMove(7U, 9U, DataType::FLOAT64);
    sf->AddMove(10U, 11U, DataType::UINT64);
    sf->AddMove(11U, 12U, DataType::FLOAT64);
    sf->AddMove(21U, 20U, DataType::UINT64);
    sf->AddMove(22U, 21U, DataType::FLOAT64);
    sf->AddSpill(15U, 5U, DataType::FLOAT64);
    expectedSf->ClearSpillFills();
    expectedSf->AddMove(10U, 11U, DataType::UINT64);
    expectedSf->AddMove(21U, 20U, DataType::UINT64);
    expectedSf->AddFill(1U, 5U, DataType::FLOAT64);

    expectedSf->AddMove(7U, 9U, DataType::FLOAT64);
    expectedSf->AddMove(6U, 7U, DataType::FLOAT64);

    expectedSf->AddMove(11U, 12U, DataType::FLOAT64);
    expectedSf->AddMove(22U, 21U, DataType::FLOAT64);
    expectedSf->AddSpill(2U, 3U, DataType::UINT64);

    expectedSf->AddSpill(7U, 4U, DataType::UINT64);
    expectedSf->AddFill(2U, 7U, DataType::UINT32);

    expectedSf->AddSpill(15U, 5U, DataType::FLOAT64);

    if (GetGraph()->GetArch() != Arch::AARCH32) {  // temp is register
        Register tempReg = GetGraph()->GetArchTempReg();
        expectedSf->AddMove(4U, tempReg, DataType::UINT64);
        expectedSf->AddMove(6U, 4U, DataType::UINT64);
        expectedSf->AddMove(5U, 6U, DataType::UINT64);
        expectedSf->AddMove(tempReg, 5U, DataType::UINT64);
    } else {  // temp is stack slot
        auto tempSlot = StackSlot(0U);
        expectedSf->AddSpill(4U, tempSlot, DataType::UINT64);
        expectedSf->AddMove(6U, 4U, DataType::UINT64);
        expectedSf->AddMove(5U, 6U, DataType::UINT64);
        expectedSf->AddFill(tempSlot, 5U, DataType::UINT64);
    }

    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);
}

void RegAllocLinearScanTest::PhiMovesOverwriting2(SpillFillsResolver &resolver, SpillFillInst *sf,
                                                  SpillFillInst *expectedSf)
{
    // zero-reg reordering
    sf->ClearSpillFills();
    sf->AddMove(31U, 5U, DataType::UINT64);
    sf->AddMove(5U, 6U, DataType::UINT64);
    expectedSf->ClearSpillFills();
    expectedSf->AddMove(5U, 6U, DataType::UINT64);
    expectedSf->AddMove(31U, 5U, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);

    // find and resolve cycle in moves sequence starts with not-cycle move
    sf->ClearSpillFills();
    sf->AddMove(18U, 7U, DataType::UINT64);
    sf->AddMove(10U, 18U, DataType::UINT64);
    sf->AddMove(18U, 10U, DataType::UINT64);
    sf->AddSpill(7U, 32U, DataType::UINT64);
    expectedSf->ClearSpillFills();
    expectedSf->AddSpill(7U, 32U, DataType::UINT64);
    expectedSf->AddMove(18U, 7U, DataType::UINT64);
    expectedSf->AddMove(10U, 18U, DataType::UINT64);
    expectedSf->AddMove(7U, 10U, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);

    // fix `moves_table_[dst].src != first_src'
    sf->ClearSpillFills();
    sf->AddMove(8U, 6U, DataType::UINT64);
    sf->AddMove(7U, 8U, DataType::UINT64);
    sf->AddMove(8U, 7U, DataType::UINT64);
    expectedSf->ClearSpillFills();
    expectedSf->AddMove(8U, 6U, DataType::UINT64);
    expectedSf->AddMove(7U, 8U, DataType::UINT64);
    expectedSf->AddMove(6U, 7U, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);

    // find and resolve cycle in moves sequence starts with not-cycle move
    sf->ClearSpillFills();
    sf->AddMove(18U, 7U, DataType::UINT64);
    sf->AddMove(10U, 18U, DataType::UINT64);
    sf->AddMove(18U, 10U, DataType::UINT64);
    sf->AddMove(7U, 6U, DataType::UINT64);
    expectedSf->ClearSpillFills();
    expectedSf->AddMove(7U, 6U, DataType::UINT64);
    expectedSf->AddMove(18U, 7U, DataType::UINT64);
    expectedSf->AddMove(10U, 18U, DataType::UINT64);
    expectedSf->AddMove(7U, 10U, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);
}

TEST_F(RegAllocLinearScanTest, PhiMovesOverwriting)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).u64().Inputs(0U);
        }
    }

    InitUsedRegs(GetGraph());
    auto resolver = SpillFillsResolver(GetGraph());
    // Use the last available register as a temp
    auto sf = GetGraph()->CreateInstSpillFill();
    auto expectedSf = GetGraph()->CreateInstSpillFill();
    TestPhiMovesOverwriting<DataType::UINT32>(GetGraph(), sf, expectedSf);
    TestPhiMovesOverwriting<DataType::UINT64>(GetGraph(), sf, expectedSf);
    TestPhiMovesOverwriting<DataType::FLOAT64>(GetGraph(), sf, expectedSf);

    // not applied
    sf->ClearSpillFills();
    sf->AddMove(4U, 5U, DataType::UINT64);
    sf->AddMove(5U, 4U, DataType::FLOAT64);
    expectedSf->ClearSpillFills();
    expectedSf->AddMove(4U, 5U, DataType::UINT64);
    expectedSf->AddMove(5U, 4U, DataType::FLOAT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);

    // not applied
    sf->ClearSpillFills();
    sf->AddMemCopy(0U, 1U, DataType::UINT64);
    expectedSf->ClearSpillFills();
    expectedSf->AddMemCopy(0U, 1U, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expectedSf);

    PhiMovesOverwritingMixed(resolver, sf, expectedSf);
    PhiMovesOverwriting2(resolver, sf, expectedSf);
}

TEST_F(RegAllocLinearScanTest, DynPhiMovesOverwriting)
{
    Graph *graph = CreateDynEmptyGraph();
    if (graph->GetArch() != Arch::AARCH64) {
        return;
    }

    GRAPH(graph)
    {
        CONSTANT(0U, 0xffff000000000000U).any();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).any().Inputs(0U);
        }
    }
    InitUsedRegs(graph);
    auto resolver = SpillFillsResolver(graph);
    // Use the last available register as a temp
    auto sf = graph->CreateInstSpillFill();
    auto expectedSf = graph->CreateInstSpillFill();
    TestPhiMovesOverwriting<DataType::ANY>(graph, sf, expectedSf);
}

TEST_F(RegAllocLinearScanTest, BrokenTriangleWithEmptyBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64().DstReg(0U);
        PARAMETER(1U, 1U).s64().DstReg(2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(4U, Opcode::Phi).s64().Inputs({{2U, 1U}, {3U, 0U}}).DstReg(0U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());

    // Create reg_mask with small amount available general registers to force spill-fills
    auto regalloc = RegAllocLinearScan(GetGraph());
    auto result = regalloc.Run();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 4U, 5U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1U, 0U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(5U, 4U)
        {
            INST(6U, Opcode::SpillFill);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(4U, Opcode::Phi).s64().Inputs({{2U, 0U}, {5U, 1U}});
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RegAllocLinearScanTest, LoadArrayPair)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U).TypeId(77U);
            INST(4U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LoadArrayPairI).s64().Inputs(5U).Imm(0x0U);
            INST(7U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x0U);
            INST(8U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x1U);

            INST(9U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(10U, Opcode::ZeroCheck).s64().Inputs(7U, 9U);
            INST(11U, Opcode::Div).s64().Inputs(10U, 8U);
            INST(12U, Opcode::Return).s64().Inputs(11U);
        }
    }
    auto result = graph->RunPass<RegAllocLinearScan>();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto &div = INS(11U);
    auto &loadPair = INS(6U);
    for (size_t i = 0; i < div.GetInputsCount(); ++i) {
        if (div.GetSrcReg(0U) != loadPair.GetDstReg(0U)) {
            auto prev = div.GetPrev();
            ASSERT_EQ(prev->GetOpcode(), Opcode::SpillFill);
            ASSERT_EQ(prev->CastToSpillFill()->GetSpillFill(0U).DstValue(), div.GetSrcReg(0U));
        }
    }
}

TEST_F(RegAllocLinearScanTest, CheckInputTypeInStore)
{
    auto graph = CreateEmptyGraph();
    if (graph->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(graph)
    {
        CONSTANT(0U, nullptr).ref();
        CONSTANT(1U, 0x2aU).s64();
        CONSTANT(2U, 1.1_D).f64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({0U, 1U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::StoreObject).f64().Inputs(4U, 2U);
            INST(6U, Opcode::Return).s64().Inputs(1U);
        }
    }
    auto result = graph->RunPass<RegAllocLinearScan>();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto &store = INS(5U);
    // check zero reg
    ASSERT_EQ(store.GetSrcReg(0U), 31U);
}

TEST_F(RegAllocLinearScanTest, NullCheckAsPhiInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).ref();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::If).SrcType(DataType::INT64).CC(CC_EQ).Inputs(1U, 2U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(6U, Opcode::SaveState).Inputs(1U, 2U).SrcVregs({0U, 1U});
            INST(7U, Opcode::NullCheck).ref().Inputs(0U, 6U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Phi).ref().Inputs(7U, 3U);
            INST(12U, Opcode::Return).ref().Inputs(11U);
        }
    }

    EXPECT_TRUE(graph->RunPass<RegAllocLinearScan>());
    auto phiLocation = Location::MakeRegister(INS(11U).GetDstReg());
    auto paramLocation = INS(0U).CastToParameter()->GetLocationData().GetDst();
    if (phiLocation != paramLocation) {
        auto sfInst = BB(3U).GetLastInst();
        ASSERT_TRUE(sfInst->IsSpillFill());
        auto spillFill = sfInst->CastToSpillFill()->GetSpillFill(0U);
        EXPECT_EQ(spillFill.GetSrc(), paramLocation);
        EXPECT_EQ(spillFill.GetDst(), phiLocation);
    }
}

TEST_F(RegAllocLinearScanTest, MultiDestInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1000U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(2U, Opcode::NewArray).ref().Inputs(44U, 0U).TypeId(1U);
            INST(3U, Opcode::LoadArrayPairI).s32().Inputs(2U).Imm(0x0U);
            INST(4U, Opcode::LoadPairPart).s32().Inputs(3U).Imm(0x0U);
            INST(5U, Opcode::LoadPairPart).s32().Inputs(3U).Imm(0x1U);
            INST(6U, Opcode::Add).s32().Inputs(1U, 4U);
            INST(7U, Opcode::Add).s32().Inputs(6U, 5U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    // Run regalloc without free regs to push all dst on the stack
    auto regalloc = RegAllocLinearScan(graph);
    RegAllocResolver(graph).ResolveCatchPhis();
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0x000FFFFFU : 0xAAAAAAFFU;
    regalloc.SetRegMask(RegMask {regMask});
    regalloc.SetVRegMask(VRegMask {});
    regalloc.Run();

    auto loadPair = &INS(3U);
    ASSERT_NE(loadPair->GetDstReg(0U), loadPair->GetDstReg(1U));
    ASSERT_EQ(INS(6U).GetSrcReg(1U), loadPair->GetDstReg(0U));
    ASSERT_EQ(INS(7U).GetSrcReg(1U), loadPair->GetDstReg(1U));
}

/// Create COUNT constants and assign COUNT registers for them
TEST_F(RegAllocLinearScanTest, DynamicRegsMask)
{
    auto graph = CreateEmptyBytecodeGraph();
    graph->CreateStartBlock();
    graph->CreateEndBlock();

    const size_t count = 100;
    auto savePoint = graph->CreateInstSafePoint();
    auto block = CreateEmptyBlock(graph);
    block->AppendInst(savePoint);
    block->AppendInst(graph->CreateInstReturnVoid());
    graph->GetStartBlock()->AddSucc(block);
    block->AddSucc(graph->GetEndBlock());

    for (size_t i = 0; i < count; i++) {
        savePoint->AppendInput(graph->FindOrCreateConstant(i));
        savePoint->SetVirtualRegister(i, VirtualRegister(i, VRegInfo::VRegType::VREG));
    }
    auto result = graph->RunPass<RegAllocLinearScan>(EmptyRegMask());
    ASSERT_TRUE(result);
    for (size_t i = 1; i < count; i++) {
        ASSERT_EQ(graph->FindOrCreateConstant(i)->GetDstReg(), i);
    }
}

TEST_F(RegAllocLinearScanTest, MultiDestAsCallInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1000U);
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(2U, Opcode::NewArray).ref().Inputs(44U, 0U).TypeId(1U);
            INST(3U, Opcode::LoadArrayPairI).s32().Inputs(2U).Imm(0x0U);
            INST(4U, Opcode::LoadPairPart).s32().Inputs(3U).Imm(0x0U);
            INST(5U, Opcode::LoadPairPart).s32().Inputs(3U).Imm(0x1U);
            INST(6U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(7U, Opcode::CallStatic).s32().InputsAutoType(4U, 5U, 6U);
            INST(8U, Opcode::ZeroCheck).s32().Inputs(5U, 6U);
            INST(9U, Opcode::Div).s32().Inputs(4U, 8U);
            INST(10U, Opcode::Return).s32().Inputs(9U);
        }
    }
    auto regalloc = RegAllocLinearScan(graph);
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0x000FFFFFU : 0xAAAAAAAFU;
    regalloc.SetRegMask(RegMask {regMask});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto loadArr = &INS(3U);
    auto div = &INS(9U);
    auto callInst = INS(7U).CastToCallStatic();
    auto spillFill = callInst->GetPrev()->CastToSpillFill();
    // Check split before call
    for (auto i = 0U; i < 2U; i++) {
        ASSERT_EQ(spillFill->GetSpillFill(i).SrcValue(), loadArr->GetDstReg(i));
        if (loadArr->GetDstReg(i) == callInst->GetSrcReg(i)) {
            // LoadArrayPairI -> R1, R2 (caller-saved assigned)
            // R1 -> Rx, R2 -> Ry
            // call(R1, R2)
            ASSERT_EQ(div->GetSrcReg(i), spillFill->GetSpillFill(i).DstValue());
        } else {
            // LoadArrayPairI -> RX, RY (callee-saved assigned)
            // RX -> R1, RY -> R2
            // call(R1, R2)
            ASSERT_EQ(div->GetSrcReg(i), spillFill->GetSpillFill(i).SrcValue());
        }
    }
}

TEST_F(RegAllocLinearScanTest, MultiDestInLoop)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 1000U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(5U, Opcode::NewArray).ref().Inputs(44U, 2U).TypeId(1U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Phi).s32().Inputs(0U, 12U);  // pair part
            INST(7U, Opcode::Phi).s32().Inputs(1U, 13U);  // pair part
            INST(8U, Opcode::Phi).s32().Inputs(0U, 10U);  // add
            INST(9U, Opcode::Phi).s32().Inputs(0U, 15U);  // add
            INST(16U, Opcode::SafePoint).Inputs(5U, 6U, 7U).SrcVregs({0U, 1U, 2U});
            INST(10U, Opcode::AddI).s32().Inputs(8U).Imm(1U);
            INST(11U, Opcode::LoadArrayPair).s32().Inputs(5U, 10U);
            INST(12U, Opcode::LoadPairPart).s32().Inputs(11U).Imm(0U);
            INST(13U, Opcode::LoadPairPart).s32().Inputs(11U).Imm(1U);
            INST(14U, Opcode::If).SrcType(DataType::INT32).CC(CC_EQ).Inputs(12U, 13U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(15U, Opcode::SubI).s32().Inputs(9U).Imm(2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(20U, Opcode::Return).s32().Inputs(9U);
        }
    }
    auto regalloc = RegAllocLinearScan(graph);
    regalloc.SetRegMask(RegMask {0xFFFFAAAAU});
    ASSERT_TRUE(regalloc.Run());

    auto loadPair = &INS(11U);
    EXPECT_NE(loadPair->GetDstReg(0U), INVALID_REG);
    EXPECT_NE(loadPair->GetDstReg(1U), INVALID_REG);
    // LoadArrayPair, SubI and AddI should use unique registers
    std::set<Register> regs;
    regs.insert(INS(10U).GetDstReg());
    regs.insert(INS(15U).GetDstReg());
    regs.insert(loadPair->GetDstReg(0U));
    regs.insert(loadPair->GetDstReg(1U));
    EXPECT_EQ(regs.size(), 4U);
}

SRC_GRAPH(ResolveSegmentedCallInputs, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::CallStatic).u64().Inputs({{DataType::UINT64, 0U}, {DataType::NO_TYPE, 2U}});
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 3U});
            INST(5U, Opcode::CallStatic)
                .u64()
                .Inputs({{DataType::UINT64, 0U}, {DataType::UINT64, 3U}, {DataType::NO_TYPE, 4U}});
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedCallInputs)
{
    auto graph = CreateEmptyGraph();
    src_graph::ResolveSegmentedCallInputs::CREATE(graph);

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0U));
    auto call0 = (&INS(3U))->CastToCallStatic();
    auto call1 = (&INS(5U))->CastToCallStatic();
    // split at save state to force usage of stack location as call's input
    auto paramSplit0 = param0->SplitAt(la.GetInstLifeIntervals(&INS(2U))->GetBegin() - 1U, GetAllocator());
    paramSplit0->SetLocation(Location::MakeStackSlot(42U));
    paramSplit0->SetType(DataType::UINT64);

    auto paramSplit1 = paramSplit0->SplitAt(la.GetInstLifeIntervals(call1)->GetBegin() - 1U, GetAllocator());
    paramSplit1->SetReg(6U);
    paramSplit1->SetType(DataType::UINT64);

    graph->SetStackSlotsCount(MAX_NUM_STACK_SLOTS);
    auto regalloc = RegAllocLinearScan(graph);
    regalloc.SetRegMask(RegMask {0x000FFFFFU});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    auto call0Sf = call0->GetPrev()->CastToSpillFill()->GetSpillFill(0U);
    EXPECT_EQ(call0Sf.SrcType(), LocationType::STACK);
    EXPECT_EQ(call0Sf.SrcValue(), 42U);

    const auto &call1Sfs = call1->GetPrev()->CastToSpillFill()->GetSpillFills();
    auto it = std::find_if(call1Sfs.begin(), call1Sfs.end(),
                           [](auto sf) { return sf.SrcType() == LocationType::REGISTER && sf.SrcValue() == 6; });
    EXPECT_NE(it, call1Sfs.end());
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedSaveStateInputs)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::CallVirtual).u64().Inputs({{DataType::REFERENCE, 3U}, {DataType::NO_TYPE, 2U}});
            INST(5U, Opcode::Add).u64().Inputs(0U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0U));
    auto call = &INS(4U);
    auto paramSplit = param0->SplitAt(la.GetInstLifeIntervals(call)->GetBegin() - 1U, GetAllocator());
    static constexpr auto REG_FOR_SPLIT = Register(20U);
    paramSplit->SetReg(REG_FOR_SPLIT);
    paramSplit->SetType(DataType::UINT64);

    auto regalloc = RegAllocLinearScan(graph);
    // on AARCH32 use only caller-saved register pairs
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFFAU;
    regalloc.SetRegMask(RegMask {regMask});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto nullCheck = &INS(3U);
    auto callSaveState = call->GetInput(call->GetInputsCount() - 1L).GetInst()->CastToSaveState();
    auto nullCheckSaveState = nullCheck->GetInput(nullCheck->GetInputsCount() - 1L).GetInst()->CastToSaveState();

    ASSERT_NE(callSaveState, nullCheckSaveState);
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedInstInputs)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(3U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0U));
    auto paramSplit0 = param0->SplitAt(la.GetInstLifeIntervals(&INS(3U))->GetBegin() - 1U, GetAllocator());
    static constexpr auto REG_FOR_SPLIT = Register(20U);
    paramSplit0->SetReg(REG_FOR_SPLIT);
    paramSplit0->SetType(DataType::UINT64);

    auto regalloc = RegAllocLinearScan(graph);
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFAAU;
    regalloc.SetRegMask(RegMask {regMask});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    auto addSf = INS(3U).GetPrev();
    ASSERT_TRUE(addSf->IsSpillFill());

    EXPECT_EQ(INS(3U).GetSrcReg(0U), REG_FOR_SPLIT);
    SpillFillData expectedSf {LocationType::REGISTER, LocationType::REGISTER, param0->GetReg(), REG_FOR_SPLIT,
                              DataType::UINT64};
    EXPECT_EQ(addSf->CastToSpillFill()->GetSpillFill(0U), expectedSf);
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedSafePointInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SafePoint).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::Return).u64().Inputs(0U);
        }
    }

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0U));
    auto sp = &INS(2U);
    auto paramSplit = param0->SplitAt(la.GetInstLifeIntervals(sp)->GetBegin() - 1U, GetAllocator());
    static constexpr auto STACK_SLOT = StackSlot(1U);
    paramSplit->SetLocation(Location::MakeStackSlot(STACK_SLOT));
    paramSplit->SetType(DataType::UINT64);
    auto retSplit = paramSplit->SplitAt(la.GetInstLifeIntervals(&INS(3U))->GetBegin() - 1U, GetAllocator());
    retSplit->SetReg(0U);
    retSplit->SetType(DataType::UINT64);

    graph->SetStackSlotsCount(MAX_NUM_STACK_SLOTS);
    auto regalloc = RegAllocLinearScan(graph);
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFAAU;
    regalloc.SetRegMask(RegMask {regMask});
    regalloc.SetVRegMask(VRegMask {});
    regalloc.SetSlotsCount(MAX_NUM_STACK_SLOTS);
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
}

SRC_GRAPH(ResolveSegmentedPhiInput, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Sub).u64().Inputs(0U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).u64().Inputs(0U, 5U);
            INST(7U, Opcode::Phi).u64().Inputs(4U, 1U);
            INST(8U, Opcode::Mul).u64().Inputs(6U, 7U);
            INST(9U, Opcode::Return).u64().Inputs(8U);
        }
    }
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedPhiInput)
{
    auto graph = CreateEmptyGraph();
    src_graph::ResolveSegmentedPhiInput::CREATE(graph);

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0U));
    auto paramSplit0 = param0->SplitAt(la.GetInstLifeIntervals(&INS(4U))->GetBegin() - 1U, GetAllocator());
    static constexpr auto REG_FOR_SPLIT = Register(20U);
    paramSplit0->SetReg(REG_FOR_SPLIT);
    paramSplit0->SetType(DataType::UINT64);

    auto regalloc = RegAllocLinearScan(graph);
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFAAU;
    regalloc.SetRegMask(RegMask {regMask});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto lastInst = INS(4U).GetBasicBlock()->GetLastInst();
    ASSERT_TRUE(lastInst->IsSpillFill());
    bool spillFillFound = false;
    SpillFillData expectedSf {LocationType::REGISTER, LocationType::REGISTER, REG_FOR_SPLIT, INS(6U).GetDstReg(),
                              DataType::UINT64};

    for (auto &sf : lastInst->CastToSpillFill()->GetSpillFills()) {
        spillFillFound |= expectedSf == sf;
    }
    ASSERT_TRUE(spillFillFound);
}

SRC_GRAPH(DISABLED_ResolveSegmentedCatchPhiInputs, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).b();

        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(3U, Opcode::Try).CatchTypeIds({0U});
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(4U, Opcode::CallStatic).b().InputsAutoType(0U, 1U, 2U, 10U);
            INST(11U, Opcode::SaveState).Inputs(0U, 1U, 2U, 4U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::CallStatic).b().InputsAutoType(0U, 1U, 2U, 11U);
            INST(5U, Opcode::And).b().Inputs(4U, 9U);
        }

        BASIC_BLOCK(4U, 6U, 5U) {}  // Try-end

        BASIC_BLOCK(5U, -1L)
        {
            INST(7U, Opcode::CatchPhi).b().Inputs(2U, 4U);
            INST(8U, Opcode::Return).b().Inputs(7U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }
}

/**
 *  Currently `CatchPhi` can be visited by `LinearScan` in the `BytecodeOptimizer` mode only, where splits are not
 * posible, so that there is assertion in the `LinearScan` that interval in not splitted, which breaks this tests
 */
TEST_F(RegAllocLinearScanTest, DISABLED_ResolveSegmentedCatchPhiInputs)
{
    auto graph = CreateEmptyBytecodeGraph();
    src_graph::DISABLED_ResolveSegmentedCatchPhiInputs::CREATE(graph);

    BB(2U).SetTryId(0U);
    BB(3U).SetTryId(0U);
    BB(4U).SetTryId(0U);

    auto catchPhi = (&INS(7U))->CastToCatchPhi();
    catchPhi->AppendThrowableInst(&INS(4U));
    catchPhi->AppendThrowableInst(&INS(9U));

    graph->AppendThrowableInst(&INS(4U), &BB(5U));
    graph->AppendThrowableInst(&INS(9U), &BB(5U));
    INS(3U).CastToTry()->SetTryEndBlock(&BB(4U));

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();

    auto con = la.GetInstLifeIntervals(&INS(2U));
    auto streq = &INS(4U);
    auto conSplit = con->SplitAt(la.GetInstLifeIntervals(streq)->GetBegin() - 1L, GetAllocator());

    auto constexpr SPLIT_REG = 10;
    conSplit->SetReg(SPLIT_REG);
    conSplit->SetType(DataType::UINT32);

    auto regalloc = RegAllocLinearScan(graph, EmptyRegMask());
    auto result = regalloc.Run();
    ASSERT_TRUE(result);

    auto catchPhiReg = la.GetInstLifeIntervals(&INS(7U))->GetReg();
    auto sfBeforeIns4 = (&INS(4U))->GetPrev()->CastToSpillFill();
    EXPECT_EQ(std::count(sfBeforeIns4->GetSpillFills().begin(), sfBeforeIns4->GetSpillFills().end(),
                         SpillFillData {LocationType::REGISTER, LocationType::REGISTER, SPLIT_REG, catchPhiReg,
                                        DataType::UINT32}),
              1U);

    auto ins4Reg = la.GetInstLifeIntervals(&INS(4U))->GetReg();
    auto sfBeforeIns9 = (&INS(9U))->GetPrev()->CastToSpillFill();
    EXPECT_EQ(std::count(sfBeforeIns9->GetSpillFills().begin(), sfBeforeIns9->GetSpillFills().end(),
                         SpillFillData {LocationType::REGISTER, LocationType::REGISTER, ins4Reg, catchPhiReg,
                                        DataType::UINT32}),
              1U);
}

SRC_GRAPH(RematConstants, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();
        CONSTANT(13U, 1000U).s32();
        CONSTANT(3U, 0.5_D).f64();
        CONSTANT(4U, 10.0_D).f64();
        CONSTANT(5U, 26.66_D).f64();
        CONSTANT(14U, 2.0_D).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(7U, Opcode::Add).s32().Inputs(6U, 2U);
            INST(15U, Opcode::Add).s32().Inputs(7U, 13U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).f64().InputsAutoType(15U, 3U, 4U, 20U);
            INST(9U, Opcode::Add).f64().Inputs(4U, 5U);
            INST(10U, Opcode::Add).f64().Inputs(8U, 3U);
            INST(16U, Opcode::Add).f64().Inputs(10U, 14U);
            INST(21U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).f64().InputsAutoType(0U, 1U, 2U, 3U, 4U, 5U, 13U, 14U, 16U, 21U);
            INST(12U, Opcode::Return).f64().Inputs(11U);
        }
    }
}

TEST_F(RegAllocLinearScanTest, RematConstants)
{
    auto graph = CreateEmptyGraph();
    src_graph::RematConstants::CREATE(graph);

    auto regalloc = RegAllocLinearScan(graph);
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFE1U : 0xFABFFFFFU;
    regalloc.SetRegMask(RegMask {regMask});
    regalloc.SetVRegMask(VRegMask {regMask});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    // Check inserted to the graph spill-fills
    EXPECT_TRUE(CheckImmediateSpillFill(&INS(15U), 1U));
    EXPECT_TRUE(CheckImmediateSpillFill(&INS(10U), 1U));
    EXPECT_TRUE(CheckImmediateSpillFill(&INS(16U), 1U));

    // Check call instruction's spill-fills
    auto callInst = INS(11U).CastToCallStatic();
    auto spillFill = callInst->GetPrev()->CastToSpillFill();
    for (auto sf : spillFill->GetSpillFills()) {
        if (sf.SrcType() == LocationType::IMMEDIATE) {
            auto inputConst = graph->GetSpilledConstant(sf.SrcValue());
            auto it = std::find_if(callInst->GetInputs().begin(), callInst->GetInputs().end(),
                                   [inputConst](Input input) { return input.GetInst() == inputConst; });
            EXPECT_NE(it, callInst->GetInputs().end());
        } else {
            EXPECT_TRUE(sf.GetSrc().IsAnyRegister());
        }
    }
}

TEST_F(RegAllocLinearScanTest, LoadPairPartDiffRegisters)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(11U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadArrayPairI).s64().Inputs(2U).Imm(0x0U);
            INST(4U, Opcode::LoadPairPart).s64().Inputs(3U).Imm(0x0U);
            INST(5U, Opcode::LoadPairPart).s64().Inputs(3U).Imm(0x1U);
            INST(10U, Opcode::Add).s64().Inputs(5U, 11U);
            INST(6U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(7U, Opcode::CallStatic).s64().InputsAutoType(4U, 5U, 10U, 6U);
            INST(8U, Opcode::Return).s64().Inputs(7U);
        }
    }
    auto regalloc = RegAllocLinearScan(GetGraph());
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xF3FFFFFFU : 0xFAFFFFFFU;
    regalloc.SetRegMask(RegMask {regMask});
    auto result = regalloc.Run();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    auto loadPairI = &INS(3U);
    EXPECT_NE(loadPairI->GetDstReg(0U), loadPairI->GetDstReg(1U));
}

TEST_F(RegAllocLinearScanTest, SpillRegistersAroundCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::CallStatic).u32().InputsAutoType(0U, 1U, 2U);
            INST(4U, Opcode::Add).u32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    auto regalloc = RegAllocLinearScan(GetGraph());
    ASSERT_TRUE(regalloc.Run());

    // parameter 0 should be splitted before call and split should be used by add
    auto spillFill = (&INS(3U))->GetPrev();
    EXPECT_TRUE(spillFill->IsSpillFill());
    auto sf = spillFill->CastToSpillFill()->GetSpillFill(0U);
    auto paramDst = (&INS(0U))->GetDstReg();
    auto callSrc = (&INS(3U))->GetSrcReg(0U);
    auto addSrc = (&INS(4U))->GetSrcReg(0U);
    ASSERT_EQ(sf.SrcValue(), paramDst);
    if (callSrc == paramDst) {
        // param -> R1 (caller-saved assigned)
        // R1 -> Rx
        // call(R1, ..)
        ASSERT_EQ(addSrc, sf.DstValue());
    } else {
        // param -> Rx (callee-saved assigned)
        // Rx -> R1
        // call(R1, ..)
        ASSERT_EQ(addSrc, sf.SrcValue());
    }
    // all caller-saved regs should be spilled at call
    auto &lr = GetGraph()->GetAnalysis<LiveRegisters>();
    auto graph = GetGraph();
    lr.VisitIntervalsWithLiveRegisters(&INS(3), [graph](const auto &li) {
        auto rd = graph->GetRegisters();
        auto callerMask =
            DataType::IsFloatType(li->GetType()) ? rd->GetCallerSavedVRegMask() : rd->GetCallerSavedRegMask();
        ASSERT_FALSE(callerMask.Test(li->GetReg()));
    });
}

TEST_F(RegAllocLinearScanTest, SplitCallIntervalAroundNextCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::CallStatic).InputsAutoType(0U, 1U, 2U).f64();
            INST(4U, Opcode::Add).Inputs(3U, 3U).f64();
            INST(5U, Opcode::SaveState).Inputs(3U, 4U).SrcVregs({3U, 4U});
            INST(6U, Opcode::CallStatic).InputsAutoType(5U).u64();
            INST(7U, Opcode::Return).Inputs(3U).f64();
        }
    }

    auto regalloc = RegAllocLinearScan(GetGraph());
    uint32_t regMask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFAAU;
    regalloc.SetRegMask(RegMask {regMask});
    regalloc.SetVRegMask(RegMask {regMask});
    ASSERT_TRUE(regalloc.Run());

    // Spill after last use before next call
    auto spill = INS(4U).GetNext()->CastToSpillFill()->GetSpillFill(0U);
    // Fill before firt use after call
    auto fill = INS(7U).GetPrev()->CastToSpillFill()->GetSpillFill(0U);
    auto callReg = Location::MakeFpRegister(INS(3U).GetDstReg());

    EXPECT_EQ(spill.GetSrc(), callReg);
    EXPECT_EQ(spill.GetDst(), fill.GetSrc());
    EXPECT_EQ(callReg, fill.GetDst());
}

static bool CheckInstsDstRegs(Inst *inst0, const Inst *inst1, Register reg)
{
    EXPECT_EQ(inst0->GetDstReg(), reg);
    EXPECT_EQ(inst1->GetDstReg(), reg);

    // Should be spill-fill before second instruction to spill the first one;
    for (auto inst : InstIter(*inst0->GetBasicBlock(), inst0)) {
        if (inst == inst1) {
            break;
        }
        if (inst->IsSpillFill()) {
            auto sfs = inst->CastToSpillFill()->GetSpillFills();
            auto it = std::find_if(sfs.cbegin(), sfs.cend(), [reg](auto sf) {
                return sf.SrcValue() == reg && sf.SrcType() == LocationType::REGISTER;
            });
            if (it != sfs.cend()) {
                return true;
            }
        }
    }
    return false;
}

TEST_F(RegAllocLinearScanTest, PreassignedRegisters)
{
    // Check with different masks:
    RegMask fullMask {0x0U};          // All registers are available for RA
    RegMask shortMask {0xFFFFFFAAU};  // only {R0, R2, R4, R6} are available for RA
    RegMask shortMaskInvert {0x55U};  // {R0, R2, R4, R6} are NOT available for RA

    for (auto &mask : {fullMask, shortMask, shortMaskInvert}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            BASIC_BLOCK(2U, -1L)
            {
                INST(10U, Opcode::SaveState).Inputs().SrcVregs({});
                INST(0U, Opcode::CallStatic).InputsAutoType(10U).u32().DstReg(0U);
                INST(1U, Opcode::CallStatic).InputsAutoType(10U).u32().DstReg(1U);
                INST(2U, Opcode::CallStatic).InputsAutoType(10U).u32().DstReg(2U);
                INST(3U, Opcode::Add).Inputs(0U, 1U).u32().DstReg(0U);
                INST(4U, Opcode::Add).Inputs(1U, 2U).u32().DstReg(1U);
                INST(5U, Opcode::Add).Inputs(0U, 2U).u32().DstReg(2U);
                INST(6U, Opcode::Add).Inputs(4U, 5U).u32();
                INST(7U, Opcode::SaveState).Inputs().SrcVregs({});
                INST(8U, Opcode::CallStatic).InputsAutoType(0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U).u32();
                INST(9U, Opcode::Return).Inputs(8U).u32();
            }
        }
        auto regalloc = RegAllocLinearScan(graph);
        regalloc.SetRegMask(mask);
        ASSERT_TRUE(regalloc.Run());
        ASSERT_TRUE(CheckInstsDstRegs(&INS(0U), &INS(3U), Register(0U)));
        ASSERT_TRUE(CheckInstsDstRegs(&INS(1U), &INS(4U), Register(1U)));
        ASSERT_TRUE(CheckInstsDstRegs(&INS(2U), &INS(5U), Register(2U)));
    }
}

TEST_F(RegAllocLinearScanTest, Select3Regs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(4U, Opcode::Select).s64().SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0U, 1U, 2U, 3U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }

    auto regalloc = RegAllocLinearScan(GetGraph());
    auto regMask = GetGraph()->GetArch() == Arch::AARCH32 ? 0xFFFFFFABU : 0xFFFFFFF1U;
    regalloc.SetRegMask(RegMask {regMask});
    auto result = regalloc.Run();
    ASSERT_FALSE(result);
}

TEST_F(RegAllocLinearScanTest, TwoInstsWithZeroReg)
{
    auto zeroReg = GetGraph()->GetZeroReg();
    if (zeroReg == INVALID_REG) {
        return;
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U).s64();
        CONSTANT(1U, nullptr).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3U, Opcode::CallStatic).InputsAutoType(1U, 2U).s64();
            INST(4U, Opcode::Add).s64().Inputs(0U, 3U);
            INST(5U, Opcode::Return).ref().Inputs(1U);
        }
    }

    auto regalloc = RegAllocLinearScan(GetGraph());
    regalloc.Run();
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    auto const0 = la.GetInstLifeIntervals(&INS(0U));
    auto nullptrInst = la.GetInstLifeIntervals(&INS(1U));
    // Constant and Nullptr should not be split
    EXPECT_EQ(const0->GetSibling(), nullptr);
    EXPECT_EQ(nullptrInst->GetSibling(), nullptr);
    // Intervals' regs and dst's regs should be 'zero_reg'
    EXPECT_EQ(const0->GetReg(), zeroReg);
    EXPECT_EQ(nullptrInst->GetReg(), zeroReg);
    EXPECT_EQ(INS(0U).GetDstReg(), zeroReg);
    EXPECT_EQ(INS(1U).GetDstReg(), zeroReg);
    // Src's reg should be 'zero_reg'
    EXPECT_EQ(INS(4U).GetSrcReg(0U), zeroReg);
    if (INS(5U).GetPrev()->IsSpillFill()) {
        auto sf = INS(5U).GetPrev()->CastToSpillFill();
        EXPECT_EQ(sf->GetSpillFill(0U).SrcValue(), zeroReg);
        EXPECT_EQ(sf->GetSpillFill(0U).DstValue(), INS(5U).GetSrcReg(0U));
    } else {
        EXPECT_EQ(INS(5U).GetSrcReg(0U), zeroReg);
    }
}

template <unsigned int CONSTANTS_NUM>
bool RegAllocLinearScanTest::FillGraphWithConstants(Graph *graph)
{
    GRAPH(graph)
    {
        for (size_t i = 0; i < CONSTANTS_NUM; ++i) {
            CONSTANT(i, i);
        }

        BASIC_BLOCK(2U, -1L)
        {
            INST(CONSTANTS_NUM, Opcode::Add).s64().Inputs(CONSTANTS_NUM - 1L, CONSTANTS_NUM - 2L);
            for (int i = CONSTANTS_NUM - 3L, j = 1; i >= 0; --i, ++j) {
                INST(CONSTANTS_NUM + j, Opcode::Add).s64().Inputs(i, j - 1L);
            }
            INST(2U * CONSTANTS_NUM + 2U, Opcode::Return).s64().Inputs(2U * CONSTANTS_NUM - 2L);
        }
    }

    auto regalloc = RegAllocLinearScan(graph);
    return regalloc.Run();
}

// Ensure that we can spill at least 255 constants to graph only:
TEST_F(RegAllocLinearScanTest, SpillConstantsGraph)
{
    ASSERT_TRUE(compiler::g_options.IsCompilerRematConst());
    ASSERT_TRUE(FillGraphWithConstants<255U>(GetGraph()));

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    for (auto interval : la.GetLifeIntervals()) {
        auto sibling = interval;
        while (sibling != nullptr) {
            if (!sibling->IsPhysical() && sibling->GetInst()->IsConst()) {
                auto loc = sibling->GetLocation();
                EXPECT_TRUE(loc.IsConstant() || loc.IsRegister()) << *sibling->GetInst();
            }
            sibling = sibling->GetSibling();
        }
    }
}

// Ensure that we can spill at least 2*255 constants to stack/graph:
TEST_F(RegAllocLinearScanTest, SpillConstantsGraphStack)
{
    ASSERT_TRUE(compiler::g_options.IsCompilerRematConst());
    ASSERT_TRUE(FillGraphWithConstants<510U>(GetGraph()));

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    for (auto interval : la.GetLifeIntervals()) {
        auto sibling = interval;
        while (sibling != nullptr) {
            if (!sibling->IsPhysical() && sibling->GetInst()->IsConst()) {
                auto loc = sibling->GetLocation();
                EXPECT_TRUE(loc.IsConstant() || loc.IsRegister() || loc.IsStack()) << *sibling->GetInst();
            }
            sibling = sibling->GetSibling();
        }
    }
}

// There are no available imm slots nor stack slots:
TEST_F(RegAllocLinearScanTest, SpillConstantsLimit)
{
    ASSERT_TRUE(compiler::g_options.IsCompilerRematConst());
    ASSERT_FALSE(FillGraphWithConstants<513U>(GetGraph()));
}

TEST_F(RegAllocLinearScanTest, ParameterWithUnavailableRegister)
{
    if (GetGraph()->GetArch() != Arch::AARCH32) {
        GTEST_SKIP() << "Unsupported target architecture";
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();
        PARAMETER(2U, 2U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u32().Inputs(0U, 1U);
            INST(4U, Opcode::Add).u32().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }

    RegAllocLinearScan ra(GetGraph());
    // r1 is blocked (but param 0 resides there)
    uint32_t regMask = 0xFFFFFFF3U;
    ra.SetRegMask(RegMask {regMask});
    ra.SetVRegMask(VRegMask {0U});
    // we can't assign a register to first parameter, so allocation will fail
    ASSERT_FALSE(ra.Run());
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
