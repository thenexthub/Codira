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
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/loop_unroll.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"

namespace ark::compiler {
bool LoopUnroll::RunImpl()
{
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Run " << GetPassName();
    RunLoopsVisitor();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << GetPassName() << " complete";
    GetGraph()->SetUnrollComplete();
    return isApplied_;
}

void LoopUnroll::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

template <typename T>
bool ConditionOverFlowImpl(const CountableLoopInfo &loopInfo, uint32_t unrollFactor)
{
    auto immValue = (static_cast<uint64_t>(unrollFactor) - 1) * loopInfo.constStep;
    auto testValue = static_cast<T>(loopInfo.test->CastToConstant()->GetIntValue());
    auto typeMin = std::numeric_limits<T>::min();
    auto typeMax = std::numeric_limits<T>::max();
    if (immValue > static_cast<uint64_t>(typeMax)) {
        return true;
    }
    if (loopInfo.isInc) {
        // condition will be updated: test_value - imm_value
        // so if (test_value - imm_value) < type_min, it's overflow
        return (typeMin + static_cast<T>(immValue)) > testValue;
    }
    // condition will be updated: test_value + imm_value
    // so if (test_value + imm_value) > type_max, it's overflow
    return (typeMax - static_cast<T>(immValue)) < testValue;
}

/// NOTE(a.popov) Create pre-header compare if it doesn't exist

bool ConditionOverFlow(const CountableLoopInfo &loopInfo, uint32_t unrollFactor)
{
    auto type = loopInfo.index->GetType();
    ASSERT(DataType::GetCommonType(type) == DataType::INT64);
    auto updateOpcode = loopInfo.update->GetOpcode();
    if (updateOpcode == Opcode::AddOverflowCheck || updateOpcode == Opcode::SubOverflowCheck) {
        return true;
    }
    if (!loopInfo.test->IsConst()) {
        return false;
    }

    switch (type) {
        case DataType::INT32:
            return ConditionOverFlowImpl<int32_t>(loopInfo, unrollFactor);
        case DataType::UINT32:
            return ConditionOverFlowImpl<uint32_t>(loopInfo, unrollFactor);
        case DataType::INT64:
            return ConditionOverFlowImpl<int64_t>(loopInfo, unrollFactor);
        case DataType::UINT64:
            return ConditionOverFlowImpl<uint64_t>(loopInfo, unrollFactor);
        default:
            return true;
    }
}

void LoopUnroll::TransformLoopImpl(Loop *loop, std::optional<uint64_t> optIterations, bool noSideExits,
                                   uint32_t unrollFactor, std::optional<CountableLoopInfo> loopInfo)
{
    auto graphCloner = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator());
    if (optIterations && noSideExits) {
        // GCC gives false positive here
#if !defined(__clang__)
#pragma GCC diagnostic push
// CC-OFFNXT(warning_suppression) GCC false positive
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        auto iterations = *optIterations;
        ASSERT(unrollFactor != 0);
        auto remainingIters = iterations % unrollFactor;
#if !defined(__clang__)
#pragma GCC diagnostic pop
#endif
        Loop *cloneLoop = remainingIters == 0 ? nullptr : graphCloner.CloneLoop(loop);
        // Unroll loop without side-exits and fix compare in the pre-header and back-edge
        graphCloner.UnrollLoopBody<UnrollType::UNROLL_WITHOUT_SIDE_EXITS>(loop, unrollFactor);
        FixCompareInst(loopInfo.value(), loop->GetHeader(), unrollFactor);
        // Unroll loop without side-exits for remaining iterations
        if (remainingIters != 0) {
            graphCloner.UnrollLoopBody<UnrollType::UNROLL_CONSTANT_ITERATIONS>(cloneLoop, remainingIters);
        }
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Unrolled without side-exits the loop with constant number of iterations (" << iterations
            << "). Loop id = " << loop->GetId();
    } else if (noSideExits) {
        auto cloneLoop = graphCloner.CloneLoop(loop);
        // Unroll loop without side-exits and fix compare in the pre-header and back-edge
        graphCloner.UnrollLoopBody<UnrollType::UNROLL_WITHOUT_SIDE_EXITS>(loop, unrollFactor);
        FixCompareInst(loopInfo.value(), loop->GetHeader(), unrollFactor);
        // Unroll loop with side-exits for remaining iterations
        graphCloner.UnrollLoopBody<UnrollType::UNROLL_POST_INCREMENT>(cloneLoop, unrollFactor - 1);
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Unrolled without side-exits the loop with unroll factor = " << unrollFactor
            << ". Loop id = " << loop->GetId();
    } else if (g_options.IsCompilerUnrollWithSideExits()) {
        graphCloner.UnrollLoopBody<UnrollType::UNROLL_WITH_SIDE_EXITS>(loop, unrollFactor);
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Unrolled with side-exits the loop with unroll factor = " << unrollFactor
                                            << ". Loop id = " << loop->GetId();
    }
}

bool LoopUnroll::TransformLoop(Loop *loop)
{
    if (!loop->GetInnerLoops().empty()) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Loop isn't unrolled since it contains loops. Loop id = " << loop->GetId();
        return false;
    }
    auto unrollParams = GetUnrollParams(loop);
    if (!g_options.IsCompilerUnrollLoopWithCalls() && unrollParams.hasCall) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Loop isn't unrolled since it contains calls. Loop id = " << loop->GetId();
        return false;
    }

    auto graphCloner = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator());
    uint32_t unrollFactor = std::min(unrollParams.unrollFactor, unrollFactor_);
    auto loopParser = CountableLoopParser(*loop);
    auto loopInfo = loopParser.Parse();
    std::optional<uint64_t> optIterations {};
    auto noBranching = false;
    if (loopInfo.has_value()) {
        optIterations = CountableLoopParser::GetLoopIterations(*loopInfo);
        if (optIterations == 0) {
            optIterations.reset();
        }
        if (optIterations.has_value()) {
            // Increase instruction limit for unroll without branching
            // <= unroll_factor * 2 because unroll without side exits would create unroll_factor * 2 - 1 copies of loop
            noBranching = unrollParams.cloneableInsts <= instLimit_ &&
                          (*optIterations <= unrollFactor * 2U || *optIterations <= unrollFactor_) &&
                          CountableLoopParser::HasPreHeaderCompare(loop, *loopInfo);
        }
    }

    if (noBranching) {
        auto iterations = *optIterations;
        graphCloner.UnrollLoopBody<UnrollType::UNROLL_CONSTANT_ITERATIONS>(loop, iterations);
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Unrolled without branching the loop with constant number of iterations (" << iterations
            << "). Loop id = " << loop->GetId();
        isApplied_ = true;
        GetGraph()->GetEventWriter().EventLoopUnroll(loop->GetId(), loop->GetHeader()->GetGuestPc(), iterations,
                                                     unrollParams.cloneableInsts, "without branching");
        return true;
    }

    if (loopInfo.has_value() && loopInfo->update->IsShift()) {
        // loop info for shl|shr|ashr update inst only used for no branching unroll.
        loopInfo.reset();
    }

    return UnrollWithBranching(unrollFactor, loop, loopInfo, optIterations);
}

bool LoopUnroll::UnrollWithBranching(uint32_t unrollFactor, Loop *loop, std::optional<CountableLoopInfo> loopInfo,
                                     std::optional<uint64_t> optIterations)
{
    auto unrollParams = GetUnrollParams(loop);

    if (unrollFactor <= 1U) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
            << "Loop isn't unrolled due to unroll factor = " << unrollFactor << ". Loop id = " << loop->GetId();
        return false;
    }

    auto noSideExits = false;
    if (loopInfo.has_value()) {
        noSideExits =
            !ConditionOverFlow(*loopInfo, unrollFactor) && CountableLoopParser::HasPreHeaderCompare(loop, *loopInfo);
    }

    TransformLoopImpl(loop, optIterations, noSideExits, unrollFactor, loopInfo);
    isApplied_ = true;
    GetGraph()->GetEventWriter().EventLoopUnroll(loop->GetId(), loop->GetHeader()->GetGuestPc(), unrollFactor,
                                                 unrollParams.cloneableInsts,
                                                 noSideExits ? "without side exits" : "with side exits");
    return true;
}

/**
 * @return - unroll parameters:
 * - maximum value of unroll factor, depends on INST_LIMIT
 * - number of cloneable instructions
 */
LoopUnroll::UnrollParams LoopUnroll::GetUnrollParams(Loop *loop)
{
    uint32_t baseInstCount = 0;
    uint32_t notCloneableCount = 0;
    bool hasCall = false;
    for (auto block : loop->GetBlocks()) {
        for (auto inst : block->AllInsts()) {
            baseInstCount++;
            if ((block->IsLoopHeader() && inst->IsPhi()) || inst->GetOpcode() == Opcode::SafePoint) {
                notCloneableCount++;
            }
            hasCall |= inst->IsCall() && !static_cast<CallInst *>(inst)->IsInlined();
        }
    }

    UnrollParams params = {1, (baseInstCount - notCloneableCount), hasCall};
    if (baseInstCount >= instLimit_) {
        return params;
    }
    uint32_t canBeClonedCount = instLimit_ - baseInstCount;
    params.unrollFactor = unrollFactor_;
    if (params.cloneableInsts > 0) {
        params.unrollFactor = (canBeClonedCount / params.cloneableInsts) + 1;
    }
    return params;
}

/**
 * @return - `if_imm`'s compare input when `if_imm` its single user,
 * otherwise create a new one Compare for this `if_imm` and return it
 */
Inst *GetOrCreateIfImmUniqueCompare(Inst *ifImm)
{
    ASSERT(ifImm->GetOpcode() == Opcode::IfImm);
    auto compare = ifImm->GetInput(0).GetInst();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    if (compare->HasSingleUser()) {
        return compare;
    }
    auto newCmp = compare->Clone(compare->GetBasicBlock()->GetGraph());
    newCmp->SetInput(0, compare->GetInput(0).GetInst());
    newCmp->SetInput(1, compare->GetInput(1).GetInst());
    ifImm->InsertBefore(newCmp);
    ifImm->SetInput(0, newCmp);
    return newCmp;
}

/// Normalize control-flow to the form: `if condition is true goto loop_header`
void NormalizeControlFlow(BasicBlock *edge, const BasicBlock *loopHeader)
{
    auto ifImm = edge->GetLastInst()->CastToIfImm();
    ASSERT(ifImm->GetImm() == 0);
    if (ifImm->GetCc() == CC_EQ) {
        ifImm->SetCc(CC_NE);
        edge->SwapTrueFalseSuccessors<true>();
    }
    auto cmp = ifImm->GetInput(0).GetInst()->CastToCompare();
    if (!cmp->HasSingleUser()) {
        auto newCmp = cmp->Clone(edge->GetGraph());
        ifImm->InsertBefore(newCmp);
        ifImm->SetInput(0, newCmp);
        cmp = newCmp->CastToCompare();
    }
    if (edge->GetFalseSuccessor() == loopHeader) {
        auto inversedCc = GetInverseConditionCode(cmp->GetCc());
        cmp->SetCc(inversedCc);
        edge->SwapTrueFalseSuccessors<true>();
    }
}

Inst *LoopUnroll::CreateNewTestInst(const CountableLoopInfo &loopInfo, Inst *constInst, Inst *preHeaderCmp)
{
    Inst *test = nullptr;
    if (loopInfo.isInc) {
        test = GetGraph()->CreateInstSub(preHeaderCmp->CastToCompare()->GetOperandsType(), preHeaderCmp->GetPc(),
                                         loopInfo.test, constInst);
#ifdef PANDA_COMPILER_DEBUG_INFO
        test->SetCurrentMethod(preHeaderCmp->GetCurrentMethod());
#endif
    } else {
        test = GetGraph()->CreateInstAdd(preHeaderCmp->CastToCompare()->GetOperandsType(), preHeaderCmp->GetPc(),
                                         loopInfo.test, constInst);
#ifdef PANDA_COMPILER_DEBUG_INFO
        test->SetCurrentMethod(preHeaderCmp->GetCurrentMethod());
#endif
    }
    preHeaderCmp->InsertBefore(test);
    return test;
}

/**
 * Replace `Compare(init, test)` with these instructions:
 *
 * Constant(unroll_factor)
 * Sub/Add(test, Constant)
 * Compare(init, SubI/AddI)
 *
 * And replace condition code if it is `CC_NE`.
 * We use Constant + Sub/Add because low-level instructions (SubI/AddI) may appear only after Lowering pass.
 */
void LoopUnroll::FixCompareInst(const CountableLoopInfo &loopInfo, BasicBlock *header, uint32_t unrollFactor)
{
    auto preHeader = header->GetLoop()->GetPreHeader();
    auto backEdge = loopInfo.ifImm->GetBasicBlock();
    ASSERT(!preHeader->IsEmpty() && preHeader->GetLastInst()->GetOpcode() == Opcode::IfImm);
    auto preHeaderIf = preHeader->GetLastInst()->CastToIfImm();
    auto preHeaderCmp = GetOrCreateIfImmUniqueCompare(preHeaderIf);
    auto backEdgeCmp = GetOrCreateIfImmUniqueCompare(loopInfo.ifImm);
    NormalizeControlFlow(preHeader, header);
    NormalizeControlFlow(backEdge, header);
    // Create Sub/Add + Const instructions and replace Compare's test inst input
    auto immValue = (static_cast<uint64_t>(unrollFactor) - 1) * loopInfo.constStep;
    auto newTest = CreateNewTestInst(loopInfo, GetGraph()->FindOrCreateConstant(immValue), preHeaderCmp);
    auto testInputIdx = 1;
    if (backEdgeCmp->GetInput(0) == loopInfo.test) {
        testInputIdx = 0;
    } else {
        ASSERT(backEdgeCmp->GetInput(1) == loopInfo.test);
    }
    ASSERT(preHeaderCmp->GetInput(testInputIdx).GetInst() == loopInfo.test);
    preHeaderCmp->SetInput(testInputIdx, newTest);
    backEdgeCmp->SetInput(testInputIdx, newTest);
    // Replace CC_NE ConditionCode
    if (loopInfo.normalizedCc == CC_NE) {
        auto cc = loopInfo.isInc ? CC_LT : CC_GT;
        if (testInputIdx == 0) {
            cc = SwapOperandsConditionCode(cc);
        }
        preHeaderCmp->CastToCompare()->SetCc(cc);
        backEdgeCmp->CastToCompare()->SetCc(cc);
    }
    // for not constant test-instruction we need to insert `overflow-check`:
    // `test - imm_value` should be less than `test` (incerement loop-index case)
    // `test + imm_value` should be greater than `test` (decrement loop-index case)
    // If overflow-check is failed goto after-loop
    if (!loopInfo.test->IsConst()) {
        auto cc = loopInfo.isInc ? CC_LT : CC_GT;
        // Create overflow_compare
        auto overflowCompare = GetGraph()->CreateInstCompare(compiler::DataType::BOOL, preHeaderCmp->GetPc(), newTest,
                                                             loopInfo.test, loopInfo.test->GetType(), cc);
#ifdef PANDA_COMPILER_DEBUG_INFO
        overflowCompare->SetCurrentMethod(preHeaderCmp->GetCurrentMethod());
#endif
        // Create (pre_header_compare AND overflow_compare) inst
        auto andInst = GetGraph()->CreateInstAnd(DataType::BOOL, preHeaderCmp->GetPc(), preHeaderCmp, overflowCompare);
#ifdef PANDA_COMPILER_DEBUG_INFO
        andInst->SetCurrentMethod(preHeaderCmp->GetCurrentMethod());
#endif
        preHeaderIf->SetInput(0, andInst);
        preHeaderIf->InsertBefore(andInst);
        andInst->InsertBefore(overflowCompare);
    }
}
}  // namespace ark::compiler
