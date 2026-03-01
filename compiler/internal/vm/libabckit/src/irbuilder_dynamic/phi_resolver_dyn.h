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

#ifndef LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_PHI_RESOLVER_H
#define LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_PHI_RESOLVER_H

#include "libabckit/src/irbuilder_dynamic/inst_builder_dyn.h"

namespace libabckit {
/**
 * Resolve phi instructions in the given graph. Resolving phi is:
 *  - remove phi if it has only SafePoint in users
 *  - if phi has no type, then set type determined from its inputs
 */
class PhiResolver {
public:
    explicit PhiResolver(ark::compiler::Graph *graph)
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
        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->AllInstsSafe()) {
                if (!inst->IsPhi() && inst->GetOpcode() != ark::compiler::Opcode::CatchPhi) {
                    continue;
                }
                if (inst->HasType() || (!inst->GetUsers().Empty() && CheckPhiInputs(inst))) {
                    continue;
                }
                CleanUp();
                inst->SetMarker(marker_);
                FindUsersRec(inst);
                if (hasSaveStateInstOnly_) {
                    RemovePhi(inst);
                } else {
                    SetTypeByInputs(inst);
                    ASSERT(inst->HasType() || (inst->IsCatchPhi() && !inst->CastToCatchPhi()->IsAcc()));
                }
                graph_->EraseMarker(marker_);
            }
        }
    }

private:
    void RemovePhi(ark::compiler::Inst *inst)
    {
        // Remove virtual registers of SafePoint instructions which input phis to be removed.
        for (auto user : realUsers_) {
            ASSERT(user->IsSaveState());
            auto saveState = static_cast<ark::compiler::SaveStateInst *>(user);
            size_t idx = 0;
            size_t inputsCount = saveState->GetInputsCount();
            while (idx < inputsCount) {
                auto inputInst = saveState->GetInput(idx).GetInst();
                if (inputInst->IsMarked(marker_)) {
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
    }

    void CleanUp()
    {
        phiUsers_.clear();
        realUsers_.clear();
        hasSaveStateInstOnly_ = true;
        marker_ = graph_->NewMarker();
    }
    static void SetTypeByInputs(ark::compiler::Inst *inst)
    {
        if (inst->IsCatchPhi() && inst->CastToCatchPhi()->IsAcc()) {
            inst->SetType(ark::compiler::DataType::REFERENCE);
            return;
        }
        for (auto input : inst->GetInputs()) {
            auto inputType = input.GetInst()->GetType();
            if (inputType != ark::compiler::DataType::NO_TYPE) {
                inst->SetType(inputType);
                break;
            }
        }
    }
    void FindUsersRec(ark::compiler::Inst *inst)
    {
        for (auto &user : inst->GetUsers()) {
            if (user.GetInst()->SetMarker(marker_)) {
                continue;
            }
            if (user.GetInst()->IsPhi() || user.GetInst()->GetOpcode() == ark::compiler::Opcode::CatchPhi) {
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

    void FindInputsRec(ark::compiler::Inst *inst)
    {
        ASSERT(inst->IsPhi() || inst->GetOpcode() == ark::compiler::Opcode::CatchPhi);
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
            if (inputInst->IsPhi() || inputInst->GetOpcode() == ark::compiler::Opcode::CatchPhi) {
                if (inputInst->GetType() != ark::compiler::DataType::NO_TYPE) {
                    realInputs_.push_back(inputInst);
                    continue;
                }
                FindInputsRec(inputInst);
            } else {
                realInputs_.push_back(inputInst);
            }
        }
    }

    static bool CheckPhiRealInputs(ark::compiler::InstVector &realInputs, ark::compiler::DataType::Type *type,
                                   bool *hasConstantInput)
    {
        for (auto inputInst : realInputs) {
            auto inputType = inputInst->GetType();
            if (inputType == ark::compiler::DataType::NO_TYPE) {
                return false;
            }
            if (inputInst->IsConst() && inputType == ark::compiler::DataType::INT64) {
                if (*type != ark::compiler::DataType::NO_TYPE &&
                    ark::compiler::DataType::GetCommonType(*type) != ark::compiler::DataType::INT64) {
                    return false;
                }
                *hasConstantInput = true;
                continue;
            }
            if (*type == ark::compiler::DataType::NO_TYPE) {
                if (*hasConstantInput &&
                    ark::compiler::DataType::GetCommonType(inputType) != ark::compiler::DataType::INT64) {
                    return false;
                }
                *type = inputType;
            } else if (*type != inputType) {
                return false;
            }
        }
        return true;
    }

    // Returns false if block with input instruction doesn't dominate the predecessor of the  PHI block
    bool CheckPhiInputs(ark::compiler::Inst *phiInst)
    {
        ASSERT(phiInst->GetOpcode() == ark::compiler::Opcode::Phi ||
               phiInst->GetOpcode() == ark::compiler::Opcode::CatchPhi);
        if (phiInst->GetOpcode() == ark::compiler::Opcode::Phi) {
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
        ark::compiler::DataType::Type type = ark::compiler::DataType::NO_TYPE;
        realInputs_.clear();
        marker_ = graph_->NewMarker();
        phiInst->SetMarker(marker_);
        FindInputsRec(phiInst);
        graph_->EraseMarker(marker_);

        bool hasConstantInput = false;
        if (!CheckPhiRealInputs(realInputs_, &type, &hasConstantInput)) {
            return false;
        }
        if (type == ark::compiler::DataType::NO_TYPE) {
            // Do not remove phi with constants-only inputs.
            if (!hasConstantInput) {
                return false;
            }
            type = ark::compiler::DataType::INT64;
        }
        phiInst->SetType(type);
        return true;
    }

private:
    ark::compiler::Graph *graph_ {nullptr};
    ark::compiler::InstVector realInputs_;
    ark::compiler::InstVector phiUsers_;
    ark::compiler::InstVector realUsers_;
    ark::compiler::Marker marker_ {ark::compiler::UNDEF_MARKER};
    bool hasSaveStateInstOnly_ {true};
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_PHI_RESOLVER_H
