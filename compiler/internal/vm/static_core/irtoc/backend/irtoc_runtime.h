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

#ifndef PANDA_IRTOC_RUNTIME_H
#define PANDA_IRTOC_RUNTIME_H

#include <limits>

#include "compiler/optimizer/ir/runtime_interface.h"
#include "compilation.h"
#include "irtoc_interface_extensions_includes.inl.h"
#include "libarkbase/mem/gc_barrier.h"
#include "runtime/mem/gc/g1/g1-allocator_constants.h"
#include "runtime/include/hclass.h"
#include "libarkbase/utils/bit_utils.h"

namespace ark::irtoc {

class IrtocRuntimeInterface : public compiler::RuntimeInterface {
public:
    Function *MethodCast(MethodPtr method) const
    {
        return reinterpret_cast<Function *>(method);
    }

    std::string GetMethodName(MethodPtr method) const override
    {
        return MethodCast(method)->GetName();
    }

    std::string GetExternalMethodName(MethodPtr method, uint32_t externalId) const override
    {
        return reinterpret_cast<Function *>(method)->GetExternalFunction(externalId);
    }

    std::string GetMethodFullName(MethodPtr method, [[maybe_unused]] bool withSignature) const override
    {
        return std::string("Irtoc::") + MethodCast(method)->GetName();
    }

    std::string GetClassName([[maybe_unused]] ClassPtr klass) const override
    {
        return "Irtoc";
    }

    std::string GetClassNameFromMethod([[maybe_unused]] MethodPtr method) const override
    {
        return "Irtoc";
    }

    compiler::DataType::Type GetMethodReturnType(MethodPtr method) const override
    {
        auto unit = reinterpret_cast<Function *>(method);
        ASSERT(unit->GetGraph()->HasEndBlock());
        auto retType = compiler::DataType::NO_TYPE;
        for (auto exit : unit->GetGraph()->GetEndBlock()->GetPredsBlocks()) {
            if (exit->IsEmpty()) {
                continue;
            }
            for (auto inst : exit->Insts()) {
                IntrinsicId id = inst->IsIntrinsic() ? inst->CastToIntrinsic()->GetIntrinsicId() : IntrinsicId::INVALID;
                auto tailCall = id == IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY || id == IntrinsicId::INTRINSIC_TAIL_CALL;
                if (inst->IsReturn() || tailCall) {
                    // Check for return type mismatches, allow mismatch for dynamic methods
                    ASSERT(unit->GetGraph()->IsDynamicMethod() || retType == compiler::DataType::NO_TYPE ||
                           retType == inst->GetType());
                    retType = inst->GetType();
                }
            }
        }
        return retType;
    }

    size_t GetMethodTotalArgumentsCount(MethodPtr method) const override
    {
        return GetMethodArgumentsCount(method);
    }

    size_t GetMethodArgumentsCount(MethodPtr method) const override
    {
        auto unit = reinterpret_cast<Function *>(method);
        return unit->GetArgsCount();
    }

    ::ark::mem::BarrierType GetPreReadType() const override
    {
#ifdef ARK_HYBRID
        return ::ark::mem::BarrierType::PRE_CMC_READ_BARRIER;
#else
        return ::ark::mem::BarrierType::PRE_RB_NONE;
#endif
    }

    ::ark::mem::BarrierType GetPreType() const override
    {
#ifdef ARK_HYBRID
        return ::ark::mem::BarrierType::PRE_WRB_NONE;
#else
        return ::ark::mem::BarrierType::PRE_SATB_BARRIER;
#endif
    }

    ::ark::mem::BarrierType GetPostType() const override
    {
#ifdef ARK_HYBRID
        return ::ark::mem::BarrierType::POST_CMC_WRITE_BARRIER;
#else
        return ::ark::mem::BarrierType::POST_INTERREGION_BARRIER;
#endif
    }

    ::ark::mem::BarrierOperand GetBarrierOperand([[maybe_unused]] ::ark::mem::BarrierPosition barrierPosition,
                                                 [[maybe_unused]] std::string_view operandName) const override
    {
        ASSERT(operandName == "REGION_SIZE_BITS");
        uint8_t regionSizeBits = helpers::math::GetIntLog2(ark::mem::G1_REGION_SIZE);
        return mem::BarrierOperand(mem::BarrierOperandType::UINT8_LITERAL, mem::BarrierOperandValue(regionSizeBits));
    }

    size_t GetFieldOffset(FieldPtr field) const override
    {
        ASSERT((reinterpret_cast<uintptr_t>(field) & 1U) == 1U);
        return reinterpret_cast<uintptr_t>(field) >> 1U;
    }

    FieldPtr GetFieldByOffset(size_t offset) const override
    {
        ASSERT(MinimumBitsToStore(offset) < std::numeric_limits<uintptr_t>::digits);
        return reinterpret_cast<FieldPtr>((offset << 1U) | 1U);
    }

    SourceLanguage GetMethodSourceLanguage(MethodPtr method) const override
    {
        return MethodCast(method)->GetLanguage();
    }

    compiler::DataType::Type GetMethodTotalArgumentType(MethodPtr method, size_t index) const override
    {
        auto unit = reinterpret_cast<Function *>(method);
        for (auto param : unit->GetGraph()->GetParameters()) {
            if (param->CastToParameter()->GetArgNumber() == index) {
                return param->GetType();
            }
        }
        return compiler::DataType::NO_TYPE;
    }
    uint32_t GetCallableMask() const override
    {
        return HClass::GetCallableMask();
    }
#include "irtoc_interface_extensions.inl.h"
};

}  // namespace ark::irtoc

#endif  // PANDA_IRTOC_RUNTIME_H
