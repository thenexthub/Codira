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

#include "libabckit/src/irbuilder_dynamic/inst_builder_dyn.h"
#include "libabckit/src/irbuilder_dynamic/phi_resolver_dyn.h"

namespace libabckit {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

void InstBuilder::Prepare()
{
    SetCurrentBlock(GetGraph()->GetStartBlock());
#ifndef PANDA_TARGET_WINDOWS
    GetGraph()->ResetParameterInfo();
#endif
    auto numArgs = GetRuntime()->GetMethodTotalArgumentsCount(GetMethod());
    // Create Parameter instructions for all arguments
    for (size_t i = 0; i < numArgs; i++) {
        auto paramInst = GetGraph()->AddNewParameter(i);
        auto type = compiler::DataType::Type::ANY;
        auto regNum = GetRuntime()->GetMethodRegistersCount(GetMethod()) + i;
        ASSERT(!GetGraph()->IsBytecodeOptimizer() || regNum != ark::compiler::GetInvalidReg());

        paramInst->SetType(type);
        SetParamSpillFill(GetGraph(), paramInst, numArgs, i, type);

        UpdateDefinition(regNum, paramInst);
    }
}

void InstBuilder::UpdateDefsForCatch()
{
    compiler::Inst *catchPhi = currentBb_->GetFirstInst();
    ASSERT(catchPhi != nullptr);
    for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
        ASSERT(catchPhi->IsCatchPhi());
        defs_[currentBb_->GetId()][vreg] = catchPhi;
        catchPhi = catchPhi->GetNext();
    }
}

void InstBuilder::UpdateDefsForLoopHead()
{
    // If current block is a loop header, then propagate all definitions from preheader's predecessors to
    // current block.
    ASSERT(currentBb_->GetLoop()->GetPreHeader());
    auto predDefs = defs_[currentBb_->GetLoop()->GetPreHeader()->GetId()];
    for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
        auto defInst = predDefs[vreg];
        if (defInst != nullptr) {
            auto phi = GetGraph()->CreateInstPhi();
            phi->SetMarker(GetNoTypeMarker());
            phi->SetLinearNumber(vreg);
            currentBb_->AppendPhi(phi);
            (*currentDefs_)[vreg] = phi;
        }
    }
}

void InstBuilder::AddPhiToDifferent()
{
    for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
        compiler::Inst *value = nullptr;
        bool different = false;
        for (auto predBb : currentBb_->GetPredsBlocks()) {
            // When irreducible loop header is visited before it's back-edge, phi should be created,
            // since we do not know if definitions are different at this point
            if (!predBb->IsMarked(visitedBlockMarker_)) {
                ASSERT(currentBb_->GetLoop()->IsIrreducible());
                different = true;
                break;
            }
            if (value == nullptr) {
                value = defs_[predBb->GetId()][vreg];
            } else if (value != defs_[predBb->GetId()][vreg]) {
                different = true;
                break;
            }
        }
        if (different) {
            auto phi = GetGraph()->CreateInstPhi();
            phi->SetMarker(GetNoTypeMarker());
            phi->SetLinearNumber(vreg);
            currentBb_->AppendPhi(phi);
            (*currentDefs_)[vreg] = phi;
        } else {
            (*currentDefs_)[vreg] = value;
        }
    }
}

void InstBuilder::UpdateDefs()
{
    currentBb_->SetMarker(visitedBlockMarker_);
    if (currentBb_->IsCatchBegin()) {
        UpdateDefsForCatch();
    } else if (currentBb_->IsLoopHeader() && !currentBb_->GetLoop()->IsIrreducible()) {
        UpdateDefsForLoopHead();
    } else if (currentBb_->GetPredsBlocks().size() == 1) {
        // Only one predecessor - simply copy all its definitions
        auto &predDefs = defs_[currentBb_->GetPredsBlocks()[0]->GetId()];
        std::copy(predDefs.begin(), predDefs.end(), currentDefs_->begin());
    } else if (currentBb_->GetPredsBlocks().size() > 1) {
        AddPhiToDifferent();
    }
}

void InstBuilder::AddCatchPhiInputs(const ArenaUnorderedSet<compiler::BasicBlock *> &catchHandlers,
                                    const compiler::InstVector &defs, compiler::Inst *throwableInst)
{
    ASSERT(!catchHandlers.empty());
    for (auto catchBb : catchHandlers) {
        auto inst = catchBb->GetFirstInst();
        while (!inst->IsCatchPhi()) {
            inst = inst->GetNext();
        }
        ASSERT(inst != nullptr);
        GetGraph()->AppendThrowableInst(throwableInst, catchBb);
        for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++, inst = inst->GetNext()) {
            ASSERT(inst->GetOpcode() == ark::compiler::Opcode::CatchPhi);
            auto catchPhi = inst->CastToCatchPhi();
            if (catchPhi->IsAcc()) {
                ASSERT(vreg == vregsAndArgsCount_);
                continue;
            }
            auto inputInst = defs[vreg];
            if (inputInst != nullptr && inputInst != catchPhi) {
                catchPhi->AppendInput(inputInst);
                catchPhi->AppendThrowableInst(throwableInst);
            }
        }
    }
}

void InstBuilder::SetParamSpillFill(compiler::Graph *graph, compiler::ParameterInst *paramInst, size_t numArgs,
                                    size_t i, compiler::DataType::Type type)
{
    if (graph->IsBytecodeOptimizer()) {
        auto regSrc = static_cast<compiler::Register>(compiler::GetFrameSize() - numArgs + i);
        compiler::DataType::Type regType;
        if (compiler::DataType::IsReference(type)) {
            regType = compiler::DataType::REFERENCE;
        } else if (compiler::DataType::Is64Bits(type, graph->GetArch())) {
            regType = compiler::DataType::UINT64;
        } else {
            regType = compiler::DataType::UINT32;
        }

        paramInst->SetLocationData(
            {compiler::LocationType::REGISTER, compiler::LocationType::REGISTER, regSrc, regSrc, regType});
    }
}

/**
 * Set type of instruction, then recursively set type to its inputs.
 */
void InstBuilder::SetTypeRec(compiler::Inst *inst, compiler::DataType::Type type)
{
    inst->SetType(type);
    inst->ResetMarker(GetNoTypeMarker());
    for (auto input : inst->GetInputs()) {
        if (input.GetInst()->IsMarked(GetNoTypeMarker())) {
            SetTypeRec(input.GetInst(), type);
        }
    }
}

// CC-OFFNXT(WordsTool.190) sensitive word conflict
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
void InstBuilder::RemoveNotDominateInputs(compiler::SaveStateInst *saveState)
{
    size_t idx = 0;
    size_t inputsCount = saveState->GetInputsCount();
    while (idx < inputsCount) {
        auto inputInst = saveState->GetInput(idx).GetInst();
        // We can don't call IsDominate, if save_state and input_inst in one basic block.
        // It's reduce number of IsDominate calls.
        if (!inputInst->InSameBlockOrDominate(saveState)) {
            saveState->RemoveInput(idx);
            inputsCount--;
        } else {
            ASSERT(inputInst->GetBasicBlock() != saveState->GetBasicBlock() || inputInst->IsDominate(saveState));
            idx++;
        }
    }
}

void InstBuilder::UpdatePreds(compiler::BasicBlock *bb, compiler::Inst *inst)
{
    inst->ReserveInputs(bb->GetPredsBlocks().size());
    for (auto &predBb : bb->GetPredsBlocks()) {
        if (inst->GetLinearNumber() == compiler::INVALID_LINEAR_NUM) {
            continue;
        }
        auto pred = defs_[predBb->GetId()][inst->GetLinearNumber()];
        if (pred == nullptr) {
            // If any input of phi instruction is not defined then we assume that phi is dead. DCE should
            // remove it.
            continue;
        }
        inst->AppendInput(pred);
    }
}

void InstBuilder::SetType(compiler::Inst *inst)
{
    if (inst->IsSaveState()) {
        RemoveNotDominateInputs(static_cast<compiler::SaveStateInst *>(inst));
        return;
    }
    auto inputIdx = 0;
    for (auto input : inst->GetInputs()) {
        if (input.GetInst()->IsMarked(GetNoTypeMarker())) {
            auto inputType = inst->GetInputType(inputIdx);
            if (inputType != compiler::DataType::NO_TYPE) {
                SetTypeRec(input.GetInst(), inputType);
            }
        }
        inputIdx++;
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
            UpdatePreds(bb, inst);
        }
    }

    // Check all instructions that have no type and fix it. Type is got from instructions with known input types.
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            SetType(inst);
        }
    }
    // Resolve dead and inconsistent phi instructions
    PhiResolver phiResolver(GetGraph());
    phiResolver.Run();
    ResolveConstants();
    CleanupCatchPhis();
}

compiler::SaveStateInst *InstBuilder::CreateSaveState([[maybe_unused]] compiler::Opcode opc, size_t pc)
{
    ASSERT(opc == ark::compiler::Opcode::SaveState);
    compiler::SaveStateInst *inst = GetGraph()->CreateInstSaveState();
    inst->SetPc(pc);
    inst->SetMethod(GetMethod());
    inst->ReserveInputs(0);
    return inst;
}

size_t InstBuilder::GetMethodArgumentsCount(uintptr_t id) const
{
    return GetRuntime()->GetMethodArgumentsCount(GetMethod(), id);
}

size_t InstBuilder::GetPc(const uint8_t *instPtr) const
{
    return instPtr - instructionsBuf_;
}

void InstBuilder::ResolveConstants()
{
    compiler::ConstantInst *currConst = GetGraph()->GetFirstConstInst();
    while (currConst != nullptr) {
        SplitConstant(currConst);
        currConst = currConst->GetNextConst();
    }
}

void InstBuilder::SplitConstant(compiler::ConstantInst *constInst)
{
    if (constInst->GetType() != compiler::DataType::INT64 || !constInst->HasUsers()) {
        return;
    }
    auto users = constInst->GetUsers();
    auto currIt = users.begin();
    while (currIt != users.end()) {
        auto user = (*currIt).GetInst();
        compiler::DataType::Type type = user->GetInputType(currIt->GetIndex());
        ++currIt;
        if (type != compiler::DataType::FLOAT32 && type != compiler::DataType::FLOAT64) {
            continue;
        }
        compiler::ConstantInst *newConst = nullptr;
        if (type == compiler::DataType::FLOAT32) {
            auto val = bit_cast<float>(static_cast<uint32_t>(constInst->GetIntValue()));
            newConst = GetGraph()->FindOrCreateConstant(val);
        } else {
            auto val = bit_cast<double, uint64_t>(constInst->GetIntValue());
            newConst = GetGraph()->FindOrCreateConstant(val);
        }
        user->ReplaceInput(constInst, newConst);
    }
}

void InstBuilder::CleanupInst(compiler::BasicBlock *block, compiler::Inst *inst)
{
    if (!inst->IsCatchPhi() || inst->GetInputs().Empty()) {
        return;
    }
    // Remove catch-phis without real users
    bool hasSsUsersOnly = true;
    for (const auto &user : inst->GetUsers()) {
        if (!user.GetInst()->IsSaveState()) {
            hasSsUsersOnly = false;
            break;
        }
    }
    if (hasSsUsersOnly) {
        auto users = inst->GetUsers();
        while (!users.Empty()) {
            auto &user = users.Front();
            user.GetInst()->RemoveInput(user.GetIndex());
        }
        block->RemoveInst(inst);
    }
}

void InstBuilder::CleanupCatchPhis()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->AllInstsSafe()) {
            CleanupInst(block, inst);
        }
    }
}

}  // namespace libabckit
