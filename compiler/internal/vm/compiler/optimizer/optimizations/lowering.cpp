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

#include "lowering.h"

namespace panda::compiler {
void Lowering::VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::IfImm);
    LowerIf(inst->CastToIfImm());
}

// Ask encoder whether Constant can be an immediate for Compare
bool Lowering::ConstantFitsCompareImm(Inst *cst, uint32_t size, ConditionCode cc)
{
    ASSERT(cst->GetOpcode() == Opcode::Constant);
    if (DataType::IsFloatType(cst->GetType())) {
        return false;
    }
    int64_t val = cst->CastToConstant()->GetRawValue();
    return (size == HALF_SIZE) && (val == 0);
}

// We'd like to swap only to make second operand immediate
bool Lowering::BetterToSwapCompareInputs(Inst *cmp)
{
    ASSERT(cmp->GetOpcode() == Opcode::Compare);
    auto in0 = cmp->GetInput(0).GetInst();
    auto in1 = cmp->GetInput(1).GetInst();
    if (DataType::IsFloatType(in0->GetType())) {
        return false;
    }

    if (in0->IsConst()) {
        if (in1->IsConst()) {
            DataType::Type type = cmp->CastToCompare()->GetOperandsType();
            uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
            auto cc = cmp->CastToCompare()->GetCc();
            return ConstantFitsCompareImm(in0, size, cc) && !ConstantFitsCompareImm(in1, size, cc);
        }
        return true;
    }
    return false;
}

// Optimize order of input arguments for decreasing using accumulator (Bytecodeoptimizer only).
void Lowering::OptimizeIfInput(compiler::Inst *if_inst)
{
    ASSERT(if_inst->GetOpcode() == compiler::Opcode::If);
    compiler::Inst *input_0 = if_inst->GetInput(0).GetInst();
    compiler::Inst *input_1 = if_inst->GetInput(1).GetInst();

    if (input_0->IsDominate(input_1)) {
        if_inst->SetInput(0, input_1);
        if_inst->SetInput(1, input_0);
        // And change CC
        auto cc = if_inst->CastToIf()->GetCc();
        cc = SwapOperandsConditionCode(cc);
        if_inst->CastToIf()->SetCc(cc);
    }
}

void Lowering::LowerIf(IfImmInst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    ASSERT(inst->GetCc() == ConditionCode::CC_NE || inst->GetCc() == ConditionCode::CC_EQ);
    ASSERT(inst->GetImm() == 0);
    if (inst->GetOperandsType() != DataType::BOOL) {
        ASSERT(!graph->SupportManagedCode() || graph->IsDynamicMethod());
        return;
    }
    auto input = inst->GetInput(0).GetInst();
    if (input->GetOpcode() != Opcode::Compare) {
        return;
    }
    // Check, that inst have only IfImm user
    for (auto &user : input->GetUsers()) {
        if (user.GetInst()->GetOpcode() != Opcode::IfImm) {
            return;
        }
    }
    // Try put constant in second input
    if (BetterToSwapCompareInputs(input)) {
        // Swap inputs
        auto in0 = input->GetInput(0).GetInst();
        auto in1 = input->GetInput(1).GetInst();
        input->SetInput(0, in1);
        input->SetInput(1, in0);
        // And change CC
        auto cc = input->CastToCompare()->GetCc();
        cc = SwapOperandsConditionCode(cc);
        input->CastToCompare()->SetCc(cc);
    }
    auto cst = input->GetInput(1).GetInst();
    DataType::Type type = input->CastToCompare()->GetOperandsType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    auto cc = input->CastToCompare()->GetCc();
    // IfImm can be inverted
    if (inst->GetCc() == ConditionCode::CC_EQ && inst->GetImm() == 0) {
        cc = GetInverseConditionCode(cc);
    }

    if (cst->IsConst() && ConstantFitsCompareImm(cst, size, cc)) {
        // In-place change for IfImm
        InPlaceLowerIfImm(inst, input, cst, cc);
    } else {
        // New instruction
        auto replace = graph->CreateInstIf(DataType::NO_TYPE, inst->GetPc(), cc);
        replace->SetMethod(inst->GetMethod());
        replace->SetOperandsType(input->CastToCompare()->GetOperandsType());
        replace->SetInput(0, input->GetInput(0).GetInst());
        replace->SetInput(1, input->GetInput(1).GetInst());
        // Replace IfImm instruction immediately because it's not removable by DCE
        inst->RemoveInputs();
        inst->GetBasicBlock()->ReplaceInst(inst, replace);
        graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
        if (graph->IsBytecodeOptimizer()) {
            OptimizeIfInput(replace);
        }
        COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
    }
}

void Lowering::InPlaceLowerIfImm(IfImmInst *inst, Inst *input, Inst *cst, ConditionCode cc)
{
    inst->SetOperandsType(input->CastToCompare()->GetOperandsType());
    auto new_input = input->GetInput(0).GetInst();
    inst->SetInput(0, new_input);

    uint64_t val = cst->CastToConstant()->GetRawValue();
    inst->SetImm(val);
    inst->SetCc(cc);
    inst->GetBasicBlock()->GetGraph()->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(),
                                                                      inst->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

void Lowering::InvalidateAnalyses()
{
}

bool Lowering::RunImpl()
{
    VisitGraph();
    return true;
}
}  // namespace panda::compiler
