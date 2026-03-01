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
#ifndef PHI_RESOLVER_H
#define PHI_RESOLVER_H

#include "inst_builder.h"
#include "compiler_logger.h"

namespace ark::compiler {
/**
 * Resolve phi instructions in the given graph. Resolving phi is:
 *  - remove phi if it has only SafePoint in users
 *  - if phi has no type, then set type determined from its inputs
 */
class PhiResolver {
public:
    explicit PhiResolver(Graph *graph)
        : graph_(graph),
          realInputs_(graph->GetLocalAllocator()->Adapter()),
          phiUsers_(graph->GetLocalAllocator()->Adapter()),
          realUsers_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(PhiResolver);
    NO_COPY_SEMANTIC(PhiResolver);
    ~PhiResolver() = default;

    void Run()
    {
        MarkerHolder markerHolder {graph_};
        hasRealUserMarker_ = markerHolder.GetMarker();
        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->AllInstsSafe()) {
                if (!inst->IsPhi() && !inst->IsCatchPhi()) {
                    continue;
                }
                MarkPhiWithRealUsers(inst);
            }
        }

        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->AllInstsSafe()) {
                if (!inst->IsPhi() && !inst->IsCatchPhi()) {
                    TryRemoveFromSaveStates(inst);
                    continue;
                }
                CheckPhiInputs(inst);
                // Do not try to remove typed phi with users, otherwise e.g. could be deleted `phi(obj, phi)` from the
                // loop-header and `obj` will be missed in the SaveStates inside the loop.
                // Or environment could be  deleted from SaveState.
                if ((inst->IsPhi() || (inst->GetType() == DataType::ANY && inst->IsCatchPhi())) && inst->HasType() &&
                    !inst->GetUsers().Empty()) {
                    TryRemoveFromSaveStates(inst);
                    continue;
                }

                if (!inst->IsMarked(hasRealUserMarker_)) {
                    // inst has only SaveState users
                    RemovePhiInst(inst);
                } else if (!inst->HasType()) {
                    SetTypeByInputs(inst);
                    ASSERT(inst->HasType() || (inst->IsCatchPhi() && !inst->CastToCatchPhi()->IsAcc()));
                }
            }
        }

        RemoveDeadPhi();
    }

private:
    void CleanUp()
    {
        phiUsers_.clear();
        realUsers_.clear();
        hasSaveStateInstOnly_ = true;
        marker_ = graph_->NewMarker();
    }
    static void SetTypeByInputs(Inst *inst)
    {
        if (inst->IsCatchPhi() && inst->CastToCatchPhi()->IsAcc()) {
            inst->SetType(DataType::REFERENCE);
            return;
        }
        for (auto input : inst->GetInputs()) {
            auto inputType = input.GetInst()->GetType();
            if (inputType != DataType::NO_TYPE) {
                inst->SetType(inputType);
                break;
            }
        }
    }
    void FindUsersRec(Inst *inst)
    {
        for (auto &user : inst->GetUsers()) {
            if (user.GetInst()->SetMarker(marker_)) {
                continue;
            }
            if (user.GetInst()->IsPhi() || user.GetInst()->IsCatchPhi()) {
                phiUsers_.push_back(user.GetInst());
                FindUsersRec(user.GetInst());
            } else {
                if (!user.GetInst()->IsSaveState()) {
                    hasSaveStateInstOnly_ = false;
                    break;
                }
                realUsers_.push_back(user.GetInst());
            }
        }
    }

    void FindInputsRec(Inst *inst)
    {
        ASSERT(inst->IsPhi() || inst->IsCatchPhi());
        // We can't set real type if there aren't inputs from Phi/CathPhi
        // We add the Phi/CathPhi in the list and return false from CheckPhiInputs
        if (inst->GetInputs().Empty()) {
            realInputs_.push_back(inst);
            return;
        }
        for (auto &input : inst->GetInputs()) {
            auto inputInst = input.GetInst();
            if (inputInst->SetMarker(marker_)) {
                continue;
            }
            if (inputInst->IsPhi() || inputInst->IsCatchPhi()) {
                if (inputInst->GetType() != DataType::NO_TYPE) {
                    realInputs_.push_back(inputInst);
                    continue;
                }
                FindInputsRec(inputInst);
            } else {
                realInputs_.push_back(inputInst);
            }
        }
    }

    bool CheckPhiRealInputs(Inst *phiInst)
    {
        DataType::Type type = DataType::NO_TYPE;
        realInputs_.clear();
        marker_ = graph_->NewMarker();
        phiInst->SetMarker(marker_);
        FindInputsRec(phiInst);
        graph_->EraseMarker(marker_);

        bool hasConstantInput = false;
        for (auto inputInst : realInputs_) {
            auto inputType = inputInst->GetType();
            if (inputType == DataType::NO_TYPE) {
                return false;
            }
            if (inputInst->IsConst() && inputType == DataType::INT64) {
                if (type != DataType::NO_TYPE && DataType::GetCommonType(type) != DataType::INT64) {
                    return false;
                }
                hasConstantInput = true;
                continue;
            }
            if (type == DataType::NO_TYPE) {
                if (hasConstantInput && DataType::GetCommonType(inputType) != DataType::INT64) {
                    return false;
                }
                type = inputType;
            } else if (type != inputType) {
                return false;
            }
        }

        if (type == DataType::NO_TYPE) {
            // Do not remove phi with constants-only inputs.
            if (!hasConstantInput) {
                return false;
            }
            type = DataType::INT64;
        }
        phiInst->SetType(type);
        return true;
    }

    // Returns false if block with input instruction doesn't dominate the predecessor of the  PHI block
    bool CheckPhiInputs(Inst *phiInst)
    {
        ASSERT(phiInst->IsPhi() || phiInst->IsCatchPhi());
        if (phiInst->HasType()) {
            return true;
        }
        if (phiInst->IsPhi()) {
            if (phiInst->GetInputsCount() != phiInst->GetBasicBlock()->GetPredsBlocks().size()) {
                return false;
            }
            for (size_t index = 0; index < phiInst->GetInputsCount(); ++index) {
                auto pred = phiInst->GetBasicBlock()->GetPredBlockByIndex(index);
                auto inputBb = phiInst->GetInput(index).GetInst()->GetBasicBlock();
                if (!inputBb->IsDominate(pred)) {
                    return false;
                }
            }
        }
        return CheckPhiRealInputs(phiInst);
    }

    void MarkHasRealUserRec(Inst *inst)
    {
        if (inst->SetMarker(hasRealUserMarker_)) {
            return;
        }
        for (auto &input : inst->GetInputs()) {
            auto inputInst = input.GetInst();
            if (inputInst->IsPhi() || inputInst->IsCatchPhi()) {
                MarkHasRealUserRec(inputInst);
            }
        }
    }

    void TryRemoveFromSaveStates(Inst *inst)
    {
        if (g_options.IsCompilerNonOptimizing()) {
            return;
        }
        MarkerHolder markerHolder {graph_};
        marker_ = markerHolder.GetMarker();
        for (auto &user : inst->GetUsers()) {
            auto userInst = user.GetInst();
            if (userInst->IsPhi() || userInst->IsCatchPhi()) {
                if (userInst->IsMarked(hasRealUserMarker_)) {
                    return;
                }
                continue;
            }
            if (userInst->IsSaveState()) {
                continue;
            }
            MarkInstsOnPaths(userInst->GetBasicBlock(), inst, userInst);
        }
        for (auto userIt = inst->GetUsers().begin(); userIt != inst->GetUsers().end();) {
            auto userInst = userIt->GetInst();
            if (!userInst->IsSaveState() || userInst->GetBasicBlock()->IsMarked(marker_) ||
                userInst->IsMarked(marker_) || userIt->GetVirtualRegister().IsEnv()) {
                ++userIt;
            } else {
                userInst->RemoveInput(userIt->GetIndex());
                userIt = inst->GetUsers().begin();
            }
        }
    }

    void MarkInstsOnPaths(BasicBlock *block, Inst *object, Inst *startFrom)
    {
        if (block->IsMarked(marker_)) {
            return;
        }
        if (startFrom != nullptr) {
            auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, startFrom);
            for (; it != block->AllInstsSafeReverse().end(); ++it) {
                auto inst = *it;
                if (inst->SetMarker(marker_) || inst == object) {
                    return;
                }
            }
        } else {
            block->SetMarker(marker_);
        }
        for (auto pred : block->GetPredsBlocks()) {
            MarkInstsOnPaths(pred, object, nullptr);
        }
    }

    // Remove virtual registers of SafePoint instructions which input phis to be removed.
    void RemovePhiInst(Inst *inst)
    {
        ASSERT(inst->IsPhi() || inst->IsCatchPhi());
        CleanUp();
        inst->SetMarker(marker_);
        FindUsersRec(inst);
        for (auto user : realUsers_) {
            ASSERT(user->IsSaveState());
            auto saveState = static_cast<SaveStateInst *>(user);
            size_t idx = 0;
            size_t inputsCount = saveState->GetInputsCount();
            while (idx < inputsCount) {
                auto inputInst = saveState->GetInput(idx).GetInst();
                if (inputInst->IsMarked(marker_)) {
                    // env values should not be marked for removal
                    ASSERT(!saveState->GetVirtualRegister(idx).IsEnv());
                    saveState->RemoveInput(idx);
                    inputsCount--;
                } else {
                    idx++;
                }
            }
        }
        // Phi has only SafePoint in users, we can remove this phi and all phi in collected list.
        inst->RemoveUsers<true>();
        inst->GetBasicBlock()->RemoveInst(inst);
        for (auto phi : phiUsers_) {
            phi->RemoveUsers<true>();
            phi->GetBasicBlock()->RemoveInst(phi);
        }
        graph_->EraseMarker(marker_);
    }

    void MarkPhiWithRealUsers(Inst *inst)
    {
        for (auto &user : inst->GetUsers()) {
            auto userInst = user.GetInst();
            auto canRemoveUser = userInst->IsPhi() || userInst->IsCatchPhi() ||
                                 (userInst->IsSaveState() && !user.GetVirtualRegister().IsEnv());
            if (!canRemoveUser) {
                // inst certainly cannot be removed from user's inputs
                MarkHasRealUserRec(inst);
                break;
            }
        }
    }

    void ReplaceDeadPhiUsers(Inst *inst, BasicBlock *bb)
    {
        if (inst->GetInputsCount() != 0) {
            auto inputInst = inst->GetInput(0).GetInst();
#ifndef NDEBUG
            if (!inst->GetUsers().Empty()) {
                for ([[maybe_unused]] auto &input : inst->GetInputs()) {
                    ASSERT(input.GetInst() == inputInst);
                }
            }
#endif
            inst->ReplaceUsers(inputInst);
        }
        bb->RemoveInst(inst);
    }

    void RemoveDeadPhi()
    {
        // If any input of phi instruction is not defined then we assume that phi is dead.
        // We move users to the input and remove the PHI
        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->PhiInstsSafe()) {
                // Skip catch phi
                if (!inst->IsPhi()) {
                    continue;
                }
                if (inst->GetInputsCount() != bb->GetPredsBlocks().size()) {
                    // if the number of PHI inputs less than the number of block predecessor and all inputs are equal
                    // Replace users to the input
                    ReplaceDeadPhiUsers(inst, bb);
                }
            }
        }
    }

private:
    Graph *graph_ {nullptr};
    InstVector realInputs_;
    InstVector phiUsers_;
    InstVector realUsers_;
    Marker marker_ {UNDEF_MARKER};
    Marker hasRealUserMarker_ {UNDEF_MARKER};
    bool hasSaveStateInstOnly_ {true};
};
}  // namespace ark::compiler

#endif  // PHI_RESOLVER_H
