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

#include "inst_builder.h"
#include "phi_resolver.h"

namespace panda::compiler {
void InstBuilder::Prepare()
{
    SetCurrentBlock(GetGraph()->GetStartBlock());
#ifndef PANDA_TARGET_WINDOWS
    GetGraph()->ResetParameterInfo();
#endif
    auto num_args = GetRuntime()->GetMethodTotalArgumentsCount(GetMethod());
    // Create Parameter instructions for all arguments
    for (size_t i = 0; i < num_args; i++) {
        auto param_inst = GetGraph()->AddNewParameter(i);
        auto type = DataType::Type::ANY;
        auto reg_num = GetRuntime()->GetMethodRegistersCount(GetMethod()) + i;
        ASSERT(!GetGraph()->IsBytecodeOptimizer() || reg_num != INVALID_REG);

        param_inst->SetType(type);
        SetParamSpillFill(GetGraph(), param_inst, num_args, i, type);

        UpdateDefinition(reg_num, param_inst);
    }
}

void InstBuilder::UpdateDefsForCatch()
{
    Inst *catch_phi = current_bb_->GetFirstInst();
    ASSERT(catch_phi != nullptr);
    for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
        ASSERT(catch_phi->IsCatchPhi());
        defs_[current_bb_->GetId()][vreg] = catch_phi;
        catch_phi = catch_phi->GetNext();
    }
}

void InstBuilder::UpdateDefsForLoopHead()
{
    // If current block is a loop header, then propagate all definitions from preheader's predecessors to
    // current block.
    ASSERT(current_bb_->GetLoop()->GetPreHeader());
    auto pred_defs = defs_[current_bb_->GetLoop()->GetPreHeader()->GetId()];
    COMPILER_LOG(DEBUG, IR_BUILDER) << "basic block is loop header";
    for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
        auto def_inst = pred_defs[vreg];
        if (def_inst != nullptr) {
            auto phi = GetGraph()->CreateInstPhi();
            phi->SetMarker(GetNoTypeMarker());
            phi->SetLinearNumber(vreg);
            current_bb_->AppendPhi(phi);
            (*current_defs_)[vreg] = phi;
            COMPILER_LOG(DEBUG, IR_BUILDER) << "create Phi(id=" << phi->GetId() << ") for r" << vreg
                                            << "(def id=" << pred_defs[vreg]->GetId() << ")";
        }
    }
}

void InstBuilder::UpdateDefs()
{
    current_bb_->SetMarker(visited_block_marker_);
    if (current_bb_->IsCatchBegin()) {
        UpdateDefsForCatch();
    } else if (current_bb_->IsLoopHeader() && !current_bb_->GetLoop()->IsIrreducible()) {
        UpdateDefsForLoopHead();
    } else if (current_bb_->GetPredsBlocks().size() == 1) {
        // Only one predecessor - simply copy all its definitions
        auto &pred_defs = defs_[current_bb_->GetPredsBlocks()[0]->GetId()];
        std::copy(pred_defs.begin(), pred_defs.end(), current_defs_->begin());
    } else if (current_bb_->GetPredsBlocks().size() > 1) {
        // If there are multiple predecessors, then add phi for each register that has different definitions
        for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
            Inst *value = nullptr;
            bool different = false;
            for (auto pred_bb : current_bb_->GetPredsBlocks()) {
                // When irreducible loop header is visited before it's back-edge, phi should be created,
                // since we do not know if definitions are different at this point
                if (!pred_bb->IsMarked(visited_block_marker_)) {
                    ASSERT(current_bb_->GetLoop()->IsIrreducible());
                    different = true;
                    break;
                }
                if (value == nullptr) {
                    value = defs_[pred_bb->GetId()][vreg];
                } else if (value != defs_[pred_bb->GetId()][vreg]) {
                    different = true;
                    break;
                }
            }
            if (different) {
                auto phi = GetGraph()->CreateInstPhi();
                phi->SetMarker(GetNoTypeMarker());
                phi->SetLinearNumber(vreg);
                current_bb_->AppendPhi(phi);
                (*current_defs_)[vreg] = phi;
                COMPILER_LOG(DEBUG, IR_BUILDER) << "create Phi(id=" << phi->GetId() << ") for r" << vreg;
            } else {
                (*current_defs_)[vreg] = value;
            }
        }
    }
}

void InstBuilder::AddCatchPhiInputs(const ArenaSet<BasicBlock *> &catch_handlers, const InstVector &defs,
                                    Inst *throwable_inst)
{
    ASSERT(!catch_handlers.empty());
    for (auto catch_bb : catch_handlers) {
        auto inst = catch_bb->GetFirstInst();
        while (!inst->IsCatchPhi()) {
            inst = inst->GetNext();
        }
        ASSERT(inst != nullptr);
        GetGraph()->AppendThrowableInst(throwable_inst, catch_bb);
        for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++, inst = inst->GetNext()) {
            ASSERT(inst->GetOpcode() == Opcode::CatchPhi);
            auto catch_phi = inst->CastToCatchPhi();
            if (catch_phi->IsAcc()) {
                ASSERT(vreg == VREGS_AND_ARGS_COUNT);
                continue;
            }
            auto input_inst = defs[vreg];
            if (input_inst != nullptr && input_inst != catch_phi) {
                catch_phi->AppendInput(input_inst);
                catch_phi->AppendThrowableInst(throwable_inst);
            }
        }
    }
}

void InstBuilder::SetParamSpillFill(Graph *graph, ParameterInst *param_inst, size_t num_args, size_t i,
                                    DataType::Type type)
{
    if (graph->IsBytecodeOptimizer()) {
        auto reg_src = static_cast<Register>(VIRTUAL_FRAME_SIZE - num_args + i);
        DataType::Type reg_type;
        if (DataType::IsReference(type)) {
            reg_type = DataType::REFERENCE;
        } else if (DataType::Is64Bits(type, graph->GetArch())) {
            reg_type = DataType::UINT64;
        } else {
            reg_type = DataType::UINT32;
        }

        param_inst->SetLocationData({LocationType::REGISTER, LocationType::REGISTER, reg_src, reg_src, reg_type});
    }
}

/**
 * Set type of instruction, then recursively set type to its inputs.
 */
void InstBuilder::SetTypeRec(Inst *inst, DataType::Type type)
{
    inst->SetType(type);
    inst->ResetMarker(GetNoTypeMarker());
    for (auto input : inst->GetInputs()) {
        if (input.GetInst()->IsMarked(GetNoTypeMarker())) {
            SetTypeRec(input.GetInst(), type);
        }
    }
}

/**
 * Remove vreg from SaveState for the case
 * BB 1
 *   ....
 * succs: [bb 2, bb 3]
 *
 * BB 2: preds: [bb 1]
 *   89.i64  Sub                        v85, v88 -> (v119, v90)
 *   90.f64  Cast                       v89 -> (v96, v92)
 * succs: [bb 3]
 *
 * BB 3: preds: [bb 1, bb 2]
 *   .....
 *   119.     SaveState                  v105(vr0), v106(vr1), v94(vr4), v89(vr8), v0(vr10), v1(vr11) -> (v120)
 *
 * v89(vr8) used only in BB 2, so we need to remove its from "119.     SaveState"
 */
/* static */
void InstBuilder::RemoveNotDominateInputs(SaveStateInst *save_state)
{
    size_t idx = 0;
    size_t inputs_count = save_state->GetInputsCount();
    while (idx < inputs_count) {
        auto input_inst = save_state->GetInput(idx).GetInst();
        // We can don't call IsDominate, if save_state and input_inst in one basic block.
        // It's reduce number of IsDominate calls.
        if (!input_inst->InSameBlockOrDominate(save_state)) {
            save_state->RemoveInput(idx);
            inputs_count--;
        } else {
            ASSERT(input_inst->GetBasicBlock() != save_state->GetBasicBlock() || input_inst->IsDominate(save_state));
            idx++;
        }
    }
}

/**
 * Fix instructions that can't be fully completed in building process.
 */
void InstBuilder::FixInstructions()
{
    // Remove dead Phi and set types to phi which have not type.
    // Phi may not have type if all it users are pseudo instructions, like SaveState
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->PhiInstsSafe()) {
            inst->ReserveInputs(bb->GetPredsBlocks().size());
            for (auto &pred_bb : bb->GetPredsBlocks()) {
                if (inst->GetLinearNumber() == INVALID_LINEAR_NUM) {
                    continue;
                }
                auto pred = defs_[pred_bb->GetId()][inst->GetLinearNumber()];
                if (pred == nullptr) {
                    // If any input of phi instruction is not defined then we assume that phi is dead. DCE should
                    // remove it.
                    continue;
                }
                inst->AppendInput(pred);
            }
        }
    }

    // Check all instructions that have no type and fix it. Type is got from instructions with known input types.
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            if (inst->IsSaveState()) {
                RemoveNotDominateInputs(static_cast<SaveStateInst *>(inst));
                continue;
            }
            auto input_idx = 0;
            for (auto input : inst->GetInputs()) {
                if (input.GetInst()->IsMarked(GetNoTypeMarker())) {
                    auto input_type = inst->GetInputType(input_idx);
                    if (input_type != DataType::NO_TYPE) {
                        SetTypeRec(input.GetInst(), input_type);
                    }
                }
                input_idx++;
            }
        }
    }
    // Resolve dead and inconsistent phi instructions
    PhiResolver phi_resolver(GetGraph());
    phi_resolver.Run();
    ResolveConstants();
    CleanupCatchPhis();
}

SaveStateInst *InstBuilder::CreateSaveState(Opcode opc, size_t pc)
{
    ASSERT(opc == Opcode::SaveState);
    SaveStateInst *inst = GetGraph()->CreateInstSaveState();
    inst->SetPc(pc);
    inst->SetMethod(GetMethod());
    inst->ReserveInputs(0);
    return inst;
}

size_t InstBuilder::GetMethodArgumentsCount(uintptr_t id) const
{
    return GetRuntime()->GetMethodArgumentsCount(GetMethod(), id);
}

size_t InstBuilder::GetPc(const uint8_t *inst_ptr) const
{
    return inst_ptr - instructions_buf_;
}

void InstBuilder::ResolveConstants()
{
    ConstantInst *curr_const = GetGraph()->GetFirstConstInst();
    while (curr_const != nullptr) {
        SplitConstant(curr_const);
        curr_const = curr_const->GetNextConst();
    }
}

void InstBuilder::SplitConstant(ConstantInst *const_inst)
{
    if (const_inst->GetType() != DataType::INT64 || !const_inst->HasUsers()) {
        return;
    }
    auto users = const_inst->GetUsers();
    auto curr_it = users.begin();
    while (curr_it != users.end()) {
        auto user = (*curr_it).GetInst();
        DataType::Type type = user->GetInputType(curr_it->GetIndex());
        ++curr_it;
        if (type != DataType::FLOAT32 && type != DataType::FLOAT64) {
            continue;
        }
        ConstantInst *new_const = nullptr;
        if (type == DataType::FLOAT32) {
            auto val = bit_cast<float>(static_cast<uint32_t>(const_inst->GetIntValue()));
            new_const = GetGraph()->FindOrCreateConstant(val);
        } else {
            auto val = bit_cast<double, uint64_t>(const_inst->GetIntValue());
            new_const = GetGraph()->FindOrCreateConstant(val);
        }
        user->ReplaceInput(const_inst, new_const);
    }
}

void InstBuilder::CleanupCatchPhis()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->AllInstsSafe()) {
            if (!inst->IsCatchPhi() || inst->GetInputs().Empty()) {
                continue;
            }
            // Remove catch-phis without real users
            bool has_ss_users_only = true;
            for (const auto &user : inst->GetUsers()) {
                if (!user.GetInst()->IsSaveState()) {
                    has_ss_users_only = false;
                    break;
                }
            }
            if (has_ss_users_only) {
                auto users = inst->GetUsers();
                while (!users.Empty()) {
                    auto &user = users.Front();
                    user.GetInst()->RemoveInput(user.GetIndex());
                }
                block->RemoveInst(inst);
            }
        }
    }
}
}  // namespace panda::compiler
