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
#include "compiler/optimizer/ir/inst.h"

namespace ark::bytecodeopt {

// If false returns also BasicBlock pointer in which search stopped
static std::pair<bool, compiler::BasicBlock *> IsAccWriteBetweenBlocks(compiler::BasicBlock *block,
                                                                       compiler::Inst *dstInst)
{
    do {
        // NOTE(rtakacs): visit all the successors to get information about the
        // accumulator usage. Only linear flow is supported right now.
        if (block->GetSuccsBlocks().size() > 1U) {
            return {true, nullptr};
        }

        ASSERT(block->GetSuccsBlocks().size() == 1U);
        block = block->GetSuccessor(0U);
        // NOTE(rtakacs): only linear flow is supported right now.
        if (!dstInst->IsPhi() && block->GetPredsBlocks().size() > 1U) {
            return {true, nullptr};
        }
    } while (block->IsEmpty() && !block->HasPhi());
    return {false, block};
}

static bool IsAccWriteInInst(compiler::Inst *inst)
{
    if (!inst->IsAccWrite()) {
        return false;
    }
    if (inst->GetBasicBlock()->GetGraph()->IsAbcKit()) {
        return true;
    }
    if (!inst->IsConst()) {
        return true;
    }
    if (inst->GetDstReg() == compiler::GetAccReg()) {
        return true;
    }
    return false;
}

// Check that acc_read instruction reads from input allocated to vreg
static bool IsAccReadFromReg(compiler::Inst *srcInst, compiler::Inst *inst)
{
    if (!inst->IsAccRead()) {
        return false;
    }
    compiler::Inst *input = inst->GetInput(AccReadIndex(inst)).GetInst();
    return (input != srcInst && input->GetDstReg() != compiler::GetAccReg());
}

/// Decide if accumulator register gets dirty between two instructions.
bool IsAccWriteBetween(compiler::Inst *srcInst, compiler::Inst *dstInst)
{
    ASSERT(srcInst != dstInst);

    compiler::BasicBlock *block = srcInst->GetBasicBlock();
    compiler::Inst *inst = srcInst->GetNext();

    while (inst != dstInst) {
        if (UNLIKELY(inst == nullptr)) {
            bool accWrite;
            std::tie(accWrite, block) = IsAccWriteBetweenBlocks(block, dstInst);
            if (accWrite) {
                // fast exit if we know the answer
                return true;
            }
            // Get first phi instruction if exist.
            // This is required if dstInst is a phi node.
            inst = *(block->AllInsts());
        } else {
            if (IsAccWriteInInst(inst)) {
                return true;
            }

            if (IsAccReadFromReg(srcInst, inst)) {
                return true;
            }

            inst = inst->GetNext();
        }
    }

    return false;
}
/// Return true if Phi instruction is marked as optimizable.
inline bool RegAccAlloc::IsPhiOptimizable(compiler::Inst *phi) const
{
    ASSERT(phi->GetOpcode() == compiler::Opcode::Phi);
    return phi->IsMarked(accMarker_);
}

/// Return true if instruction can read the accumulator.
bool RegAccAlloc::IsAccRead(compiler::Inst *inst) const
{
    return UNLIKELY(inst->IsPhi()) ? IsPhiOptimizable(inst) : inst->IsAccRead();
}

static bool IsCommutative(compiler::Inst *inst)
{
    if (inst->GetOpcode() == compiler::Opcode::If) {
        auto cc = inst->CastToIf()->GetCc();
        return cc == compiler::ConditionCode::CC_EQ || cc == compiler::ConditionCode::CC_NE;
    }
    return inst->IsCommutative();
}

bool UserNeedSwapInputs(compiler::Inst *inst, compiler::Inst *user)
{
    if (!IsCommutative(user)) {
        return false;
    }
    return user->GetInput(AccReadIndex(user)).GetInst() != inst;
}

/// Return true if instruction can write the accumulator.
bool RegAccAlloc::IsAccWrite(compiler::Inst *inst) const
{
    return UNLIKELY(inst->IsPhi()) ? IsPhiOptimizable(inst) : inst->IsAccWrite();
}

bool RegAccAlloc::CanIntrinsicReadAcc(compiler::IntrinsicInst *inst) const
{
    ASSERT(GetGraph()->IsAbcKit());
    return inst->IsAccRead();
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

    for (auto &input : user->GetInputs()) {
        auto inputInst = input.GetInst();
        if (inputInst != user && inputInst->GetDstReg() == compiler::GetAccReg()) {
            return false;
        }
        if (inst->IsPhi() && inputInst->IsConst() &&
            (inputInst->GetBasicBlock()->GetId() != user->GetBasicBlock()->GetId())) {
            return false;
        }
    }

    if (GetGraph()->IsAbcKit()) {
        if (user->IsIntrinsic()) {
            return CanIntrinsicReadAcc(user->CastToIntrinsic()) && user->GetInput(AccReadIndex(user)).GetInst() == inst;
        }
        if (user->IsCall()) {
            return user->GetInputsCount() <= (MAX_NUM_NON_RANGE_ARGS);
        }
    }

    if (user->IsCallOrIntrinsic()) {
        return user->GetInputsCount() <= (MAX_NUM_NON_RANGE_ARGS + 1U);  // +1 for SaveState
    }

    return user->GetInput(AccReadIndex(user)).GetInst() == inst || IsCommutative(user);
}

/**
 * Check if all the Phi inputs and outputs can use the accumulator register.
 *
 * Return true, if Phi can be optimized.
 */
bool RegAccAlloc::IsPhiAccReady(compiler::Inst *phi) const
{
    ASSERT(phi->GetOpcode() == compiler::Opcode::Phi);

    // NOTE(rtakacs): there can be cases when the input/output of a Phi is an other Phi.
    // These cases are not optimized for accumulator.
    for (auto input : phi->GetInputs()) {
        compiler::Inst *phiInput = input.GetInst();

        if (!IsAccWrite(phiInput) || IsAccWriteBetween(phiInput, phi)) {
            return false;
        }
    }

    std::unordered_set<compiler::Inst *> usersThatRequiredSwapInputs;
    for (auto &user : phi->GetUsers()) {
        compiler::Inst *uinst = user.GetInst();

        if (!CanUserReadAcc(phi, uinst)) {
            return false;
        }
        if (UserNeedSwapInputs(phi, uinst)) {
            usersThatRequiredSwapInputs.insert(uinst);
        }
    }
    for (auto uinst : usersThatRequiredSwapInputs) {
        uinst->SwapInputs();
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
    if (IsCall(inst)) {
        return;
    }

    compiler::Register reg = need ? compiler::GetInvalidReg() : compiler::GetAccReg();
    inst->SetSrcReg(AccReadIndex(inst), reg);
}

static inline bool MaybeRegDst(compiler::Inst *inst)
{
    compiler::Opcode opcode = inst->GetOpcode();
#ifdef ENABLE_LIBABCKIT
    if (inst->GetBasicBlock()->GetGraph()->IsAbcKit() && inst->IsIntrinsic()) {
        auto id = inst->CastToIntrinsic()->GetIntrinsicId();
        return id == ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_OBJECT ||
               id == ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_OBJECT_WIDE ||
               id == ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_OBJECT_OBJECT;
    }
#endif
    return inst->IsConst() || inst->IsBinaryInst() || inst->IsBinaryImmInst() || opcode == compiler::Opcode::LoadObject;
}

static void InitRegistersForInst(compiler::Inst *inst)
{
    if (inst->IsSaveState() || inst->IsCatchPhi()) {
        return;
    }
    if (inst->IsConst()) {
        inst->SetFlag(compiler::inst_flags::ACC_WRITE);
    }
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        inst->SetSrcReg(i, compiler::GetInvalidReg());
        if (MaybeRegDst(inst)) {
            inst->SetDstReg(compiler::GetInvalidReg());
        }
    }
}

static void InitRegisters(compiler::Graph *graph)
{
    for (auto block : graph->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            InitRegistersForInst(inst);
        }
    }
}

static bool HasUnsupportedOpcode(compiler::Graph *graph)
{
    if (graph->IsDynamicMethod()) {
        return false;
    }
    for (auto block : graph->GetBlocksRPO()) {
        for (auto inst : block->AllInsts()) {
            if (inst->GetOpcode() == compiler::Opcode::Builtin) {
                return true;
            }
        }
    }
    return false;
}

void RegAccAlloc::MarkPhiInstructions() const
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto phi : block->PhiInsts()) {
            if (IsPhiAccReady(phi)) {
                phi->SetMarker(accMarker_);
            }
        }
    }
}

void RegAccAlloc::ClearAccForInstAndUsers(compiler::Inst *inst)
{
    inst->ClearFlag(compiler::inst_flags::ACC_WRITE);
    for (auto &user : inst->GetUsers()) {
        compiler::Inst *uinst = user.GetInst();
        if (uinst->IsSaveState()) {
            continue;
        }
        SetNeedLda(uinst, true);
    }
}

void RegAccAlloc::MarkInstruction(compiler::Inst *inst)
{
    if (inst->NoDest() || !IsAccWrite(inst)) {
        return;
    }

    bool useAccDstReg = true;

    std::unordered_set<compiler::Inst *> usersThatRequiredSwapInputs;
    for (auto &user : inst->GetUsers()) {
        compiler::Inst *uinst = user.GetInst();
        if (uinst->IsSaveState()) {
            continue;
        }
        if (CanUserReadAcc(inst, uinst)) {
            if (UserNeedSwapInputs(inst, uinst)) {
                usersThatRequiredSwapInputs.insert(uinst);
            }
            SetNeedLda(uinst, false);
        } else {
            useAccDstReg = false;
        }
    }
    for (auto uinst : usersThatRequiredSwapInputs) {
        uinst->SwapInputs();
    }

    if (useAccDstReg) {
        inst->SetDstReg(compiler::GetAccReg());
    } else if (MaybeRegDst(inst)) {
        ClearAccForInstAndUsers(inst);
    }
}

bool MustMoveInputsFromAcc(compiler::Inst *inst)
{
    bool mustMove = false;
    if (inst->IsPhi()) {
        auto inputs = inst->GetInputs();
        bool allInputsIsConst = true;
        for (auto input : inputs) {
            auto inputInst = input.GetInst();
            if (!inputInst->IsConst()) {
                allInputsIsConst = false;
                break;
            }
        }
        mustMove = allInputsIsConst;
    }
    return mustMove;
}

void RegAccAlloc::MoveInputsFromAcc(compiler::Inst *inst)
{
    inst->SetDstReg(compiler::GetInvalidReg());
    for (auto input : inst->GetInputs()) {
        auto inputInst = input.GetInst();
        inputInst->SetDstReg(compiler::GetInvalidReg());
    }

    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (IsAccRead(userInst)) {
            SetNeedLda(userInst, true);
        }
    }
}

void RegAccAlloc::MarkInstructions()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->AllInsts()) {
            MarkInstruction(inst);

            if (MustMoveInputsFromAcc(inst)) {
                MoveInputsFromAcc(inst);
            }
        }

        // Check if acc is written between inst and its intput
        for (auto inst : block->Insts()) {
            if (inst->GetInputsCount() == 0U) {
                continue;
            }
            if (IsCall(inst)) {
                continue;
            }

            compiler::Inst *input = inst->GetInput(AccReadIndex(inst)).GetInst();

            if (!IsAccWriteBetween(input, inst)) {
                continue;
            }
            input->SetDstReg(compiler::GetInvalidReg());
            SetNeedLda(inst, true);

            if (MaybeRegDst(input)) {
                ClearAccForInstAndUsers(input);
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
    InitRegisters(GetGraph());

    if (HasUnsupportedOpcode(GetGraph())) {
        return false;
    }

    MarkPhiInstructions();
    MarkInstructions();

#ifdef COMPILER_DEBUG_CHECKS
    GetGraph()->SetRegAccAllocApplied();
#endif  // COMPILER_DEBUG_CHECKS

    return true;
}

}  // namespace ark::bytecodeopt
