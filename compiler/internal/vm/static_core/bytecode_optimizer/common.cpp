/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "common.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "optimizer/ir/runtime_interface.h"

namespace ark::bytecodeopt {

#ifndef NDEBUG
static bool IsStaticAccIndexDefined(const compiler::Inst *inst)
{
    if (!inst->IsCallOrIntrinsic()) {
        return true;
    }
    if (inst->GetBasicBlock()->GetGraph()->IsAbcKit() && !inst->IsCall()) {
        return true;
    }
    if (inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() ==
                                   inst->GetBasicBlock()->GetGraph()->GetRuntime()->GetCompilerNullcheckIntrinsicId()) {
        return true;
    }
    return false;
}
#endif  // !NDEBUG

uint8_t AccReadIndex(const compiler::Inst *inst)
{
    // For calls we cannot tell static index for acc position, thus
    // ensure that we don't invoke this for calls
    ASSERT(IsStaticAccIndexDefined(inst));
    switch (inst->GetOpcode()) {
        case compiler::Opcode::LoadArray:
        case compiler::Opcode::StoreObject:
        case compiler::Opcode::StoreStatic:
        case compiler::Opcode::NewArray:
            return 1U;
        case compiler::Opcode::StoreArray:
            return 2U;
        default: {
            if (inst->IsIntrinsic() && inst->IsAccRead()) {
                if (inst->GetBasicBlock()->GetGraph()->IsAbcKit()) {
                    ASSERT(inst->GetInputsCount() >= 1U);
                    return 0U;
                }
                ASSERT(inst->CastToIntrinsic()->GetIntrinsicId() != compiler::RuntimeInterface::IntrinsicId::INVALID);
                if (inst->CastToIntrinsic()->GetIntrinsicId() ==
                    inst->GetBasicBlock()->GetGraph()->GetRuntime()->GetCompilerNullcheckIntrinsicId()) {
                    ASSERT(inst->GetInputsCount() >= 1U);
                    return 0U;
                }
                ASSERT(inst->GetBasicBlock()->GetGraph()->IsDynamicMethod());
                ASSERT(inst->GetInputsCount() >= 2U);
                return inst->GetInputsCount() - 2L;
            }
            return 0U;
        }
    }
}

#ifdef ENABLE_LIBABCKIT
#include "generated/abckit_intrinsics.inl"
#else
bool IsAbcKitIntrinsicRange([[maybe_unused]] compiler::RuntimeInterface::IntrinsicId intrinsicId)
{
    UNREACHABLE();
}
bool IsAbcKitIntrinsic([[maybe_unused]] compiler::RuntimeInterface::IntrinsicId intrinsicId)
{
    UNREACHABLE();
}
#endif

// This method is used by bytecode optimizer's codegen.
bool CanConvertToIncI(const compiler::BinaryImmOperation *binop)
{
    ASSERT(binop->GetBasicBlock()->GetGraph()->IsRegAllocApplied());
    ASSERT(binop->GetOpcode() == compiler::Opcode::AddI || binop->GetOpcode() == compiler::Opcode::SubI);

    // IncI works on the same register.
    if (binop->GetSrcReg(0U) != binop->GetDstReg()) {
        return false;
    }

    // IncI cannot write accumulator.
    if (binop->GetSrcReg(0U) == compiler::GetAccReg()) {
        return false;
    }

    // IncI users cannot read from accumulator.
    // While Addi/SubI stores the output in accumulator, IncI works directly on registers.
    for (const auto &user : binop->GetUsers()) {
        const auto *uinst = user.GetInst();

        if (uinst->IsCallOrIntrinsic()) {
            continue;
        }

        const uint8_t index = AccReadIndex(uinst);
        if (uinst->GetInput(index).GetInst() == binop && uinst->GetSrcReg(index) == compiler::GetAccReg()) {
            return false;
        }
    }

    constexpr uint64_t BITMASK = 0xffffffff;
    // Define min and max values of i4 type.
    // NOLINTNEXTLINE(readability-identifier-naming)
    constexpr int32_t min = -8;
    // NOLINTNEXTLINE(readability-identifier-naming)
    constexpr int32_t max = 7;

    int32_t imm = binop->GetImm() & BITMASK;
    // Note: subi 3 is the same as inci v2, -3.
    if (binop->GetOpcode() == compiler::Opcode::SubI) {
        imm = -imm;
    }

    // IncI works only with 4 bits immediates.
    return imm >= min && imm <= max;
}

bool IsCall(compiler::Inst *inst)
{
    if (inst->GetBasicBlock()->GetGraph()->IsAbcKit()) {
        if (inst->IsCall()) {
            return true;
        }
    } else {
        if (inst->IsCallOrIntrinsic()) {
            return true;
        }
    }
    return false;
}

}  // namespace ark::bytecodeopt
