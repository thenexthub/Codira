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

#include "module_constant_analyzer.h"
#include "bytecode_optimizer/constant_propagation/constant_value.h"

namespace panda::bytecodeopt {

ModuleConstantAnalyzer::ModuleConstantAnalyzer(compiler::Graph *graph,
                                               const std::unordered_set<uint32_t> &const_local_export_slots,
                                               ModuleConstantAnalysisResult &analysis_result,
                                               const BytecodeOptIrInterface *iface)
    : Analysis(graph),
      const_local_export_slots_(const_local_export_slots),
      analysis_result_(analysis_result),
      ir_interface_(iface)
{
}

bool ModuleConstantAnalyzer::RunImpl()
{
    VisitGraph();
    return true;
}

void ModuleConstantAnalyzer::VisitIntrinsic(compiler::GraphVisitor *visitor, compiler::Inst *inst)
{
    constexpr int STMODULEVAR_VALUE_INPUT_INDEX = 0;
    constexpr int STMODULEVAR_SLOT_NUMBER_IMM_INDEX = 0;
    auto module_constant_analyzer = static_cast<ModuleConstantAnalyzer *>(visitor);
    auto intrinsic_inst = inst->CastToIntrinsic();
    auto id = intrinsic_inst->GetIntrinsicId();
    switch (id) {
        case RuntimeInterface::IntrinsicId::STMODULEVAR_IMM8:
        case RuntimeInterface::IntrinsicId::WIDE_STMODULEVAR_PREF_IMM16: {
            int module_var_slot = intrinsic_inst->GetImms()[STMODULEVAR_SLOT_NUMBER_IMM_INDEX];
            if (!module_constant_analyzer->IsConstModuleVar(module_var_slot)) {
                break;
            }
            auto input_inst = inst->GetDataFlowInput(STMODULEVAR_VALUE_INPUT_INDEX);
            auto input_const_value = module_constant_analyzer->GetInstConstValue(input_inst);
            if (input_const_value != nullptr) {
                module_constant_analyzer->RecordModuleConstValue(module_var_slot, input_const_value);
            }
            break;
        }
        default:
            break;
    }
}

ConstantValue *ModuleConstantAnalyzer::GetInstConstValue(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::Constant:
            return GetConstantInstConstValue(inst->CastToConstant());
        case Opcode::Intrinsic:
            return GetIntrinsicInstConstValue(inst->CastToIntrinsic());
        case Opcode::LoadString:
            return GetLoadStringInstConstValue(inst->CastToLoadString());
        case Opcode::CastValueToAnyType:
            // According to inst_builder.cpp:
            // 1. the fldai bytecode generates Constant(double) -> CastValueToAnyType,
            // 1. the lda.str bytecode generates LoadString -> CastValueToAnyType,
            // We handle such case here. 0 stands for the DataFlowInput corresponding to the Constant/LoadString
            if (inst->GetDataFlowInput(0)->IsConst() ||
                inst->GetDataFlowInput(0)->GetOpcode() == compiler::Opcode::LoadString) {
                return GetInstConstValue(inst->GetDataFlowInput(0));
            }
            return nullptr;
        default:
            return nullptr;
    }
}

ConstantValue *ModuleConstantAnalyzer::GetConstantInstConstValue(compiler::ConstantInst *const_inst)
{
    switch (const_inst->GetType()) {
        case compiler::DataType::INT32: {
            auto val = static_cast<int32_t>(const_inst->GetInt32Value());
            return GetGraph()->GetAllocator()->New<ConstantValue>(val);
        }
        case compiler::DataType::INT64: {
            auto val = static_cast<int64_t>(const_inst->GetRawValue());
            return GetGraph()->GetAllocator()->New<ConstantValue>(val);
        }
        case compiler::DataType::FLOAT64: {
            auto val = const_inst->GetDoubleValue();
            return GetGraph()->GetAllocator()->New<ConstantValue>(val);
        }
        default:
            UNREACHABLE();
    }
}

ConstantValue *ModuleConstantAnalyzer::GetIntrinsicInstConstValue(compiler::IntrinsicInst *intrinsic_inst)
{
    switch (intrinsic_inst->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::LDTRUE:
        case RuntimeInterface::IntrinsicId::LDFALSE: {
            bool is_true = intrinsic_inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::LDTRUE;
            return GetGraph()->GetAllocator()->New<ConstantValue>(is_true);
        }
        default:
            return nullptr;
    }
}

ConstantValue *ModuleConstantAnalyzer::GetLoadStringInstConstValue(compiler::LoadFromPool *inst)
{
    return GetGraph()->GetAllocator()->New<ConstantValue>(
        ir_interface_->GetStringIdByOffset(inst->GetTypeId()));
}

bool ModuleConstantAnalyzer::IsConstModuleVar(uint32_t slot)
{
    return const_local_export_slots_.count(slot) == 1;
}

void ModuleConstantAnalyzer::RecordModuleConstValue(uint32_t slot, ConstantValue *value)
{
    analysis_result_.emplace(slot, value);
}

}  // namespace panda::bytecodeopt
