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

#include "reg_acc_alloc.h"
#include "common.h"
#include "compiler/optimizer/ir/basicblock.h"

namespace panda::bytecodeopt {

/**
 * Decide if accumulator register gets dirty between two instructions.
 */
bool IsAccWriteBetween(compiler::Inst *src_inst, compiler::Inst *dst_inst)
{
    ASSERT(src_inst != dst_inst);

    compiler::BasicBlock *block = src_inst->GetBasicBlock();
    compiler::Inst *inst = src_inst->GetNext();

    while (inst != dst_inst) {
        if (UNLIKELY(inst == nullptr)) {
            do {
                // TODO(rtakacs): visit all the successors to get information about the
                // accumulator usage. Only linear flow is supported right now.
                if (block->GetSuccsBlocks().size() > 1) {
                    return true;
                }

                ASSERT(block->GetSuccsBlocks().size() == 1);
                block = block->GetSuccessor(0);
                // TODO(rtakacs): only linear flow is supported right now.
                if (!dst_inst->IsPhi() && block->GetPredsBlocks().size() > 1) {
                    return true;
                }
            } while (block->IsEmpty() && !block->HasPhi());

            // Get first phi instruction if exist.
            // This is requred if dst_inst is a phi node.
            inst = *(block->AllInsts());
        } else {
            if (inst->IsAccWrite()) {
                return true;
            }

            if (inst->IsAccRead()) {
                compiler::Inst *input = inst->GetInput(AccReadIndex(inst)).GetInst();

                if (input->GetDstReg() != compiler::ACC_REG_ID) {
                    return true;
                }
            }

            inst = inst->GetNext();
        }
    }

    return false;
}

/**
 * Return true if Phi instruction is marked as optimizable.
 */
inline bool RegAccAlloc::IsPhiOptimizable(compiler::Inst *phi) const
{
    ASSERT(phi->GetOpcode() == compiler::Opcode::Phi);
    return phi->IsMarked(acc_marker_);
}

/**
 * Return true if instruction can read the accumulator.
 */
bool RegAccAlloc::IsAccRead(compiler::Inst *inst) const
{
    return UNLIKELY(inst->IsPhi()) ? IsPhiOptimizable(inst) : inst->IsAccRead();
}

/**
 * Return true if instruction can write the accumulator.
 */
bool RegAccAlloc::IsAccWrite(compiler::Inst *inst) const
{
    return UNLIKELY(inst->IsPhi()) ? IsPhiOptimizable(inst) : inst->IsAccWrite();
}

/**
 * Decide if user can use accumulator as source.
 * Do modifications on the order of inputs if necessary.
 *
 * Return true, if user can be optimized.
 */
bool RegAccAlloc::CanUserReadAcc(compiler::Inst *inst, compiler::Inst *user) const
{
    if (user->IsPhi()) {
        return IsPhiOptimizable(user);
    }

    if (!IsAccRead(user) || IsAccWriteBetween(inst, user)) {
        return false;
    }

    bool found = false;
    // Check if the instrucion occures more times as input.
    // v2. SUB v0, v1
    // v3. Add v2, v2
    for (auto input : user->GetInputs()) {
        compiler::Inst *uinput = input.GetInst();

        if (uinput != inst) {
            continue;
        }

        if (!found) {
            found = true;
        } else {
            return false;
        }
    }

    if (user->IsCall()) {
        return user->GetInputsCount() <= (MAX_NUM_NON_RANGE_ARGS + 1);  // +1 for SaveState
    }

    return user->GetInput(AccReadIndex(user)).GetInst() == inst || user->IsCommutative();
}

/**
 * Check if all the Phi inputs and outputs can use the accumulator register.
 *
 * Return true, if Phi can be optimized.
 */
bool RegAccAlloc::IsPhiAccReady(compiler::Inst *phi) const
{
    ASSERT(phi->GetOpcode() == compiler::Opcode::Phi);

    // TODO(rtakacs): there can be cases when the input/output of a Phi is an other Phi.
    // These cases are not optimized for accumulator.
    for (auto input : phi->GetInputs()) {
        compiler::Inst *phi_input = input.GetInst();

        if (!IsAccWrite(phi_input) || IsAccWriteBetween(phi_input, phi)) {
            return false;
        }
    }

    for (auto &user : phi->GetUsers()) {
        compiler::Inst *uinst = user.GetInst();

        if (!CanUserReadAcc(phi, uinst)) {
            return false;
        }
    }
    return true;
}

/**
 * For most insts we can use their src_reg on the acc-read position
 * to characterise whether we need lda in the codegen pass.
 */
void RegAccAlloc::SetNeedLda(compiler::Inst *inst, bool need)
{
    if (inst->IsPhi() || inst->IsCatchPhi()) {
        return;
    }
    if (!IsAccRead(inst)) {
        return;
    }
    if (inst->IsCall()) {  // we never need lda for calls
        return;
    }
    compiler::Register reg = need ? compiler::INVALID_REG : compiler::ACC_REG_ID;
    inst->SetSrcReg(AccReadIndex(inst), reg);
}

void RegAccAlloc::InitializeSourceRegisters()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (inst->IsSaveState() || inst->IsCatchPhi()) {
                continue;
            }
            if (inst->IsConst()) {
                inst->SetFlag(compiler::inst_flags::ACC_WRITE);
            }
            for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
                inst->SetSrcReg(i, compiler::INVALID_REG);
            }
            if (inst->IsConst()) {
                inst->SetDstReg(compiler::INVALID_REG);
            }
        }
    }
}

void RegAccAlloc::MarkAccForPhiInstructions()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto phi : block->PhiInsts()) {
            if (IsPhiAccReady(phi)) {
                phi->SetMarker(acc_marker_);
            }
        }
    }
}

void RegAccAlloc::MarkAccForInstructions(compiler::BasicBlock *block)
{
    for (auto inst : block->AllInsts()) {
        if (inst->NoDest() || !IsAccWrite(inst)) {
            continue;
        }

        bool use_acc_dst_reg = true;

        for (auto &user : inst->GetUsers()) {
            compiler::Inst *uinst = user.GetInst();
            if (uinst->IsSaveState()) {
                continue;
            }
            if (CanUserReadAcc(inst, uinst)) {
                SetNeedLda(uinst, false);
            } else {
                use_acc_dst_reg = false;
            }
        }

        if (use_acc_dst_reg) {
            inst->SetDstReg(compiler::ACC_REG_ID);
            continue;
        }

        if (!inst->IsConst()) {
            continue;
        }

        inst->ClearFlag(compiler::inst_flags::ACC_WRITE);

        for (auto &user : inst->GetUsers()) {
            compiler::Inst *uinst = user.GetInst();

            if (uinst->IsSaveState()) {
                continue;
            }

            SetNeedLda(uinst, true);
        }
    }
}

void RegAccAlloc::UpdateInstructionsAfterMark(compiler::BasicBlock *block)
{
    for (auto inst : block->Insts()) {
        if (inst->GetInputsCount() == 0) {
            continue;
        }

        if (inst->IsCall()) {
            continue;
        }

        compiler::Inst *input = inst->GetInput(AccReadIndex(inst)).GetInst();
        if (!IsAccWriteBetween(input, inst)) {
            continue;
        }

        input->SetDstReg(compiler::INVALID_REG);
        SetNeedLda(inst, true);

        if (input->IsConst()) {
            input->ClearFlag(compiler::inst_flags::ACC_WRITE);
            for (auto &user : input->GetUsers()) {
                compiler::Inst *uinst = user.GetInst();
                SetNeedLda(uinst, true);
            }
        }
    }
}

/**
 * Determine the accumulator usage between instructions.
 * Eliminate unnecessary register allocations by applying
 * a special value (ACC_REG_ID) to the destination and
 * source registers.
 * This special value is a marker for the code generator
 * not to produce lda/sta instructions.
 */
bool RegAccAlloc::RunImpl()
{
    GetGraph()->InitDefaultLocations();
    // Initialize all source register of all instructions.
    InitializeSourceRegisters();
    // Mark Phi instructions if they can be optimized for acc.
    MarkAccForPhiInstructions();
    // Mark instructions if they can be optimized for acc.
    for (auto block : GetGraph()->GetBlocksRPO()) {
        MarkAccForInstructions(block);
        UpdateInstructionsAfterMark(block);
    }

#ifndef NDEBUG
    GetGraph()->SetRegAccAllocApplied();
#endif  // NDEBUG

    return true;
}

}  // namespace panda::bytecodeopt
