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

#include "compiler_logger.h"
#include "if_conversion.h"

#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace ark::compiler {
bool IfConversion::RunImpl()
{
    COMPILER_LOG(DEBUG, IFCONVERSION) << "Run " << GetPassName();
    bool result = false;
    // Post order (without reverse)
    for (auto it = GetGraph()->GetBlocksRPO().rbegin(); it != GetGraph()->GetBlocksRPO().rend(); it++) {
        auto block = *it;
        if (block->GetSuccsBlocks().size() == MAX_SUCCS_NUM) {
            result |= TryTriangle(block) || TryDiamond(block);
        }
    }
    result |= TryOptimizeSelectInsts();
    COMPILER_LOG(DEBUG, IFCONVERSION) << GetPassName() << " complete";
    return result;
}

/*
   The methods handles the recognition of the following pattern that have two variations:
   BB -- basic block the recognition starts from
   TBB -- TrueSuccessor of BB
   FBB -- FalseSuccessor of BB

       P0          P1
      [BB]        [BB]
       |  \        |  \
       |  [TBB]    |  [FBB]
       |  /        |  /
      [FBB]       [TBB]
*/
bool IfConversion::TryTriangle(BasicBlock *bb)
{
    ASSERT(bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    BasicBlock *tbb;
    BasicBlock *fbb;
    bool swapped = false;
    // interpret P1 variation as P0
    if (bb->GetTrueSuccessor()->GetPredsBlocks().size() == 1) {
        tbb = bb->GetTrueSuccessor();
        fbb = bb->GetFalseSuccessor();
    } else if (bb->GetFalseSuccessor()->GetPredsBlocks().size() == 1) {
        tbb = bb->GetFalseSuccessor();
        fbb = bb->GetTrueSuccessor();
        swapped = true;
    } else {
        return false;
    }

    if (tbb->GetSuccsBlocks().size() > 1 || fbb->GetPredsBlocks().size() <= 1 || tbb->GetSuccessor(0) != fbb) {
        return false;
    }

    if (LoopInvariantPreventConversion(bb)) {
        return false;
    }

    uint32_t instCount = 0;
    uint32_t phiCount = 0;
    auto limit = GetIfcLimit(bb);
    if (IsConvertable(tbb, &instCount) && IsPhisAllowed(fbb, tbb, bb, &phiCount)) {
        COMPILER_LOG(DEBUG, IFCONVERSION)
            << "Triangle pattern was found in Block #" << bb->GetId() << " with " << instCount
            << " convertible instruction(s) and " << phiCount << " Phi(s) to process";
        if (instCount <= limit && phiCount <= limit) {
            // Joining tbb into bb
            bb->JoinBlocksUsingSelect(tbb, nullptr, swapped);

            if (fbb->GetPredsBlocks().size() == 1 && bb->IsTry() == fbb->IsTry()) {
                bb->JoinSuccessorBlock();  // join fbb
                COMPILER_LOG(DEBUG, IFCONVERSION) << "Merged blocks " << tbb->GetId() << " and " << fbb->GetId()
                                                  << " into " << bb->GetId() << " using Select";
                GetGraph()->GetEventWriter().EventIfConversion(bb->GetId(), bb->GetGuestPc(), "Triangle", tbb->GetId(),
                                                               fbb->GetId(), -1);
            } else {
                COMPILER_LOG(DEBUG, IFCONVERSION)
                    << "Merged block " << tbb->GetId() << " into " << bb->GetId() << " using Select";
                GetGraph()->GetEventWriter().EventIfConversion(bb->GetId(), bb->GetGuestPc(), "Triangle_Phi",
                                                               tbb->GetId(), -1, -1);
            }
            return true;
        }
    }

    return false;
}

uint32_t IfConversion::GetIfcLimit(BasicBlock *bb)
{
    ASSERT(bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    auto trueCounter = GetGraph()->GetBranchCounter(bb, true);
    auto falseCounter = GetGraph()->GetBranchCounter(bb, false);
    if (trueCounter == 0 && falseCounter == 0) {
        return limit_;
    }
    auto minCounter = std::min(trueCounter, falseCounter);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto percent = (minCounter * 100) / (trueCounter + falseCounter);
    if (percent < g_options.GetCompilerIfConversionIncraseLimitThreshold()) {
        return limit_;
    }
    // The limit is increased by 4 times for branch with a large number of mispredicts
    return limit_ << 2U;
}

/*
   The methods handles the recognition of the following pattern:
   BB -- basic block the recognition starts from
   TBB -- TrueSuccessor of BB
   FBB -- FalseSuccessor of BB
   JBB -- Joint basic block of branches

      [BB]
     /    \
   [TBB] [FBB]
     \    /
      [JBB]
*/
bool IfConversion::TryDiamond(BasicBlock *bb)
{
    ASSERT(bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    BasicBlock *tbb = bb->GetTrueSuccessor();
    if (tbb->GetPredsBlocks().size() != 1 || tbb->GetSuccsBlocks().size() != 1) {
        return false;
    }

    BasicBlock *fbb = bb->GetFalseSuccessor();
    if (fbb->GetPredsBlocks().size() != 1 || fbb->GetSuccsBlocks().size() != 1) {
        return false;
    }

    BasicBlock *jbb = tbb->GetSuccessor(0);
    if (jbb->GetPredsBlocks().size() <= 1 || fbb->GetSuccessor(0) != jbb) {
        return false;
    }

    if (LoopInvariantPreventConversion(bb)) {
        return false;
    }

    uint32_t tbbInst = 0;
    uint32_t fbbInst = 0;
    uint32_t phiCount = 0;
    auto limit = GetIfcLimit(bb);
    if (IsConvertable(tbb, &tbbInst) && IsConvertable(fbb, &fbbInst) && IsPhisAllowed(jbb, tbb, fbb, &phiCount)) {
        COMPILER_LOG(DEBUG, IFCONVERSION)
            << "Diamond pattern was found in Block #" << bb->GetId() << " with " << (tbbInst + fbbInst)
            << " convertible instruction(s) and " << phiCount << " Phi(s) to process";
        if (tbbInst + fbbInst <= limit && phiCount <= limit) {
            // Joining tbb into bb
            bb->JoinBlocksUsingSelect(tbb, fbb, false);

            bb->JoinSuccessorBlock();  // joining fbb

            if (jbb->GetPredsBlocks().size() == 1 && bb->IsTry() == jbb->IsTry()) {
                bb->JoinSuccessorBlock();  // joining jbb
                COMPILER_LOG(DEBUG, IFCONVERSION) << "Merged blocks " << tbb->GetId() << ", " << fbb->GetId() << " and "
                                                  << jbb->GetId() << " into " << bb->GetId() << " using Select";
                GetGraph()->GetEventWriter().EventIfConversion(bb->GetId(), bb->GetGuestPc(), "Diamond", tbb->GetId(),
                                                               fbb->GetId(), jbb->GetId());
            } else {
                COMPILER_LOG(DEBUG, IFCONVERSION) << "Merged blocks " << tbb->GetId() << " and " << fbb->GetId()
                                                  << " into " << bb->GetId() << " using Select";
                GetGraph()->GetEventWriter().EventIfConversion(bb->GetId(), bb->GetGuestPc(), "Diamond_Phi",
                                                               tbb->GetId(), fbb->GetId(), -1);
            }
            return true;
        }
    }
    return false;
}

bool IfConversion::LoopInvariantPreventConversion(BasicBlock *bb)
{
    if (!g_options.IsCompilerLicmConditions()) {
        // Need to investigate may be it is always better to avoid IfConv for loop invariant condition
        return false;
    }
    if (!bb->IsLoopValid()) {
        return false;
    }
    auto loop = bb->GetLoop();
    if (loop->IsRoot()) {
        return false;
    }
    auto lastInst = bb->GetLastInst();
    for (auto &input : lastInst->GetInputs()) {
        if (input.GetInst()->GetBasicBlock()->GetLoop() == loop) {
            return false;
        }
    }
    return true;
}

bool IfConversion::IsConvertable(BasicBlock *bb, uint32_t *instCount)
{
    uint32_t total = 0;
    for (auto inst : bb->AllInsts()) {
        ASSERT(inst->GetOpcode() != Opcode::Phi);
        if (!inst->IsIfConvertable()) {
            return false;
        }
        total += static_cast<uint32_t>(inst->HasUsers());
    }
    *instCount = total;
    return true;
}

bool IfConversion::IsPhisAllowed(BasicBlock *bb, BasicBlock *pred1, BasicBlock *pred2, uint32_t *phiCount)
{
    uint32_t total = 0;

    for (auto phi : bb->PhiInsts()) {
        size_t index1 = phi->CastToPhi()->GetPredBlockIndex(pred1);
        size_t index2 = phi->CastToPhi()->GetPredBlockIndex(pred2);
        ASSERT(index1 != index2);

        auto inst1 = phi->GetInput(index1).GetInst();
        auto inst2 = phi->GetInput(index2).GetInst();
        if (inst1 == inst2) {
            // Otherwise DCE should remove Phi
            [[maybe_unused]] constexpr auto IMM_2 = 2;
            ASSERT(bb->GetPredsBlocks().size() > IMM_2);
            // No select needed
            continue;
        }

        // Select can be supported for float operands on the specific architectures (arm64 now)
        if (DataType::IsFloatType(phi->GetType()) && !canEncodeFloatSelect_) {
            return false;
        }

        // NOTE: LICM conditions pass moves condition chain invariants outside the loop
        // and consider branch probability. IfConversion can replaces it with SelectInst
        // but their If inst users lose branch probability information. This code prevents
        // such conversion until IfConversion can estimate branch probability.
        if (IsConditionChainPhi(phi)) {
            return false;
        }

        // One more Select
        total++;
    }
    *phiCount = total;
    return true;
}

bool IfConversion::IsConditionChainPhi(Inst *phi)
{
    if (!g_options.IsCompilerLicmConditions()) {
        return false;
    }

    auto loop = phi->GetBasicBlock()->GetLoop();
    for (auto &user : phi->GetUsers()) {
        auto userBb = user.GetInst()->GetBasicBlock();
        if (!userBb->GetLoop()->IsInside(loop)) {
            return false;
        }
    }
    return true;
}

bool IfConversion::TryOptimizeSelectInsts()
{
    bool result = false;
    for (BasicBlock *bb : GetGraph()->GetBlocksRPO()) {
        for (Inst *inst : bb->AllInsts()) {
            if (inst->GetOpcode() == Opcode::Select || inst->GetOpcode() == Opcode::SelectImm) {
                result |= TryOptimizeSelectInst(inst);
            }
        }
    }
    return result;
}

bool IfConversion::TryOptimizeSelectInst(Inst *selectInst)
{
    Inst *v0 = selectInst->GetInput(0).GetInst();
    Inst *v1 = selectInst->GetInput(1).GetInst();
    return TryOptimizeSelectInstByMergingUnaryOperation(selectInst, v0, v1) ||
           TryOptimizeSelectInstWithIncrement(selectInst, v0, v1) ||
           TryOptimizeSelectInstWithBitwiseInversion(selectInst, v0, v1) ||
           TryOptimizeSelectInstWithArithmeticNegation(selectInst, v0, v1);
}

static bool UnaryOperationsAreMergable(Inst *v0, Inst *v1, bool *hasImm, uint64_t *imm)
{
    if (v0->GetOpcode() != v1->GetOpcode() || v0->GetType() != v1->GetType()) {
        return false;
    }
    switch (v0->GetOpcode()) {
        case Opcode::Neg:
        case Opcode::Abs:
        case Opcode::Not:
            *hasImm = false;
            return true;
        case Opcode::AddI:
        case Opcode::SubI:
        case Opcode::MulI:
        case Opcode::DivI:
        case Opcode::ModI:
        case Opcode::ShlI:
        case Opcode::ShrI:
        case Opcode::AShrI:
        case Opcode::AndI:
        case Opcode::OrI:
        case Opcode::XorI:
            *hasImm = true;
            *imm = static_cast<BinaryImmOperation *>(v0)->GetImm();
            return (static_cast<BinaryImmOperation *>(v1)->GetImm() == *imm);
        default:
            return false;
    }
}

// Case: (SomeUnaryOp see above)
// 1. SomeUnaryOp vA
// 2. SomeUnaryOp vB
// 3. Select[Imm] v1, v2, vX, vY|Imm, CC
// ===>
// 1. Select[Imm] vA, vB, vX, vY|Imm, CC    <= Optimizations can be applied recursively
// 2. SomeUnaryOp v1
bool IfConversion::TryOptimizeSelectInstByMergingUnaryOperation(Inst *selectInst, Inst *v0, Inst *v1)
{
    bool hasImm = false;
    uint64_t imm = 0;
    if (!UnaryOperationsAreMergable(v0, v1, &hasImm, &imm)) {
        return false;
    }
    ASSERT(v0->GetType() == selectInst->GetType());

    selectInst->SetInput(0, v0->GetInput(0).GetInst());
    selectInst->SetInput(1, v1->GetInput(0).GetInst());

    Inst *delayedUnaryInst = GetGraph()->CreateInst(v0->GetOpcode());
    selectInst->ReplaceUsers(delayedUnaryInst);
    delayedUnaryInst->SetType(selectInst->GetType());
    delayedUnaryInst->SetInput(0, selectInst);
    if (hasImm) {
        static_cast<BinaryImmOperation *>(delayedUnaryInst)->SetImm(imm);
    }
    selectInst->InsertAfter(delayedUnaryInst);
    // Applies optimization recursively
    TryOptimizeSelectInst(selectInst);
    return true;
}

// Case 1.1: v1 and v2 share the same operand, v2 == v1 + 1
//   1. AddI vA, n                                              or: SubI vA, n
//   2. AddI vA, n+1                                            or: SubI vA, n-1
//   3. Select[Imm] v1, v2, vX, vY|Imm, CC
//   ====>
//   1. AddI vA, n                                              or: SubI vA, n
//   2. Select[Imm]Transform v1, v1, vX, vY|Imm, CC, INC
// Case 1.2: v1 and v2 share the same operand, v1 == v2 + 1
//   1. AddI vA, n                                              or: SubI vA, n
//   2. AddI vA, n-1                                            or: SubI vA, n+1
//   3. Select[Imm] v1, v2, vX, vY|Imm, CC
//   ====>
//   1. AddI vA, n-1
//   2. Select[Imm]Transform v1, v1, vX, vY|Imm, inv(CC), INC   <= Condition code inversed
// Case 2.1: Merges vB + 1 to Select[Imm]Transform INC
//   1. AddI vB, 1
//   2. Select[Imm] vA, v1, vX, vY|Imm, CC
//   ====>
//   1. Select[Imm]Transform vA, vB, vX, vY|Imm, CC, INC
// Case 2.2: Merges vA + 1 to Select[Imm]Transform INC
//   1. AddI vA, 1
//   2. Select[Imm] v1, vB, vX, vY|Imm, CC
//   ====>
//   1. Select[Imm]Transform vB, vA, vX, vY|Imm, inv(CC), INC   <= Condition code inversed
bool IfConversion::TryOptimizeSelectInstWithIncrement(Inst *selectInst, Inst *v0, Inst *v1)
{
    ASSERT(selectInst->GetOpcode() == Opcode::Select || selectInst->GetOpcode() == Opcode::SelectImm);
    constexpr auto TRANSFORM = SelectTransformType::INC;
    auto opc0 = v0->GetOpcode();
    auto opc1 = v1->GetOpcode();
    if ((opc0 == Opcode::AddI && opc1 == Opcode::AddI) || (opc0 == Opcode::SubI && opc1 == Opcode::SubI)) {
        Inst *u0 = v0->GetInput(0).GetInst();
        Inst *u1 = v1->GetInput(0).GetInst();
        if (u0 == u1) {
            uint64_t imm0 = static_cast<BinaryImmOperation *>(v0)->GetImm();
            uint64_t imm1 = static_cast<BinaryImmOperation *>(v1)->GetImm();
            bool v1IsGreaterByOne =
                (opc0 == Opcode::AddI && imm1 == imm0 + 1) || (opc0 == Opcode::SubI && imm0 == imm1 + 1);
            bool v0IsGreaterByOne =
                (opc0 == Opcode::AddI && imm1 == imm0 - 1) || (opc0 == Opcode::SubI && imm0 == imm1 - 1);
            // Case (1.1): inputsSwapped=false, secondInputIsIdentity=true
            if (v1IsGreaterByOne && OptimizeSelectInstWithTransform(TRANSFORM, selectInst, v0, v0, false, true)) {
                return true;
            }
            // Case (1.2): inputsSwapped=true, secondInputIsIdentity=true
            if (v0IsGreaterByOne && OptimizeSelectInstWithTransform(TRANSFORM, selectInst, v1, v1, true, true)) {
                return true;
            }
        }
    }
    if (v1->GetOpcode() == Opcode::AddI && v1->CastToAddI()->GetImm() == 1) {
        if (OptimizeSelectInstWithTransform(TRANSFORM, selectInst, v0, v1, false)) {
            return true;  // Case (2.1)
        }
    }
    if (v0->GetOpcode() == Opcode::AddI && v0->CastToAddI()->GetImm() == 1) {
        if (OptimizeSelectInstWithTransform(TRANSFORM, selectInst, v1, v0, true)) {
            return true;  // Case (2.2)
        }
    }
    return false;
}

// Case 1: Merges ~vB to Select[Imm]Transform INV
//   1. Not vB
//   2. Select[Imm] vA, v1, vX, vY|Imm, CC
//   ====>
//   1. Select[Imm]Transform vA, vB, vX, vY|Imm, CC, INV
// Case 2: Merges ~vA to Select[Imm]Transform INV
//   1. Not vA
//   2. Select[Imm] v1, vB, vX, vY|Imm, CC
//   ====>
//   1. Select[Imm]Transform vB, vA, vX, vY|Imm, inv(CC), INV   <= Condition code inversed
bool IfConversion::TryOptimizeSelectInstWithBitwiseInversion(Inst *selectInst, Inst *v0, Inst *v1)
{
    ASSERT(selectInst->GetOpcode() == Opcode::Select || selectInst->GetOpcode() == Opcode::SelectImm);
    constexpr auto TRANSFORM = SelectTransformType::INV;
    if (v1->GetOpcode() == Opcode::Not && OptimizeSelectInstWithTransform(TRANSFORM, selectInst, v0, v1, false)) {
        return true;
    }
    if (v0->GetOpcode() == Opcode::Not && OptimizeSelectInstWithTransform(TRANSFORM, selectInst, v1, v0, true)) {
        return true;
    }
    return false;
}

// Case 1: Merges -vB to Select[Imm]Transform NEG
//   1. Neg vB
//   2. Select[Imm] vA, v1, vX, vY|Imm, CC
//   ====>
//   1. Select[Imm]Transform vA, vB, vX, vY|Imm, CC, NEG
// Case 2: Merges -vB to Select[Imm]Transform NEG
//   1. Neg vA
//   2. Select[Imm] v1, vB, vX, vY|Imm, CC
//   ====>
//   1. Select[Imm]Transform vB, vA, vX, vY|Imm, inv(CC), NEG   <= Condition code inversed
bool IfConversion::TryOptimizeSelectInstWithArithmeticNegation(Inst *selectInst, Inst *v0, Inst *v1)
{
    ASSERT(selectInst->GetOpcode() == Opcode::Select || selectInst->GetOpcode() == Opcode::SelectImm);
    constexpr auto TRANSFORM = SelectTransformType::NEG;
    if (v1->GetOpcode() == Opcode::Neg && OptimizeSelectInstWithTransform(TRANSFORM, selectInst, v0, v1, false)) {
        return true;
    }
    if (v0->GetOpcode() == Opcode::Neg && OptimizeSelectInstWithTransform(TRANSFORM, selectInst, v1, v0, true)) {
        return true;
    }
    return false;
}

// Note: inputs (v0, v1) are expected to be swapped by the caller.
// CC-OFFNXT(G.FUN.01-CPP) Internal helper function
bool IfConversion::OptimizeSelectInstWithTransform(SelectTransformType transform, Inst *selectInst, Inst *v0, Inst *v1,
                                                   bool inputsSwapped, bool secondInputIsIdentity)
{
    Encoder *enc = GetGraph()->GetEncoder();
    auto t0 = v0->GetType();
    auto t1 = v1->GetType();
    if (t0 != t1 || !enc->CanEncodeSelectTransformFor(t0)) {
        return false;
    }
    Inst *u1 = secondInputIsIdentity ? v1 : v1->GetInput(0).GetInst();
    Inst *x0 = selectInst->GetInput(2).GetInst();
    Inst *newSelectInst = nullptr;

    if (selectInst->GetOpcode() == Opcode::Select) {
        Inst *x1 = selectInst->GetInput(3).GetInst();
        auto cc = selectInst->CastToSelect()->GetCc();
        if (inputsSwapped) {
            cc = GetInverseConditionCode(cc);
        }
        newSelectInst = GetGraph()->CreateInstSelectTransform(selectInst, std::array {v0, u1, x0, x1}, x0->GetType(),
                                                              cc, transform);
    } else {
        ASSERT(selectInst->GetOpcode() == Opcode::SelectImm);
        uint64_t x1Imm = selectInst->CastToSelectImm()->GetImm();
        auto cc = selectInst->CastToSelectImm()->GetCc();
        if (inputsSwapped) {
            cc = GetInverseConditionCode(cc);
        }
        newSelectInst = GetGraph()->CreateInstSelectImmTransform(selectInst, std::array {v0, u1, x0}, x1Imm,
                                                                 x0->GetType(), cc, transform);
    }
    selectInst->InsertBefore(newSelectInst);
    selectInst->ReplaceUsers(newSelectInst);
    return true;
}

void IfConversion::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
}
}  // namespace ark::compiler
