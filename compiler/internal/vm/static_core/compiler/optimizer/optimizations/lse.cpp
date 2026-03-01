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
#include "compiler/optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/lse.h"

namespace ark::compiler {

static std::string LogInst(const Inst *inst)
{
    return "v" + std::to_string(inst->GetId()) + " (" + GetOpcodeString(inst->GetOpcode()) + ")";
}

static Lse::EquivClass GetEquivClass(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
        case Opcode::LoadArrayI:
        case Opcode::StoreArray:
        case Opcode::StoreArrayI:
        case Opcode::LoadArrayPair:
        case Opcode::LoadArrayPairI:
        case Opcode::StoreArrayPair:
        case Opcode::StoreArrayPairI:
        case Opcode::LoadConstArray:
        case Opcode::FillConstArray:
            return Lse::EquivClass::EQ_ARRAY;
        case Opcode::LoadStatic:
        case Opcode::StoreStatic:
        case Opcode::UnresolvedStoreStatic:
        case Opcode::LoadResolvedObjectFieldStatic:
        case Opcode::StoreResolvedObjectFieldStatic:
            return Lse::EquivClass::EQ_STATIC;
        case Opcode::LoadString:
        case Opcode::LoadType:
        case Opcode::UnresolvedLoadType:
            return Lse::EquivClass::EQ_POOL;
        default:
            return Lse::EquivClass::EQ_OBJECT;
    }
}

static bool PhiHasInput(Inst *phi, Inst *inst)
{
    const auto &inputs = phi->GetInputs();
    return std::any_of(inputs.begin(), inputs.end(), [inst](const Input &x) { return x.GetInst() == inst; });
}

class LseVisitor {
public:
    explicit LseVisitor(Graph *graph, Lse::HeapEqClasses *heaps)
        : aa_(graph->GetAnalysis<AliasAnalysis>()),
          heaps_(*heaps),
          eliminations_(graph->GetLocalAllocator()->Adapter()),
          shadowedStores_(graph->GetLocalAllocator()->Adapter()),
          disabledObjects_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(LseVisitor);
    NO_COPY_SEMANTIC(LseVisitor);
    ~LseVisitor() = default;

    void MarkForElimination(Inst *inst, Inst *reason, const Lse::HeapValue *hvalue)
    {
        if (Lse::CanEliminateInstruction(inst)) {
            auto heap = *hvalue;
            while (eliminations_.find(heap.val) != eliminations_.end()) {
                heap = eliminations_[heap.val];
            }
            eliminations_[inst] = heap;
            COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " is eliminated because of " << LogInst(reason);
        }
    }

    bool CheckMustAlias(Inst *baseObject)
    {
        for (auto disabledObject : disabledObjects_) {
            aliasCalls_++;
            if (aa_.CheckRefAlias(baseObject, disabledObject) == MUST_ALIAS) {
                return true;
            }
        }
        return false;
    }

    void VisitStore(Inst *inst, Inst *val)
    {
        if (val == nullptr) {
            /* Instruction has no stored value (e.g. FillConstArray) */
            return;
        }
        auto baseObject = inst->GetDataFlowInput(0);
        if (CheckMustAlias(baseObject)) {
            return;
        }
        auto hvalue = GetHeapValue(inst);
        /* Value can be eliminated already */
        auto alive = val;
        auto eliminated = eliminations_.find(val);
        if (eliminated != eliminations_.end()) {
            alive = eliminated->second.val;
        }
        /* If store was assigned to VAL then we can eliminate the second assignment */
        if (hvalue != nullptr && hvalue->val == alive) {
            MarkForElimination(inst, hvalue->origin, hvalue);
            return;
        }
        if (hvalue != nullptr && hvalue->origin->IsStore() && !hvalue->read) {
            if (hvalue->origin->GetBasicBlock() == inst->GetBasicBlock()) {
                const Lse::HeapValue heap = {inst, alive, false, false};
                MarkForElimination(hvalue->origin, inst, &heap);
            } else {
                auto it =
                    shadowedStores_.try_emplace(hvalue->origin, aa_.GetGraph()->GetLocalAllocator()->Adapter()).first;
                it->second.push_back(inst);
            }
        }
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " updated heap with v" << alive->GetId();

        /* Stores added to eliminations_ above aren't checked versus phis -> no double instruction elimination */
        UpdatePhis(inst);

        auto &blockHeap = heaps_[GetEquivClass(inst)].first.at(inst->GetBasicBlock());
        uint32_t encounters = 0;
        /* Erase all aliased values, because they may be overwritten */
        EraseAliasedValues(blockHeap, inst, baseObject, encounters);

        // If we reached limit for this object, remove all its MUST_ALIASed instructions from heap
        // and disable this object for this BB
        if (encounters > Lse::LS_ACCESS_LIMIT) {
            for (auto heapIter = blockHeap.begin(), heapLast = blockHeap.end(); heapIter != heapLast;) {
                auto hinst = heapIter->first;
                aliasCalls_++;
                if (HasBaseObject(hinst) && aa_.CheckRefAlias(baseObject, hinst->GetDataFlowInput(0)) == MUST_ALIAS) {
                    COMPILER_LOG(DEBUG, LSE_OPT)
                        << "\tDrop from heap { " << LogInst(hinst) << ", v" << heapIter->second.val->GetId() << "}";
                    heapIter = blockHeap.erase(heapIter);
                } else {
                    heapIter++;
                }
            }
            disabledObjects_.push_back(baseObject);
            return;
        }

        /* Set value of the inst to VAL */
        blockHeap[inst] = {inst, alive, false, false};
    }

    void VisitLoad(Inst *inst)
    {
        if (HasBaseObject(inst)) {
            auto input = inst->GetDataFlowInput(0);
            for (auto disabledObject : disabledObjects_) {
                aliasCalls_++;
                if (aa_.CheckRefAlias(input, disabledObject) == MUST_ALIAS) {
                    return;
                }
            }
        }
        /* If we have a heap value for this load instruction then we can eliminate it */
        auto hvalue = GetHeapValue(inst);
        if (hvalue != nullptr) {
            MarkForElimination(inst, hvalue->origin, hvalue);
            return;
        }
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " updated heap with v" << inst->GetId();

        /* Loads added to eliminations_ above are not checked versus phis -> no double instruction elimination */
        UpdatePhis(inst);

        /* Otherwise set the value of instruction to itself and update MUST_ALIASes */
        heaps_[GetEquivClass(inst)].first.at(inst->GetBasicBlock())[inst] = {inst, inst, true, false};
    }

    bool HasBaseObject(Inst *inst)
    {
        ASSERT(inst->IsMemory());
        if (inst->GetInputsCount() == 0 || inst->GetInput(0).GetInst()->IsSaveState() ||
            inst->GetDataFlowInput(0)->GetType() == DataType::POINTER) {
            return false;
        }
        ASSERT(inst->GetDataFlowInput(0)->IsReferenceOrAny());
        return true;
    }

    void VisitIntrinsicCheckInvariant(Inst *inv)
    {
        for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
            auto &blockHeap = heaps_[eqClass].first.at(inv->GetBasicBlock());
            for (auto heapIter = blockHeap.begin(), heapLast = blockHeap.end(); heapIter != heapLast;) {
                auto hinst = heapIter->first;
                aliasCalls_++;

                if (!HasBaseObject(hinst) || aa_.CheckRefAlias(inv, hinst->GetDataFlowInput(0)) == NO_ALIAS) {
                    heapIter++;
                } else {
                    COMPILER_LOG(DEBUG, LSE_OPT)
                        << "\tDrop from heap { " << LogInst(hinst) << ", v" << heapIter->second.val->GetId() << "}";
                    heapIter = blockHeap.erase(heapIter);
                }
            }
        }
    }
    void VisitIntrinsic(Inst *inst, InstVector *invs)
    {
        // CC-OFFNXT(C_RULE_SWITCH_BRANCH_CHECKER) autogenerated code
        switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
#include "intrinsics_lse_heap_inv_args.inl"
            default:
                return;
        }
        for (auto inv : *invs) {
            VisitIntrinsicCheckInvariant(inv);
        }
        invs->clear();
    }

    /// Completely resets the accumulated state: heap and phi candidates.
    void InvalidateHeap(BasicBlock *block)
    {
        for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
            heaps_[eqClass].first.at(block).clear();
            auto loop = block->GetLoop();
            while (!loop->IsRoot()) {
                heaps_[eqClass].second.at(loop).clear();
                loop = loop->GetOuterLoop();
            }
        }
        disabledObjects_.clear();
    }

    /// Clears heap of local only values as we don't want them to affect analysis further
    void ClearLocalValuesFromHeap(BasicBlock *block)
    {
        for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
            auto &blockHeap = heaps_[eqClass].first.at(block);

            auto heapIt = blockHeap.begin();
            while (heapIt != blockHeap.end()) {
                if (heapIt->second.local) {
                    heapIt = blockHeap.erase(heapIt);
                } else {
                    heapIt++;
                }
            }
        }
    }

    /// Release objects and reset Alias Analysis count
    void ResetLimits()
    {
        disabledObjects_.clear();
        aliasCalls_ = 0;
    }

    uint32_t GetAliasAnalysisCallCount()
    {
        return aliasCalls_;
    }

    /// Marks all values currently on heap as potentially read
    void SetHeapAsRead(BasicBlock *block)
    {
        for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
            auto &bheap = heaps_[eqClass].first.at(block);
            for (auto it = bheap.begin(); it != bheap.end();) {
                it->second.read = true;
                it++;
            }
        }
    }

    auto &GetEliminations()
    {
        return eliminations_;
    }

    bool ProcessBackedges(PhiInst *phi, Loop *loop, Inst *cand, InstVector *insts)
    {
        // Now find which values are alive for each backedge. Due to MUST_ALIAS requirement,
        // there should only be one
        auto &heap = heaps_[GetEquivClass(cand)].first;
        for (auto bb : loop->GetHeader()->GetPredsBlocks()) {
            if (bb == loop->GetPreHeader()) {
                phi->AppendInput(cand->IsStore() ? InstStoredValue(cand) : cand);
                continue;
            }
            auto &blockHeap = heap.at(bb);
            Inst *alive = nullptr;
            for (auto &entry : blockHeap) {
                if (std::find(insts->begin(), insts->end(), entry.first) != insts->end()) {
                    alive = entry.first;
                    break;
                }
            }
            if (alive == nullptr) {
                COMPILER_LOG(DEBUG, LSE_OPT)
                    << "Skipping phi candidate " << LogInst(cand) << ": no alive insts found for backedge block";
                return false;
            }
            // There are several cases when a MUST_ALIAS load is alive at backedge while stores
            // exist in the loop. The one we're interested here is inner loops.
            if (alive->IsLoad() && !loop->GetInnerLoops().empty()) {
                auto aliveIt = std::find(insts->rbegin(), insts->rend(), alive);
                // We've checked that no stores exist in inner loops earlier, which allows us to just check
                // the first store before the load
                auto it = std::find_if(aliveIt, insts->rend(), [](auto *inst) { return inst->IsStore(); });
                if (it != insts->rend() && (*it)->IsDominate(alive)) {
                    auto val = heap.at((*it)->GetBasicBlock())[(*it)].val;
                    // Use the store instead of load
                    phi->AppendInput(val);
                    // And eliminate load
                    struct Lse::HeapValue hvalue = {*it, val, false, false};
                    MarkForElimination(alive, *it, &hvalue);
                    continue;
                }
            }
            phi->AppendInput(blockHeap[alive].val);
        }

        if (ExistReadsOfModifiedValues(phi, loop, insts)) {
            COMPILER_LOG(DEBUG, LSE_OPT) << "Skipping phi candidate " << LogInst(cand)
                                         << ": load inst of a modified value found";
            return false;
        }

        return true;
    }

    bool ExistReadsOfModifiedValues(PhiInst *phi, [[maybe_unused]] Loop *loop, InstVector *insts)
    {
        // Check if any loads may read a modified value
        bool hasLaterRPOLoad = false;
        for (auto instIt = insts->rbegin(); instIt != insts->rend(); ++instIt) {
            auto *inst = *instIt;
            auto isBackedgeDominated = [loop, inst](BasicBlock *bb) {
                return bb == loop->GetPreHeader() || inst->GetBasicBlock()->IsDominate(bb);
            };
            if (inst->IsStore()) {
                const auto &preds = loop->GetHeader()->GetPredsBlocks();
                if (hasLaterRPOLoad && !std::all_of(preds.begin(), preds.end(), isBackedgeDominated)) {
                    // RPO later load may (not precise) see this conditional store
                    return true;
                }
            } else if (inst->IsLoad() && !PhiHasInput(phi, inst)) {
                // Don't track phi inputs - they won't be eliminated anyway
                hasLaterRPOLoad = true;
            }
        }
        return false;
    }

    void LoopDoElimination(Inst *cand, Loop *loop, PhiInst *phi, InstVector *insts)
    {
        auto replacement = phi != nullptr ? phi : cand;
        struct Lse::HeapValue hvalue = {
            replacement, replacement->IsStore() ? InstStoredValue(replacement) : replacement, false, false};
        for (auto inst : *insts) {
            // No need to check MUST_ALIAS again, but we need to check for double elim
            if (eliminations_.find(inst) != eliminations_.end()) {
                continue;
            }

            // Don't replace loads that are also phi inputs
            if (phi != nullptr && PhiHasInput(phi, inst)) {
                continue;
            }
            if (inst->IsLoad()) {
                MarkForElimination(inst, replacement, &hvalue);
            }
        }
        // And fix savestates for loads
        if (phi != nullptr) {
            for (size_t i = 0; i < phi->GetInputsCount(); i++) {
                auto bb = loop->GetHeader()->GetPredecessor(i);
                auto phiInput = phi->GetInput(i).GetInst();
                if (bb == loop->GetPreHeader() && cand->IsLoad() && cand->IsMovableObject()) {
                    ssb_.SearchAndCreateMissingObjInSaveState(bb->GetGraph(), cand, phi);
                } else if (phiInput->IsMovableObject()) {
                    ssb_.SearchAndCreateMissingObjInSaveState(bb->GetGraph(), phiInput, bb->GetLastInst(), nullptr, bb);
                }
            }
        } else {
            ASSERT(hvalue.val != nullptr);
            if (hvalue.val->IsMovableObject()) {
                ASSERT(!loop->IsIrreducible());
                auto head = loop->GetHeader();
                // Add cand value to all SaveStates in this loop and inner loops
                ssb_.SearchAndCreateMissingObjInSaveState(head->GetGraph(), hvalue.val, head->GetLastInst(), nullptr,
                                                          head);
            }
        }
    }

    /// Add eliminations inside loops if there is no overwrites on backedges.
    void FinalizeLoops(Graph *graph, const ArenaVector<Loop *> &rpoLoops)
    {
        for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
            for (auto loopIt = rpoLoops.rbegin(); loopIt != rpoLoops.rend(); loopIt++) {
                auto loop = *loopIt;
                auto &phis = heaps_[eqClass].second.at(loop);
                COMPILER_LOG(DEBUG, LSE_OPT) << "Finalizing loop #" << loop->GetId();
                FinalizeLoopsWithPhiCands(graph, loop, phis);
            }

            COMPILER_LOG(DEBUG, LSE_OPT) << "Fixing elimination list after backedge substitutions";
            for (auto &entry : eliminations_) {
                auto hvalue = entry.second;
                if (eliminations_.find(hvalue.val) == eliminations_.end()) {
                    continue;
                }

                [[maybe_unused]] auto initial = hvalue.val;
                while (eliminations_.find(hvalue.val) != eliminations_.end()) {
                    auto elimValue = eliminations_[hvalue.val];
                    COMPILER_LOG(DEBUG, LSE_OPT) << "\t" << LogInst(hvalue.val)
                                                 << " is eliminated. Trying to replace by " << LogInst(elimValue.val);
                    hvalue = elimValue;
                    ASSERT_PRINT(initial != hvalue.val, "A cyclic elimination has been detected");
                }
                entry.second = hvalue;
            }
        }
    }

    bool ExistsPathWithoutShadowingStores(BasicBlock *start, BasicBlock *block, Marker marker)
    {
        if (block->IsEndBlock()) {
            // Found a path without shadowing stores
            return true;
        }
        if (block->IsMarked(marker)) {
            // Found a path with shadowing store
            return false;
        }
        ASSERT(start->GetLoop() == block->GetLoop());
        for (auto succ : block->GetSuccsBlocks()) {
            if (block->GetLoop() != succ->GetLoop()) {
                // Edge to a different loop. We currently don't carry heap values through loops and don't
                // handle irreducible loops, so we can't be sure there are shadows on this path.
                return true;
            }
            if (succ == block->GetLoop()->GetHeader()) {
                // If next block is a loop header for this block's loop it means that instruction shadows itself
                return true;
            }
            if (ExistsPathWithoutShadowingStores(start, succ, marker)) {
                return true;
            }
        }
        return false;
    }

    void FinalizeShadowedStores()
    {
        // We want to see if there are no paths from the store that evade any of its shadow stores
        for (auto &entry : shadowedStores_) {
            auto inst = entry.first;
            auto marker = inst->GetBasicBlock()->GetGraph()->NewMarker();
            auto &shadows = shadowedStores_.at(inst);
            for (auto shadow : shadows) {
                shadow->GetBasicBlock()->SetMarker(marker);
            }
            if (!ExistsPathWithoutShadowingStores(inst->GetBasicBlock(), inst->GetBasicBlock(), marker)) {
                COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " is fully shadowed by aliased stores";
                auto alive = InstStoredValue(entry.second[0]);
                auto eliminated = eliminations_.find(alive);
                if (eliminated != eliminations_.end()) {
                    alive = eliminated->second.val;
                }
                const Lse::HeapValue heap = {entry.second[0], alive, false, false};
                MarkForElimination(inst, entry.second[0], &heap);
            }
            inst->GetBasicBlock()->GetGraph()->EraseMarker(marker);
        }
    }

    bool CheckRefMustAlias(Inst *first, Inst *second)
    {
        ASSERT(first != second);
        auto firstIt = eliminations_.find(first);
        if (firstIt != eliminations_.end() && firstIt->second.origin == second) {
            return true;
        }
        auto secondIt = eliminations_.find(second);
        if (secondIt != eliminations_.end() && secondIt->second.origin == first) {
            return true;
        }
        return firstIt != eliminations_.end() && secondIt != eliminations_.end() &&
               firstIt->second.val == secondIt->second.val;
    }

    bool CheckInstMustAlias(Inst *first, Inst *second)
    {
        aliasCalls_++;
        auto aliasType = aa_.CheckInstAlias<true>(first, second);
        COMPILER_LOG(DEBUG, LSE_OPT) << "Alias type: " << aliasType;
        if (aliasType != ALIAS_IF_BASE_EQUALS) {
            return aliasType == MUST_ALIAS;
        }
        return CheckRefMustAlias(first->GetDataFlowInput(0), second->GetDataFlowInput(0));
    }

private:
    /// Return a MUST_ALIASed heap entry, nullptr if not present.
    const Lse::HeapValue *GetHeapValue(Inst *inst)
    {
        auto &blockHeap = heaps_[GetEquivClass(inst)].first.at(inst->GetBasicBlock());
        for (auto &entry : blockHeap) {
            if (CheckInstMustAlias(inst, entry.first)) {
                return &entry.second;
            }
        }
        return nullptr;
    }

    /// Update phi candidates with aliased accesses
    void UpdatePhis(Inst *inst)
    {
        Loop *loop = inst->GetBasicBlock()->GetLoop();

        while (!loop->IsRoot()) {
            auto &phis = heaps_[GetEquivClass(inst)].second.at(loop);
            for (auto &[mem, values] : phis) {
                aliasCalls_++;
                if (aa_.CheckInstAlias(inst, mem) != NO_ALIAS) {
                    values.push_back(inst);
                }
            }
            loop = loop->GetOuterLoop();
        }
    }

    void EraseAliasedValues(Lse::BasicBlockHeap &blockHeap, Inst *inst, Inst *baseObject, uint32_t &encounters)
    {
        for (auto heapIter = blockHeap.begin(), heapLast = blockHeap.end(); heapIter != heapLast;) {
            auto hinst = heapIter->first;
            ASSERT(GetEquivClass(inst) == GetEquivClass(hinst));
            aliasCalls_++;
            if (aa_.CheckInstAlias(inst, hinst) == NO_ALIAS) {
                // Keep track if it's the same object but with different offset
                heapIter++;
                aliasCalls_++;
                if (aa_.CheckRefAlias(baseObject, hinst->GetDataFlowInput(0)) == MUST_ALIAS) {
                    encounters++;
                }
            } else {
                COMPILER_LOG(DEBUG, LSE_OPT)
                    << "\tDrop from heap { " << LogInst(hinst) << ", v" << heapIter->second.val->GetId() << "}";
                heapIter = blockHeap.erase(heapIter);
            }
        }
    }

    void FinalizeLoopsWithPhiCands(Graph *graph, Loop *loop, ArenaMap<Inst *, InstVector> &phis)
    {
        InstVector instsMustAlias(graph->GetLocalAllocator()->Adapter());
        for (auto &[cand, insts] : phis) {
            if (insts.empty()) {
                COMPILER_LOG(DEBUG, LSE_OPT) << "Skipping phi candidate " << LogInst(cand) << " (no users)";
                continue;
            }

            instsMustAlias.clear();

            COMPILER_LOG(DEBUG, LSE_OPT) << "Processing phi candidate: " << LogInst(cand);
            bool hasStores = false;
            bool hasLoads = false;
            bool valid = true;

            for (auto inst : insts) {
                // Skip eliminated instructions
                if (eliminations_.find(inst) != eliminations_.end()) {
                    continue;
                }
                auto aliasResult = aa_.CheckInstAlias(cand, inst);
                if (aliasResult == MAY_ALIAS && inst->IsLoad()) {
                    // Ignore MAY_ALIAS loads, they won't interfere with our analysis
                    continue;
                } else if (aliasResult == MAY_ALIAS) {  // NOLINT(readability-else-after-return)
                    // If we have a MAY_ALIAS store we can't be sure about our values
                    // in phi creation
                    ASSERT(inst->IsStore());
                    COMPILER_LOG(DEBUG, LSE_OPT)
                        << "Skipping phi candidate " << LogInst(cand) << ": MAY_ALIAS by " << LogInst(inst);
                    valid = false;
                    break;
                }
                ASSERT(aliasResult == MUST_ALIAS);

                if (inst->IsStore() && inst->GetBasicBlock()->GetLoop() != loop) {
                    // We can handle if loads are in inner loop, but if a store is in inner loop
                    // then we can't replace anything
                    COMPILER_LOG(DEBUG, LSE_OPT)
                        << "Skipping phi candidate " << LogInst(cand) << ": " << LogInst(inst) << " is in inner loop";
                    valid = false;
                    break;
                }

                instsMustAlias.push_back(inst);
                if (inst->IsStore()) {
                    hasStores = true;
                } else if (inst->IsLoad()) {
                    hasLoads = true;
                }
            }
            // Other than validity, it's also possible that all instructions are already eliminated
            if (!valid || instsMustAlias.empty()) {
                continue;
            }

            TryLoopDoElimination(cand, loop, &instsMustAlias, hasLoads, hasStores);
        }
    }

    void TryLoopDoElimination(Inst *cand, Loop *loop, InstVector *insts, bool hasLoads, bool hasStores)
    {
        if (hasStores) {
            if (!hasLoads) {
                // Nothing to replace
                COMPILER_LOG(DEBUG, LSE_OPT)
                    << "Skipping phi candidate " << LogInst(cand) << ": no loads to convert to phi";
                return;
            }

            auto phi = cand->GetBasicBlock()->GetGraph()->CreateInstPhi(cand->GetType(), cand->GetPc());
            loop->GetHeader()->AppendPhi(phi);

            if (!ProcessBackedges(phi, loop, cand, insts)) {
                loop->GetHeader()->RemoveInst(phi);
                return;
            }

            LoopDoElimination(cand, loop, phi, insts);
        } else {
            ASSERT(hasLoads);
            // Without stores, we can replace all MUST_ALIAS loads with instruction itself
            LoopDoElimination(cand, loop, nullptr, insts);
        }
    }

private:
    AliasAnalysis &aa_;
    Lse::HeapEqClasses &heaps_;
    /* Map of instructions to be deleted with values to replace them with */
    Lse::BasicBlockHeap eliminations_;
    ArenaMap<Inst *, ArenaVector<Inst *>> shadowedStores_;
    SaveStateBridgesBuilder ssb_;
    InstVector disabledObjects_;
    uint32_t aliasCalls_ {0};
};

/// Returns true if the instruction invalidates the whole heap
bool Lse::IsHeapInvalidatingInst(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::LoadStatic:
            return inst->CastToLoadStatic()->GetVolatile();
        case Opcode::LoadObject:
            return inst->CastToLoadObject()->GetVolatile();
        case Opcode::InitObject:
        case Opcode::InitClass:
        case Opcode::LoadAndInitClass:
        case Opcode::UnresolvedLoadAndInitClass:
        case Opcode::UnresolvedStoreStatic:
        case Opcode::ResolveObjectField:
        case Opcode::ResolveObjectFieldStatic:
            return true;  //  runtime calls invalidate the heap
        case Opcode::CallVirtual:
            return !inst->CastToCallVirtual()->IsInlined();
        case Opcode::CallResolvedVirtual:
            return !inst->CastToCallResolvedVirtual()->IsInlined();
        case Opcode::CallStatic:
            return !inst->CastToCallStatic()->IsInlined();
        case Opcode::CallDynamic:
            return !inst->CastToCallDynamic()->IsInlined();
        case Opcode::CallResolvedStatic:
            return !inst->CastToCallResolvedStatic()->IsInlined();
        case Opcode::Monitor:
            return inst->CastToMonitor()->IsEntry();
        default:
            return inst->GetFlag(compiler::inst_flags::HEAP_INV);
    }
}

/// Returns true if the instruction reads from heap and we're not sure about its effects
static bool IsHeapReadingInst(Inst *inst)
{
    if (inst->CanThrow()) {
        return true;
    }
    if (inst->IsIntrinsic()) {
        for (auto input : inst->GetInputs()) {
            if (input.GetInst()->GetType() == DataType::REFERENCE) {
                return true;
            }
        }
    }
    if (inst->IsStore() && IsVolatileMemInst(inst)) {
        // Heap writes before the volatile store should be visible from another thread after a corresponding volatile
        // load
        return true;
    }
    if (inst->GetOpcode() == Opcode::Monitor) {
        return inst->CastToMonitor()->IsExit();
    }
    return false;
}

bool Lse::CanEliminateInstruction(Inst *inst)
{
    if (inst->IsBarrier()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was suppressed: a barrier";
        return false;
    }
    auto loop = inst->GetBasicBlock()->GetLoop();
    if (loop->IsIrreducible()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was suppressed: an irreducible loop";
        return false;
    }
    if (loop->IsOsrLoop()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was suppressed: an OSR loop";
        return false;
    }
    if (loop->IsTryCatchLoop()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was suppressed: an try-catch loop";
        return false;
    }
    return true;
}

void Lse::InitializeHeap(BasicBlock *block, HeapEqClasses *heaps)
{
    for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
        auto &heap = (*heaps)[eqClass].first;
        auto &phiCands = (*heaps)[eqClass].second;
        [[maybe_unused]] auto it = heap.emplace(block, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(it.second);
        if (block->IsLoopHeader()) {
            COMPILER_LOG(DEBUG, LSE_OPT) << "Append loop #" << block->GetLoop()->GetId();
            if (std::find(rpoLoops_.begin(), rpoLoops_.end(), block->GetLoop()) == rpoLoops_.end()) {
                rpoLoops_.emplace_back(block->GetLoop());
            }
            [[maybe_unused]] auto phit = phiCands.emplace(block->GetLoop(), GetGraph()->GetLocalAllocator()->Adapter());
            ASSERT(phit.second);
        }
    }
}

/**
 * While entering in the loop we put all heap values obtained from loads as phi candidates.
 * Further phi candidates would replace MUST_ALIAS accesses in the loop if no aliased stores were met.
 */
void Lse::MergeHeapValuesForLoop(BasicBlock *block, HeapEqClasses *heaps)
{
    ASSERT(block->IsLoopHeader());

    // Do not eliminate anything in irreducible or osr loops
    auto loop = block->GetLoop();
    if (loop->IsIrreducible() || loop->IsOsrLoop() || loop->IsTryCatchLoop()) {
        return;
    }

    auto preheader = loop->GetPreHeader();

    for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
        auto &preheaderHeap = (*heaps)[eqClass].first.at(preheader);

        auto &blockPhis = (*heaps)[eqClass].second.at(loop);
        for (auto mem : preheaderHeap) {
            blockPhis.try_emplace(mem.second.origin, GetGraph()->GetLocalAllocator()->Adapter());
            COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(mem.first) << " is a phi cand for BB #" << block->GetId();
        }
    }
}

/// Merge heap values for passed block from its direct predecessors.
void Lse::MergeHeapValuesForBlock(BasicBlock *block, HeapEqClasses *heaps, Marker phiFixupMrk)
{
    for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
        auto &heap = (*heaps)[eqClass].first;
        auto &blockHeap = heap.at(block);
        /* Copy a heap of one of predecessors */
        auto preds = block->GetPredsBlocks();
        auto predIt = preds.begin();
        if (predIt != preds.end()) {
            blockHeap.insert(heap.at(*predIt).begin(), heap.at(*predIt).end());
            predIt++;
        }

        ProcessHeapValuesForBlock(&heap, block, phiFixupMrk);
    }
}

void Lse::ProcessHeapValuesForBlock(Heap *heap, BasicBlock *block, Marker phiFixupMrk)
{
    auto &blockHeap = heap->at(block);
    auto preds = block->GetPredsBlocks();
    auto predIt = preds.begin();
    if (predIt != preds.end()) {
        predIt++;
    }

    /* Erase from the heap anything that disappeared or was changed in other predecessors */
    while (predIt != preds.end()) {
        auto predHeap = heap->at(*predIt);
        auto heapIt = blockHeap.begin();
        while (heapIt != blockHeap.end()) {
            auto &heapValue = heapIt->second;
            auto predInstIt = ProcessPredecessorHeap(predHeap, heapValue, block, heapIt->first);
            if (predInstIt == predHeap.end() ||
                !ProcessHeapValues(heapValue, block, predInstIt, {preds.begin(), predIt}, phiFixupMrk)) {
                heapIt = blockHeap.erase(heapIt);
                continue;
            }
            if (predInstIt->second.val == heapValue.val) {
                heapIt->second.read |= predInstIt->second.read;
            }

            heapIt++;
        }
        predIt++;
    }
}

Lse::BasicBlockHeapIter Lse::ProcessPredecessorHeap(BasicBlockHeap &predHeap, HeapValue &heapValue, BasicBlock *block,
                                                    Inst *curInst)
{
    auto predInstIt = predHeap.begin();
    while (predInstIt != predHeap.end()) {
        if (predInstIt->first == curInst) {
            break;
        }
        if (visitor_->CheckInstMustAlias(curInst, predInstIt->first)) {
            break;
        }
        predInstIt++;
    }

    if (predInstIt == predHeap.end()) {
        // If this is a phi we're creating during merge, delete it
        if (heapValue.val->IsPhi() && heapValue.local) {
            block->RemoveInst(heapValue.val);
        }
    }
    return predInstIt;
}

bool Lse::ProcessHeapValues(HeapValue &heapValue, BasicBlock *block, BasicBlockHeapIter predInstIt,
                            PredBlocksItersPair iters, Marker phiFixupMrk)
{
    auto [predsBegin, predIt] = iters;
    if (predInstIt->second.val != heapValue.val) {
        // Try to create a phi instead
        // We limit possible phis to cases where value originated in the same predecessor
        // in order to not increase register usage too much
        if (block->GetLoop()->IsIrreducible() || block->IsCatch() ||
            predInstIt->second.origin->GetBasicBlock() != *predIt) {
            // If this is a phi we're creating during merge, delete it
            if (heapValue.val->IsPhi() && heapValue.local) {
                block->RemoveInst(heapValue.val);
            }
            return false;
        }
        if (heapValue.val->IsPhi() && heapValue.local) {
            heapValue.val->AppendInput(predInstIt->second.val);
        } else {
            // Values can only originate in a single block. If this predecessor is not
            // the second predecessor, that means that this value did not originate in other
            // predecessors, thus we don't create a phi
            if (heapValue.origin->GetBasicBlock() != *(predsBegin) || std::distance(predsBegin, predIt) > 1) {
                return false;
            }
            auto phi = block->GetGraph()->CreateInstPhi(heapValue.origin->GetType(), heapValue.origin->GetPc());
            block->AppendPhi(phi);
            phi->AppendInput(heapValue.val);
            phi->AppendInput(predInstIt->second.val);
            phi->SetMarker(phiFixupMrk);
            heapValue.val = phi;
            heapValue.local = true;
        }
    }
    return true;
}

/**
 * When creating phis while merging predecessor heaps, we don't know yet if
 * we're creating a useful phi, and can't fix SaveStates because of that.
 * Do that here.
 */
void Lse::FixupPhisInBlock(BasicBlock *block, Marker phiFixupMrk)
{
    for (auto phiInst : block->PhiInstsSafe()) {
        auto phi = phiInst->CastToPhi();
        if (!phi->IsMarked(phiFixupMrk)) {
            continue;
        }
        if (!phi->HasUsers()) {
            block->RemoveInst(phi);
        } else if (GetGraph()->IsBytecodeOptimizer() || !phi->IsReferenceOrAny()) {
            continue;
        }
        // Here case: !GetGraph()->IsBytecodeOptimizer() && phi->IsReferenceOrAny()
        for (auto i = 0U; i < phi->GetInputsCount(); i++) {
            auto input = phi->GetInput(i);
            if (input.GetInst()->IsMovableObject()) {
                auto bb = phi->GetPhiInputBb(i);
                ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), input.GetInst(), bb->GetLastInst(), nullptr, bb);
            }
        }
    }
}

/**
 * Returns the elimination code in two letter format.
 *
 * The first letter describes a [L]oad or [S]tore that was eliminated.
 * The second letter describes the dominant [L]oad or [S]tore that is the
 * reason why instruction was eliminated.
 */
const char *Lse::GetEliminationCode(Inst *inst, Inst *origin)
{
    ASSERT(inst->IsMemory() && (origin->IsMemory() || origin->IsPhi()));
    if (inst->IsLoad()) {
        if (origin->IsLoad()) {
            return "LL";
        }
        if (origin->IsStore()) {
            return "LS";
        }
        if (origin->IsPhi()) {
            return "LP";
        }
    }
    if (inst->IsStore()) {
        if (origin->IsLoad()) {
            return "SL";
        }
        if (origin->IsStore()) {
            return "SS";
        }
        if (origin->IsPhi()) {
            return "SP";
        }
    }
    UNREACHABLE();
}

/**
 * In the codegen of bytecode optimizer, we don't have corresponding pandasm
 * for the IR `Cast` of with some pairs of input types and output types. So
 * in the bytecode optimizer mode, we need to avoid generating such `Cast` IR.
 * The following function gives the list of legal pairs of types.
 * This function should not be used in compiler mode.
 */

static bool IsTypeLegalForCast(DataType::Type output, DataType::Type input)
{
    ASSERT(output != input);
    switch (input) {
        case DataType::INT32:
        case DataType::INT64:
        case DataType::FLOAT64:
            switch (output) {
                case DataType::FLOAT64:
                case DataType::INT64:
                case DataType::UINT32:
                case DataType::INT32:
                case DataType::INT16:
                case DataType::UINT16:
                case DataType::INT8:
                case DataType::UINT8:
                case DataType::ANY:
                    return true;
                default:
                    return false;
            }
        case DataType::REFERENCE:
            return output == DataType::ANY;
        default:
            return false;
    }
}

/**
 * Replace inputs of INST with VALUE and delete this INST.  If deletion led to
 * appearance of instruction that has no users delete this instruction too.
 */
void Lse::DeleteInstruction(Inst *inst, Inst *value)
{
    // Have to cast a value to the type of eliminated inst. Actually required only for loads.
    if (inst->GetType() != value->GetType() && inst->HasUsers()) {
        ASSERT(inst->GetType() != DataType::REFERENCE && value->GetType() != DataType::REFERENCE);
        // We will do nothing in bytecode optimizer mode when the types are not legal for cast.
        if (GetGraph()->IsBytecodeOptimizer() && !IsTypeLegalForCast(inst->GetType(), value->GetType())) {
            COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " was not eliminated: requires an inappropriate cast";
            return;
        }
        auto cast = GetGraph()->CreateInstCast(inst->GetType(), inst->GetPc(), value, value->GetType());
        inst->InsertAfter(cast);
        value = cast;
    }
    inst->ReplaceUsers(value);

    ArenaQueue<Inst *> queue(GetGraph()->GetLocalAllocator()->Adapter());
    queue.push(inst);
    while (!queue.empty()) {
        Inst *frontInst = queue.front();
        BasicBlock *block = frontInst->GetBasicBlock();
        queue.pop();

        // Have been already deleted or could not be deleted
        if (block == nullptr || frontInst->HasUsers()) {
            continue;
        }

        for (auto &input : frontInst->GetInputs()) {
            /* Delete only instructions that has no data flow impact */
            if (input.GetInst()->HasPseudoDestination()) {
                queue.push(input.GetInst());
            }
        }
        block->RemoveInst(frontInst);
        applied_ = true;
    }
}

void Lse::DeleteInstructions(const BasicBlockHeap &eliminated)
{
    for (auto elim : eliminated) {
        Inst *inst = elim.first;
        Inst *origin = elim.second.origin;
        Inst *value = elim.second.val;

        ASSERT_DO(eliminated.find(value) == eliminated.end(),
                  (std::cerr << "Instruction:\n", inst->Dump(&std::cerr),
                   std::cerr << "is replaced by eliminated value:\n", value->Dump(&std::cerr)));

        while (origin->GetBasicBlock() == nullptr) {
            auto elimIt = eliminated.find(origin);
            ASSERT(elimIt != eliminated.end());
            origin = elimIt->second.origin;
        }

        GetGraph()->GetEventWriter().EventLse(inst->GetId(), inst->GetPc(), origin->GetId(), origin->GetPc(),
                                              GetEliminationCode(inst, origin));
        // Try to update savestates
        if (!GetGraph()->IsBytecodeOptimizer() && value->IsMovableObject()) {
            if (!value->IsPhi() && origin->IsMovableObject() && origin->IsLoad() && origin->IsDominate(inst)) {
                // this branch is not required, but can be faster if origin is closer to inst than value
                ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), origin, inst);
            } else {
                ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), value, inst);
            }
        }
        DeleteInstruction(inst, value);
    }
}

void Lse::ApplyHoistToCandidate(Loop *loop, Inst *alive)
{
    ASSERT(alive->IsLoad());
    COMPILER_LOG(DEBUG, LSE_OPT) << " v" << alive->GetId();
    if (alive->GetBasicBlock()->GetLoop() != loop) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "\tFailed because inst is part of a more inner loop";
        return;
    }
    if (GetGraph()->IsInstThrowable(alive)) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "\tFailed because inst is throwable";
        return;
    }
    for (const auto &input : alive->GetInputs()) {
        if (!input.GetInst()->GetBasicBlock()->IsDominate(loop->GetPreHeader())) {
            COMPILER_LOG(DEBUG, LSE_OPT) << "\tFailed because of def-use chain of inputs: " << LogInst(input.GetInst());
            return;
        }
    }
    const auto &rpo = GetGraph()->GetBlocksRPO();
    auto blockIter = std::find(rpo.rbegin(), rpo.rend(), alive->GetBasicBlock());
    ASSERT(blockIter != rpo.rend());
    auto inst = alive->GetPrev();
    while (*blockIter != loop->GetPreHeader()) {
        while (inst != nullptr) {
            if (IsHeapInvalidatingInst(inst) || (inst->IsMemory() && GetEquivClass(inst) == GetEquivClass(alive) &&
                                                 GetGraph()->CheckInstAlias(inst, alive) != NO_ALIAS)) {
                COMPILER_LOG(DEBUG, LSE_OPT) << "\tFailed because of invalidating inst:" << LogInst(inst);
                return;
            }
            inst = inst->GetPrev();
        }
        blockIter++;
        inst = (*blockIter)->GetLastInst();
    }
    alive->GetBasicBlock()->EraseInst(alive, true);
    auto lastInst = loop->GetPreHeader()->GetLastInst();
    if (lastInst != nullptr && lastInst->IsControlFlow()) {
        loop->GetPreHeader()->InsertBefore(alive, lastInst);
    } else {
        loop->GetPreHeader()->AppendInst(alive);
    }
    if (!GetGraph()->IsBytecodeOptimizer() && alive->IsMovableObject()) {
        ASSERT(!loop->IsIrreducible());
        // loop backedges will be walked inside SSB
        ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), alive, loop->GetHeader()->GetLastInst(), nullptr,
                                                  loop->GetHeader());
    }
    applied_ = true;
}

void Lse::TryToHoistLoadFromLoop(Loop *loop, HeapEqClasses *heaps, const BasicBlockHeap *eliminated)
{
    for (auto innerLoop : loop->GetInnerLoops()) {
        TryToHoistLoadFromLoop(innerLoop, heaps, eliminated);
    }

    if (loop->IsIrreducible() || loop->IsOsrLoop()) {
        return;
    }

    auto &backBbs = loop->GetBackEdges();
    beAlive_.clear();

    // Initiate alive set
    auto backBb = backBbs.begin();
    ASSERT(backBb != backBbs.end());
    for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
        for (const auto &entry : (*heaps)[eqClass].first.at(*backBb)) {
            // Do not touch Stores and eliminated ones
            if (entry.first->IsLoad() && eliminated->find(entry.first) == eliminated->end()) {
                beAlive_.insert(entry.first);
            }
        }
    }

    // Throw values not alive on other backedges
    while (++backBb != backBbs.end()) {
        auto alive = beAlive_.begin();
        while (alive != beAlive_.end()) {
            auto &heap = heaps->at(GetEquivClass(*alive)).first;
            if (heap.at(*backBb).find(*alive) == heap.at(*backBb).end()) {
                alive = beAlive_.erase(alive);
            } else {
                alive++;
            }
        }
    }

    orderedAlive_.clear();

    COMPILER_LOG(DEBUG, LSE_OPT) << "Loop #" << loop->GetId() << " has the following motion candidates:";
    for (auto alive : beAlive_) {
        orderedAlive_.insert(std::make_pair(instOrder_[alive->GetId()], alive));
    }
    for (auto alive : orderedAlive_) {
        ApplyHoistToCandidate(loop, alive.second);
    }
}

void Lse::RecordInstOrder()
{
    instOrder_.clear();
    uint32_t order = 0;
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            instOrder_[inst->GetId()] = order++;
        }
    }
}

void Lse::ProcessAllBBs(HeapEqClasses *heaps, Marker phiFixupMrk)
{
    InstVector invs(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto block : GetGraph()->GetBlocksRPO()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "Processing BB " << block->GetId();
        InitializeHeap(block, heaps);

        if (block->IsLoopHeader()) {
            MergeHeapValuesForLoop(block, heaps);
        } else {
            MergeHeapValuesForBlock(block, heaps, phiFixupMrk);
        }

        for (auto inst : block->Insts()) {
            if (IsHeapReadingInst(inst)) {
                visitor_->SetHeapAsRead(block);
            }
            if (IsHeapInvalidatingInst(inst)) {
                COMPILER_LOG(DEBUG, LSE_OPT) << LogInst(inst) << " invalidates heap";
                visitor_->InvalidateHeap(block);
            } else if (inst->IsLoad()) {
                visitor_->VisitLoad(inst);
            } else if (inst->IsStore()) {
                visitor_->VisitStore(inst, InstStoredValue(inst));
            }
            if (inst->IsIntrinsic()) {
                visitor_->VisitIntrinsic(inst, &invs);
            }
            // If we call Alias Analysis too much, we assume that this block has too many
            // instructions and we should bail in favor of performance.
            if (visitor_->GetAliasAnalysisCallCount() > Lse::AA_CALLS_LIMIT) {
                COMPILER_LOG(DEBUG, LSE_OPT) << "Exiting BB " << block->GetId() << ": too many Alias Analysis calls";
                visitor_->InvalidateHeap(block);
                break;
            }
        }
        visitor_->ClearLocalValuesFromHeap(block);
        visitor_->ResetLimits();
    }
}

bool Lse::RunImpl()
{
    if (GetGraph()->IsBytecodeOptimizer() && GetGraph()->IsDynamicMethod()) {
        COMPILER_LOG(DEBUG, LSE_OPT) << "Load-Store Elimination skipped: es bytecode optimizer";
        return false;
    }

    HeapEqClasses heaps(GetGraph()->GetLocalAllocator()->Adapter());
    for (int eqClass = Lse::EquivClass::EQ_ARRAY; eqClass != Lse::EquivClass::EQ_LAST; eqClass++) {
        std::pair<Heap, PhiCands> heapPhi(GetGraph()->GetLocalAllocator()->Adapter(),
                                          GetGraph()->GetLocalAllocator()->Adapter());
        heaps.emplace_back(heapPhi);
    }

    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<AliasAnalysis>();

    visitor_ = GetGraph()->GetLocalAllocator()->New<LseVisitor>(GetGraph(), &heaps);
    auto markerHolder = MarkerHolder(GetGraph());
    auto phiFixupMrk = markerHolder.GetMarker();

    RecordInstOrder();
    ProcessAllBBs(&heaps, phiFixupMrk);

    visitor_->FinalizeShadowedStores();
    visitor_->FinalizeLoops(GetGraph(), rpoLoops_);

    auto &eliminated = visitor_->GetEliminations();
    GetGraph()->RunPass<DominatorsTree>();
    if (hoistLoads_) {
        for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
            TryToHoistLoadFromLoop(loop, &heaps, &eliminated);
        }
    }

    DeleteInstructions(visitor_->GetEliminations());

    for (auto block : GetGraph()->GetBlocksRPO()) {
        FixupPhisInBlock(block, phiFixupMrk);
    }

    COMPILER_LOG(DEBUG, LSE_OPT) << "Load-Store Elimination complete";
    return applied_;
}
}  // namespace ark::compiler
