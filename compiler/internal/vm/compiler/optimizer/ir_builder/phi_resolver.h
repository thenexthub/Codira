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

#ifndef COMPILER_OPTIMIZER_IR_BUILDER_PHI_RESOLVER_H
#define COMPILER_OPTIMIZER_IR_BUILDER_PHI_RESOLVER_H

#include "inst_builder.h"
#include "compiler_logger.h"

namespace panda::compiler {
/**
 * Resolve phi instructions in the given graph. Resolving phi is:
 *  - remove phi if it has only SafePoint in users
 *  - if phi has no type, then set type determined from its inputs
 */
class PhiResolver {
public:
    explicit PhiResolver(Graph *graph)
        : graph_(graph),
          real_inputs_(graph->GetLocalAllocator()->Adapter()),
          phi_users_(graph->GetLocalAllocator()->Adapter()),
          real_users_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(PhiResolver);
    NO_COPY_SEMANTIC(PhiResolver);
    ~PhiResolver() = default;

    void Run()
    {
        dead_marker_ = graph_->NewMarker();
        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->AllInstsSafe()) {
                if (!inst->IsPhi() && inst->GetOpcode() != Opcode::CatchPhi) {
                    continue;
                }
                if (inst->IsMarked(dead_marker_)) {
                    continue;
                }
                if (inst->HasType() || (!inst->GetUsers().Empty() && CheckPhiInputs(inst))) {
                    continue;
                }
                CleanUp();
                inst->SetMarker(marker_);
                FindUsersRec(inst);
                if (has_save_state_inst_only_) {
                    // Remove virtual registers of SafePoint instructions which input phis to be removed.
                    for (auto user : real_users_) {
                        ASSERT(user->IsSaveState());
                        auto save_state = static_cast<SaveStateInst *>(user);
                        size_t idx = 0;
                        size_t inputs_count = save_state->GetInputsCount();
                        while (idx < inputs_count) {
                            auto input_inst = save_state->GetInput(idx).GetInst();
                            if (input_inst->IsMarked(marker_)) {
                                save_state->RemoveInput(idx);
                                inputs_count--;
                            } else {
                                idx++;
                            }
                        }
                    }
                    // Phi has only SafePoint in users, we can remove this phi and all phi in collected list.
                    inst->RemoveUsers<true>();
                    inst->SetMarker(dead_marker_);
                    inst->GetBasicBlock()->RemoveInst(inst);
                    for (auto phi : phi_users_) {
                        phi->RemoveUsers<true>();
                        phi->SetMarker(dead_marker_);
                        phi->GetBasicBlock()->RemoveInst(phi);
                    }
                } else {
                    SetTypeByInputs(inst);
                    ASSERT(inst->HasType() || (inst->IsCatchPhi() && !inst->CastToCatchPhi()->IsAcc()));
                }
                graph_->EraseMarker(marker_);
            }
        }
        graph_->EraseMarker(dead_marker_);
    }

private:
    void CleanUp()
    {
        phi_users_.clear();
        real_users_.clear();
        has_save_state_inst_only_ = true;
        marker_ = graph_->NewMarker();
    }
    static void SetTypeByInputs(Inst *inst)
    {
        if (inst->IsCatchPhi() && inst->CastToCatchPhi()->IsAcc()) {
            inst->SetType(DataType::REFERENCE);
            return;
        }
        for (auto input : inst->GetInputs()) {
            auto input_type = input.GetInst()->GetType();
            if (input_type != DataType::NO_TYPE) {
                inst->SetType(input_type);
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
            if (user.GetInst()->IsPhi() || user.GetInst()->GetOpcode() == Opcode::CatchPhi) {
                phi_users_.push_back(user.GetInst());
                FindUsersRec(user.GetInst());
            } else {
                if (!user.GetInst()->IsSaveState()) {
                    has_save_state_inst_only_ = false;
                    break;
                }
                real_users_.push_back(user.GetInst());
            }
        }
    }

    void FindInputsRec(Inst *inst)
    {
        ASSERT(inst->IsPhi() || inst->GetOpcode() == Opcode::CatchPhi);
        // We can't set real type if there aren't inputs from Phi/CathPhi
        // We add the Phi/CathPhi in the list and return false from CheckPhiInputs
        if (inst->GetInputs().Empty()) {
            real_inputs_.push_back(inst);
            return;
        }
        for (auto &input : inst->GetInputs()) {
            auto input_inst = input.GetInst();
            if (input_inst->SetMarker(marker_)) {
                continue;
            }
            if (input_inst->IsPhi() || input_inst->GetOpcode() == Opcode::CatchPhi) {
                if (input_inst->GetType() != DataType::NO_TYPE) {
                    real_inputs_.push_back(input_inst);
                    continue;
                }
                FindInputsRec(input_inst);
            } else {
                real_inputs_.push_back(input_inst);
            }
        }
    }
    // Returns false if block with input instruction doesn't dominate the predecessor of the  PHI block
    bool CheckPhiInputs(Inst *phi_inst)
    {
        ASSERT(phi_inst->GetOpcode() == Opcode::Phi || phi_inst->GetOpcode() == Opcode::CatchPhi);
        if (phi_inst->GetOpcode() == Opcode::Phi) {
            if (phi_inst->GetInputsCount() != phi_inst->GetBasicBlock()->GetPredsBlocks().size()) {
                return false;
            }
            for (size_t index = 0; index < phi_inst->GetInputsCount(); ++index) {
                auto pred = phi_inst->GetBasicBlock()->GetPredBlockByIndex(index);
                auto input_bb = phi_inst->GetInput(index).GetInst()->GetBasicBlock();
                if (!input_bb->IsDominate(pred)) {
                    return false;
                }
            }
        }
        DataType::Type type = DataType::NO_TYPE;
        real_inputs_.clear();
        marker_ = graph_->NewMarker();
        phi_inst->SetMarker(marker_);
        FindInputsRec(phi_inst);
        graph_->EraseMarker(marker_);

        bool has_constant_input = false;
        for (auto input_inst : real_inputs_) {
            auto input_type = input_inst->GetType();
            if (input_type == DataType::NO_TYPE) {
                return false;
            }
            if (input_inst->IsConst() && input_type == DataType::INT64) {
                if (type != DataType::NO_TYPE && DataType::GetCommonType(type) != DataType::INT64) {
                    return false;
                }
                has_constant_input = true;
                continue;
            }
            if (type == DataType::NO_TYPE) {
                if (has_constant_input && DataType::GetCommonType(input_type) != DataType::INT64) {
                    return false;
                }
                type = input_type;
            } else if (type != input_type) {
                return false;
            }
        }

        if (type == DataType::NO_TYPE) {
            // Do not remove phi with constants-only inputs.
            if (!has_constant_input) {
                return false;
            }
            type = DataType::INT64;
        }
        phi_inst->SetType(type);
        return true;
    }

private:
    Graph *graph_ {nullptr};
    InstVector real_inputs_;
    InstVector phi_users_;
    InstVector real_users_;
    Marker marker_ {UNDEF_MARKER};
    Marker dead_marker_ {UNDEF_MARKER};
    bool has_save_state_inst_only_ {true};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_BUILDER_PHI_RESOLVER_H
