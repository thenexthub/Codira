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

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "basicblock.h"
#include "compiler_options.h"
#include "inst.h"
#include "graph.h"
#include "dump.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/code_generator/target_info.h"

namespace ark::compiler {

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

ArenaString IdToString(uint32_t id, ArenaAllocator *allocator, bool vReg, bool isPhi)
{
    ArenaString reg(vReg ? "v" : "", allocator->Adapter());
    ArenaString phi(isPhi ? "p" : "", allocator->Adapter());
    return reg + GetId(id, allocator) + phi;
}

// If print without brackets, then we print with space.
void PrintIfValidLocation(Location location, Arch arch, std::ostream *out, bool withBrackets = false)
{
    if (!location.IsInvalid() && !location.IsUnallocatedRegister()) {
        auto string = location.ToString(arch);
        if (withBrackets) {
            (*out) << "(" << string << ")";
        } else {
            (*out) << string << " ";
        }
    }
}

ArenaString InstId(const Inst *inst, ArenaAllocator *allocator)
{
    if (inst != nullptr) {
        if (inst->IsSaveState() && g_options.IsCompilerDumpCompact()) {
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
        PrintIfValidLocation(inst->GetDstLocation(i), arch, out);
    }
    bool flFirst = true;
    for (auto &nodeInst : inst->GetUsers()) {
        auto user = nodeInst.GetInst();
        (*out) << (flFirst ? "(" : ", ") << InstId(user, allocator);
        if (flFirst) {
            flFirst = false;
        }
    }
    if (!flFirst) {
        (*out) << ')';
    }
}

ArenaString PcToString(uint32_t pc, ArenaAllocator *allocator)
{
    std::ostringstream outString;
    outString << "bc: 0x" << std::setfill('0') << std::setw(HEX_PTR_SIZE) << std::hex << pc;
    return ArenaString(outString.str(), allocator->Adapter());
}

template <typename T>
void BBDependence(const char *type, const T &bbVector, std::ostream *out, ArenaAllocator *allocator)
{
    bool flFirst = true;
    (*out) << type << ": [";
    for (auto blockIt : bbVector) {
        (*out) << (flFirst ? "" : ", ") << "bb " << BBId(blockIt, allocator);
        if (flFirst) {
            flFirst = false;
        }
    }
    (*out) << ']';
}

ArenaString FieldToString(RuntimeInterface *runtime, ObjectType type, RuntimeInterface::FieldPtr field,
                          ArenaAllocator *allocator)
{
    const auto &adapter = allocator->Adapter();
    if (type != ObjectType::MEM_STATIC && type != ObjectType::MEM_OBJECT) {
        return ArenaString(ObjectTypeToString(type), adapter);
    }

    if (!runtime->HasFieldMetadata(field)) {
        auto offset = runtime->GetFieldOffset(field);
        return ArenaString("Unknown.Unknown", adapter) + ArenaString(std::to_string(offset), adapter);
    }

    ArenaString dot(".", adapter);
    ArenaString clsName(runtime->GetClassName(runtime->GetClassForField(field)), adapter);
    ArenaString fieldName(runtime->GetFieldName(field), adapter);
    return clsName + dot + fieldName;
}

void DumpTypedFieldOpcode(std::ostream *out, Opcode opcode, uint32_t typeId, const ArenaString &fieldName,
                          ArenaAllocator *allocator)
{
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opc(GetOpcodeString(opcode), adapter);
    ArenaString id(IdToString(typeId, allocator), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opc + space + id + space + fieldName + space;
}

void DumpTypedOpcode(std::ostream *out, Opcode opcode, uint32_t typeId, ArenaAllocator *allocator)
{
    ArenaString space(" ", allocator->Adapter());
    ArenaString opc(GetOpcodeString(opcode), allocator->Adapter());
    ArenaString id(IdToString(typeId, allocator), allocator->Adapter());
    (*out) << std::setw(INDENT_OPCODE) << opc + space + id + space;
}

void DumpTypedOpcode(std::ostream *out, Opcode opcode, uint32_t typeId, const ArenaString &flags,
                     ArenaAllocator *allocator)
{
    ArenaString space(" ", allocator->Adapter());
    ArenaString opc(GetOpcodeString(opcode), allocator->Adapter());
    ArenaString id(IdToString(typeId, allocator), allocator->Adapter());
    (*out) << std::setw(INDENT_OPCODE) << opc + flags + space + id + space;
}

bool Inst::DumpInputs(std::ostream *out) const
{
    const auto &allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    auto arch = GetBasicBlock()->GetGraph()->GetArch();
    bool flFirst = true;
    unsigned i = 0;
    for (auto nodeInst : GetInputs()) {
        Inst *input = nodeInst.GetInst();
        (*out) << (flFirst ? "" : ", ") << InstId(input, allocator);
        PrintIfValidLocation(GetLocation(i), arch, out, true);
        i++;
        flFirst = false;
    }

    if (!GetTmpLocation().IsInvalid()) {
        (*out) << (flFirst ? "" : ", ") << "Tmp(" << GetTmpLocation().ToString(arch) << ")";
    }

    return !flFirst;
}

bool SaveStateInst::DumpInputs(std::ostream *out) const
{
    const auto &allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const char *sep = "";
    for (size_t i = 0; i < GetInputsCount(); i++) {
        (*out) << sep << std::dec << InstId(GetInput(i).GetInst(), allocator);
        if (GetVirtualRegister(i).IsSpecialReg()) {
            (*out) << "(" << VRegInfo::VRegTypeToString(GetVirtualRegister(i).GetVRegType()) << ")";
        } else if (GetVirtualRegister(i).IsBridge()) {
            (*out) << "(bridge)";
        } else {
            (*out) << "(vr" << GetVirtualRegister(i).Value() << ")";
        }
        sep = ", ";
    }
    if (GetImmediatesCount() > 0) {
        for (auto imm : *GetImmediates()) {
            (*out) << sep << std::hex << "0x" << imm.value;
            if (imm.vregType == VRegType::VREG) {
                (*out) << std::dec << "(vr" << imm.vreg << ")";
            } else {
                (*out) << "(" << VRegInfo::VRegTypeToString(imm.vregType) << ")";
            }
            sep = ", ";
        }
    }
    if (GetCallerInst() != nullptr) {
        (*out) << sep << "caller=" << GetCallerInst()->GetId();
    }
    (*out) << sep << "inlining_depth=" << GetInliningDepth();
    return true;
}

bool BinaryImmOperation::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool BinaryShiftedRegisterOperation::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", " << GetShiftTypeStr(GetShiftType()) << " 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool ExtractBitfieldInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", " << sourceBit_ << ", " << width_;
    (*out) << " [" << (signExt_ ? "SignExt" : "ZeroExt") << ']';
    return true;
}

bool UnaryShiftedRegisterOperation::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", " << GetShiftTypeStr(GetShiftType()) << " 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool SelectImmInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
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
    bool flFirst = true;
    for (size_t idx = 0; idx < GetInputsCount(); ++idx) {
        Inst *input = GetInput(idx).GetInst();
        auto block = GetPhiInputBb(idx);
        (*out) << (flFirst ? "" : ", ") << InstId(input, allocator) << "(bb" << BBId(block, allocator) << ")";
        if (flFirst) {
            flFirst = false;
        }
    }
    return !flFirst;
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
            (*out) << "0x" << std::hex << GetRawValue() << std::dec;
            break;
        default:
            UNREACHABLE();
    }
    return true;
}

bool SpillFillInst::DumpInputs(std::ostream *out) const
{
    bool first = true;
    for (auto spillFill : GetSpillFills()) {
        if (!first) {
            (*out) << ", ";
        }
        first = false;
        (*out) << sf_data::ToString(spillFill, GetBasicBlock()->GetGraph()->GetArch());
    }
    return true;
}

bool ParameterInst::DumpInputs(std::ostream *out) const
{
    auto argNum = GetArgNumber();
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    ArenaString nums("nums", allocator->Adapter());
    (*out) << "arg " << ((argNum == ParameterInst::DYNAMIC_NUM_ARGS) ? nums : IdToString(argNum, allocator));
    return true;
}

void CompareInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetConditionCodeString(GetCc()), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

static void DumpOpcodeAnyTypeMixin(std::ostream &out, const Inst *inst)
{
    const auto *mixinInst = static_cast<const AnyTypeMixin<FixedInputsInst1> *>(inst);
    ASSERT(mixinInst != nullptr);
    auto allocator = mixinInst->GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(mixinInst->GetOpcode()), adapter);
    ArenaString anyBaseType(AnyTypeTypeToString(mixinInst->GetAnyType()), adapter);
    out << std::setw(INDENT_OPCODE)
        << opcode + space + anyBaseType + (mixinInst->IsIntegerWasSeen() ? " i" : "") +
               (mixinInst->IsSpecialWasSeen() ? " s" : "") + (mixinInst->IsTypeWasProfiled() ? " p" : "") + space;
}

void PhiInst::DumpOpcode(std::ostream *out) const
{
    if (GetBasicBlock()->GetGraph()->IsDynamicMethod()) {
        DumpOpcodeAnyTypeMixin(*out, this);
    } else {
        Inst::DumpOpcode(out);
    }
}

void CompareAnyTypeInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeAnyTypeMixin(*out, this);
}

void GetAnyTypeNameInst::DumpOpcode(std::ostream *out) const
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

void AnyTypeCheckInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString anyBaseType(AnyTypeTypeToString(GetAnyType()), adapter);
    (*out) << std::setw(INDENT_OPCODE)
           << (opcode + space + anyBaseType + (IsIntegerWasSeen() ? " i" : "") + (IsSpecialWasSeen() ? " s" : "") +
               (IsTypeWasProfiled() ? " p" : "") + space);
}

void HclassCheckInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString open("[", adapter);
    ArenaString close("]", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    bool isFirst = true;
    ArenaString summary = opcode + space + open;
    if (GetCheckIsFunction()) {
        summary += ArenaString("IsFunc", adapter);
        isFirst = false;
    }
    if (GetCheckFunctionIsNotClassConstructor()) {
        if (!isFirst) {
            summary += ArenaString(", ", adapter);
        }
        summary += ArenaString("IsNotClassConstr", adapter);
    }
    summary += close + space;
    (*out) << std::setw(INDENT_OPCODE) << summary;
}

void LoadImmediateInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString open("(", adapter);
    ArenaString close(") ", adapter);
    if (IsClass()) {
        ArenaString type("class: ", adapter);
        ArenaString className(GetBasicBlock()->GetGraph()->GetRuntime()->GetClassName(GetObject()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type + className + close;
    } else if (IsMethod()) {
        ArenaString type("method: ", adapter);
        ArenaString methodName(GetBasicBlock()->GetGraph()->GetRuntime()->GetMethodName(GetObject()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type + methodName + close;
    } else if (IsString()) {
        ArenaString type("string: 0x", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type << std::hex << GetString() << close;
    } else if (IsPandaFileOffset()) {
        ArenaString type("PandaFileOffset: ", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type << std::hex << GetPandaFileOffset() << close;
    } else if (IsConstantPool()) {
        ArenaString type("constpool: 0x", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type << std::hex << GetConstantPool() << close;
    } else if (IsObject()) {
        ArenaString type("object: 0x", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type << std::hex << GetObject() << close;
    } else if (IsTlsOffset()) {
        ArenaString type("TlsOffset: 0x", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type << std::hex << GetTlsOffset() << close;
    } else {
        UNREACHABLE();
    }
}

void FunctionImmediateInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString prefix(" 0x", adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode << prefix << std::hex << GetFunctionPtr() << " ";
}

void LoadObjFromConstInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString prefix(" 0x", adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode << prefix << std::hex << GetObjPtr() << " ";
}

static ArenaString MakeOpcodeStringForSelectInsts(ArenaAllocator *allocator, Opcode opc, ConditionCode cc,
                                                  DataType::Type type)
{
    ArenaString res(allocator->Adapter());
    res.reserve(INDENT_OPCODE);
    res += GetOpcodeString(opc);
    res += ' ';
    res += GetConditionCodeString(cc);
    res += ' ';
    res += DataType::ToString(type);
    return res;
}

template <typename InstType>
static void DumpOpcodeForSelectInsts(const InstType *inst, std::ostream *out)
{
    ArenaAllocator *allocator = inst->GetBasicBlock()->GetGraph()->GetLocalAllocator();
    auto str = MakeOpcodeStringForSelectInsts(allocator, inst->GetOpcode(), inst->GetCc(), inst->GetOperandsType());
    (*out) << std::setw(INDENT_OPCODE) << str;
}

template <typename InstType>
static void DumpOpcodeForSelectTransformInsts(const InstType *inst, std::ostream *out)
{
    ArenaAllocator *allocator = inst->GetBasicBlock()->GetGraph()->GetLocalAllocator();
    auto str = MakeOpcodeStringForSelectInsts(allocator, inst->GetOpcode(), inst->GetCc(), inst->GetOperandsType());
    str += ' ';
    str += GetSelectTransformTypeString(inst->GetSelectTransformType());
    (*out) << std::setw(INDENT_OPCODE) << str;
}

void SelectInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeForSelectInsts(this, out);
}

void SelectTransformInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeForSelectTransformInsts(this, out);
}

void SelectImmInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeForSelectInsts(this, out);
}

void SelectImmTransformInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeForSelectTransformInsts(this, out);
}

void IfInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetConditionCodeString(GetCc()), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

void IfImmInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetConditionCodeString(GetCc()), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

void MonitorInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString suffix(IsExit() ? ".Exit" : ".Entry", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + suffix;
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

void CastInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString space(" ", adapter);
    (*out) << std::setw(INDENT_OPCODE)
           << (ArenaString(GetOpcodeString(GetOpcode()), adapter) + space +
               ArenaString(DataType::ToString(GetOperandsType()), adapter));
}

void NewObjectInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void NewArrayInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    auto adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opc(GetOpcodeString(GetOpcode()), adapter);
    ArenaString id(IdToString(GetTypeId(), allocator), adapter);
    ArenaString size("", adapter);
    ASSERT(GetInputsCount() > 1);
    auto sizeInst = GetDataFlowInput(1);
    if (sizeInst->IsConst()) {
        auto sizeValue = sizeInst->CastToConstant()->GetIntValue();
        size = ArenaString("(size=", adapter) + ToArenaString(sizeValue, allocator) + ")";
    }
    (*out) << std::setw(INDENT_OPCODE) << opc + space + size + space + id + space;
}

void LoadConstArrayInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void FillConstArrayInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadObjectInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto fieldName = FieldToString(graph->GetRuntime(), GetObjectType(), GetObjField(), graph->GetLocalAllocator());
    DumpTypedFieldOpcode(out, GetOpcode(), GetTypeId(), fieldName, graph->GetLocalAllocator());
}

class ObjectPairParams {
public:
    const Graph *graph;
    Opcode opc;
    RuntimeInterface::FieldPtr field0;
    RuntimeInterface::FieldPtr field1;
    uint32_t typeId0;
    uint32_t typeId1;
};

void DumpObjectPairOpcode(std::ostream *out, ObjectPairParams &params)
{
    auto graph = params.graph;
    auto runtime = graph->GetRuntime();
    auto *allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();

    auto field0 = params.field0;
    auto field1 = params.field1;

    ArenaString space(" ", adapter);
    ArenaString dot(".", adapter);

    ArenaString clsName("", adapter);
    ArenaString fieldName0("", adapter);
    ArenaString fieldName1("", adapter);

    ArenaString id0(IdToString(params.typeId0, allocator), adapter);
    auto offset0 = space + ArenaString(std::to_string(runtime->GetFieldOffset(field0)), adapter);
    ArenaString id1(IdToString(params.typeId1, allocator), adapter);
    auto offset1 = space + ArenaString(std::to_string(runtime->GetFieldOffset(field1)), adapter);
    if (!runtime->HasFieldMetadata(field0)) {
        clsName = ArenaString("Unknown ", adapter);
        fieldName0 = id0 + space + dot + ArenaString(".Unknown", adapter) + offset0;
        fieldName1 = id1 + space + dot + ArenaString(".Unknown", adapter) + offset1;
    } else {
        clsName = ArenaString(runtime->GetClassName(runtime->GetClassForField(field0)), adapter) + space;
        fieldName0 = id0 + space + dot + ArenaString(runtime->GetFieldName(field0), adapter) + offset0;
        fieldName1 = id1 + space + dot + ArenaString(runtime->GetFieldName(field1), adapter) + offset1;
    }
    ArenaString opc(GetOpcodeString(params.opc), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opc + space + clsName + space + fieldName0 + space + fieldName1 + space;
}

void LoadObjectPairInst::DumpOpcode(std::ostream *out) const
{
    ObjectPairParams params {
        GetBasicBlock()->GetGraph(), GetOpcode(), GetObjField0(), GetObjField1(), GetTypeId0(), GetTypeId1()};
    DumpObjectPairOpcode(out, params);
}

void StoreObjectPairInst::DumpOpcode(std::ostream *out) const
{
    ObjectPairParams params {
        GetBasicBlock()->GetGraph(), GetOpcode(), GetObjField0(), GetObjField1(), GetTypeId0(), GetTypeId1()};
    DumpObjectPairOpcode(out, params);
}

void LoadMemInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetType(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void ResolveObjectFieldInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadResolvedObjectFieldInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void StoreObjectInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto fieldName = FieldToString(graph->GetRuntime(), GetObjectType(), GetObjField(), graph->GetLocalAllocator());
    DumpTypedFieldOpcode(out, GetOpcode(), GetTypeId(), fieldName, graph->GetLocalAllocator());
}

void StoreResolvedObjectFieldInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void StoreMemInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetType(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadStaticInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto fieldName =
        FieldToString(graph->GetRuntime(), ObjectType::MEM_STATIC, GetObjField(), graph->GetLocalAllocator());
    DumpTypedFieldOpcode(out, GetOpcode(), GetTypeId(), fieldName, graph->GetLocalAllocator());
}

void ResolveObjectFieldStaticInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadResolvedObjectFieldStaticInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void StoreStaticInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto fieldName =
        FieldToString(graph->GetRuntime(), ObjectType::MEM_STATIC, GetObjField(), graph->GetLocalAllocator());
    DumpTypedFieldOpcode(out, GetOpcode(), GetTypeId(), fieldName, graph->GetLocalAllocator());
}

void UnresolvedStoreStaticInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void StoreResolvedObjectFieldStaticInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadFromPool::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadFromPoolDynamic::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void ClassInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();

    ArenaString opc(GetOpcodeString(GetOpcode()), adapter);
    ArenaString space(" ", adapter);
    ArenaString qt("'", adapter);
    ArenaString className(GetClass() == nullptr ? ArenaString("", adapter)
                                                : ArenaString(graph->GetRuntime()->GetClassName(GetClass()), adapter));
    (*out) << std::setw(INDENT_OPCODE) << opc + space + qt + className + qt << " ";
}

void RuntimeClassInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();

    ArenaString space(" ", adapter);
    ArenaString qt("'", adapter);
    ArenaString opc(GetOpcodeString(GetOpcode()), adapter);
    ArenaString className(GetClass() == nullptr ? ArenaString("", adapter)
                                                : ArenaString(graph->GetRuntime()->GetClassName(GetClass()), adapter));
    (*out) << std::setw(INDENT_OPCODE) << opc + space + qt + className + qt << " ";
}

void GlobalVarInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void CheckCastInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString flags("", adapter);
    if (CanDeoptimize()) {
        flags = " D";
    }
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), flags, allocator);
}

void IsInstanceInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void IntrinsicInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString intrinsic(IsBuiltin() ? ArenaString("BuiltinIntrinsic.", adapter) : ArenaString("Intrinsic.", adapter));
    ArenaString opcode(GetIntrinsicName(intrinsicId_), adapter);
    (*out) << std::setw(INDENT_OPCODE) << intrinsic + opcode << " ";
}

void Inst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString flags("", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    if (CanDeoptimize()) {
        flags += "D";
    }
    if (GetFlag(inst_flags::MEM_BARRIER)) {
        static constexpr auto FENCE = "F";
        flags += FENCE;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + flags;
}

namespace {
inline std::optional<uint32_t> GetMethodFileIdIfAotMode(const Graph *graph, RuntimeInterface::MethodPtr method)
{
    if (method != nullptr && graph->IsAotMode()) {
        return graph->GetRuntime()->GetAOTBinaryFileSnapshotIndexForMethod(method);
    }
    return std::nullopt;
}

void DumpMethodFileInfo(std::ostream *out, std::optional<uint32_t> fileId)
{
    if (fileId.has_value()) {
        (*out) << " [file=" << fileId.value() << ']';
    }
    (*out) << ' ';
}
}  // unnamed namespace

void ResolveStaticInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString methodId(ToArenaString(GetCallMethodId(), allocator));
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    if (GetCallMethod() != nullptr) {
        ArenaString method(graph->GetRuntime()->GetMethodFullName(GetCallMethod()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + ' ' + methodId + ' ' + method;
    } else {
        (*out) << std::setw(INDENT_OPCODE) << opcode + ' ' + methodId;
    }
    DumpMethodFileInfo(out, GetMethodFileIdIfAotMode(graph, GetCallMethod()));
}

void ResolveVirtualInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString methodId(ToArenaString(GetCallMethodId(), allocator));
    if (GetCallMethod() != nullptr) {
        ArenaString method(graph->GetRuntime()->GetMethodFullName(GetCallMethod()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + ' ' + methodId + ' ' + method;
    } else {
        (*out) << std::setw(INDENT_OPCODE) << opcode + ' ' + methodId;
    }
    DumpMethodFileInfo(out, GetMethodFileIdIfAotMode(graph, GetCallMethod()));
}

void InitStringInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    if (IsFromString()) {
        ArenaString mode(" FromString", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + mode << ' ';
    } else {
        ASSERT(IsFromCharArray());
        ArenaString mode(" FromCharArray", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + mode << ' ';
    }
}

void CallInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString inlined(IsInlined() ? ".Inlined " : " ", adapter);
    ArenaString methodId(ToArenaString(GetCallMethodId(), allocator));
    if (!IsUnresolved() && GetCallMethod() != nullptr) {
        ArenaString method(graph->GetRuntime()->GetMethodFullName(GetCallMethod()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + inlined + methodId + ' ' + method;
    } else {
        (*out) << std::setw(INDENT_OPCODE) << opcode + inlined + methodId;
    }
    DumpMethodFileInfo(out, GetMethodFileIdIfAotMode(graph, GetCallMethod()));
}

void DeoptimizeInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString type(DeoptimizeTypeToString(GetDeoptimizeType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + ArenaString(" ", adapter) + type << ' ';
}

void DeoptimizeIfInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString type(DeoptimizeTypeToString(GetDeoptimizeType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + ArenaString(" ", adapter) + type << ' ';
}

void DeoptimizeCompareInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(ArenaString(GetOpcodeString(GetOpcode()), adapter).append(" "));
    ArenaString cc(ArenaString(GetConditionCodeString(GetCc()), adapter).append(" "));
    ArenaString cmpType(ArenaString(DataType::ToString(GetOperandsType()), adapter).append(" "));
    ArenaString type(ArenaString(DeoptimizeTypeToString(GetDeoptimizeType()), adapter).append(" "));
    (*out) << std::setw(INDENT_OPCODE) << opcode.append(cc).append(cmpType).append(type);
}

void DeoptimizeCompareImmInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(ArenaString(GetOpcodeString(GetOpcode()), adapter).append(" "));
    ArenaString cc(ArenaString(GetConditionCodeString(GetCc()), adapter).append(" "));
    ArenaString type(ArenaString(DeoptimizeTypeToString(GetDeoptimizeType()), adapter).append(" "));
    ArenaString cmpType(ArenaString(DataType::ToString(GetOperandsType()), adapter).append(" "));
    (*out) << std::setw(INDENT_OPCODE) << opcode.append(cc).append(cmpType).append(type);
}

bool DeoptimizeCompareImmInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool BoundsCheckInstI::DumpInputs(std::ostream *out) const
{
    Inst *lenInput = GetInput(0).GetInst();
    Inst *ssInput = GetInput(1).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();

    (*out) << InstId(lenInput, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    (*out) << ", " << InstId(ssInput, allocator);
    return true;
}

bool StoreInstI::DumpInputs(std::ostream *out) const
{
    Inst *arrInput = GetInput(0).GetInst();
    Inst *ssInput = GetInput(1).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    auto arch = graph->GetArch();
    const auto &allocator = graph->GetLocalAllocator();

    (*out) << InstId(arrInput, allocator);
    PrintIfValidLocation(GetLocation(0), arch, out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    (*out) << ", " << InstId(ssInput, allocator);
    PrintIfValidLocation(GetLocation(1), arch, out, true);
    return true;
}

bool StoreMemInstI::DumpInputs(std::ostream *out) const
{
    Inst *arrInput = GetInput(0).GetInst();
    Inst *ssInput = GetInput(1).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    const auto &allocator = graph->GetLocalAllocator();

    (*out) << InstId(arrInput, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    (*out) << ", " << InstId(ssInput, allocator);
    return true;
}

bool LoadInstI::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool LoadMemInstI::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool LoadMemInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    if (GetScale() != 0) {
        (*out) << " Scale " << GetScale();
    }
    return true;
}

bool StoreMemInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    if (GetScale() != 0) {
        (*out) << " Scale " << GetScale();
    }
    return true;
}

bool LoadPairPartInst::DumpInputs(std::ostream *out) const
{
    Inst *arrInput = GetInput(0).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    const auto &allocator = graph->GetLocalAllocator();

    (*out) << InstId(arrInput, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool LoadArrayPairInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    if (GetImm() > 0) {
        (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    }
    return true;
}

bool LoadArrayPairInstI::DumpInputs(std::ostream *out) const
{
    Inst *arrInput = GetInput(0).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    const auto &allocator = graph->GetLocalAllocator();
    (*out) << InstId(arrInput, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool StoreArrayPairInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    if (GetImm() > 0) {
        (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    }
    return true;
}

bool StoreArrayPairInstI::DumpInputs(std::ostream *out) const
{
    Inst *arrInput = GetInput(0).GetInst();
    Inst *fssInput = GetInput(1).GetInst();
    constexpr auto IMM_2 = 2;
    Inst *sssInput = GetInput(IMM_2).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();

    (*out) << InstId(arrInput, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    (*out) << ", " << InstId(fssInput, allocator);
    (*out) << ", " << InstId(sssInput, allocator);
    return true;
}

bool ReturnInstI::DumpInputs(std::ostream *out) const
{
    (*out) << "0x" << std::hex << GetImm() << std::dec;
    return true;
}

void IntrinsicInst::DumpImms(std::ostream *out) const
{
    if (!HasImms()) {
        return;
    }

    const auto &imms = GetImms();
    ASSERT(!imms.empty());
    (*out) << "0x" << std::hex << imms[0U];
    for (size_t i = 1U; i < imms.size(); ++i) {
        (*out) << ", 0x" << imms[i];
    }
    (*out) << ' ' << std::dec;
}

bool IntrinsicInst::DumpInputs(std::ostream *out) const
{
    DumpImms(out);
    return Inst::DumpInputs(out);
}

void Inst::DumpBytecode(std::ostream *out) const
{
    if (pc_ != INVALID_PC) {
        auto graph = GetBasicBlock()->GetGraph();
        auto byteCode = graph->GetRuntime()->GetBytecodeString(graph->GetMethod(), pc_);
        if (!byteCode.empty()) {
            (*out) << byteCode << '\n';
        }
    }
}

#ifdef PANDA_COMPILER_DEBUG_INFO
void Inst::DumpSourceLine(std::ostream *out) const
{
    auto currentMethod = GetCurrentMethod();
    auto pc = GetPc();
    if (currentMethod != nullptr && pc != INVALID_PC) {
        auto line = GetBasicBlock()->GetGraph()->GetRuntime()->GetLineNumberAndSourceFile(currentMethod, pc);
        (*out) << " (" << line << " )";
    }
}
#endif  // PANDA_COMPILER_DEBUG_INFO

void Inst::Dump(std::ostream *out, bool newLine) const
{
    // issue: #25677 - if GetBasicBlock() returns nullptr there will be segfault in GetBasicBlock()->GetGraph()
    if (g_options.IsCompilerDumpCompact() && (IsSaveState() || GetOpcode() == Opcode::NOP)) {
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
    auto operandsPos = out->tellp();
    // inputs
    bool hasInput = DumpInputs(out);
    // users
    if (hasInput && !GetUsers().Empty()) {
        (*out) << " -> ";
    }
    DumpUsers(this, out);
    // Align rest of the instruction info
    static constexpr auto ALIGN_BUF_SIZE = 64;
    if (auto posDiff = out->tellp() - operandsPos; posDiff < ALIGN_BUF_SIZE) {
        posDiff = ALIGN_BUF_SIZE - posDiff;
        static std::array<char, ALIGN_BUF_SIZE + 1> spaceBuf;
        if (spaceBuf[0] != ' ') {
            std::fill(spaceBuf.begin(), spaceBuf.end(), ' ');
        }
        spaceBuf[posDiff] = 0;
        (*out) << spaceBuf.data();
        spaceBuf[posDiff] = ' ';
    }
    // bytecode pointer
    if (pc_ != INVALID_PC && !g_options.IsCompilerDumpCompact()) {
        (*out) << ' ' << PcToString(pc_, allocator);
    }
#ifdef PANDA_COMPILER_DEBUG_INFO
    if (g_options.IsCompilerDumpSourceLine()) {
        DumpSourceLine(out);
    }
#endif
    if (newLine) {
        (*out) << '\n';
    }
    if (g_options.IsCompilerDumpBytecode()) {
        DumpBytecode(out);
    }
    if (GetOpcode() == Opcode::Parameter) {
        auto spillFill = static_cast<const ParameterInst *>(this)->GetLocationData();
        if (spillFill.DstValue() != GetInvalidReg()) {
            (*out) << sf_data::ToString(spillFill, GetBasicBlock()->GetGraph()->GetArch());
            if (newLine) {
                *out << std::endl;
            }
        }
    }
}

void CheckPrintPropsFlag(std::ostream *out, bool *printPropsFlag)
{
    if (!(*printPropsFlag)) {
        (*out) << "prop: ";
        (*printPropsFlag) = true;
    } else {
        (*out) << ", ";
    }
}

void PrintLoopInfo(std::ostream *out, Loop *loop)
{
    (*out) << "loop" << (loop->IsIrreducible() ? " (irreducible) " : " ") << loop->GetId();
    if (loop->GetDepth() > 0) {
        (*out) << ", depth " << loop->GetDepth();
    }
}

static void CheckStartEnd(const BasicBlock *block, std::ostream *out, bool *printPropsFlag)
{
    if (block->IsStartBlock()) {
        CheckPrintPropsFlag(out, printPropsFlag);
        (*out) << "start";
    }
    if (block->IsEndBlock()) {
        CheckPrintPropsFlag(out, printPropsFlag);
        (*out) << "end";
    }
}

static void CheckLoop(const BasicBlock *block, std::ostream *out, bool *printPropsFlag)
{
    if (block->IsLoopPreHeader()) {
        CheckPrintPropsFlag(out, printPropsFlag);
        (*out) << "prehead (loop " << block->GetNextLoop()->GetId() << ")";
    }
    if (block->IsLoopValid() && !block->GetLoop()->IsRoot()) {
        if (block->IsLoopHeader()) {
            CheckPrintPropsFlag(out, printPropsFlag);
            (*out) << "head";
        }
        CheckPrintPropsFlag(out, printPropsFlag);
        PrintLoopInfo(out, block->GetLoop());
    }
}

static void CheckTryCatch(const BasicBlock *block, std::ostream *out, bool *printPropsFlag)
{
    if (block->IsTryBegin()) {
        CheckPrintPropsFlag(out, printPropsFlag);
        (*out) << "try_begin (id " << block->GetTryId() << ")";
    }
    if (block->IsTry()) {
        CheckPrintPropsFlag(out, printPropsFlag);
        (*out) << "try (id " << block->GetTryId() << ")";
    }
    if (block->IsTryEnd()) {
        CheckPrintPropsFlag(out, printPropsFlag);
        (*out) << "try_end (id " << block->GetTryId() << ")";
    }
    if (block->IsCatchBegin()) {
        CheckPrintPropsFlag(out, printPropsFlag);
        (*out) << "catch_begin";
    }
    if (block->IsCatch()) {
        CheckPrintPropsFlag(out, printPropsFlag);
        (*out) << "catch";
    }
}

void BlockProps(const BasicBlock *block, std::ostream *out)
{
    bool printPropsFlag = false;
    CheckStartEnd(block, out, &printPropsFlag);
    CheckLoop(block, out, &printPropsFlag);
    CheckTryCatch(block, out, &printPropsFlag);

    if (block->GetGuestPc() != INVALID_PC) {
        CheckPrintPropsFlag(out, &printPropsFlag);
        (*out) << PcToString(block->GetGuestPc(), block->GetGraph()->GetLocalAllocator());
    }
    if (printPropsFlag) {
        (*out) << std::endl;
    }
}

void BasicBlock::Dump(std::ostream *out) const
{
    const auto &allocator = GetGraph()->GetLocalAllocator();
    (*out) << "BB " << IdToString(bbId_, allocator);
    // predecessors
    if (!preds_.empty()) {
        (*out) << "  ";
        BBDependence("preds", preds_, out, allocator);
    }
    (*out) << '\n';
    // properties
    BlockProps(this, out);
    (*out) << "hotness=" << GetHotness() << '\n';
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
    ArenaString returnType(DataType::ToString(runtime->GetMethodReturnType(method)), adapter);
    (*out) << "Method: " << runtime->GetMethodFullName(method, true) << " " << method << std::endl;
    if (IsOsrMode()) {
        (*out) << "OSR mode\n";
    }
    (*out) << std::endl;

    auto &blocks = GetAnalysis<LinearOrder>().IsValid() ? GetBlocksLinearOrder() : GetBlocksRPO();
    for (const auto &blockIt : blocks) {
        if (!blockIt->GetPredsBlocks().empty() || !blockIt->GetSuccsBlocks().empty()) {
            blockIt->Dump(out);
            (*out) << '\n';
        } else {
            // to print the dump before cleanup, still unconnected nodes exist
            (*out) << "BB " << blockIt->GetId() << " is unconnected\n\n";
        }
    }
}
}  // namespace ark::compiler
