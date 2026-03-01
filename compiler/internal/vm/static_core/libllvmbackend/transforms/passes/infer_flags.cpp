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

#include "infer_flags.h"

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Analysis/MemoryBuiltins.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Operator.h>
#include <llvm/Support/KnownBits.h>

#include "transforms/transform_utils.h"

#define DEBUG_TYPE "infer-flags"

namespace {

bool CanOverflow(const llvm::KnownBits &start, const llvm::KnownBits &step, uint64_t tripCount)
{
    ASSERT(start.getBitWidth() == step.getBitWidth());
    // Check overflow for simple recurrence like:
    //     i32 v0 = phi i32 [start bb0, v1 bb1]
    // Where:
    //     v1 = op v0, step

    // Map range [stepMin; stepMax) to [stepMin * tripCount; step * tripCount)
    auto tripByStep = llvm::ConstantRange::fromKnownBits(
        llvm::KnownBits::mul(step, llvm::KnownBits::makeConstant(llvm::APInt {step.getBitWidth(), tripCount})), true);

    // Get signed ranges for step, and start
    auto stepRange = llvm::ConstantRange::fromKnownBits(step, true);
    auto startRange = llvm::ConstantRange::fromKnownBits(start, true);
    // Check actual overflow
    bool overflow = startRange.signedAddMayOverflow(tripByStep) != llvm::ConstantRange::OverflowResult::NeverOverflows;
    LLVM_DEBUG(llvm::dbgs() << "stepRange = " << stepRange << ", startRange = " << startRange
                            << ", tripByStep = " << tripByStep << ", tripCount = " << tripCount
                            << ", canOverflow = " << llvm::toStringRef(overflow) << "\n");
    return overflow;
}
}  // namespace

namespace ark::llvmbackend::passes {

bool InferFlags::ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return true;
}

llvm::PreservedAnalyses InferFlags::run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager)
{
    LLVM_DEBUG(llvm::dbgs() << "Running on '" << function.getName() << "'\n");

    bool changed = false;

    auto &scalarEvolution = analysisManager.getResult<llvm::ScalarEvolutionAnalysis>(function);
    auto &loopAnalysis = analysisManager.getResult<llvm::LoopAnalysis>(function);

    for (auto &loop : loopAnalysis) {
        changed |= RunOnLoop(loop, &scalarEvolution);
    }

    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

bool InferFlags::RunOnLoop(llvm::Loop *loop, llvm::ScalarEvolution *scalarEvolution)
{
    bool changed = false;
    for (auto basicBlock : loop->blocks()) {
        changed |= RunOnBasicBlock(loop, basicBlock, scalarEvolution);
    }
    return changed;
}

bool InferFlags::RunOnBasicBlock(llvm::Loop *loop, llvm::BasicBlock *basicBlock, llvm::ScalarEvolution *scalarEvolution)
{
    bool changed = false;

    for (auto &phi : basicBlock->phis()) {
        if (!scalarEvolution->isSCEVable(phi.getType())) {
            continue;
        }
        llvm::BinaryOperator *binaryOperator;
        llvm::Value *step;
        llvm::Value *start;

        LLVM_DEBUG(llvm::dbgs() << "Trying to match simple  recurrence for phi '" << phi << "'\n");
        if (!llvm::matchSimpleRecurrence(&phi, binaryOperator, start, step)) {
            continue;
        }
        LLVM_DEBUG(llvm::dbgs() << "Matched simple recurrence '" << *binaryOperator << "'\n");
        if (!llvm::isa<llvm::OverflowingBinaryOperator>(binaryOperator)) {
            continue;
        }
        // Support only add because sub is untested
        if (binaryOperator->getOpcode() != llvm::Instruction::Add) {
            continue;
        }

        auto tripCount = scalarEvolution->getSmallConstantMaxTripCount(loop);
        LLVM_DEBUG(llvm::dbgs() << "tripCount = '" << tripCount << "'\n");
        if (tripCount == 0) {
            continue;
        }
        // Now infer range
        auto dataLayout = basicBlock->getModule()->getDataLayout();
        auto knownStart = llvm::computeKnownBits(start, dataLayout);
        auto knownStep = llvm::computeKnownBits(step, dataLayout);
        if (knownStart.isUnknown() || knownStep.isUnknown()) {
            LLVM_DEBUG(llvm::dbgs() << "Start or step is unknown\n");
            continue;
        }
        ASSERT(!phi.getType()->isPointerTy());
        if (!binaryOperator->hasNoSignedWrap() && !CanOverflow(knownStart, knownStep, tripCount)) {
            LLVM_DEBUG(llvm::dbgs() << "Set nsw to '" << *binaryOperator << "'\n");
            binaryOperator->setHasNoSignedWrap(true);
            changed = true;
        }
    }
    return changed;
}

}  // namespace ark::llvmbackend::passes
