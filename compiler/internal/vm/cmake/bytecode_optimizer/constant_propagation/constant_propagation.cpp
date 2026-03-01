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

#include "constant_propagation.h"
#include "bytecode_optimizer/bytecode_analysis_results.h"

namespace panda::bytecodeopt {

enum class InputCount {
    NONE_INPUT,
    ONE_INPUT,
    TWO_INPUT,
};

ConstantPropagation::ConstantPropagation(compiler::Graph *graph, const BytecodeOptIrInterface *iface)
    : Optimization(graph),
      lattices_(graph->GetLocalAllocator()->Adapter()),
      flow_edges_(graph->GetLocalAllocator()->Adapter()),
      ssa_edges_(graph->GetLocalAllocator()->Adapter()),
      executable_flag_(graph->GetLocalAllocator()->Adapter()),
      executed_node_(graph->GetLocalAllocator()->Adapter()),
      ir_interface_(iface)
{
    std::string classname = GetGraph()->GetRuntime()->GetClassNameFromMethod(GetGraph()->GetMethod());
    record_name_ = pandasm::Type::FromDescriptor(classname).GetName();
}

bool ConstantPropagation::RunImpl()
{
    auto entry = GetGraph()->GetStartBlock();
    AddUntraversedFlowEdges(nullptr, entry);
    while (!flow_edges_.empty() || !ssa_edges_.empty()) {
        RunFlowEdge();
        RunSsaEdge();
    };
    ReWriteInst();
    return true;
}

void ConstantPropagation::RunFlowEdge()
{
    while (!flow_edges_.empty()) {
        auto &edge = flow_edges_.front();
        flow_edges_.pop();

        if (executable_flag_.count(edge) != 0) {
            continue;
        }
        executable_flag_.insert(edge);
        auto dest = edge.second;
        for (auto phi : dest->PhiInstsSafe()) {
            VisitInstruction(phi);
        }
        // If the destination basic block (BB) has already been visited, there is no need to process its non-phi
        // instructions.
        if (executed_node_.count(dest->GetId()) != 0) {
            continue;
        }
        executed_node_.insert(dest->GetId());
        VisitInsts(dest);
    }
}

void ConstantPropagation::RunSsaEdge()
{
    while (!ssa_edges_.empty()) {
        auto inst = ssa_edges_.front();
        ssa_edges_.pop();
        for (auto &user : inst->GetUsers()) {
            if (user.GetInst()->IsPhi()) {
                VisitInstruction(user.GetInst()->CastToPhi());
            } else if (executed_node_.count(user.GetInst()->GetBasicBlock()->GetId()) != 0) {
                VisitInstruction(user.GetInst());
            }
        }
    }
}

void ConstantPropagation::ReWriteInst()
{
    for (auto &pair : lattices_) {
        ASSERT(pair.second != nullptr);
        if (!pair.second->IsConstantElement() || pair.first->IsConst()) {
            continue;
        }
        // Skip the CastValueToAnyType instruction to preserve the original IR structure of
        // "LoadString/Constant(double) -> CastValueToAnyType".
        // This structure is necessary for certain GraphChecker checks.
        if (pair.first->GetOpcode() == compiler::Opcode::CastValueToAnyType) {
            continue;
        }
        // Skip generate replacement inst for string constants for now, since it requires generating
        // a string with a unique offset
        if (pair.second->AsConstant()->GetType() == ConstantValue::CONSTANT_STRING) {
            continue;
        }
        if (pair.first->IsIntrinsic()) {
            auto id = pair.first->CastToIntrinsic()->GetIntrinsicId();
            // Skip Intrinsic.ldtrue and Intrinsic.ldfalse since replacing them would create the same instruction.
            // Skip ldlocal/externalmodulevar and ldobjbyname in case of possible side effects when accessing module
            // constants.
            switch (id) {
                case compiler::RuntimeInterface::IntrinsicId::LDFALSE:
                case compiler::RuntimeInterface::IntrinsicId::LDTRUE:
                    pair.first->ClearFlag(compiler::inst_flags::NO_DCE);
                    continue;
                case RuntimeInterface::IntrinsicId::LDLOCALMODULEVAR_IMM8:
                case RuntimeInterface::IntrinsicId::WIDE_LDLOCALMODULEVAR_PREF_IMM16:
                case RuntimeInterface::IntrinsicId::LDEXTERNALMODULEVAR_IMM8:
                case RuntimeInterface::IntrinsicId::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16:
                case RuntimeInterface::IntrinsicId::LDOBJBYNAME_IMM8_ID16:
                case RuntimeInterface::IntrinsicId::LDOBJBYNAME_IMM16_ID16:
                    continue;
                default:
                    break;
            }
        }
        auto replace_inst = CreateReplaceInst(pair.first, pair.second->AsConstant());
        // This should never happen, but left here as a precaution for unexpected scenarios.
        if (replace_inst == nullptr) {
            continue;
        }
        pair.first->ReplaceUsers(replace_inst);
        pair.first->ClearFlag(compiler::inst_flags::NO_DCE);
    }
}

void ConstantPropagation::VisitInsts(BasicBlock *bb)
{
    for (auto inst : bb->AllInsts()) {
        if (inst->IsPhi()) {
            continue;
        }
        VisitInstruction(inst);
    }
    // If branch
    Inst *lastInst = bb->GetLastInst();
    if (lastInst && lastInst->GetOpcode() == Opcode::IfImm) {
        return;
    }
    auto &succs = bb->GetSuccsBlocks();
    for (auto succ : succs) {
        AddUntraversedFlowEdges(bb, succ);
    }
}

void ConstantPropagation::VisitDefault(Inst *inst)
{
    auto current = GetOrCreateDefaultLattice(inst);
    if (current->IsBottomElement()) {
        return;
    }
    CheckAndAddToSsaEdge(inst, current, BottomElement::GetInstance());
}

void ConstantPropagation::VisitIfImm(GraphVisitor *v, Inst *inst)
{
    ASSERT(v);
    ASSERT(inst && inst->GetOpcode() == Opcode::IfImm);
    auto propagation = static_cast<ConstantPropagation *>(v);
    auto input_inst = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input_lattice = propagation->GetOrCreateDefaultLattice(input_inst);
    auto bb = inst->GetBasicBlock();
    // According to inst_builder_gen.cpp.erb, the input of the IfImm IR is always a Compare IR generates DataType::BOOL.
    if (input_lattice->IsConstantElement() &&
        input_lattice->AsConstant()->GetType() == ConstantValue::CONSTANT_BOOL) {
        auto cst = static_cast<uint64_t>(input_lattice->AsConstant()->GetValue<bool>());
        auto dstBlock =
            propagation->GetIfTargetBlock(inst->CastToIfImm(), cst) ? bb->GetTrueSuccessor() : bb->GetFalseSuccessor();
        propagation->AddUntraversedFlowEdges(bb, dstBlock);
    } else {
        propagation->AddUntraversedFlowEdges(bb, bb->GetTrueSuccessor());
        propagation->AddUntraversedFlowEdges(bb, bb->GetFalseSuccessor());
    }
}

void ConstantPropagation::VisitCompare(GraphVisitor *v, Inst *inst)
{
    ASSERT(v);
    ASSERT(inst && inst->GetOpcode() == Opcode::Compare);
    auto propagation = static_cast<ConstantPropagation *>(v);
    auto current = propagation->GetOrCreateDefaultLattice(inst);
    if (current->IsBottomElement()) {
        return;
    }
    auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    auto lattices0 = propagation->GetOrCreateDefaultLattice(input0);
    auto lattices1 = propagation->GetOrCreateDefaultLattice(input1);
    LatticeElement *element = BottomElement::GetInstance();
    if (lattices0->IsConstantElement() && lattices1->IsConstantElement()) {
        element = propagation->FoldingCompare(lattices0->AsConstant(), lattices1->AsConstant(),
                                              inst->CastToCompare()->GetCc());
    }
    propagation->CheckAndAddToSsaEdge(inst, current, element);
}

void ConstantPropagation::VisitIntrinsic(GraphVisitor *v, Inst *inst)
{
    ASSERT(v);
    ASSERT(inst && inst->IsIntrinsic());
    auto propagation = static_cast<ConstantPropagation *>(v);
    auto current = propagation->GetOrCreateDefaultLattice(inst);
    if (current->IsBottomElement()) {
        return;
    }
    auto id = inst->CastToIntrinsic()->GetIntrinsicId();
    LatticeElement *element = propagation->FoldingModuleOperation(inst->CastToIntrinsic());
    if (element != nullptr) {
        propagation->CheckAndAddToSsaEdge(inst, current, element);
        return;
    }
    element = BottomElement::GetInstance();
    auto count = inst->GetInputsCount();
    if (inst->RequireState()) {
        count--;
    }
    auto input_count = static_cast<InputCount>(count);
    switch (input_count) {
        case InputCount::TWO_INPUT: {
            auto input0 = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
            auto input1 = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
            auto lattices0 = propagation->GetOrCreateDefaultLattice(input0);
            auto lattices1 = propagation->GetOrCreateDefaultLattice(input1);
            if (lattices0->IsConstantElement() && lattices1->IsConstantElement()) {
                element = propagation->FoldingConstant(lattices0->AsConstant(), lattices1->AsConstant(), id);
            }
            break;
        }
        case InputCount::ONE_INPUT: {
            auto input = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
            auto lattice = propagation->GetOrCreateDefaultLattice(input);
            if (lattice->IsConstantElement()) {
                element = propagation->FoldingConstant(lattice->AsConstant(), id);
            }
            break;
        }
        case InputCount::NONE_INPUT: {
            element = propagation->FoldingConstant(id);
            break;
        }
        default:
            break;
    }
    propagation->CheckAndAddToSsaEdge(inst, current, element);
}

void ConstantPropagation::VisitCastValueToAnyType(GraphVisitor *v, Inst *inst)
{
    ASSERT(v);
    ASSERT(inst && inst->GetOpcode() == Opcode::CastValueToAnyType);
    auto propagation = static_cast<ConstantPropagation *>(v);
    auto current = propagation->GetOrCreateDefaultLattice(inst);
    if (current->IsBottomElement()) {
        return;
    }
    LatticeElement *element = BottomElement::GetInstance();
    auto type = inst->CastToCastValueToAnyType()->GetAnyType();
    // According to inst_builder.cpp:
    // 1. The fldai bytecode will generate: Constant(double) -> CastValueToAnyType(ECMASCRIPT_DOUBLE_TYPE).
    // 2. The lda.str bytecode will generate: LoadString -> CastValueToAnyType(ECMASCRIPT_STRING_TYPE).
    // We create the lattice value of the original constant here and leave the IR unchanged.
    if (type == compiler::AnyBaseType::ECMASCRIPT_DOUBLE_TYPE) {
        ASSERT(inst->GetDataFlowInput(0)->IsConst());
        element = propagation->GetGraph()->GetLocalAllocator()->New<ConstantElement>(
            inst->GetDataFlowInput(0)->CastToConstant()->GetDoubleValue());
    } else if (type == compiler::AnyBaseType::ECMASCRIPT_STRING_TYPE) {
        ASSERT(inst->GetDataFlowInput(0)->GetOpcode() == compiler::Opcode::LoadString);
        auto load_string_inst = inst->GetDataFlowInput(0)->CastToLoadString();
        element = propagation->GetGraph()->GetLocalAllocator()->New<ConstantElement>(
            propagation->ir_interface_->GetStringIdByOffset(load_string_inst->GetTypeId()));
    }
    propagation->CheckAndAddToSsaEdge(inst, current, element);
}

void ConstantPropagation::VisitConstant(GraphVisitor *v, Inst *inst)
{
    ASSERT(v);
    ASSERT(inst && inst->IsConst());
    auto propagation = static_cast<ConstantPropagation *>(v);
    auto const_inst = inst->CastToConstant();
    auto current = propagation->GetOrCreateDefaultLattice(inst);
    if (current->IsBottomElement()) {
        return;
    }
    LatticeElement *element = BottomElement::GetInstance();
    switch (const_inst->GetType()) {
        case compiler::DataType::INT32: {
            auto val = static_cast<int32_t>(const_inst->GetInt32Value());
            element = propagation->GetGraph()->GetLocalAllocator()->New<ConstantElement>(val);
            break;
        }
        case compiler::DataType::INT64: {
            auto val = static_cast<int64_t>(const_inst->GetRawValue());
            element = propagation->GetGraph()->GetLocalAllocator()->New<ConstantElement>(val);
            break;
        }
        case compiler::DataType::FLOAT64: {
            auto val = const_inst->GetDoubleValue();
            element = propagation->GetGraph()->GetLocalAllocator()->New<ConstantElement>(val);
            break;
        }
        default:
            break;
    }
    propagation->CheckAndAddToSsaEdge(inst, current, element);
}

void ConstantPropagation::VisitLoadString(GraphVisitor *v, Inst *inst)
{
    ASSERT(v);
    ASSERT(inst && inst->GetOpcode() == compiler::Opcode::LoadString);
    auto propagation = static_cast<ConstantPropagation *>(v);
    auto load_string_inst = inst->CastToLoadString();
    auto current = propagation->GetOrCreateDefaultLattice(inst);
    if (current->IsBottomElement()) {
        return;
    }
    auto val = propagation->ir_interface_->GetStringIdByOffset(load_string_inst->GetTypeId());
    LatticeElement *element = propagation->GetGraph()->GetLocalAllocator()->New<ConstantElement>(val);
    propagation->CheckAndAddToSsaEdge(inst, current, element);
}

void ConstantPropagation::VisitPhi(GraphVisitor *v, Inst *inst)
{
    ASSERT(v);
    ASSERT(inst->IsPhi());
    auto propagation = static_cast<ConstantPropagation *>(v);
    auto current = propagation->GetOrCreateDefaultLattice(inst);
    if (current->IsBottomElement()) {
        return;
    }
    auto bb = inst->GetBasicBlock();
    auto &pres = bb->GetPredsBlocks();
    LatticeElement *element = TopElement::GetInstance();
    for (size_t i = 0; i < pres.size(); i++) {
        if (propagation->executable_flag_.count(Edge(pres[i], bb))) {
            auto input_lattice = propagation->GetOrCreateDefaultLattice(inst->GetInput(i).GetInst());
            element = element->Meet(input_lattice);
            if (element->IsBottomElement()) {
                break;
            }
        }
    }
    propagation->CheckAndAddToSsaEdge(inst, current, element);
}

void ConstantPropagation::AddUntraversedFlowEdges(BasicBlock *src, BasicBlock *dst)
{
    auto edge = Edge(src, dst);
    if (executable_flag_.count(edge) != 0) {
        return;
    }
    flow_edges_.push(edge);
}

bool ConstantPropagation::GetIfTargetBlock(compiler::IfImmInst *inst, uint64_t val)
{
    auto imm = inst->GetImm();
    auto cc = inst->GetCc();
    switch (cc) {
        case compiler::ConditionCode::CC_NE:
            return imm != val;
        case compiler::ConditionCode::CC_EQ:
            return imm == val;
        default:
            UNREACHABLE();
    }
    return false;
}

LatticeElement *ConstantPropagation::GetOrCreateDefaultLattice(Inst *inst)
{
    auto iter = lattices_.find(inst);
    if (iter == lattices_.end()) {
        lattices_.emplace(inst, TopElement::GetInstance());
        return TopElement::GetInstance();
    }
    return iter->second;
}

LatticeElement *ConstantPropagation::FoldingConstant(ConstantElement *left, ConstantElement *right,
                                                     RuntimeInterface::IntrinsicId id)
{
    switch (id) {
        case RuntimeInterface::IntrinsicId::GREATER_IMM8_V8:
        case RuntimeInterface::IntrinsicId::GREATEREQ_IMM8_V8: {
            return FoldingGreater(left, right, id == RuntimeInterface::IntrinsicId::GREATEREQ_IMM8_V8);
        }
        case RuntimeInterface::IntrinsicId::LESS_IMM8_V8:
        case RuntimeInterface::IntrinsicId::LESSEQ_IMM8_V8: {
            return FoldingLess(left, right, id == RuntimeInterface::IntrinsicId::LESSEQ_IMM8_V8);
        }
        case RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8:
        case RuntimeInterface::IntrinsicId::EQ_IMM8_V8: {
            return FoldingEq(left, right);
        }
        case RuntimeInterface::IntrinsicId::STRICTNOTEQ_IMM8_V8:
        case RuntimeInterface::IntrinsicId::NOTEQ_IMM8_V8: {
            return FoldingNotEq(left, right);
        }
        default:
            return BottomElement::GetInstance();
    }
}

LatticeElement *ConstantPropagation::FoldingModuleOperation(compiler::IntrinsicInst *inst)
{
    auto id = inst->GetIntrinsicId();
    switch (id) {
        case RuntimeInterface::IntrinsicId::LDLOCALMODULEVAR_IMM8:
        case RuntimeInterface::IntrinsicId::WIDE_LDLOCALMODULEVAR_PREF_IMM16:
            return FoldingLdlocalmodulevar(inst);
        case RuntimeInterface::IntrinsicId::LDEXTERNALMODULEVAR_IMM8:
        case RuntimeInterface::IntrinsicId::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16:
            return FoldingLdexternalmodulevar(inst);
        case RuntimeInterface::IntrinsicId::LDOBJBYNAME_IMM8_ID16:
        case RuntimeInterface::IntrinsicId::LDOBJBYNAME_IMM16_ID16:
            return FoldingLdobjbyname(inst);
        default:
            return nullptr;
    }
}

LatticeElement *ConstantPropagation::FoldingConstant(RuntimeInterface::IntrinsicId id)
{
    switch (id) {
        case RuntimeInterface::IntrinsicId::LDFALSE:
            return GetGraph()->GetLocalAllocator()->New<ConstantElement>(false);
        case RuntimeInterface::IntrinsicId::LDTRUE:
            return GetGraph()->GetLocalAllocator()->New<ConstantElement>(true);
        default:
            return BottomElement::GetInstance();
    }
}

LatticeElement *ConstantPropagation::FoldingConstant(ConstantElement *lattice, RuntimeInterface::IntrinsicId id)
{
    switch (id) {
        case RuntimeInterface::IntrinsicId::ISFALSE:
        case RuntimeInterface::IntrinsicId::ISTRUE:
        case RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8:
        case RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8: {
            if (lattice->GetType() != ConstantValue::CONSTANT_BOOL) {
                return BottomElement::GetInstance();
            }
            auto cst = lattice->GetValue<bool>();
            bool is_true_ins = (id == RuntimeInterface::IntrinsicId::ISTRUE) ||
                (id == RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8);
            return GetGraph()->GetLocalAllocator()->New<ConstantElement>(
                is_true_ins ? cst : !cst);
        }
        default:
            return BottomElement::GetInstance();
    }
}

LatticeElement *ConstantPropagation::FoldingCompare(const ConstantElement *left, const ConstantElement *right,
                                                    compiler::ConditionCode cc)
{
    ASSERT(left);
    ASSERT(right);
    // According to inst_builder_gen.cpp.erb, the input of the Compare IR are always ISTRUE/ISFALSE and a zero of i64.
    if (left->GetType() != ConstantValue::CONSTANT_BOOL || right->GetType() != ConstantValue::CONSTANT_INT64) {
        return BottomElement::GetInstance();
    }
    auto left_value = left->GetValue<bool>();
    auto right_value = right->GetValue<int64_t>();
    if ((right_value != 0) && (right_value != 1)) {
        return BottomElement::GetInstance();
    }
    switch (cc) {
        case compiler::CC_EQ: {
            return GetGraph()->GetLocalAllocator()->New<ConstantElement>(left_value == right_value);
        }
        case compiler::CC_NE: {
            return GetGraph()->GetLocalAllocator()->New<ConstantElement>(left_value != right_value);
        }
        default:
            return BottomElement::GetInstance();
    }
}

LatticeElement *ConstantPropagation::FoldingGreater(const ConstantElement *left, const ConstantElement *right,
                                                    bool equal)
{
    if (left->GetType() != right->GetType() || left->GetType() == ConstantValue::CONSTANT_STRING) {
        return BottomElement::GetInstance();
    }
    auto left_value = left->GetValue();
    auto right_value = right->GetValue();
    auto result = equal ? left_value >= right_value : left_value > right_value;
    return GetGraph()->GetLocalAllocator()->New<ConstantElement>(result);
}

LatticeElement *ConstantPropagation::FoldingLess(const ConstantElement *left, const ConstantElement *right, bool equal)
{
    if (left->GetType() != right->GetType() || left->GetType() == ConstantValue::CONSTANT_STRING) {
        return BottomElement::GetInstance();
    }
    auto left_value = left->GetValue();
    auto right_value = right->GetValue();
    auto result = equal ? left_value <= right_value : left_value < right_value;
    return GetGraph()->GetLocalAllocator()->New<ConstantElement>(result);
}

LatticeElement *ConstantPropagation::FoldingEq(const ConstantElement *left, const ConstantElement *right)
{
    if (left->GetType() != right->GetType()) {
        return BottomElement::GetInstance();
    }
    auto left_value = left->GetValue();
    auto right_value = right->GetValue();
    return GetGraph()->GetLocalAllocator()->New<ConstantElement>(left_value == right_value);
}

LatticeElement *ConstantPropagation::FoldingNotEq(const ConstantElement *left, const ConstantElement *right)
{
    if (left->GetType() != right->GetType()) {
        return BottomElement::GetInstance();
    }
    auto left_value = left->GetValue();
    auto right_value = right->GetValue();
    return GetGraph()->GetLocalAllocator()->New<ConstantElement>(left_value != right_value);
}

LatticeElement *ConstantPropagation::FoldingLdlocalmodulevar(compiler::IntrinsicInst *inst)
{
    constexpr uint32_t LOCAL_EXPORT_SLOT_IDX = 0;
    uint32_t local_export_slot = inst->GetImms()[LOCAL_EXPORT_SLOT_IDX];
    ConstantValue constant_value;
    if (bytecodeopt::BytecodeAnalysisResults::GetLocalExportConstForRecord(record_name_,
                                                                           local_export_slot,
                                                                           constant_value)) {
        return GetGraph()->GetLocalAllocator()->New<ConstantElement>(constant_value);
    }
    return BottomElement::GetInstance();
}

LatticeElement *ConstantPropagation::FoldingLdexternalmodulevar(compiler::IntrinsicInst *inst)
{
    constexpr uint32_t REGULAR_IMPORT_SLOT_IDX = 0;
    auto regular_import_slot = inst->GetImms()[REGULAR_IMPORT_SLOT_IDX];
    ConstantValue constant_value;
    if (bytecodeopt::BytecodeAnalysisResults::GetRegularImportConstForRecord(record_name_,
                                                                             regular_import_slot,
                                                                             constant_value)) {
        return GetGraph()->GetLocalAllocator()->New<ConstantElement>(constant_value);
    }
    return BottomElement::GetInstance();
}

LatticeElement *ConstantPropagation::FoldingLdobjbyname(compiler::IntrinsicInst *inst)
{
    constexpr uint32_t PROPERTY_NAME_STRING_OFFSET_IDX = 1;
    constexpr uint32_t MODULE_NAMESPACE_SLOT_IDX = 0;
    constexpr uint32_t OBJECT_INPUT_IDX = 0;
    Inst *object_inst = inst->GetDataFlowInput(OBJECT_INPUT_IDX);
    if (object_inst->IsIntrinsic()) {
        auto id = object_inst->CastToIntrinsic()->GetIntrinsicId();
        if (id == RuntimeInterface::IntrinsicId::GETMODULENAMESPACE_IMM8 ||
            id == RuntimeInterface::IntrinsicId::WIDE_GETMODULENAMESPACE_PREF_IMM16) {
            uint32_t property_name_offset = inst->GetImms()[PROPERTY_NAME_STRING_OFFSET_IDX];
            std::string property_name = ir_interface_->GetStringIdByOffset(property_name_offset);
            uint32_t module_namespace_slot = object_inst->CastToIntrinsic()->GetImms()[MODULE_NAMESPACE_SLOT_IDX];
            ConstantValue constant_value;
            if (bytecodeopt::BytecodeAnalysisResults::GetModuleNamespaceConstForRecord(record_name_,
                                                                                       module_namespace_slot,
                                                                                       property_name,
                                                                                       constant_value)) {
                return GetGraph()->GetLocalAllocator()->New<ConstantElement>(constant_value);
            }
        }
    }
    return BottomElement::GetInstance();
}

Inst *ConstantPropagation::CreateReplaceInst(Inst *base_inst, ConstantElement *lattice)
{
    auto const_type = lattice->GetType();
    Inst *replace_inst = nullptr;
    switch (const_type) {
        case ConstantValue::CONSTANT_BOOL: {
            auto inst_intrinsic_id = lattice->GetValue<bool>() ? RuntimeInterface::IntrinsicId::LDTRUE
                                                               : RuntimeInterface::IntrinsicId::LDFALSE;
            auto inst = GetGraph()->CreateInstIntrinsic(compiler::DataType::ANY, base_inst->GetPc(), inst_intrinsic_id);
            size_t args_count {1U};  // 1: input count
            inst->ReserveInputs(args_count);
            inst->AllocateInputTypes(GetGraph()->GetAllocator(), args_count);
            compiler::SaveStateInst *save_state_inst = GetGraph()->CreateInstSaveState();
            save_state_inst->SetPc(base_inst->GetPc());
            save_state_inst->SetMethod(GetGraph()->GetMethod());
            save_state_inst->ReserveInputs(0);
            inst->SetFlag(compiler::inst_flags::ACC_WRITE);
            inst->ClearFlag(compiler::inst_flags::NO_DCE);
            inst->AppendInput(save_state_inst);
            inst->AddInputType(compiler::DataType::NO_TYPE);
            InsertSaveState(base_inst, save_state_inst);
            base_inst->GetBasicBlock()->InsertAfter(inst, save_state_inst);
            replace_inst = inst;
            break;
        }
        case ConstantValue::CONSTANT_INT32: {
            replace_inst = GetGraph()->FindOrCreateConstant(static_cast<uint32_t>(lattice->GetValue<int32_t>()));
            break;
        }
        case ConstantValue::CONSTANT_INT64: {
            replace_inst = GetGraph()->FindOrCreateConstant(static_cast<uint64_t>(lattice->GetValue<int64_t>()));
            break;
        }
        case ConstantValue::CONSTANT_DOUBLE: {
            replace_inst = GetGraph()->FindOrCreateConstant(lattice->GetValue<double>());
            break;
        }
        default:
            UNREACHABLE();
    }
    ASSERT(replace_inst);
    return replace_inst;
}

void ConstantPropagation::InsertSaveState(Inst *base_inst, Inst *save_state)
{
    auto bb = base_inst->GetBasicBlock();
    if (!base_inst->IsPhi()) {
        bb->InsertAfter(save_state, base_inst);
        return;
    }
    auto first_inst = bb->Insts().begin();
    if (first_inst != bb->Insts().end()) {
        bb->InsertBefore(save_state, *first_inst);
    } else {
        bb->AppendInst(save_state);
    }
}

void ConstantPropagation::CheckAndAddToSsaEdge(Inst *inst, LatticeElement *current, LatticeElement *new_lattice)
{
    auto dst = current->Meet(new_lattice);
    if (dst != current) {
        ssa_edges_.push(inst);
        lattices_[inst] = dst;
    }
}

}  // namespace panda::bytecodeopt

