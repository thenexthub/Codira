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

#include "optimizer/ir/analysis.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "liveness_analyzer.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/locations_builder.h"
#include "optimizer/optimizations/regalloc/reg_type.h"

namespace ark::compiler {
LivenessAnalyzer::LivenessAnalyzer(Graph *graph)
    : Analysis(graph),
      allocator_(graph->GetAllocator()),
      linearBlocks_(graph->GetAllocator()->Adapter()),
      instLifeNumbers_(graph->GetAllocator()->Adapter()),
      instLifeIntervals_(graph->GetAllocator()->Adapter()),
      instsByLifeNumber_(graph->GetAllocator()->Adapter()),
      blockLiveRanges_(graph->GetAllocator()->Adapter()),
      blockLiveSets_(graph->GetLocalAllocator()->Adapter()),
      pendingCatchPhiInputs_(graph->GetAllocator()->Adapter()),
      physicalGeneralIntervals_(graph->GetAllocator()->Adapter()),
      physicalVectorIntervals_(graph->GetAllocator()->Adapter()),
      intervalsForTemps_(graph->GetAllocator()->Adapter()),
      useTable_(graph->GetAllocator()),
      hasSafepointDuringCall_(graph->GetRuntime()->HasSafepointDuringCall())
{
}

bool LivenessAnalyzer::RunImpl()
{
    if (!RunLocationsBuilder(GetGraph())) {
        return false;
    }
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();
    ResetLiveness();
    BuildBlocksLinearOrder();
    BuildInstLifeNumbers();
    BuildInstLifeIntervals();
    Finalize();
    if (!pendingCatchPhiInputs_.empty()) {
        COMPILER_LOG(ERROR, LIVENESS_ANALYZER)
            << "Graph contains CatchPhi instructions whose inputs were not processed";
        return false;
    }
    std::copy_if(physicalGeneralIntervals_.begin(), physicalGeneralIntervals_.end(),
                 std::back_inserter(instLifeIntervals_), [](auto li) { return li != nullptr; });
    std::copy_if(physicalVectorIntervals_.begin(), physicalVectorIntervals_.end(),
                 std::back_inserter(instLifeIntervals_), [](auto li) { return li != nullptr; });
    std::copy(intervalsForTemps_.begin(), intervalsForTemps_.end(), std::back_inserter(instLifeIntervals_));
    COMPILER_LOG(DEBUG, LIVENESS_ANALYZER) << "Liveness analysis is constructed";
    return true;
}

void LivenessAnalyzer::ResetLiveness()
{
    instLifeNumbers_.clear();
    instLifeIntervals_.clear();
    blockLiveSets_.clear();
    blockLiveRanges_.clear();
    physicalGeneralIntervals_.clear();
    physicalVectorIntervals_.clear();
    intervalsForTemps_.clear();
    if (GetGraph()->GetArch() != Arch::NONE) {
        physicalGeneralIntervals_.resize(REGISTERS_NUM);
        physicalVectorIntervals_.resize(VREGISTERS_NUM);
    }
#ifndef NDEBUG
    finalized_ = false;
#endif
}

/*
 * Linear blocks order means:
 * - all dominators of a block are visiting before this block;
 * - all blocks belonging to the same loop are contiguous;
 */
void LivenessAnalyzer::BuildBlocksLinearOrder()
{
    ASSERT_PRINT(GetGraph()->IsAnalysisValid<DominatorsTree>(), "Liveness Analyzer needs valid Dom Tree");
    auto size = GetGraph()->GetBlocksRPO().size();
    linearBlocks_.reserve(size);
    linearBlocks_.clear();
    marker_ = GetGraph()->NewMarker();
    ASSERT_PRINT(marker_ != UNDEF_MARKER, "There are no free markers");
    if (GetGraph()->IsBytecodeOptimizer() && !GetGraph()->GetTryBeginBlocks().empty()) {
        LinearizeBlocks<true>();
    } else {
        LinearizeBlocks<false>();
    }
    ASSERT(linearBlocks_.size() == size);
    GetGraph()->EraseMarker(marker_);
    ASSERT_PRINT(CheckLinearOrder(), "Linear block order isn't correct");

    blockLiveSets_.resize(GetGraph()->GetVectorBlocks().size());
    blockLiveRanges_.resize(GetGraph()->GetVectorBlocks().size());
}

/*
 * Check if all forward edges of loop header were visited to get the resulting block order in RPO.
 * Predecessors which are not in the same loop with a header - are forward edges.
 */
bool LivenessAnalyzer::AllForwardEdgesVisited(BasicBlock *block)
{
    if (!block->IsLoopHeader()) {
        for (auto pred : block->GetPredsBlocks()) {
            if (!pred->IsMarked(marker_)) {
                return false;
            }
        }
    } else {
        // Head of irreducible loop can not dominate other blocks in the loop
        if (block->GetLoop()->IsIrreducible()) {
            return true;
        }
        // Predecessors which are not dominated - are forward edges,
        for (auto pred : block->GetPredsBlocks()) {
            if (!block->IsDominate(pred) && !pred->IsMarked(marker_)) {
                return false;
            }
        }
    }
    return true;
}

template <bool USE_PC_ORDER>
void LivenessAnalyzer::LinearizeBlocks()
{
    ArenaList<BasicBlock *> pending {GetGraph()->GetLocalAllocator()->Adapter()};
    pending.push_back(GetGraph()->GetStartBlock());

    while (!pending.empty()) {
        auto current = pending.front();
        pending.pop_front();

        linearBlocks_.push_back(current);
        current->SetMarker(marker_);

        auto succs = current->GetSuccsBlocks();
        // Each block is inserted into pending list before all already inserted blocks
        // from the same loop. To process edges forwarding to a "true" branches successors
        // should be processed in reversed order.
        for (auto it = succs.rbegin(); it != succs.rend(); ++it) {
            auto succ = *it;
            if (succ->IsMarked(marker_) || !AllForwardEdgesVisited(succ)) {
                continue;
            }
            InsertSuccToPendingList<USE_PC_ORDER>(pending, succ);
        }
    }
}

template <bool USE_PC_ORDER>
void LivenessAnalyzer::InsertSuccToPendingList(ArenaList<BasicBlock *> &pending, BasicBlock *succ)
{
    if constexpr (USE_PC_ORDER) {  // NOLINT(readability-braces-around-statements)
        auto pcCompare = [succ](auto block) { return block->GetGuestPc() > succ->GetGuestPc(); };
        auto insertBefore = std::find_if(pending.begin(), pending.end(), pcCompare);
        pending.insert(insertBefore, succ);
    } else {
        // Insert successor right before the first block not from an inner loop.
        // Such ordering guarantee that a loop and all it's inner loops will be processed
        // before following edges leading to outer loop blocks.
        auto isNotInnerLoop = [succ](auto block) { return !block->GetLoop()->IsInside(succ->GetLoop()); };
        auto insertBefore = std::find_if(pending.begin(), pending.end(), isNotInnerLoop);
        pending.insert(insertBefore, succ);
    }
}

/*
 * Check linear order correctness, using dominators tree
 */
bool LivenessAnalyzer::CheckLinearOrder()
{
    ArenaVector<size_t> blockPos(GetGraph()->GetVectorBlocks().size(), 0, GetAllocator()->Adapter());
    size_t position = 0;
    for (auto block : linearBlocks_) {
        blockPos[block->GetId()] = position++;
    }

    for (auto block : linearBlocks_) {
        if (block->GetDominator() != nullptr) {
            ASSERT_PRINT(blockPos[block->GetDominator()->GetId()] < blockPos[block->GetId()],
                         "Each block should be visited after its dominator");
        }
        if (!block->IsTryEnd()) {
            continue;
        }
        ASSERT(block->GetTryId() != INVALID_ID);
        for (auto bb : linearBlocks_) {
            if (bb->IsTry() && bb->GetTryId() == block->GetTryId()) {
                ASSERT_PRINT(blockPos[bb->GetId()] < blockPos[block->GetId()],
                             "Each try-block should be visited before its try-end block");
            }
        }
    }

    return true;
}

/*
 * Set lifetime number and visiting number for each instruction
 */
void LivenessAnalyzer::BuildInstLifeNumbers()
{
    LifeNumber blockBegin;
    LifeNumber lifeNumber = 0;
    LinearNumber linearNumber = 0;

    for (auto block : GetLinearizedBlocks()) {
        blockBegin = lifeNumber;
        // set the same number for each phi in the block
        for (auto phi : block->PhiInsts()) {
            phi->SetLinearNumber(linearNumber++);
            SetInstLifeNumber(phi, lifeNumber);
            CreateLifeIntervals(phi);
        }
        // ignore PHI instructions
        instsByLifeNumber_.push_back(nullptr);
        // set a unique number for each instruction in the block, differing by 2
        // for the reason of adding spill/fill instructions
        for (auto inst : block->Insts()) {
            inst->SetLinearNumber(linearNumber++);
            CreateLifeIntervals(inst);
            if (IsPseudoUserOfMultiOutput(inst)) {
                // Should be the same life number as pseudo-user, since actually they have the same definition
                SetInstLifeNumber(inst, lifeNumber);
                GetInstLifeIntervals(inst)->AddUsePosition(lifeNumber);
                continue;
            }
            lifeNumber += LIFE_NUMBER_GAP;
            SetInstLifeNumber(inst, lifeNumber);
            SetUsePositions(inst, lifeNumber);
            instsByLifeNumber_.push_back(inst);

            if (inst->RequireTmpReg()) {
                CreateIntervalForTemp(lifeNumber);
            }
        }
        lifeNumber += LIFE_NUMBER_GAP;
        SetBlockLiveRange(block, {blockBegin, lifeNumber});
    }
}

void LivenessAnalyzer::SetUsePositions(Inst *userInst, LifeNumber lifeNumber)
{
    if (GetGraph()->IsBytecodeOptimizer()) {
        return;
    }
    if (userInst->IsCatchPhi() || userInst->IsSaveState()) {
        return;
    }
    if (!InstHasPseudoInputs(userInst)) {
        for (size_t i = 0; i < userInst->GetInputsCount(); i++) {
            auto location = userInst->GetLocation(i);
            if (!location.IsAnyRegister()) {
                continue;
            }
            auto inputInst = userInst->GetDataFlowInput(i);
            auto li = GetInstLifeIntervals(inputInst);
            if (location.IsRegisterValid()) {
                useTable_.AddUseOnFixedLocation(inputInst, location, lifeNumber);
            } else if (location.IsUnallocatedRegister()) {
                li->AddUsePosition(lifeNumber);
            }
        }
    }

    // Constant can be defined without register
    if (userInst->IsConst() && g_options.IsCompilerRematConst()) {
        return;
    }

    // If instruction required dst register, set use position in the beginning of the interval
    auto li = GetInstLifeIntervals(userInst);
    if (!li->NoDest()) {
        li->AddUsePosition(lifeNumber);
    }
}

/*
 * Get lifetime intervals for each instruction
 */
void LivenessAnalyzer::BuildInstLifeIntervals()
{
    for (auto it = GetLinearizedBlocks().rbegin(); it != GetLinearizedBlocks().rend(); it++) {
        auto block = *it;
        auto liveSet = GetInitInstLiveSet(block);
        ProcessBlockLiveInstructions(block, liveSet);
    }
}

/*
 * The initial set of live instructions in the `block` is the the union of all live instructions at the beginning of
 * the block's successors.
 * Also for each phi-instruction of the successors: input corresponding to the `block` is added to the live set.
 */
InstLiveSet *LivenessAnalyzer::GetInitInstLiveSet(BasicBlock *block)
{
    unsigned instructionCount = instLifeIntervals_.size();
    auto liveSet = GetAllocator()->New<InstLiveSet>(instructionCount, GetAllocator());
    ASSERT(liveSet != nullptr);
    for (auto succ : block->GetSuccsBlocks()) {
        // catch-begin is pseudo successor, its live set will be processed for blocks with throwable instructions
        if (succ->IsCatchBegin()) {
            continue;
        }
        liveSet->Union(GetBlockLiveSet(succ));
        for (auto phi : succ->PhiInsts()) {
            auto phiInput = phi->CastToPhi()->GetPhiDataflowInput(block);
            liveSet->Add(phiInput->GetLinearNumber());
        }
    }

    // if basic block contains throwable instruction, all instructions live in the catch-handlers should be live in this
    // block
    if (auto inst = block->GetFistThrowableInst(); inst != nullptr) {
        auto handlers = GetGraph()->GetThrowableInstHandlers(inst);
        for (auto catchHandler : handlers) {
            liveSet->Union(GetBlockLiveSet(catchHandler));
        }
    }
    return liveSet;
}

LifeNumber LivenessAnalyzer::GetLoopEnd(Loop *loop)
{
    LifeNumber loopEnd = 0;
    // find max LifeNumber of inner loops
    for (auto inner : loop->GetInnerLoops()) {
        loopEnd = std::max(loopEnd, GetLoopEnd(inner));
    }
    // find max LifeNumber of back_edges
    for (auto backEdge : loop->GetBackEdges()) {
        loopEnd = std::max(loopEnd, GetBlockLiveRange(backEdge).GetEnd());
    }
    return loopEnd;
}

/*
 * Append and adjust the lifetime intervals for each instruction live in the block and for their inputs
 */
void LivenessAnalyzer::ProcessBlockLiveInstructions(BasicBlock *block, InstLiveSet *liveSet)
{
    ASSERT(liveSet != nullptr);
    // For each live instruction set initial life range equals to the block life range
    for (auto &interval : instLifeIntervals_) {
        if (liveSet->IsSet(interval->GetInst()->GetLinearNumber())) {
            interval->AppendRange(GetBlockLiveRange(block));
        }
    }

    for (Inst *inst : block->InstsSafeReverse()) {
        // Shorten instruction lifetime to the position where its defined
        // and remove from the block's live set
        auto instLifeNumber = GetInstLifeNumber(inst);
        auto interval = GetInstLifeIntervals(inst);
        interval->StartFrom(instLifeNumber);
        liveSet->Remove(inst->GetLinearNumber());
        if (inst->IsCatchPhi()) {
            // catch-phi's liveness should overlap all linked try blocks' livenesses
            for (auto pred : inst->GetBasicBlock()->GetPredsBlocks()) {
                instLifeNumber = std::min(instLifeNumber, GetBlockLiveRange(pred).GetBegin());
            }
            interval->StartFrom(instLifeNumber);
            AdjustCatchPhiInputsLifetime(inst);
        } else {
            if (inst->GetOpcode() == Opcode::LiveOut) {
                ProcessOpcodeLiveOut(block, interval, instLifeNumber);
            }
            auto currentLiveRange = LiveRange {GetBlockLiveRange(block).GetBegin(), instLifeNumber};
            AdjustInputsLifetime(inst, currentLiveRange, liveSet);
        }

        BlockFixedRegisters(inst);
    }

    // The lifetime interval of phis instructions starts at the beginning of the block
    for (auto phi : block->PhiInsts()) {
        liveSet->Remove(phi->GetLinearNumber());
    }

    // All instructions live at the beginning ot the loop header
    // must be live for the entire loop
    if (block->IsLoopHeader()) {
        LifeNumber loopEndPosition = GetLoopEnd(block->GetLoop());

        for (auto &interval : instLifeIntervals_) {
            if (liveSet->IsSet(interval->GetInst()->GetLinearNumber())) {
                interval->AppendGroupRange({GetBlockLiveRange(block).GetBegin(), loopEndPosition});
            }
        }
    }
    SetBlockLiveSet(block, liveSet);
}

void LivenessAnalyzer::ProcessOpcodeLiveOut(BasicBlock *block, LifeIntervals *interval, LifeNumber instLifeNumber)
{
    if (block->GetSuccsBlocks().size() == 1 && block->GetSuccessor(0)->IsEndBlock()) {
        interval->AppendRange({instLifeNumber, GetBlockLiveRange(block).GetEnd()});
    } else {
        interval->AppendRange({instLifeNumber, GetBlockLiveRange(GetGraph()->GetEndBlock()).GetBegin()});
    }
}

/* static */
LiveRange LivenessAnalyzer::GetPropagatedLiveRange(Inst *inst, LiveRange liveRange)
{
    /*
     * Implicit null check encoded as no-op and if the reference to check is null
     * then SIGSEGV will be raised at the first (closest) user. Regmap generated for
     * NullCheck's SaveState should be valid at that user so we need to extend
     * life intervals of SaveState's inputs until NullCheck user.
     */
    if (inst->IsNullCheck() && !inst->GetUsers().Empty() && inst->CastToNullCheck()->IsImplicit()) {
        auto extendUntil = std::numeric_limits<LifeNumber>::max();
        for (auto &user : inst->GetUsers()) {
            // Skip the users dominating the NullCheck as their live ranges
            // start earlier than NullCheck's live range.
            if (!user.GetInst()->IsDominate(inst)) {
                auto li = GetInstLifeIntervals(user.GetInst());
                ASSERT(li != nullptr);
                extendUntil = std::min<LifeNumber>(extendUntil, li->GetBegin() + 1);
            }
        }
        liveRange.SetEnd(extendUntil);
        return liveRange;
    }
    /*
     * We need to propagate liveness for instruction with CallRuntime to save registers before call;
     * Otherwise, we will not be able to restore the value of the virtual registers
     */
    if (inst->IsPropagateLiveness()) {
        liveRange.SetEnd(liveRange.GetEnd() + 1);
    } else if (inst->GetOpcode() == Opcode::ReturnInlined && inst->CastToReturnInlined()->IsExtendedLiveness()) {
        /*
         * [ReturnInlined]
         * [ReturnInlined]
         * ...
         * [Deoptimize/Throw]
         *
         * In this case we propagate ReturnInlined inputs liveness up to the end of basic block
         */
        liveRange.SetEnd(GetBlockLiveRange(inst->GetBasicBlock()).GetEnd());
    }
    return liveRange;
}

/*
 * Adjust instruction inputs lifetime and add them to the block's live set
 */
void LivenessAnalyzer::AdjustInputsLifetime(Inst *inst, LiveRange liveRange, InstLiveSet *liveSet)
{
    for (auto input : inst->GetInputs()) {
        auto inputInst = inst->GetDataFlowInput(input.GetInst());
        liveSet->Add(inputInst->GetLinearNumber());
        if (inst->GetOpcode() == Opcode::WrapObjectNative) {
            auto *nativeCall = inst->GetUsers().Front().GetInst();
            // need to ensure that ref inputs will live until Native API call, +1 for spilling on stack
            liveRange.SetEnd(GetInstLifeNumber(nativeCall) + 1U);
        }
        SetInputRange(inst, inputInst, liveRange);
    }

    // Extend SaveState inputs lifetime to the end of SaveState's lifetime
    if (inst->RequireState()) {
        auto saveState = inst->GetSaveState();
        auto propagatedRange = GetPropagatedLiveRange(inst, liveRange);
        while (true) {
            ASSERT(saveState != nullptr);
            for (auto ssInput : saveState->GetInputs()) {
                auto inputInst = saveState->GetDataFlowInput(ssInput.GetInst());
                liveSet->Add(inputInst->GetLinearNumber());
                GetInstLifeIntervals(inputInst)->AppendRange(propagatedRange);
            }
            auto callerInst = saveState->GetCallerInst();
            if (callerInst == nullptr) {
                break;
            }
            saveState = callerInst->GetSaveState();
        }
    }

    // Handle CatchPhi inputs associated with inst
    auto range = pendingCatchPhiInputs_.equal_range(inst);
    for (auto it = range.first; it != range.second; ++it) {
        auto throwableInput = it->second;
        auto throwableInputInterval = GetInstLifeIntervals(throwableInput);
        liveSet->Add(throwableInput->GetLinearNumber());
        throwableInputInterval->AppendRange(liveRange);
    }
    pendingCatchPhiInputs_.erase(inst);
}

/*
 * Increase ref-input liveness in the 'no-async-jit' mode, since GC can be triggered and delete ref during callee-method
 * compilation
 */
void LivenessAnalyzer::SetInputRange(const Inst *inst, const Inst *input, LiveRange liveRange) const
{
    if (hasSafepointDuringCall_ && inst->IsCall() && DataType::IsReference(input->GetType())) {
        GetInstLifeIntervals(input)->AppendRange(liveRange.GetBegin(), liveRange.GetEnd() + 1U);
    } else {
        GetInstLifeIntervals(input)->AppendRange(liveRange);
    }
}

/*
 * CatchPhi does not handle inputs as regular instructions - instead of performing
 * some action at CatchPhi's definition copy instruction are added before throwable instructions.
 * Instead of extending input life interval until CatchPhi it is extended until throwable instruction.
 */
void LivenessAnalyzer::AdjustCatchPhiInputsLifetime(Inst *inst)
{
    auto catchPhi = inst->CastToCatchPhi();

    for (auto inputIdx = static_cast<ssize_t>(catchPhi->GetInputsCount() - 1); inputIdx >= 0; inputIdx--) {
        auto inputInst = catchPhi->GetDataFlowInput(inputIdx);
        auto throwableInst = const_cast<Inst *>(catchPhi->GetThrowableInst(inputIdx));

        pendingCatchPhiInputs_.insert({throwableInst, inputInst});
    }
}

void LivenessAnalyzer::SetInstLifeNumber([[maybe_unused]] const Inst *inst, LifeNumber number)
{
    ASSERT(instLifeNumbers_.size() == inst->GetLinearNumber());
    instLifeNumbers_.push_back(number);
}

LifeNumber LivenessAnalyzer::GetInstLifeNumber(const Inst *inst) const
{
    return instLifeNumbers_[inst->GetLinearNumber()];
}

/*
 * Create new lifetime intervals for instruction, check that instruction linear number is equal to intervals
 * position in vector
 */
void LivenessAnalyzer::CreateLifeIntervals(Inst *inst)
{
    ASSERT(!finalized_);
    ASSERT(inst->GetLinearNumber() == instLifeIntervals_.size());
    auto interval = GetAllocator()->New<LifeIntervals>(GetAllocator(), inst);
    instLifeIntervals_.push_back(interval);
}

void LivenessAnalyzer::CreateIntervalForTemp(LifeNumber ln)
{
    ASSERT(!finalized_);
    auto interval = GetAllocator()->New<LifeIntervals>(GetAllocator());
    ASSERT(interval != nullptr);
    interval->AppendRange({ln - 1, ln});
    interval->AddUsePosition(ln);
    // DataType is INT64, since general register is reserved (for 32-bits arch will be converted to INT32)
    interval->SetType(ConvertRegType(GetGraph(), DataType::INT64));
    intervalsForTemps_.push_back(interval);
}

LifeIntervals *LivenessAnalyzer::GetInstLifeIntervals(const Inst *inst) const
{
    ASSERT(inst->GetLinearNumber() != INVALID_LINEAR_NUM);
    return instLifeIntervals_[inst->GetLinearNumber()];
}

void LivenessAnalyzer::SetBlockLiveRange(BasicBlock *block, LiveRange lifeRange)
{
    blockLiveRanges_[block->GetId()] = lifeRange;
}

const ArenaVector<BasicBlock *> &LivenessAnalyzer::GetLinearizedBlocks() const
{
    return linearBlocks_;
}

Inst *LivenessAnalyzer::GetInstByLifeNumber(LifeNumber ln) const
{
    return instsByLifeNumber_[ln / LIFE_NUMBER_GAP];
}

BasicBlock *LivenessAnalyzer::GetBlockCoversPoint(LifeNumber ln) const
{
    for (auto bb : linearBlocks_) {
        if (GetBlockLiveRange(bb).Contains(ln)) {
            return bb;
        }
    }
    return nullptr;
}

void LivenessAnalyzer::Cleanup()
{
    for (auto *interv : instLifeIntervals_) {
        if (!interv->IsPhysical() && !interv->IsPreassigned()) {
            interv->ClearLocation();
        }
        if (interv->GetSibling() != nullptr) {
            interv->MergeSibling();
        }
    }
}

void LivenessAnalyzer::Finalize()
{
    for (auto *interv : instLifeIntervals_) {
        interv->Finalize();
    }
    for (auto *interv : intervalsForTemps_) {
        interv->Finalize();
    }
#ifndef NDEBUG
    finalized_ = true;
#endif
}

const ArenaVector<LifeIntervals *> &LivenessAnalyzer::GetLifeIntervals() const
{
    return instLifeIntervals_;
}

LiveRange LivenessAnalyzer::GetBlockLiveRange(const BasicBlock *block) const
{
    return blockLiveRanges_[block->GetId()];
}

void LivenessAnalyzer::SetBlockLiveSet(BasicBlock *block, InstLiveSet *liveSet)
{
    blockLiveSets_[block->GetId()] = liveSet;
}

InstLiveSet *LivenessAnalyzer::GetBlockLiveSet(BasicBlock *block) const
{
    return blockLiveSets_[block->GetId()];
}

const UseTable &LivenessAnalyzer::GetUseTable() const
{
    return useTable_;
}

LifeIntervals *LivenessAnalyzer::GetTmpRegInterval(const Inst *inst)
{
    auto instLn = GetInstLifeNumber(inst);
    auto it = std::find_if(intervalsForTemps_.begin(), intervalsForTemps_.end(),
                           [instLn](auto li) { return li->GetBegin() == instLn - 1; });
    return it == intervalsForTemps_.end() ? nullptr : *it;
}

const char *LivenessAnalyzer::GetPassName() const
{
    return "LivenessAnalysis";
}

size_t LivenessAnalyzer::GetBlocksCount() const
{
    return blockLiveRanges_.size();
}

ArenaAllocator *LivenessAnalyzer::GetAllocator()
{
    return allocator_;
}

void LivenessAnalyzer::DumpLifeIntervals(std::ostream &out) const
{
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->GetId() >= GetBlocksCount()) {
            continue;
        }
        auto blockRange = GetBlockLiveRange(bb);
        out << "BB " << bb->GetId() << "\t" << blockRange.ToString() << std::endl;

        for (auto inst : bb->AllInsts()) {
            if (inst->GetLinearNumber() == INVALID_LINEAR_NUM) {
                continue;
            }
            auto interval = GetInstLifeIntervals(inst);

            out << "v" << inst->GetId() << "\t";
            for (auto sibling = interval; sibling != nullptr; sibling = sibling->GetSibling()) {
                out << sibling->ToString<false>() << "@ " << sibling->GetLocation().ToString(GetGraph()->GetArch())
                    << "; ";
            }
            out << std::endl;
        }
    }

    out << "Temps:" << std::endl;
    for (auto interval : intervalsForTemps_) {
        out << interval->ToString<false>() << "@ " << interval->GetLocation().ToString(GetGraph()->GetArch())
            << std::endl;
    }
    DumpLocationsUsage(out);
}

void LivenessAnalyzer::DumpLocationsUsage(std::ostream &out) const
{
    std::map<Register, std::vector<LifeIntervals *>> regsIntervals;
    std::map<Register, std::vector<LifeIntervals *>> vregsIntervals;
    std::map<Register, std::vector<LifeIntervals *>> slotsIntervals;
    for (auto &interval : instLifeIntervals_) {
        for (auto sibling = interval; sibling != nullptr; sibling = sibling->GetSibling()) {
            auto location = sibling->GetLocation();
            if (location.IsFpRegister()) {
                ASSERT(DataType::IsFloatType(interval->GetType()));
                vregsIntervals[location.GetValue()].push_back(sibling);
            } else if (location.IsRegister()) {
                regsIntervals[location.GetValue()].push_back(sibling);
            } else if (location.IsStack()) {
                slotsIntervals[location.GetValue()].push_back(sibling);
            }
        }
    }

    for (auto intervalsMap : {&regsIntervals, &vregsIntervals, &slotsIntervals}) {
        std::string locSymbol;
        if (intervalsMap == &regsIntervals) {
            out << std::endl << "Registers intervals" << std::endl;
            locSymbol = "r";
        } else if (intervalsMap == &vregsIntervals) {
            out << std::endl << "Vector registers intervals" << std::endl;
            locSymbol = "vr";
        } else {
            ASSERT(intervalsMap == &slotsIntervals);
            out << std::endl << "Stack slots intervals" << std::endl;
            locSymbol = "s";
        }

        if (intervalsMap->empty()) {
            out << "-" << std::endl;
            continue;
        }
        for (auto &[reg, intervals] : *intervalsMap) {
            std::sort(intervals.begin(), intervals.end(),
                      [](const auto &lhs, const auto &rhs) { return lhs->GetBegin() < rhs->GetBegin(); });
            out << locSymbol << std::to_string(reg) << ": ";
            auto delim = "";
            for (auto &interval : intervals) {
                out << delim << interval->ToString<false>();
                delim = "; ";
            }
            out << std::endl;
        }
    }
}

void LivenessAnalyzer::BlockFixedRegisters(Inst *inst)
{
    if (GetGraph()->IsBytecodeOptimizer() || GetGraph()->GetMode().IsFastPath()) {
        return;
    }

    auto blockFrom = GetInstLifeNumber(inst);
    if (IsCallBlockingRegisters(inst)) {
        BlockPhysicalRegisters<false>(inst, blockFrom);
        BlockPhysicalRegisters<true>(inst, blockFrom);
    }
    for (auto i = 0U; i < inst->GetInputsCount(); ++i) {
        BlockFixedLocationRegister(inst->GetLocation(i), blockFrom);
    }
    BlockFixedLocationRegister(inst->GetDstLocation(), blockFrom);

    if (inst->IsParameter()) {
        // Block a register starting from the position preceding entry block start to
        // correctly handle register blocked by "current" instruction from registers blocked
        // by other instructions during register allocation.
        constexpr auto FIRST_AVAILABLE_LIFE_NUMBER = LIFE_NUMBER_GAP - 1;
        // Registers holding parameters should be blocked starting from the beginning of entry block
        // to avoid its assignment to parameter instructions in case of high register pressure.
        // The required interval is blocked using two calls to correctly mark first use position of a
        // blocked register.
        BlockFixedLocationRegister(inst->CastToParameter()->GetLocationData().GetSrc(), blockFrom);
        BlockFixedLocationRegister(inst->CastToParameter()->GetLocationData().GetSrc(), FIRST_AVAILABLE_LIFE_NUMBER,
                                   blockFrom - 1, false);
        // Block second parameter register that contains number of actual arguments in case of
        // dynamic methods as it is used in parameter's code generation
        if (GetGraph()->GetMode().IsDynamicMethod() &&
            GetGraph()->FindParameter(ParameterInst::DYNAMIC_NUM_ARGS) == nullptr) {
            BlockReg<false>(Target(GetGraph()->GetArch()).GetParamRegId(1), blockFrom, blockFrom + 1U, true);
            BlockReg<false>(Target(GetGraph()->GetArch()).GetParamRegId(1), FIRST_AVAILABLE_LIFE_NUMBER, blockFrom - 1,
                            false);
        }
    }
}

bool LivenessAnalyzer::IsNativeApiRuntimeCall(Inst *inst)
{
    return (inst->IsNativeApiCall() && inst->IsRuntimeCall()) ||
           (inst->IsIntrinsic() && (inst->CastToIntrinsic()->GetIntrinsicId() ==
                                        RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_BEGIN_NATIVE_METHOD ||
                                    inst->CastToIntrinsic()->GetIntrinsicId() ==
                                        RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_END_NATIVE_METHOD_PRIM ||
                                    inst->CastToIntrinsic()->GetIntrinsicId() ==
                                        RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_END_NATIVE_METHOD_OBJ));
}

template <bool IS_FP>
void LivenessAnalyzer::BlockPhysicalRegisters(Inst *inst, LifeNumber blockFrom)
{
    auto arch = GetGraph()->GetArch();
    RegMask callerRegs {GetCallerRegsMask(arch, IS_FP)};
    for (auto reg = GetFirstCallerReg(arch, IS_FP); reg <= GetLastCallerReg(arch, IS_FP); ++reg) {
        if (callerRegs.test(reg)) {
            BlockReg<IS_FP>(reg, blockFrom, blockFrom + 1U, true);
        }
    }

    // callee registers should not live through runtime native api calls, callees should be spilled on stack
    if (IsNativeApiRuntimeCall(inst)) {
        RegMask calleeRegs = GetCalleeRegsMask(arch, IS_FP);
        for (auto reg = GetFirstCalleeReg(arch, IS_FP); reg <= GetLastCalleeReg(arch, IS_FP); ++reg) {
            if (calleeRegs.test(reg)) {
                BlockReg<IS_FP>(reg, blockFrom, blockFrom + 1U, true);
            }
        }
    }
}

void LivenessAnalyzer::BlockFixedLocationRegister(Location location, LifeNumber ln)
{
    BlockFixedLocationRegister(location, ln, ln + 1U, true);
}

void LivenessAnalyzer::BlockFixedLocationRegister(Location location, LifeNumber blockFrom, LifeNumber blockTo,
                                                  bool isUse)
{
    if (location.IsRegister() && location.IsRegisterValid()) {
        BlockReg<false>(location.GetValue(), blockFrom, blockTo, isUse);
    } else if (location.IsFpRegister() && location.IsRegisterValid()) {
        BlockReg<true>(location.GetValue(), blockFrom, blockTo, isUse);
    }
}

template <bool IS_FP>
void LivenessAnalyzer::BlockReg(Register reg, LifeNumber blockFrom, LifeNumber blockTo, bool isUse)
{
    ASSERT(!finalized_);
    auto &intervals = IS_FP ? physicalVectorIntervals_ : physicalGeneralIntervals_;
    auto interval = intervals.at(reg);
    if (interval == nullptr) {
        interval = GetGraph()->GetAllocator()->New<LifeIntervals>(GetGraph()->GetAllocator());
        ASSERT(interval != nullptr);
        interval->SetPhysicalReg(reg, IS_FP ? DataType::FLOAT64 : DataType::UINT64);
        intervals.at(reg) = interval;
    }
    interval->AppendRange(blockFrom, blockTo);
    if (isUse) {
        interval->AddUsePosition(blockFrom);
    }
}

bool LivenessAnalyzer::IsCallBlockingRegisters(Inst *inst) const
{
    if (inst->IsCall() && !static_cast<CallInst *>(inst)->IsInlined()) {
        return true;
    }
    if (inst->IsIntrinsic() && inst->CastToIntrinsic()->IsNativeCall()) {
        return true;
    }
    return false;
}

LifeIntervals *LifeIntervals::SplitAt(LifeNumber ln, ArenaAllocator *alloc)
{
    ASSERT(!IsPhysical());
    ASSERT(ln > GetBegin() && ln <= GetEnd());
    auto splitChild = alloc->New<LifeIntervals>(alloc, GetInst());
    ASSERT(splitChild != nullptr);
#ifndef NDEBUG
    ASSERT(finalized_);
    splitChild->finalized_ = true;
#endif
    if (sibling_ != nullptr) {
        splitChild->sibling_ = sibling_;
    }
    splitChild->isSplitSibling_ = true;

    sibling_ = splitChild;
    splitChild->SetType(GetType());

    auto i = liveRanges_.size();
    while (i > 0 && liveRanges_[i - 1].GetEnd() <= ln) {
        i--;
    }
    if (i > 0) {
        auto &range = liveRanges_[i - 1];
        if (range.GetBegin() < ln) {
            splitChild->AppendRange(range.GetBegin(), ln);
            range.SetBegin(ln);
        }
    }
    splitChild->liveRanges_.insert(splitChild->liveRanges_.end(), liveRanges_.begin() + i, liveRanges_.end());
    liveRanges_.erase(liveRanges_.begin() + i, liveRanges_.end());
    std::swap(liveRanges_, splitChild->liveRanges_);

    // Move use positions to the child
    auto it = std::lower_bound(usePositions_.begin(), usePositions_.end(), ln);
    splitChild->usePositions_.insert(splitChild->usePositions_.end(), it, usePositions_.end());
    usePositions_.erase(it, usePositions_.end());

    return splitChild;
}

void LifeIntervals::SplitAroundUses(ArenaAllocator *alloc)
{
    if (GetUsePositions().empty()) {
        return;
    }
    auto use = *GetUsePositions().begin();
    if (use > GetBegin() + 1) {
        auto split = SplitAt(use - 1, alloc);
        split->SplitAroundUses(alloc);
    } else if (use < GetEnd() - 1) {
        auto split = SplitAt(use + 1, alloc);
        ASSERT(split != nullptr);
        split->SplitAroundUses(alloc);
    }
}

void LifeIntervals::MergeSibling()
{
    ASSERT(sibling_ != nullptr);
#ifndef NDEBUG
    ASSERT(finalized_);
    ASSERT(sibling_->finalized_);
    if (!usePositions_.empty() && !sibling_->usePositions_.empty()) {
        ASSERT(usePositions_.back() <= sibling_->usePositions_.front());
    }
#endif
    for (auto range : liveRanges_) {
        sibling_->AppendRange(range);
    }
    liveRanges_ = std::move(sibling_->liveRanges_);

    for (auto &usePos : sibling_->usePositions_) {
        AddUsePosition(usePos);
    }
    sibling_ = nullptr;
}

LifeIntervals *LifeIntervals::FindSiblingAt(LifeNumber ln)
{
    ASSERT(!IsSplitSibling());
    for (auto head = this; head != nullptr; head = head->GetSibling()) {
        if (head->GetBegin() <= ln && ln <= head->GetEnd()) {
            return head;
        }
    }
    return nullptr;
}

bool LifeIntervals::Intersects(const LiveRange &range) const
{
    return  // the interval starts within the range
        (range.GetBegin() <= GetBegin() && GetBegin() <= range.GetEnd()) ||
        // the interval ends within the range
        (range.GetBegin() <= GetEnd() && GetEnd() <= range.GetEnd()) ||
        // the range is fully covered by the interval
        (GetBegin() <= range.GetBegin() && range.GetEnd() <= GetEnd());
}

LifeNumber LifeIntervals::GetFirstIntersectionWith(const LifeIntervals *other, LifeNumber searchFrom) const
{
    for (auto it = GetRanges().rbegin(); it != GetRanges().rend(); it++) {
        auto range = *it;
        if (range.GetEnd() <= searchFrom) {
            continue;
        }
        for (auto otherIt = other->GetRanges().rbegin(); otherIt != other->GetRanges().rend(); otherIt++) {
            auto otherRange = *otherIt;
            if (otherRange.GetEnd() <= searchFrom) {
                continue;
            }
            auto rangeBegin = std::max<LifeNumber>(searchFrom, range.GetBegin());
            auto otherRangeBegin = std::max<LifeNumber>(searchFrom, otherRange.GetBegin());
            if (rangeBegin <= otherRangeBegin && otherRangeBegin < range.GetEnd()) {
                // [range]
                //    [other]
                return otherRangeBegin;
                // NOLINTNEXTLINE(readability-else-after-return)
            } else if (rangeBegin > otherRangeBegin && rangeBegin < otherRange.GetEnd()) {
                //     [range]
                // [other]
                return rangeBegin;
            }
        }
    }
    return INVALID_LIFE_NUMBER;
}

bool LifeIntervals::FindRangeCoveringPosition(LifeNumber ln, LiveRange *dst) const
{
    for (auto &range : GetRanges()) {
        if (range.Contains(ln)) {
            *dst = range;
            return true;
        }
    }

    return false;
}

static float GetSpillWeightAt(const LivenessAnalyzer &la, LifeNumber ln)
{
    static constexpr float LOOP_MULT = 10.0;
    auto block = la.GetBlockCoversPoint(ln);
    ASSERT(block != nullptr);
    return std::pow<float>(LOOP_MULT, block->GetLoop()->GetDepth());
}

float CalcSpillWeight(const LivenessAnalyzer &la, LifeIntervals *interval)
{
    if (interval->IsPhysical()) {
        return std::numeric_limits<float>::max();
    }

    size_t length = interval->GetEnd() - interval->GetBegin();
    // Interval can't be splitted
    if (length <= LIFE_NUMBER_GAP) {
        return std::numeric_limits<float>::max();
    }

    // Constant intervals are the first candidates to spill
    if (interval->GetInst()->IsConst()) {
        return std::numeric_limits<float>::min();
    }

    float useWeight = 0;
    if (interval->GetInst()->IsPhi()) {
        useWeight += GetSpillWeightAt(la, interval->GetBegin());
    }

    for (auto use : interval->GetUsePositions()) {
        if (use == interval->GetBegin()) {
            useWeight += GetSpillWeightAt(la, use + 1);
        } else {
            useWeight += GetSpillWeightAt(la, use - 1);
        }
    }
    return useWeight;
}

}  // namespace ark::compiler
