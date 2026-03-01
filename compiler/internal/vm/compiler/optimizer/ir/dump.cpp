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

#include "basicblock.h"
#include "graph.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "dump.h"

namespace panda::compiler {

// indent constants for dump instructions
static const int INDENT_ID = 6;
static const int INDENT_TYPE = 5;
static const int INDENT_OPCODE = 27;
static const int HEX_PTR_SIZE = sizeof(void *);

template <class T>
std::enable_if_t<std::is_integral_v<T>, ArenaString> ToArenaString(T value, ArenaAllocator *allocator)
{
    ArenaString res(std::to_string(value), allocator->Adapter());
    return res;
}

ArenaString GetId(uint32_t id, ArenaAllocator *allocator)
{
    return (id == INVALID_ID ? ArenaString("XX", allocator->Adapter()) : ToArenaString(id, allocator));
}

ArenaString IdToString(uint32_t id, ArenaAllocator *allocator, bool v_reg, bool is_phi)
{
    ArenaString reg(v_reg ? "v" : "", allocator->Adapter());
    ArenaString phi(is_phi ? "p" : "", allocator->Adapter());
    return reg + GetId(id, allocator) + phi;
}

// If print without brackets, then we print with space.
void PrintIfValidLocation(Location location, Arch arch, std::ostream *out, bool with_brackets = false)
{
    if (!location.IsInvalid() && !location.IsUnallocatedRegister()) {
        auto string = location.ToString(arch);
        if (with_brackets) {
            (*out) << "(" << string << ")";
        } else {
            (*out) << string << " ";
        }
    }
}

ArenaString InstId(const Inst *inst, ArenaAllocator *allocator)
{
    if (inst != nullptr) {
        if (inst->IsSaveState() && options.IsCompilerDumpCompact()) {
            return ArenaString("ss", allocator->Adapter()) +
                   ArenaString(std::to_string(inst->GetId()), allocator->Adapter());
        }
        return IdToString(static_cast<uint32_t>(inst->GetId()), allocator, true, inst->IsPhi());
    }
    ArenaString null("null", allocator->Adapter());
    return null;
}

ArenaString BBId(const BasicBlock *block, ArenaAllocator *allocator)
{
    if (block != nullptr) {
        return IdToString(static_cast<uint32_t>(block->GetId()), allocator);
    }
    ArenaString null("null", allocator->Adapter());
    return null;
}

void DumpUsers(const Inst *inst, std::ostream *out)
{
    auto allocator = inst->GetBasicBlock()->GetGraph()->GetLocalAllocator();
    auto arch = inst->GetBasicBlock()->GetGraph()->GetArch();
    for (size_t i = 0; i < inst->GetDstCount(); ++i) {
        PrintIfValidLocation(inst->GetDstLocation(), arch, out);
    }
    bool fl_first = true;
    for (auto &node_inst : inst->GetUsers()) {
        auto user = node_inst.GetInst();
        (*out) << (fl_first ? "(" : ", ") << InstId(user, allocator);
        if (fl_first) {
            fl_first = false;
        }
    }
    if (!fl_first) {
        (*out) << ')';
    }
}

ArenaString GetCondCodeToString(ConditionCode cc, ArenaAllocator *allocator)
{
    switch (cc) {
        case ConditionCode::CC_EQ:
            return ArenaString("EQ", allocator->Adapter());
        case ConditionCode::CC_NE:
            return ArenaString("NE", allocator->Adapter());

        case ConditionCode::CC_LT:
            return ArenaString("LT", allocator->Adapter());
        case ConditionCode::CC_LE:
            return ArenaString("LE", allocator->Adapter());
        case ConditionCode::CC_GT:
            return ArenaString("GT", allocator->Adapter());
        case ConditionCode::CC_GE:
            return ArenaString("GE", allocator->Adapter());

        case ConditionCode::CC_B:
            return ArenaString("B", allocator->Adapter());
        case ConditionCode::CC_BE:
            return ArenaString("BE", allocator->Adapter());
        case ConditionCode::CC_A:
            return ArenaString("A", allocator->Adapter());
        case ConditionCode::CC_AE:
            return ArenaString("AE", allocator->Adapter());

        case ConditionCode::CC_TST_EQ:
            return ArenaString("TST_EQ", allocator->Adapter());
        case ConditionCode::CC_TST_NE:
            return ArenaString("TST_NE", allocator->Adapter());
        default:
            UNREACHABLE();
    }
}

ArenaString PcToString(uint32_t pc, ArenaAllocator *allocator)
{
    std::ostringstream out_string;
    out_string << "bc: 0x" << std::setfill('0') << std::setw(HEX_PTR_SIZE) << std::hex << pc;
    return ArenaString(out_string.str(), allocator->Adapter());
}

void BBDependence(const char *type, const ArenaVector<BasicBlock *> &bb_vector, std::ostream *out,
                  ArenaAllocator *allocator)
{
    bool fl_first = true;
    (*out) << type << ": [";
    for (auto block_it : bb_vector) {
        (*out) << (fl_first ? "" : ", ") << "bb " << BBId(block_it, allocator);
        if (fl_first) {
            fl_first = false;
        }
    }
    (*out) << ']';
}

void DumpTypedFieldOpcode(std::ostream *out, Opcode opcode, uint32_t type_id, const ArenaString &field_name,
                          ArenaAllocator *allocator)
{
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opc(GetOpcodeString(opcode), adapter);
    ArenaString id(IdToString(type_id, allocator), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opc + space + id + space + field_name + space;
}

void DumpTypedOpcode(std::ostream *out, Opcode opcode, uint32_t type_id, ArenaAllocator *allocator)
{
    ArenaString space(" ", allocator->Adapter());
    ArenaString opc(GetOpcodeString(opcode), allocator->Adapter());
    ArenaString id(IdToString(type_id, allocator), allocator->Adapter());
    (*out) << std::setw(INDENT_OPCODE) << opc + space + id;
}

bool Inst::DumpInputs(std::ostream *out) const
{
    const auto &allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    auto arch = GetBasicBlock()->GetGraph()->GetArch();
    bool fl_first = true;
    unsigned i = 0;
    for (auto node_inst : GetInputs()) {
        Inst *input = node_inst.GetInst();
        (*out) << (fl_first ? "" : ", ") << InstId(input, allocator);
        PrintIfValidLocation(GetLocation(i), arch, out, true);
        i++;
        fl_first = false;
    }
    return !fl_first;
}

bool SaveStateInst::DumpInputs(std::ostream *out) const
{
    const auto &allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const char *sep = "";
    for (size_t i = 0; i < GetInputsCount(); i++) {
        (*out) << sep << std::dec << InstId(GetInput(i).GetInst(), allocator);
        if (GetVirtualRegister(i).IsAccumulator()) {
            (*out) << "(acc)";
        } else {
            (*out) << "(vr" << GetVirtualRegister(i).Value() << ")";
        }
        sep = ", ";
    }
    if (GetImmediatesCount() > 0) {
        for (auto imm : *GetImmediates()) {
            (*out) << sep << std::hex << "0x" << imm.value;
            if (imm.is_acc) {
                (*out) << "(acc)";
            } else {
                (*out) << std::dec << "(vr" << imm.vreg << ")";
            }
            sep = ", ";
        }
    }
    return true;
}

bool IfImmInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool PhiInst::DumpInputs(std::ostream *out) const
{
    const auto &allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    bool fl_first = true;
    for (size_t idx = 0; idx < GetInputsCount(); ++idx) {
        Inst *input = GetInput(idx).GetInst();
        auto block = GetPhiInputBb(idx);
        (*out) << (fl_first ? "" : ", ") << InstId(input, allocator) << "(bb" << BBId(block, allocator) << ")";
        if (fl_first) {
            fl_first = false;
        }
    }
    return !fl_first;
}

bool ConstantInst::DumpInputs(std::ostream *out) const
{
    switch (GetType()) {
        case DataType::Type::REFERENCE:
        case DataType::Type::BOOL:
        case DataType::Type::UINT8:
        case DataType::Type::INT8:
        case DataType::Type::UINT16:
        case DataType::Type::INT16:
        case DataType::Type::UINT32:
        case DataType::Type::INT32:
        case DataType::Type::UINT64:
        case DataType::Type::INT64:
            (*out) << "0x" << std::hex << GetIntValue() << std::dec;
            break;
        case DataType::Type::FLOAT32:
            (*out) << GetFloatValue();
            break;
        case DataType::Type::FLOAT64:
            (*out) << GetDoubleValue();
            break;
        case DataType::Type::ANY:
            (*out) << "0x" << std::hex << GetRawValue();
            break;
        default:
            UNREACHABLE();
    }
    return true;
}

bool SpillFillInst::DumpInputs(std::ostream *out) const
{
    bool first = true;
    for (auto spill_fill : GetSpillFills()) {
        if (!first) {
            (*out) << ", ";
        }
        first = false;
        (*out) << sf_data::ToString(spill_fill, GetBasicBlock()->GetGraph()->GetArch());
    }
    return true;
}

bool ParameterInst::DumpInputs(std::ostream *out) const
{
    (*out) << "arg " << IdToString(GetArgNumber(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
    return true;
}

void CompareInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetCondCodeToString(GetCc(), allocator), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

static void DumpOpcodeAnyTypeMixin(std::ostream &out, const Inst *inst)
{
    const auto *mixin_inst = static_cast<const AnyTypeMixin<FixedInputsInst1> *>(inst);
    ASSERT(mixin_inst != nullptr);
    auto allocator = mixin_inst->GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(mixin_inst->GetOpcode()), adapter);
    ArenaString any_base_type(AnyTypeTypeToString(mixin_inst->GetAnyType()), adapter);
    out << std::setw(INDENT_OPCODE) << opcode + space + any_base_type + space;
}

void CompareAnyTypeInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeAnyTypeMixin(*out, this);
}

void CastAnyTypeValueInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeAnyTypeMixin(*out, this);
}

void CastValueToAnyTypeInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeAnyTypeMixin(*out, this);
}

void IfInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetCondCodeToString(GetCc(), allocator), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

void IfImmInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetCondCodeToString(GetCc(), allocator), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

void CmpInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    auto type = GetOperandsType();
    ArenaString suffix = ArenaString(" ", adapter) + ArenaString(DataType::ToString(type), adapter);
    if (IsFloatType(type)) {
        (*out) << std::setw(INDENT_OPCODE) << ArenaString(IsFcmpg() ? "Fcmpg" : "Fcmpl", adapter) + suffix;
    } else if (IsTypeSigned(type)) {
        (*out) << std::setw(INDENT_OPCODE) << ArenaString("Cmp", adapter) + ArenaString(" ", adapter) + suffix;
    } else {
        (*out) << std::setw(INDENT_OPCODE) << ArenaString("Ucmp", adapter) + suffix;
    }
}

void LoadFromPool::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void IntrinsicInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString intrinsic(ArenaString("Intrinsic.", adapter));
    ArenaString opcode(GetIntrinsicOpcodeName(), adapter);
    (*out) << std::setw(INDENT_OPCODE) << intrinsic + opcode << " ";
}

void Inst::DumpOpcode(std::ostream *out) const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    (*out) << std::setw(INDENT_OPCODE) << GetOpcodeString(opcode_);
}

void Inst::Dump(std::ostream *out, bool new_line) const
{
    if (options.IsCompilerDumpCompact() && IsSaveState()) {
        return;
    }
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    // Id
    (*out) << std::setw(INDENT_ID) << std::setfill(' ') << std::right
           << IdToString(id_, allocator, false, IsPhi()) + '.';
    // Type
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    (*out) << std::setw(INDENT_TYPE) << std::left << DataType::ToString(GetType());
    // opcode
    DumpOpcode(out);
    auto operands_pos = out->tellp();
    // inputs
    bool has_input = DumpInputs(out);
    // users
    if (has_input && !GetUsers().Empty()) {
        (*out) << " -> ";
    }
    DumpUsers(this, out);
    // Align rest of the instruction info
    static constexpr auto ALIGN_BUF_SIZE = 64;
    if (auto pos_diff = out->tellp() - operands_pos; pos_diff < ALIGN_BUF_SIZE) {
        pos_diff = ALIGN_BUF_SIZE - pos_diff;
        static std::array<char, ALIGN_BUF_SIZE + 1> space_buf;
        if (space_buf[0] != ' ') {
            std::fill(space_buf.begin(), space_buf.end(), ' ');
        }
        space_buf[pos_diff] = 0;
        (*out) << space_buf.data();
        space_buf[pos_diff] = ' ';
    }
    // bytecode pointer
    if (pc_ != INVALID_PC && !options.IsCompilerDumpCompact()) {
        (*out) << ' ' << PcToString(pc_, allocator);
    }
    if (new_line) {
        (*out) << '\n';
    }
    if (GetOpcode() == Opcode::Parameter) {
        auto spill_fill = static_cast<const ParameterInst *>(this)->GetLocationData();
        if (spill_fill.DstValue() != INVALID_REG) {
            (*out) << sf_data::ToString(spill_fill, GetBasicBlock()->GetGraph()->GetArch());
            if (new_line) {
                *out << std::endl;
            }
        }
    }
}

void CheckPrintPropsFlag(std::ostream *out, bool *print_props_flag)
{
    if (!(*print_props_flag)) {
        (*out) << "prop: ";
        (*print_props_flag) = true;
    } else {
        (*out) << ", ";
    }
}

void BlockProps(const BasicBlock *block, std::ostream *out)
{
    bool print_props_flag = false;
    if (block->IsStartBlock()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "start";
    }
    if (block->IsEndBlock()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "end";
    }
    if (block->IsLoopValid() && !block->GetLoop()->IsRoot()) {
        if (block->IsLoopHeader()) {
            CheckPrintPropsFlag(out, &print_props_flag);
            (*out) << "head";
        }
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "loop" << (block->GetLoop()->IsIrreducible() ? " (irreducible) " : " ") << block->GetLoop()->GetId();
    }
    if (block->IsTryBegin()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "try_begin (id " << block->GetTryId() << ")";
    }
    if (block->IsTry()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "try (id " << block->GetTryId() << ")";
    }
    if (block->IsTryEnd()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "try_end (id " << block->GetTryId() << ")";
    }
    if (block->IsCatchBegin()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "catch_begin";
    }
    if (block->IsCatch()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "catch";
    }
    if (block->IsCatchEnd()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "catch_end";
    }

    if (block->GetGuestPc() != INVALID_PC) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << PcToString(block->GetGuestPc(), block->GetGraph()->GetLocalAllocator());
    }
    if (print_props_flag) {
        (*out) << std::endl;
    }
}

void BasicBlock::Dump(std::ostream *out) const
{
    const auto &allocator = GetGraph()->GetLocalAllocator();
    (*out) << "BB " << IdToString(bb_id_, allocator);
    // predecessors
    if (!preds_.empty()) {
        (*out) << "  ";
        BBDependence("preds", preds_, out, allocator);
    }
    (*out) << '\n';
    // properties
    BlockProps(this, out);
    // instructions
    for (auto inst : this->AllInsts()) {
        inst->Dump(out);
    }
    // successors
    if (!succs_.empty()) {
        BBDependence("succs", succs_, out, allocator);
        (*out) << '\n';
    }
}

void Graph::Dump(std::ostream *out) const
{
    const auto &runtime = GetRuntime();
    const auto &method = GetMethod();
    const auto &adapter = GetLocalAllocator()->Adapter();
    ArenaString return_type(DataType::ToString(DataType::Type::ANY), adapter);
    (*out) << "Method: " << runtime->GetMethodFullName(method, true) << std::endl;
    if (IsOsrMode()) {
        (*out) << "OSR mode\n";
    }
    (*out) << std::endl;

    auto &blocks = GetAnalysis<LinearOrder>().IsValid() ? GetBlocksLinearOrder() : GetBlocksRPO();
    for (const auto &block_it : blocks) {
        block_it->Dump(out);
        (*out) << '\n';
    }
}
}  // namespace panda::compiler
