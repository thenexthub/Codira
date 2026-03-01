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
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/licm.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/analysis.h"

namespace ark::compiler {
Licm::Licm(Graph *graph, uint32_t hoistLimit)
    : Optimization(graph), hoistLimit_(hoistLimit), hoistableInstructions_(graph->GetLocalAllocator()->Adapter())
{
}

// NOTE (a.popov) Use `LoopTransform` base class similarly `LoopUnroll`, `LoopPeeling`
bool Licm::RunImpl()
{
    if (!GetGraph()->GetAnalysis<LoopAnalyzer>().IsValid()) {
        GetGraph()->GetAnalysis<LoopAnalyzer>().Run();
    }

    ASSERT(GetGraph()->GetRootLoop() != nullptr);
    if (!GetGraph()->GetRootLoop()->GetInnerLoops().empty()) {
        markerLoopExit_ = GetGraph()->NewMarker();
        MarkLoopExits(GetGraph(), markerLoopExit_);
        for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
            LoopSearchDFS(loop);
        }
        GetGraph()->EraseMarker(markerLoopExit_);
    }
    return (hoistedInstCount_ != 0);
}

void Licm::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

/*
 * Check if block is loop exit, exclude loop header
 */
bool Licm::IsBlockLoopExit(BasicBlock *block)
{
    return block->IsMarked(markerLoopExit_) && !block->IsLoopHeader();
}

/*
 * Search loops in DFS order to visit inner loops firstly
 */
void Licm::LoopSearchDFS(Loop *loop)
{
    ASSERT(loop != nullptr);
    for (auto innerLoop : loop->GetInnerLoops()) {
        LoopSearchDFS(innerLoop);
    }
    if (IsLoopVisited(*loop)) {
        VisitLoop(loop);
    }
}

bool Licm::IsLoopVisited(const Loop &loop) const
{
    if (loop.IsIrreducible()) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Irreducible loop isn't visited, id = " << loop.GetId();
        return false;
    }
    if (loop.IsOsrLoop()) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "OSR entry isn't visited, loop id = " << loop.GetId();
        return false;
    }
    if (loop.IsTryCatchLoop()) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Try-catch loop isn't visited, loop id = " << loop.GetId();
        return false;
    }
    if (hoistedInstCount_ >= hoistLimit_) {
        COMPILER_LOG(DEBUG, LICM_OPT) << "Limit is exceeded, loop isn't visited, id = " << loop.GetId();
        return false;
    }
    COMPILER_LOG(DEBUG, LICM_OPT) << "Visit Loop, id = " << loop.GetId();
    return true;
}

bool AllInputsDominate(Inst *inst, Inst *dom)
{
    for (auto input : inst->GetInputs()) {
        if (!input.GetInst()->IsDominate(dom)) {
            return false;
        }
    }
    return true;
}

void Licm::TryAppendHoistableInst(Inst *inst, BasicBlock *block, Loop *loop)
{
    if (hoistedInstCount_ >= hoistLimit_) {
        COMPILER_LOG(DEBUG, LICM_OPT) << "Limit is exceeded, Can't hoist instruction with id = " << inst->GetId();
        return;
    }
    if (inst->IsResolver()) {
        // If it is not "do-while" or "while(true)" loops then the resolver
        // can not be hoisted out as it might perform class loading/initialization.
        if (block != loop->GetHeader() && loop->GetHeader()->IsMarked(markerLoopExit_)) {
            COMPILER_LOG(DEBUG, LICM_OPT)
                << "Header is a loop exit, Can't hoist (resolver) instruction with id = " << inst->GetId();
            return;
        }
    }
    hoistableInstructions_.push_back(inst);
    inst->SetMarker(markerHoistInst_);
    if (!inst->RequireState()) {
        // Don't increment hoisted instruction count until check
        // if there is another suitable SaveState for it.
        COMPILER_LOG(DEBUG, LICM_OPT) << "Hoist instruction with id = " << inst->GetId();
        hoistedInstCount_++;
    }
}

static bool IsUnsafeForResolver(const Inst *inst)
{
    // Field and Method resolvers may call GetField and InitializeClass methods
    // of the class linker resulting to class loading/initialization.
    // An order of class initialization must be preserved, so the resolver
    // must not cross other instructions performing class initialization
    // We have to be conservative with respect to calls.
    return inst->IsClassInst() || inst->IsResolver() || inst->IsCall();
}

static bool EndOfPreheaderIsSafe(Inst *hoisted, Inst *insertBefore)
{
    auto *preHeader = insertBefore->GetBasicBlock();
    for (auto *inst : InstIter(*preHeader, insertBefore)) {
        if (hoisted->IsResolver() && IsUnsafeForResolver(inst)) {
            return false;
        }
        if (std::find(hoisted->GetInputs().begin(), hoisted->GetInputs().end(), inst) != hoisted->GetInputs().end()) {
            // Not all inputs will be dominate
            return false;
        }
        if (hoisted->IsMovableObject() && inst->IsRuntimeCall()) {
            // We cannot insert movable object between `inst` and its SaveState
            return false;
        }
    }
    return true;
}

void Licm::UnmarkHoistUsers(Inst *inst)
{
    for (auto &user : inst->GetUsers()) {
        user.GetInst()->ResetMarker(markerHoistInst_);
    }
}

static void MoveInstToPreHeader(Inst *inst, BasicBlock *preHeader, Inst *insertBefore)
{
    // Find position to move: either add first instruction to the empty block or after the last instruction
    auto *lastInst = preHeader->GetLastInst();

    inst->GetBasicBlock()->EraseInst(inst);
    if (lastInst == nullptr || lastInst->IsPhi()) {
        preHeader->AppendInst(inst);
    } else {
        ASSERT(lastInst->GetBasicBlock() == preHeader);
        Inst *ss = preHeader->FindSaveStateDeoptimize();
        // If insertBefore was set, we cannot insert inst after it
        if (ss != nullptr && (insertBefore == nullptr || ss->IsPrecedingInSameBlock(insertBefore)) &&
            EndOfPreheaderIsSafe(inst, ss)) {
            insertBefore = ss;
        }
        if (insertBefore != nullptr) {
            insertBefore->InsertBefore(inst);
        } else {
            lastInst->InsertAfter(inst);
        }
    }
}

void Licm::MoveInstructions(BasicBlock *preHeader, Loop *loop)
{
    // Move instructions
    for (auto inst : hoistableInstructions_) {
        if (!inst->IsMarked(markerHoistInst_)) {
            if (!inst->RequireState()) {
                hoistedInstCount_--;
            }
            UnmarkHoistUsers(inst);
            continue;
        }
        Inst *insertBefore = nullptr;
        if (inst->RequireState()) {
            auto ss = FindSaveStateForHoist(inst, preHeader, &insertBefore);
            if (ss != nullptr) {
                ASSERT(ss->IsSaveState());
                COMPILER_LOG(DEBUG, LICM_OPT)
                    << "Hoist instruction with id = " << inst->GetId() << " and link with SaveState " << ss->GetId();
                inst->SetSaveState(ss);
                inst->SetPc(ss->GetPc());
                hoistedInstCount_++;
            } else {
                UnmarkHoistUsers(inst);
                continue;
            }
        }
        auto *target = inst->GetPrev();
        auto *targetBB = inst->GetBasicBlock();
        MoveInstToPreHeader(inst, preHeader, insertBefore);
        GetGraph()->GetEventWriter().EventLicm(inst->GetId(), inst->GetPc(), loop->GetId(),
                                               preHeader->GetLoop()->GetId());
        if (inst->IsMovableObject()) {
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), inst, target, nullptr, targetBB);
        }
    }
}

/*
 * Iterate over all instructions in the loop and move hoistable instructions to the loop preheader
 */
void Licm::VisitLoop(Loop *loop)
{
    auto markerHolder = MarkerHolder(GetGraph());
    markerHoistInst_ = markerHolder.GetMarker();
    hoistableInstructions_.clear();
    // Collect instruction, which can be hoisted
    for (auto block : loop->GetBlocks()) {
        ASSERT(block->GetLoop() == loop);
        for (auto inst : block->InstsSafe()) {
            if (!IsInstHoistable(inst)) {
                continue;
            }
            TryAppendHoistableInst(inst, block, loop);
        }
    }
    auto preHeader = loop->GetPreHeader();
    ASSERT(preHeader != nullptr);
    auto header = loop->GetHeader();
    if (preHeader->GetSuccsBlocks().size() > 1) {
        ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
        // Create new pre-header with single successor: loop header
        auto newPreHeader = preHeader->InsertNewBlockToSuccEdge(header);
        preHeader->GetLoop()->AppendBlock(newPreHeader);
        // Fix Dominators info
        auto &domTree = GetGraph()->GetAnalysis<DominatorsTree>();
        domTree.SetValid(true);
        ASSERT(header->GetDominator() == preHeader);
        preHeader->RemoveDominatedBlock(header);
        domTree.SetDomPair(newPreHeader, header);
        domTree.SetDomPair(preHeader, newPreHeader);
        // Change pre-header
        loop->SetPreHeader(newPreHeader);
        preHeader = newPreHeader;
    }
    MoveInstructions(preHeader, loop);
}

static bool FindUnsafeInstBetween(const BasicBlock *domBb, BasicBlock *currBb, Marker visited, const Inst *resolver,
                                  Inst *startInst = nullptr)
{
    ASSERT(resolver->IsResolver());

    if (domBb == currBb) {
        return false;
    }
    if (currBb->SetMarker(visited)) {
        return false;
    }
    // Do not cross a try block because the resolver
    // can throw NoSuch{Method|Field}Exception
    if (currBb->IsTry()) {
        return true;
    }
    for (auto inst : InstSafeReverseIter(*currBb, startInst)) {
        if (IsUnsafeForResolver(inst)) {
            return true;
        }
    }
    for (auto pred : currBb->GetPredsBlocks()) {
        if (FindUnsafeInstBetween(domBb, pred, visited, resolver)) {
            return true;
        }
    }
    return false;
}

Inst *Licm::FindSaveStateForHoist(Inst *hoisted, const BasicBlock *preHeader, Inst **insertBefore)
{
    ASSERT(!hoisted->IsMovableObject());
    ASSERT(preHeader->GetSuccsBlocks().size() == 1);
    // Lookup for an appropriate SaveState in the preheader
    Inst *ss = nullptr;
    for (const auto &inst : preHeader->InstsSafeReverse()) {
        if (hoisted->IsRuntimeCall() && inst->IsMovableObject()) {
            // In this case we have to avoid instructions, which may trigger GC and move objects after `inst`,
            // thus we can move the instruction to the pre_header only before `inst`.
            *insertBefore = inst;
        }
        if (inst->GetOpcode() == Opcode::SaveState) {
            // Give up further checking if SaveState came from an inlined function.
            if (static_cast<SaveStateInst *>(inst)->GetCallerInst() != nullptr) {
                return nullptr;
            }
            ss = inst;
            break;
        }
    }
    if (ss == nullptr) {
        return nullptr;
    }
    ASSERT(ss->IsDominate(hoisted));
    if (*insertBefore != nullptr && !EndOfPreheaderIsSafe(hoisted, *insertBefore)) {
        return nullptr;
    }
    if (hoisted->IsResolver()) {
        // Check if the path from the resolver instruction to the SaveState block is safe
        MarkerHolder visited(GetGraph());
        if (FindUnsafeInstBetween(preHeader, hoisted->GetBasicBlock(), visited.GetMarker(), hoisted,
                                  hoisted->GetPrev())) {
            return nullptr;
        }
    }
    return ss;
}

/*
 * Instruction is not hoistable if one of the following conditions are true:
 *  - it has input which is defined in the same loop and not mark as hoistable
 *  - it has input which is not dominate instruction's loop preheader
 */
bool Licm::InstInputDominatesPreheader(Inst *inst)
{
    ASSERT(!inst->IsPhi());
    auto instLoop = inst->GetBasicBlock()->GetLoop();
    for (auto input : inst->GetInputs()) {
        // Graph is in SSA form and the instructions not PHI,
        // so every 'input' must dominate 'inst'.
        ASSERT(input.GetInst()->IsDominate(inst));
        auto inputLoop = input.GetInst()->GetBasicBlock()->GetLoop();
        if (instLoop == inputLoop) {
            if (input.GetInst()->IsSaveState()) {
                // Inst is coupled with SaveState that is not hoistable.
                // We will try to find an appropriate SaveState in the preheader.
                continue;
            }
            if (!input.GetInst()->IsMarked(markerHoistInst_)) {
                return false;
            }
        } else if (instLoop->GetOuterLoop() == inputLoop->GetOuterLoop()) {
            if (!input.GetInst()->GetBasicBlock()->IsDominate(instLoop->GetPreHeader())) {
                return false;
            }
        } else if (!instLoop->IsInside(inputLoop)) {
            return false;
        }
    }
    return true;
}

/*
 * Hoistable instruction must dominate all loop exits
 */
bool Licm::InstDominatesLoopExits(Inst *inst)
{
    auto instLoop = inst->GetBasicBlock()->GetLoop();
    for (auto block : instLoop->GetBlocks()) {
        if (IsBlockLoopExit(block) && !inst->GetBasicBlock()->IsDominate(block)) {
            return false;
        }
    }
    return true;
}

/*
 * Check if instruction can be moved to the loop preheader
 */
bool Licm::IsInstHoistable(Inst *inst)
{
    ASSERT(!inst->IsPhi());
    if (inst->IsNotHoistable()) {
        return false;
    }
    // Do not hoist the resolver if it came from an inlined function.
    auto saveState = inst->GetSaveState();
    ASSERT(!inst->IsResolver() || saveState != nullptr);
    if (inst->IsResolver() && saveState->GetCallerInst() != nullptr) {
        return false;
    }
    if (inst->GetOpcode() == Opcode::LenArray &&
        !BoundsAnalysis::IsInstNotNull(inst->GetDataFlowInput(0), inst->GetBasicBlock()->GetLoop()->GetHeader())) {
        return false;
    }
    return InstInputDominatesPreheader(inst) && InstDominatesLoopExits(inst) && !GetGraph()->IsInstThrowable(inst);
}
}  // namespace ark::compiler
