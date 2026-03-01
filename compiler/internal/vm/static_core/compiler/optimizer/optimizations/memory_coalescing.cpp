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

#include "compiler_logger.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/optimizations/memory_coalescing.h"

namespace ark::compiler {
/**
 * Basic analysis for variables used in loops. It works as follows:
 * 1) Identify variables that are derived from another variables and their difference (AddI, SubI supported).
 * 2) Based on previous step reveal loop variables and their iteration increment if possible.
 */
class VariableAnalysis {
public:
    struct BaseVariable {
        int64_t initial;
        int64_t step;
    };
    struct DerivedVariable {
        Inst *base;
        int64_t diff;
    };

    explicit VariableAnalysis(Graph *graph)
        : base_(graph->GetLocalAllocator()->Adapter()), derived_(graph->GetLocalAllocator()->Adapter())
    {
        for (auto block : graph->GetBlocksRPO()) {
            for (auto inst : block->AllInsts()) {
                if (GetCommonType(inst->GetType()) == DataType::INT64) {
                    AddUsers(inst);
                }
            }
        }
        for (auto loop : graph->GetRootLoop()->GetInnerLoops()) {
            if (loop->IsIrreducible()) {
                continue;
            }
            auto header = loop->GetHeader();
            for (auto phi : header->PhiInsts()) {
                constexpr auto INPUTS_COUNT = 2;
                if (phi->GetInputsCount() != INPUTS_COUNT || GetCommonType(phi->GetType()) != DataType::INT64) {
                    continue;
                }
                auto var = phi->CastToPhi();
                Inst *initial = var->GetPhiInput(var->GetPhiInputBb(0));
                Inst *update = var->GetPhiInput(var->GetPhiInputBb(1));
                if (var->GetPhiInputBb(0) != loop->GetPreHeader()) {
                    std::swap(initial, update);
                }

                if (!initial->IsConst()) {
                    continue;
                }

                if (derived_.find(update) != derived_.end()) {
                    auto initVal = static_cast<int64_t>(initial->CastToConstant()->GetIntValue());
                    base_[var] = {initVal, derived_[update].diff};
                }
            }
        }

        COMPILER_LOG(DEBUG, MEMORY_COALESCING) << "Evolution variables:";
        for (auto entry : base_) {
            COMPILER_LOG(DEBUG, MEMORY_COALESCING)
                << "v" << entry.first->GetId() << " = {" << entry.second.initial << ", " << entry.second.step << "}";
        }
        COMPILER_LOG(DEBUG, MEMORY_COALESCING) << "Loop variables:";
        for (auto entry : derived_) {
            COMPILER_LOG(DEBUG, MEMORY_COALESCING)
                << "v" << entry.first->GetId() << " = v" << entry.second.base->GetId() << " + " << entry.second.diff;
        }
    }

    DEFAULT_MOVE_SEMANTIC(VariableAnalysis);
    DEFAULT_COPY_SEMANTIC(VariableAnalysis);
    ~VariableAnalysis() = default;

    bool IsAnalyzed(Inst *inst) const
    {
        return derived_.find(inst) != derived_.end();
    }

    Inst *GetBase(Inst *inst) const
    {
        return derived_.at(inst).base;
    }

    int64_t GetInitial(Inst *inst) const
    {
        auto var = derived_.at(inst);
        return base_.at(var.base).initial + var.diff;
    }

    int64_t GetDiff(Inst *inst) const
    {
        return derived_.at(inst).diff;
    }

    int64_t GetStep(Inst *inst) const
    {
        return base_.at(derived_.at(inst).base).step;
    }

    bool IsEvoluted(Inst *inst) const
    {
        return derived_.at(inst).base->IsPhi();
    }

    bool HasKnownEvolution(Inst *inst) const
    {
        Inst *base = derived_.at(inst).base;
        return base->IsPhi() && base_.find(base) != base_.end();
    }

private:
    /// Add derived variables if we can deduce the change from INST
    void AddUsers(Inst *inst)
    {
        auto acc = 0;
        auto base = inst;
        if (derived_.find(inst) != derived_.end()) {
            acc += derived_[inst].diff;
            base = derived_[inst].base;
        } else {
            derived_[inst] = {inst, 0};
        }
        for (auto &user : inst->GetUsers()) {
            auto uinst = user.GetInst();
            ASSERT(uinst->IsPhi() || derived_.find(uinst) == derived_.end());
            switch (uinst->GetOpcode()) {
                case Opcode::AddI: {
                    auto val = static_cast<int64_t>(uinst->CastToAddI()->GetImm());
                    derived_[uinst] = {base, acc + val};
                    break;
                }
                case Opcode::SubI: {
                    auto val = static_cast<int64_t>(uinst->CastToSubI()->GetImm());
                    derived_[uinst] = {base, acc - val};
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    ArenaUnorderedMap<Inst *, struct BaseVariable> base_;
    ArenaUnorderedMap<Inst *, struct DerivedVariable> derived_;
};

/**
 * The visitor collects pairs of memory instructions that can be coalesced.
 * It operates in scope of basic block. During observation of instructions we
 * collect memory instructions in one common queue of candidates that can be merged.
 *
 * Candidate is marked as invalid in the following conditions:
 * - it has been paired already
 * - it is a store and SaveState has been met
 * - a BARRIER or CAN_TROW instruction has been met
 *
 * To pair valid array accesses:
 * - check that accesses happen on the consecutive indices of the same array
 * - find the lowest position the dominator access can be sunk
 * - find the highest position the dominatee access can be hoisted
 * - if highest position dominates lowest position the coalescing is possible
 */
class PairCreatorVisitor : public GraphVisitor {
public:
    explicit PairCreatorVisitor(Graph *graph, const AliasAnalysis &aliases, const VariableAnalysis &vars, Marker mrk,
                                bool aligned)
        : alignedOnly_(aligned),
          mrkInvalid_(mrk),
          graph_(graph),
          aliases_(aliases),
          vars_(vars),
          pairs_(graph->GetLocalAllocator()->Adapter()),
          candidates_(graph->GetLocalAllocator()->Adapter())
    {
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return graph_->GetBlocksRPO();
    }

    NO_MOVE_SEMANTIC(PairCreatorVisitor);
    NO_COPY_SEMANTIC(PairCreatorVisitor);
    ~PairCreatorVisitor() override = default;

    static void VisitLoadArray(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleArrayAccess(inst);
    }

    static void VisitStoreArray(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleArrayAccess(inst);
    }

    static void VisitLoadArrayI(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleArrayAccessI(inst);
    }

    static void VisitStoreArrayI(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleArrayAccessI(inst);
    }

    static void VisitLoadObject(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleObjectAccess(inst->CastToLoadObject());
    }

    static void VisitStoreObject(GraphVisitor *v, Inst *inst)
    {
        static_cast<PairCreatorVisitor *>(v)->HandleObjectAccess(inst->CastToStoreObject());
    }

    static bool IsNotAcceptableForStore(Inst *inst)
    {
        if (inst->GetOpcode() == Opcode::SaveState) {
            for (auto &user : inst->GetUsers()) {
                auto *ui = user.GetInst();
                if (ui->CanThrow() || ui->CanDeoptimize()) {
                    return true;
                }
            }
        }
        return inst->GetOpcode() == Opcode::SaveStateDeoptimize;
    }

    void VisitDefault(Inst *inst) override
    {
        if (inst->IsMemory()) {
            candidates_.push_back(inst);
            return;
        }
        if (inst->IsSaveState()) {
            // 1. Load & Store can be moved through SafePoint
            if (inst->GetOpcode() == Opcode::SafePoint) {
                return;
            }
            // 2. Load & Store can't be moved through SaveStateOsr
            if (inst->GetOpcode() == Opcode::SaveStateOsr) {
                candidates_.clear();
                return;
            }
            // 3. Load can be moved through SaveState and SaveStateDeoptimize
            // 4. Store can't be moved through SaveStateDeoptimize and SaveState with Users that are IsCheck or
            //    CanDeoptimize. It is checked in IsNotAcceptableForStore
            if (IsNotAcceptableForStore(inst)) {
                InvalidateStores();
                return;
            }
        }
        if (inst->IsBarrier()) {
            candidates_.clear();
            return;
        }
        if (inst->CanThrow()) {
            InvalidateStores();
            return;
        }
    }

    void Reset()
    {
        candidates_.clear();
    }

    ArenaUnorderedMap<Inst *, MemoryCoalescing::CoalescedPair> &GetPairs()
    {
        return pairs_;
    }

#include "optimizer/ir/visitor.inc"
private:
    void InvalidateStores()
    {
        for (auto cand : candidates_) {
            if (cand->IsStore()) {
                cand->SetMarker(mrkInvalid_);
            }
        }
    }

    static bool IsPairInst(Inst *inst)
    {
        switch (inst->GetOpcode()) {
            case Opcode::LoadArrayPair:
            case Opcode::LoadArrayPairI:
            case Opcode::StoreArrayPair:
            case Opcode::StoreArrayPairI:
            case Opcode::LoadObjectPair:
            case Opcode::StoreObjectPair:
                return true;
            default:
                return false;
        }
    }

    /**
     * Return the highest instructions that INST can be inserted after (in scope of basic block).
     * Consider aliased memory accesses and volatile operations. CHECK_CFG enables the check of INST inputs
     * as well.
     */
    Inst *FindUpperInsertAfter(Inst *inst, Inst *bound, bool checkCfg)
    {
        ASSERT(bound != nullptr);
        auto upperAfter = bound;
        // We do not move higher than bound
        auto lowerInput = upperAfter;
        if (checkCfg) {
            // Update upper bound according to def-use chains
            for (auto &inputItem : inst->GetInputs()) {
                auto input = inputItem.GetInst();
                if (input->GetBasicBlock() == inst->GetBasicBlock() && lowerInput->IsPrecedingInSameBlock(input)) {
                    ASSERT(input->IsPrecedingInSameBlock(inst));
                    lowerInput = input;
                }
            }
            upperAfter = lowerInput;
        }

        auto boundIt = std::find(candidates_.rbegin(), candidates_.rend(), bound);
        ASSERT(boundIt != candidates_.rend());
        for (auto it = candidates_.rbegin(); it != boundIt; it++) {
            auto cand = *it;
            if (checkCfg && cand->IsPrecedingInSameBlock(lowerInput)) {
                return lowerInput;
            }
            // Can't hoist load over aliased store and store over aliased memory instructions
            if (inst->IsStore() || cand->IsStore()) {
                auto checkInst = cand;
                if (IsPairInst(cand)) {
                    // We have already checked the second inst. We now want to check the first one
                    // for alias.
                    auto pair = pairs_[cand];
                    checkInst = pair.first->IsPrecedingInSameBlock(pair.second) ? pair.first : pair.second;
                }
                if (aliases_.CheckInstAlias(inst, checkInst) != NO_ALIAS) {
                    return cand;
                }
            }
            // Can't hoist over volatile load
            if (cand->IsLoad() && IsVolatileMemInst(cand)) {
                return cand;
            }
        }
        return upperAfter;
    }

    /**
     * Return the lowest instructions that INST can be inserted after (in scope of basic block).
     * Consider aliased memory accesses and volatile operations. CHECK_CFG enables the check of INST users
     * as well.
     */
    Inst *FindLowerInsertAfter(Inst *inst, Inst *bound, bool checkCfg = true)
    {
        ASSERT(bound != nullptr);
        auto lowerAfter = bound->GetPrev();
        // We do not move lower than bound
        auto upperUser = lowerAfter;
        ASSERT(upperUser != nullptr);
        if (checkCfg) {
            // Update lower bound according to def-use chains
            for (auto &userItem : inst->GetUsers()) {
                auto user = userItem.GetInst();
                if (!user->IsPhi() && user->GetBasicBlock() == inst->GetBasicBlock() &&
                    user->IsPrecedingInSameBlock(upperUser)) {
                    ASSERT(inst->IsPrecedingInSameBlock(user));
                    upperUser = user->GetPrev();
                    ASSERT(upperUser != nullptr);
                }
            }
            lowerAfter = upperUser;
        }

        auto instIt = std::find(candidates_.begin(), candidates_.end(), inst);
        ASSERT(instIt != candidates_.end());
        for (auto it = instIt + 1; it != candidates_.end(); it++) {
            auto cand = *it;
            if (checkCfg && upperUser->IsPrecedingInSameBlock(cand)) {
                return upperUser;
            }
            // Can't lower load over aliased store and store over aliased memory instructions
            if (inst->IsStore() || cand->IsStore()) {
                auto checkInst = cand;
                if (IsPairInst(cand)) {
                    // We have already checked the first inst. We now want to check the second one
                    // for alias.
                    auto pair = pairs_[cand];
                    checkInst = pair.first->IsPrecedingInSameBlock(pair.second) ? pair.second : pair.first;
                }
                if (aliases_.CheckInstAlias(inst, checkInst) != NO_ALIAS) {
                    ASSERT(cand->GetPrev() != nullptr);
                    return cand->GetPrev();
                }
            }
            // Can't lower over volatile store
            if (cand->IsStore() && IsVolatileMemInst(cand)) {
                ASSERT(cand->GetPrev() != nullptr);
                return cand->GetPrev();
            }
        }
        return lowerAfter;
    }

    /// Add a pair if a difference between indices equals to one. The first in pair is with lower index.
    bool TryAddCoalescedPair(Inst *inst, int64_t instIdx, Inst *cand, int64_t candIdx)
    {
        Inst *first = nullptr;
        Inst *second = nullptr;
        Inst *insertAfter = nullptr;
        if (instIdx == candIdx - 1) {
            first = inst;
            second = cand;
        } else if (candIdx == instIdx - 1) {
            first = cand;
            second = inst;
        } else {
            return false;
        }

        ASSERT(inst->IsMemory() && cand->IsMemory());
        ASSERT(inst->GetOpcode() == cand->GetOpcode());
        ASSERT(inst != cand && cand->IsPrecedingInSameBlock(inst));
        Inst *candLowerAfter = nullptr;
        Inst *instUpperAfter = nullptr;
        if (first->IsLoad()) {
            // Consider dominance of load users
            bool checkCfg = true;
            candLowerAfter = FindLowerInsertAfter(cand, inst, checkCfg);
            // Do not need index if v0[v1] preceeds v0[v1 + 1] because v1 + 1 is not used in paired load.
            checkCfg = second->IsPrecedingInSameBlock(first);
            instUpperAfter = FindUpperInsertAfter(inst, cand, checkCfg);
        } else if (first->IsStore()) {
            // Store instructions do not have users. Don't check them
            bool checkCfg = false;
            candLowerAfter = FindLowerInsertAfter(cand, inst, checkCfg);
            // Should check that stored value is ready
            checkCfg = true;
            instUpperAfter = FindUpperInsertAfter(inst, cand, checkCfg);
        } else {
            UNREACHABLE();
        }

        // No intersection in reordering ranges
        if (!instUpperAfter->IsPrecedingInSameBlock(candLowerAfter)) {
            return false;
        }
        if (cand->IsPrecedingInSameBlock(instUpperAfter)) {
            insertAfter = instUpperAfter;
        } else {
            insertAfter = cand;
        }

        first->SetMarker(mrkInvalid_);
        second->SetMarker(mrkInvalid_);
        InsertPair(first, second, insertAfter);
        return true;
    }

    void HandleArrayAccessI(Inst *inst)
    {
        Inst *obj = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
        uint64_t idx = GetInstImm(inst);
        if (!MemoryCoalescing::AcceptedType(inst->GetType())) {
            candidates_.push_back(inst);
            return;
        }
        /* Last candidates more likely to be coalesced */
        for (auto iter = candidates_.rbegin(); iter != candidates_.rend(); iter++) {
            auto cand = *iter;
            /* Skip not interesting candidates */
            if (cand->IsMarked(mrkInvalid_) || cand->GetOpcode() != inst->GetOpcode()) {
                continue;
            }

            Inst *candObj = cand->GetDataFlowInput(cand->GetInput(0).GetInst());
            /* Array objects must alias each other */
            if (aliases_.CheckRefAlias(obj, candObj) != MUST_ALIAS) {
                continue;
            }
            /* The difference between indices should be equal to one */
            uint64_t candIdx = GetInstImm(cand);
            /* To keep alignment the lowest index should be even */
            if (alignedOnly_ && ((idx < candIdx && (idx & 1U) != 0) || (candIdx < idx && (candIdx & 1U) != 0))) {
                continue;
            }
            if (TryAddCoalescedPair(inst, idx, cand, candIdx)) {
                break;
            }
        }

        candidates_.push_back(inst);
    }

    bool HandleKnownEvolutionArrayAccessVar(Inst *idx, Inst *candIdx, int64_t idxInitial, int64_t candInitial)
    {
        /* Accesses inside loop */
        auto idxStep = vars_.GetStep(idx);
        auto candStep = vars_.GetStep(candIdx);
        /* Indices should be incremented at the same value and their
            increment should be even to hold alignment */
        if (idxStep != candStep) {
            return false;
        }
        /* To keep alignment we need to have even step and even lowest initial */
        constexpr auto IMM_2 = 2;
        // NOLINTBEGIN(readability-simplify-boolean-expr)
        if (alignedOnly_ && idxStep % IMM_2 != 0 &&
            ((idxInitial < candInitial && idxInitial % IMM_2 != 0) ||
             (candInitial < idxInitial && candInitial % IMM_2 != 0))) {
            return false;
        }
        return true;
        // NOLINTEND(readability-simplify-boolean-expr)
    }

    void HandleArrayAccess(Inst *inst)
    {
        Inst *obj = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
        Inst *idx = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
        if (!vars_.IsAnalyzed(idx) || !MemoryCoalescing::AcceptedType(inst->GetType())) {
            candidates_.push_back(inst);
            return;
        }
        /* Last candidates more likely to be coalesced */
        for (auto iter = candidates_.rbegin(); iter != candidates_.rend(); iter++) {
            auto cand = *iter;
            /* Skip not interesting candidates */
            if (cand->IsMarked(mrkInvalid_) || cand->GetOpcode() != inst->GetOpcode()) {
                continue;
            }

            Inst *candObj = cand->GetDataFlowInput(cand->GetInput(0).GetInst());
            auto candIdx = cand->GetDataFlowInput(cand->GetInput(1).GetInst());
            /* We need to have info about candidate's index and array objects must alias each other */
            if (!vars_.IsAnalyzed(candIdx) || aliases_.CheckRefAlias(obj, candObj) != MUST_ALIAS) {
                continue;
            }
            if (vars_.HasKnownEvolution(idx) && vars_.HasKnownEvolution(candIdx)) {
                auto idxInitial = vars_.GetInitial(idx);
                auto candInitial = vars_.GetInitial(candIdx);
                if (!HandleKnownEvolutionArrayAccessVar(idx, candIdx, idxInitial, candInitial)) {
                    continue;
                }
                if (TryAddCoalescedPair(inst, idxInitial, cand, candInitial)) {
                    break;
                }
            } else if (!alignedOnly_ && !vars_.HasKnownEvolution(idx) && !vars_.HasKnownEvolution(candIdx)) {
                /* Accesses outside loop */
                if (vars_.GetBase(idx) != vars_.GetBase(candIdx)) {
                    continue;
                }
                if (TryAddCoalescedPair(inst, vars_.GetDiff(idx), cand, vars_.GetDiff(candIdx))) {
                    break;
                }
            }
        }

        candidates_.push_back(inst);
    }

    template <typename T>
    void CheckForObjectCandidates(T *inst, uint8_t fieldSize, size_t fieldOffset)
    {
        Inst *obj = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
        /* Last candidates more likely to be coalesced */
        for (auto iter = candidates_.rbegin(); iter != candidates_.rend(); iter++) {
            auto cand = *iter;
            if (cand->GetOpcode() != Opcode::LoadObject && cand->GetOpcode() != Opcode::StoreObject) {
                continue;
            }
            Inst *candObj = cand->GetDataFlowInput(cand->GetInput(0).GetInst());
            if (aliases_.CheckRefAlias(obj, candObj) != MUST_ALIAS) {
                if (aliases_.CheckInstAlias(inst, cand) == MAY_ALIAS) {
                    cand->SetMarker(mrkInvalid_);
                    inst->SetMarker(mrkInvalid_);
                    break;
                }
                continue;
            }
            if (cand->IsMarked(mrkInvalid_)) {
                continue;
            }
            if (cand->GetOpcode() != inst->GetOpcode() || cand->GetType() != inst->GetType()) {
                continue;
            }
            size_t candFieldOffset;
            if constexpr (std::is_same_v<T, LoadObjectInst>) {
                auto candLoadObj = cand->CastToLoadObject();
                candFieldOffset = GetObjectOffset(graph_, candLoadObj->GetObjectType(), candLoadObj->GetObjField(),
                                                  candLoadObj->GetTypeId());
            } else {
                auto candStoreObj = cand->CastToStoreObject();
                candFieldOffset = GetObjectOffset(graph_, candStoreObj->GetObjectType(), candStoreObj->GetObjField(),
                                                  candStoreObj->GetTypeId());
            }
            auto candFieldSize = GetTypeByteSize(cand->GetType(), graph_->GetArch());
            if ((fieldOffset + fieldSize == candFieldOffset && TryAddCoalescedPair(inst, 0, cand, 1)) ||
                (candFieldOffset + candFieldSize == fieldOffset && TryAddCoalescedPair(inst, 1, cand, 0))) {
                break;
            }
        }
    }

    template <typename T>
    void HandleObjectAccess(T *inst)
    {
        ObjectType objType = inst->GetObjectType();
        auto fieldSize = GetTypeByteSize(inst->GetType(), graph_->GetArch());
        size_t fieldOffset = GetObjectOffset(graph_, objType, inst->GetObjField(), inst->GetTypeId());
        bool isVolatile = inst->GetVolatile();
        if (isVolatile) {
            inst->SetMarker(mrkInvalid_);
        }
        if (!MemoryCoalescing::AcceptedType(inst->GetType()) || objType != MEM_OBJECT || isVolatile) {
            candidates_.push_back(inst);
            return;
        }
        CheckForObjectCandidates(inst, fieldSize, fieldOffset);
        candidates_.push_back(inst);
    }

    void InsertPair(Inst *first, Inst *second, Inst *insertAfter)
    {
        COMPILER_LOG(DEBUG, MEMORY_COALESCING)
            << "Access that may be coalesced: v" << first->GetId() << " v" << second->GetId();

        ASSERT(first->GetType() == second->GetType());
        Inst *paired = nullptr;
        switch (first->GetOpcode()) {
            case Opcode::LoadArray:
                paired = ReplaceLoadArray(first, second, insertAfter);
                break;
            case Opcode::LoadArrayI:
                paired = ReplaceLoadArrayI(first, second, insertAfter);
                break;
            case Opcode::StoreArray:
                paired = ReplaceStoreArray(first, second, insertAfter);
                break;
            case Opcode::StoreArrayI:
                paired = ReplaceStoreArrayI(first, second, insertAfter);
                break;
            case Opcode::LoadObject:
                paired = ReplaceLoadObject(first, second, insertAfter);
                break;
            case Opcode::StoreObject:
                paired = ReplaceStoreObject(first, second, insertAfter);
                break;
            default:
                UNREACHABLE();
        }

        ASSERT(paired != nullptr);
        COMPILER_LOG(DEBUG, MEMORY_COALESCING) << "Coalescing of {v" << first->GetId() << " v" << second->GetId()
                                               << "} by " << paired->GetId() << " is successful";
        graph_->GetEventWriter().EventMemoryCoalescing(first->GetId(), first->GetPc(), second->GetId(), second->GetPc(),
                                                       paired->GetId(), paired->IsStore() ? "Store" : "Load");

        pairs_[paired] = {first, second};
        paired->SetMarker(mrkInvalid_);
        candidates_.insert(std::find_if(candidates_.rbegin(), candidates_.rend(),
                                        [paired](auto x) { return x->IsPrecedingInSameBlock(paired); })
                               .base(),
                           paired);
    }

    Inst *ReplaceLoadArray(Inst *first, Inst *second, Inst *insertAfter)
    {
        ASSERT(first->GetOpcode() == Opcode::LoadArray);
        ASSERT(second->GetOpcode() == Opcode::LoadArray);

        auto pload = graph_->CreateInstLoadArrayPair(first->GetType(), INVALID_PC, first->GetInput(0).GetInst(),
                                                     first->GetInput(1).GetInst());
        pload->CastToLoadArrayPair()->SetNeedBarrier(first->CastToLoadArray()->GetNeedBarrier() ||
                                                     second->CastToLoadArray()->GetNeedBarrier());
        insertAfter->InsertAfter(pload);
        if (first->CanThrow() || second->CanThrow()) {
            pload->SetFlag(compiler::inst_flags::CAN_THROW);
        }
        MemoryCoalescing::RemoveAddI(pload);
        return pload;
    }

    Inst *ReplaceLoadObject(Inst *first, Inst *second, Inst *insertAfter)
    {
        ASSERT(first->GetOpcode() == Opcode::LoadObject);
        ASSERT(second->GetOpcode() == Opcode::LoadObject);
        ASSERT(!first->CastToLoadObject()->GetVolatile());
        ASSERT(!second->CastToLoadObject()->GetVolatile());

        auto pload = graph_->CreateInstLoadObjectPair(first->GetType(), INVALID_PC);
        pload->SetInput(InputOrd::INP0, first->GetInput(InputOrd::INP0).GetInst());
        pload->SetType(first->GetType());
        pload->SetTypeId0(first->CastToLoadObject()->GetTypeId());
        pload->SetTypeId1(second->CastToLoadObject()->GetTypeId());
        pload->SetObjField0(first->CastToLoadObject()->GetObjField());
        pload->SetObjField1(second->CastToLoadObject()->GetObjField());

        pload->CastToLoadObjectPair()->SetNeedBarrier(first->CastToLoadObject()->GetNeedBarrier() ||
                                                      second->CastToLoadObject()->GetNeedBarrier());
        if (first->CanThrow() || second->CanThrow()) {
            pload->SetFlag(compiler::inst_flags::CAN_THROW);
        }
        insertAfter->InsertAfter(pload);

        return pload;
    }

    Inst *ReplaceLoadArrayI(Inst *first, Inst *second, Inst *insertAfter)
    {
        ASSERT(first->GetOpcode() == Opcode::LoadArrayI);
        ASSERT(second->GetOpcode() == Opcode::LoadArrayI);

        auto pload = graph_->CreateInstLoadArrayPairI(first->GetType(), INVALID_PC, first->GetInput(0).GetInst(),
                                                      first->CastToLoadArrayI()->GetImm());
        pload->CastToLoadArrayPairI()->SetNeedBarrier(first->CastToLoadArrayI()->GetNeedBarrier() ||
                                                      second->CastToLoadArrayI()->GetNeedBarrier());
        insertAfter->InsertAfter(pload);
        if (first->CanThrow() || second->CanThrow()) {
            pload->SetFlag(compiler::inst_flags::CAN_THROW);
        }

        return pload;
    }

    Inst *ReplaceStoreArray(Inst *first, Inst *second, Inst *insertAfter)
    {
        ASSERT(first->GetOpcode() == Opcode::StoreArray);
        ASSERT(second->GetOpcode() == Opcode::StoreArray);

        auto pstore = graph_->CreateInstStoreArrayPair(
            first->GetType(), INVALID_PC,
            std::array<Inst *, 4U> {first->GetInput(0).GetInst(), first->CastToStoreArray()->GetIndex(),
                                    first->CastToStoreArray()->GetStoredValue(),
                                    second->CastToStoreArray()->GetStoredValue()});
        pstore->CastToStoreArrayPair()->SetNeedBarrier(first->CastToStoreArray()->GetNeedBarrier() ||
                                                       second->CastToStoreArray()->GetNeedBarrier());
        insertAfter->InsertAfter(pstore);
        if (first->CanThrow() || second->CanThrow()) {
            pstore->SetFlag(compiler::inst_flags::CAN_THROW);
        }
        MemoryCoalescing::RemoveAddI(pstore);
        return pstore;
    }

    Inst *ReplaceStoreObject(Inst *first, Inst *second, Inst *insertAfter)
    {
        ASSERT(first->GetOpcode() == Opcode::StoreObject);
        ASSERT(second->GetOpcode() == Opcode::StoreObject);
        ASSERT(!first->CastToStoreObject()->GetVolatile());
        ASSERT(!second->CastToStoreObject()->GetVolatile());

        auto pstore = graph_->CreateInstStoreObjectPair();
        pstore->SetType(first->GetType());
        pstore->SetTypeId0(first->CastToStoreObject()->GetTypeId());
        pstore->SetTypeId1(second->CastToStoreObject()->GetTypeId());
        pstore->SetInput(InputOrd::INP0, first->GetInput(InputOrd::INP0).GetInst());
        pstore->SetInput(InputOrd::INP1, first->GetInput(InputOrd::INP1).GetInst());
        pstore->SetInput(InputOrd::INP2, second->GetInput(InputOrd::INP1).GetInst());
        pstore->CastToStoreObjectPair()->SetObjField0(first->CastToStoreObject()->GetObjField());
        pstore->CastToStoreObjectPair()->SetObjField1(second->CastToStoreObject()->GetObjField());

        pstore->CastToStoreObjectPair()->SetNeedBarrier(first->CastToStoreObject()->GetNeedBarrier() ||
                                                        second->CastToStoreObject()->GetNeedBarrier());
        if (first->CanThrow() || second->CanThrow()) {
            pstore->SetFlag(compiler::inst_flags::CAN_THROW);
        }
        insertAfter->InsertAfter(pstore);

        return pstore;
    }

    Inst *ReplaceStoreArrayI(Inst *first, Inst *second, Inst *insertAfter)
    {
        ASSERT(first->GetOpcode() == Opcode::StoreArrayI);
        ASSERT(second->GetOpcode() == Opcode::StoreArrayI);

        auto pstore = graph_->CreateInstStoreArrayPairI(
            first->GetType(), INVALID_PC, first->GetInput(0).GetInst(), first->CastToStoreArrayI()->GetStoredValue(),
            second->CastToStoreArrayI()->GetStoredValue(), first->CastToStoreArrayI()->GetImm());
        pstore->CastToStoreArrayPairI()->SetNeedBarrier(first->CastToStoreArrayI()->GetNeedBarrier() ||
                                                        second->CastToStoreArrayI()->GetNeedBarrier());
        insertAfter->InsertAfter(pstore);
        if (first->CanThrow() || second->CanThrow()) {
            pstore->SetFlag(compiler::inst_flags::CAN_THROW);
        }

        return pstore;
    }

    uint64_t GetInstImm(Inst *inst)
    {
        switch (inst->GetOpcode()) {
            case Opcode::LoadArrayI:
                return inst->CastToLoadArrayI()->GetImm();
            case Opcode::StoreArrayI:
                return inst->CastToStoreArrayI()->GetImm();
            default:
                UNREACHABLE();
        }
    }

private:
    bool alignedOnly_;
    Marker mrkInvalid_;
    Graph *graph_ {nullptr};
    const AliasAnalysis &aliases_;
    const VariableAnalysis &vars_;
    ArenaUnorderedMap<Inst *, MemoryCoalescing::CoalescedPair> pairs_;
    InstVector candidates_;
};

static void ReplaceLoadByPair(Inst *load, Inst *pairedLoad, int32_t dstIdx)
{
    auto graph = pairedLoad->GetBasicBlock()->GetGraph();
    auto pairGetter = graph->CreateInstLoadPairPart(load->GetType(), INVALID_PC, pairedLoad, dstIdx);
    load->ReplaceUsers(pairGetter);
    pairedLoad->InsertAfter(pairGetter);
}

void MemoryCoalescing::RemoveAddI(Inst *inst)
{
    auto opcode = inst->GetOpcode();
    ASSERT(opcode == Opcode::LoadArrayPair || opcode == Opcode::StoreArrayPair);
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->GetOpcode() == Opcode::AddI) {
        uint64_t imm = input1->CastToAddI()->GetImm();
        if (opcode == Opcode::LoadArrayPair) {
            inst->CastToLoadArrayPair()->SetImm(imm);
        } else if (opcode == Opcode::StoreArrayPair) {
            inst->CastToStoreArrayPair()->SetImm(imm);
        }
        inst->SetInput(1, input1->GetInput(0).GetInst());
    }
}

/**
 * This optimization coalesces two loads (stores) that read (write) values from (to) the consecutive memory into
 * a single operation.
 *
 * 1) If we have two memory instruction that can be coalesced then we are trying to find a position for
 *    coalesced operation. If it is possible, the memory operations are coalesced and skipped otherwise.
 * 2) The instruction of Aarch64 requires memory address alignment. For arrays
 *    it means we can coalesce only accesses that starts from even index.
 * 3) The implemented coalescing for arrays supposes there is no volatile array element accesses.
 */
bool MemoryCoalescing::RunImpl()
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        COMPILER_LOG(INFO, MEMORY_COALESCING) << "Skipping Memory Coalescing for unsupported architecture";
        return false;
    }
    COMPILER_LOG(DEBUG, MEMORY_COALESCING) << "Memory Coalescing running";
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<AliasAnalysis>();

    VariableAnalysis variables(GetGraph());
    auto &aliases = GetGraph()->GetValidAnalysis<AliasAnalysis>();
    Marker mrk = GetGraph()->NewMarker();
    PairCreatorVisitor collector(GetGraph(), aliases, variables, mrk, alignedOnly_);
    for (auto block : GetGraph()->GetBlocksRPO()) {
        collector.VisitBlock(block);
        collector.Reset();
    }
    GetGraph()->EraseMarker(mrk);
    for (auto pair : collector.GetPairs()) {
        auto bb = pair.first->GetBasicBlock();
        if (pair.first->IsLoad()) {
            ReplaceLoadByPair(pair.second.second, pair.first, 1);
            ReplaceLoadByPair(pair.second.first, pair.first, 0);
        }
        bb->RemoveInst(pair.second.first);
        bb->RemoveInst(pair.second.second);
    }

    if (!collector.GetPairs().empty()) {
        SaveStateBridgesBuilder ssb;
        for (auto bb : GetGraph()->GetBlocksRPO()) {
            if (!bb->IsEmpty() && !bb->IsStartBlock()) {
                ssb.FixSaveStatesInBB(bb);
            }
        }
    }
    COMPILER_LOG(DEBUG, MEMORY_COALESCING) << "Memory Coalescing completed";
    return !collector.GetPairs().empty();
}
}  // namespace ark::compiler
