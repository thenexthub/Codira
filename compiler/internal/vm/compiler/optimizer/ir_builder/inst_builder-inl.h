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

#ifndef COMPILER_OPTIMIZER_IR_BUILDER_INST_BUILDER_INL_H
#define COMPILER_OPTIMIZER_IR_BUILDER_INST_BUILDER_INL_H

#include "inst_builder.h"

namespace panda::compiler {
// NOLINTNEXTLINE(misc-definitions-in-headers)
// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode opcode>
void InstBuilder::BuildLoadFromPool(const BytecodeInstruction *bc_inst)
{
    auto method = GetGraph()->GetMethod();
    uint32_t type_id;
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    LoadFromPool *inst;
    // NOLINTNEXTLINE(readability-magic-numbers)
    static_assert(opcode == Opcode::LoadString);
    type_id = GetRuntime()->ResolveOffsetByIndex(GetGraph()->GetMethod(), bc_inst->GetId(0).AsIndex());
    inst = GetGraph()->CreateInstLoadString(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    inst->SetTypeId(type_id);
    inst->SetMethod(method);
    inst->SetInput(0, save_state);

    AddInstruction(save_state);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    BuildCastToAnyString(bc_inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyString(const BytecodeInstruction *bc_inst)
{
    auto input = GetDefinitionAcc();
    ASSERT(input->GetType() == DataType::REFERENCE);

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto any_type = GetAnyStringType(language);
    ASSERT(any_type != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bc_inst->GetAddress()));
    box->SetAnyType(any_type);
    box->SetInput(0, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyNumber(const BytecodeInstruction *bc_inst)
{
    auto input = GetDefinitionAcc();
    auto type = input->GetType();
    if (input->IsConst() && !DataType::IsFloatType(type)) {
        auto const_insn = input->CastToConstant();
        if (const_insn->GetType() == DataType::INT64) {
            auto value = input->CastToConstant()->GetInt64Value();
            if (value == static_cast<uint32_t>(value)) {
                type = DataType::INT32;
            }
        }
    }

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto any_type = NumericDataTypeToAnyType(type, language);
    ASSERT(any_type != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bc_inst->GetAddress()));
    box->SetAnyType(any_type);
    box->SetInput(0, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_BUILDER_INST_BUILDER_INL_H
