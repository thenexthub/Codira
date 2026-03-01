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

#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/optimizations/balance_expressions.h"
#include "compiler_logger.h"

namespace ark::compiler {
bool BalanceExpressions::RunImpl()
{
    processedInstMrk_ = GetGraph()->NewMarker();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        ProcessBB(bb);
    }
    GetGraph()->EraseMarker(processedInstMrk_);
    return isApplied_;
}

void BalanceExpressions::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
}

/**
 *  Iterate over instructions in reverse order, find every expression-chain by detecting
 *  the final operator of each chain, analyze the chain and optimize it if necessary.
 */
void BalanceExpressions::ProcessBB(BasicBlock *bb)
{
    ASSERT(bb != nullptr);
    SetBB(bb);

    auto it = bb->InstsReverse().begin();
    for (; it != it.end(); ++it) {
        ASSERT(*it != nullptr);
        if ((*it)->SetMarker(processedInstMrk_)) {
            // The instruction is already processed;
            continue;
        }
        if (SuitableInst(*it)) {
            // The final operator of the chain is found, start analyzing:
            auto instToContinueCycle = ProccesExpressionChain(*it);
            it.SetCurrent(instToContinueCycle);
        }
    }
}

bool BalanceExpressions::SuitableInst(Inst *inst)
{
    // Floating point operations are not associative:
    if (inst->IsCommutative() && !IsFloatType(inst->GetType())) {
        SetOpcode(inst->GetOpcode());
        return true;
    }
    return false;
}

Inst *BalanceExpressions::ProccesExpressionChain(Inst *lastOperator)
{
    ASSERT(lastOperator != nullptr);
    AnalyzeInputsRec(lastOperator);

    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "\nConsidering expression:";
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << *this;

    auto instToContinue = NeedsOptimization() ? OptimizeExpression(lastOperator->GetNext()) : lastOperator;

    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "Expression considered.";

    Reset();
    return instToContinue;
}

/**
 * Optimizes expression.
 *
 * By the end of the algorithm, `operators_.front()` points to the first instruction in expression and
 * `operators_.back()` points to the last (`operators_.front()` dominates `operators_.back()`).
 */
Inst *BalanceExpressions::OptimizeExpression(Inst *instAfterExpr)
{
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "Optimizing expression:";
    AllocateSourcesRec<true>(0, sources_.size() - 1);

    size_t size = operators_.size();
    operators_.front()->SetNext(operators_[1]);
    constexpr auto IMM_2 = 2;
    operators_.back()->SetPrev(operators_[size - IMM_2]);
    for (size_t i = 1; i < size - 1; i++) {
        operators_[i]->SetNext(operators_[i + 1]);
        operators_[i]->SetPrev(operators_[i - 1]);
    }
    if (instAfterExpr == nullptr) {
        GetBB()->AppendRangeInst(operators_.front(), operators_.back());
    } else {
        GetBB()->InsertRangeBefore(operators_.front(), operators_.back(), instAfterExpr);
    }

    SetIsApplied(true);
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "\nOptimized expression:";
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << *this;

    // Need to return pointer to the next instruction in order to correctly continue the cycle:
    Inst *instToContinueCycle = operators_.front();
    return instToContinueCycle;
}

/**
 * Generates expression for sources_ in range from @param first_idx to @param last_idx by splitting them on
 * two parts, calling itself for each part and binding them to an instruction from operators_.
 *
 * By the end of the algorithm, `operators_` are sorted in execution order
 * (`operators_.front()` is the first, `operators_.back()` is the last).
 */
template <bool IS_FIRST_CALL>
Inst *BalanceExpressions::AllocateSourcesRec(size_t firstIdx, size_t lastIdx)
{
    ASSERT(lastIdx > firstIdx);
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "Allocating operators for sources_[" << firstIdx << " to " << lastIdx << "]";
    size_t memSize = firstIdx + BitFloor(lastIdx - firstIdx);
    size_t splitIdx = memSize > 0 ? memSize - 1 : 0;

    Inst *lhs = GetOperand(firstIdx, splitIdx);
    Inst *rhs = LIKELY((splitIdx + 1) != lastIdx) ? GetOperand(splitIdx + 1, lastIdx) : sources_[splitIdx + 1];
    // `(split_idx + 1) == last_idx` means an odd number of `sources_` and we are considering
    // the last (unpaired) source. This situation may occur only with `rhs`.
    ASSERT(firstIdx != splitIdx);

    // Operator allocation:
    ASSERT(operatorsAllocIdx_ < operators_.size());
    Inst *allocatedOperator = operators_[operatorsAllocIdx_];
    operatorsAllocIdx_++;

    // Operator initialization:
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_FIRST_CALL) {
        // The first call allocates and generates at the same time the last operator of expression and
        // its users should be saved.
        allocatedOperator->RemoveInputs();
        allocatedOperator->GetBasicBlock()->EraseInst(allocatedOperator);
    } else {  // NOLINT(readability-misleading-indentation)
        allocatedOperator->GetBasicBlock()->RemoveInst(allocatedOperator);
    }
    allocatedOperator->SetBasicBlock(GetBB());
    allocatedOperator->SetInput(0, lhs);
    allocatedOperator->SetInput(1, rhs);

    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "sources_[" << firstIdx << " to " << lastIdx << "] allocated";
    return allocatedOperator;
}

Inst *BalanceExpressions::GetOperand(size_t firstIdx, size_t lastIdx)
{
    ASSERT(lastIdx > firstIdx);
    return (lastIdx - firstIdx == 1) ? GenerateElementalOperator(sources_[firstIdx], sources_[lastIdx])
                                     : AllocateSourcesRec<false>(firstIdx, lastIdx);
}

/**
 * Create an operator with direct sources
 * (i.e. `lhs` and `rhs` are from sources_)
 */
Inst *BalanceExpressions::GenerateElementalOperator(Inst *lhs, Inst *rhs)
{
    ASSERT(lhs && rhs);
    ASSERT(operatorsAllocIdx_ < operators_.size());
    Inst *allocatedOperator = operators_[operatorsAllocIdx_];
    operatorsAllocIdx_++;
    allocatedOperator->GetBasicBlock()->RemoveInst(allocatedOperator);

    allocatedOperator->SetBasicBlock(GetBB());

    // There is no need to clean users of lhs and rhs because it is cleaned during RemoveInst()
    // (as soon as every of operator_insts_ is removed before further usage)

    allocatedOperator->SetInput(0, lhs);
    allocatedOperator->SetInput(1, rhs);
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "Created an elemental operator:\n" << *allocatedOperator;
    return allocatedOperator;
}

/**
 * Recursively checks inputs.
 * Fills arrays of source-insts and opreation-insts.
 */
void BalanceExpressions::AnalyzeInputsRec(Inst *inst)
{
    exprCurDepth_++;
    ASSERT(inst != nullptr);

    auto lhsInput = inst->GetInput(0).GetInst();
    auto rhsInput = inst->GetInput(1).GetInst();

    TryExtendChainRec(lhsInput);
    TryExtendChainRec(rhsInput);
    operators_.push_back(inst);

    if (exprMaxDepth_ < exprCurDepth_) {
        exprMaxDepth_ = exprCurDepth_;
    }
    exprCurDepth_--;
}

/**
 * Recursively checks if the instruction should be added in the current expression chain.
 * If not, the considered instruction is the expression's term (source) and we save it for a later step.
 */
void BalanceExpressions::TryExtendChainRec(Inst *inst)
{
    ASSERT(inst);
    if (inst->GetOpcode() == GetOpcode()) {
        if (inst->HasSingleUser() && !inst->IsIntrinsic()) {
            inst->SetMarker(processedInstMrk_);

            AnalyzeInputsRec(inst);

            return;
        }
    }
    sources_.push_back(inst);
}

/**
 * Finds optimal depth and compares to the current.
 * Both of the numbers are represented as pow(x, 2).
 */
bool BalanceExpressions::NeedsOptimization()
{
    if (sources_.size() <= 3U) {
        return false;
    }
    // Avoid large shift exponent for size_t
    if (exprMaxDepth_ >= std::numeric_limits<size_t>::digits) {
        return false;
    }
    size_t current = 1UL << (exprMaxDepth_);
    size_t optimal = BitCeil(sources_.size());
    return current > optimal;
}

void BalanceExpressions::Reset()
{
    sources_.clear();
    operators_.clear();
    exprCurDepth_ = 0;
    exprMaxDepth_ = 0;
    operatorsAllocIdx_ = 0;
    SetOpcode(Opcode::INVALID);
}

void BalanceExpressions::Dump(std::ostream *out) const
{
    (*out) << "Sources:\n";
    for (auto i : sources_) {
        (*out) << *i << '\n';
    }

    (*out) << "Operators:\n";
    for (auto i : operators_) {
        (*out) << *i << '\n';
    }
}
}  // namespace ark::compiler
