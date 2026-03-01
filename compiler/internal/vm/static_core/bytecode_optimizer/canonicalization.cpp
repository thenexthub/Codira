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
#include "canonicalization.h"

#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/inst.h"

namespace ark::bytecodeopt {

bool Canonicalization::RunImpl()
{
    Canonicalization visitor(GetGraph());
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            if (inst->IsCommutative()) {
                visitor.VisitCommutative(inst);
            } else {
                visitor.VisitInstruction(inst);
            }
        }
    }
    return visitor.GetStatus();
}

static bool IsDominateReverseInputs(const compiler::Inst *inst)
{
    auto input0 = inst->GetInput(0U).GetInst();
    auto input1 = inst->GetInput(1U).GetInst();
    return input0->IsDominate(input1);
}

static bool ConstantFitsCompareImm(const Inst *cst, uint32_t size)
{
    ASSERT(cst->GetOpcode() == Opcode::Constant);
    if (compiler::DataType::IsFloatType(cst->GetType())) {
        return false;
    }
    auto val = cst->CastToConstant()->GetIntValue();
    return (size == compiler::HALF_SIZE) && (val == 0U);
}

static bool BetterToSwapCompareInputs(const compiler::Inst *inst, const compiler::Inst *input0,
                                      const compiler::Inst *input1)
{
    if (!input0->IsConst()) {
        return false;
    }
    if (!input1->IsConst()) {
        return true;
    }

    compiler::DataType::Type type = inst->CastToCompare()->GetOperandsType();
    uint32_t size = (type == compiler::DataType::UINT64 || type == compiler::DataType::INT64) ? compiler::WORD_SIZE
                                                                                              : compiler::HALF_SIZE;
    return ConstantFitsCompareImm(input0, size) && !ConstantFitsCompareImm(input1, size);
}

static bool SwapInputsIfNecessary(compiler::Inst *inst, const bool necessary)
{
    if (!necessary) {
        return false;
    }
    auto input0 = inst->GetInput(0U).GetInst();
    auto input1 = inst->GetInput(1U).GetInst();
    if ((inst->GetOpcode() == compiler::Opcode::Compare) && !BetterToSwapCompareInputs(inst, input0, input1)) {
        return false;
    }

    inst->SwapInputs();
    return true;
}

bool Canonicalization::TrySwapConstantInput(Inst *inst)
{
    return SwapInputsIfNecessary(inst, inst->GetInput(0U).GetInst()->IsConst());
}

bool Canonicalization::TrySwapReverseInput(Inst *inst)
{
    return SwapInputsIfNecessary(inst, IsDominateReverseInputs(inst));
}

void Canonicalization::VisitCommutative(Inst *inst)
{
    ASSERT(inst->IsCommutative());
    // Exactly 2 data inputs expected for commutative operations (COMMUTATIVE_INPUT_COUNT)
    ASSERT((inst->GetInputsCount() == 2U && !inst->RequireState()) ||
           (inst->GetInputsCount() == 3U && inst->RequireState()));
    if (g_options.GetOptLevel() > 1) {
        result_ = TrySwapReverseInput(inst);
    }
    result_ = TrySwapConstantInput(inst) || result_;
}

// It is not allowed to move a constant input1 with a single user (it's processed Compare instruction).
// This is necessary for further merging of the constant and the If instruction in the Lowering pass
bool AllowSwap(const compiler::Inst *inst)
{
    auto input1 = inst->GetInput(1U).GetInst();
    if (!input1->IsConst()) {
        return true;
    }
    for (const auto &user : input1->GetUsers()) {
        if (user.GetInst() != inst) {
            return true;
        }
    }
    return false;
}

void Canonicalization::VisitCompare([[maybe_unused]] GraphVisitor *v, Inst *instBase)
{
    auto inst = instBase->CastToCompare();
    if (AllowSwap(inst) && SwapInputsIfNecessary(inst, IsDominateReverseInputs(inst))) {
        auto revertCc = SwapOperandsConditionCode(inst->GetCc());
        inst->SetCc(revertCc);
    }
}

}  // namespace ark::bytecodeopt
