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

#include "inst.h"
#include "graph.h"
#include "basicblock.h"
#include "graph_visitor.h"
#include "optimizer/optimizations/vn.h"
#include "profiling/profiling.h"

namespace ark::compiler {
const char *GetConditionCodeString(ConditionCode code)
{
    switch (code) {
        case ConditionCode::CC_EQ:
            return "EQ";
        case ConditionCode::CC_NE:
            return "NE";

        case ConditionCode::CC_LT:
            return "LT";
        case ConditionCode::CC_LE:
            return "LE";
        case ConditionCode::CC_GT:
            return "GT";
        case ConditionCode::CC_GE:
            return "GE";

        case ConditionCode::CC_B:
            return "B";
        case ConditionCode::CC_BE:
            return "BE";
        case ConditionCode::CC_A:
            return "A";
        case ConditionCode::CC_AE:
            return "AE";

        case ConditionCode::CC_TST_EQ:
            return "TST_EQ";
        case ConditionCode::CC_TST_NE:
            return "TST_NE";
        default:
            UNREACHABLE();
            return nullptr;  // Dummy
    }
}

ConditionCode GetInverseConditionCode(ConditionCode code)
{
    switch (code) {
        case ConditionCode::CC_EQ:
            return ConditionCode::CC_NE;
        case ConditionCode::CC_NE:
            return ConditionCode::CC_EQ;

        case ConditionCode::CC_LT:
            return ConditionCode::CC_GE;
        case ConditionCode::CC_LE:
            return ConditionCode::CC_GT;
        case ConditionCode::CC_GT:
            return ConditionCode::CC_LE;
        case ConditionCode::CC_GE:
            return ConditionCode::CC_LT;

        case ConditionCode::CC_B:
            return ConditionCode::CC_AE;
        case ConditionCode::CC_BE:
            return ConditionCode::CC_A;
        case ConditionCode::CC_A:
            return ConditionCode::CC_BE;
        case ConditionCode::CC_AE:
            return ConditionCode::CC_B;

        case ConditionCode::CC_TST_EQ:
            return ConditionCode::CC_TST_NE;
        case ConditionCode::CC_TST_NE:
            return ConditionCode::CC_TST_EQ;

        default:
            UNREACHABLE();
    }
}

ConditionCode InverseSignednessConditionCode(ConditionCode code)
{
    switch (code) {
        case ConditionCode::CC_EQ:
            return ConditionCode::CC_EQ;
        case ConditionCode::CC_NE:
            return ConditionCode::CC_NE;

        case ConditionCode::CC_LT:
            return ConditionCode::CC_B;
        case ConditionCode::CC_LE:
            return ConditionCode::CC_BE;
        case ConditionCode::CC_GT:
            return ConditionCode::CC_A;
        case ConditionCode::CC_GE:
            return ConditionCode::CC_AE;

        case ConditionCode::CC_B:
            return ConditionCode::CC_LT;
        case ConditionCode::CC_BE:
            return ConditionCode::CC_LE;
        case ConditionCode::CC_A:
            return ConditionCode::CC_GT;
        case ConditionCode::CC_AE:
            return ConditionCode::CC_GE;

        case ConditionCode::CC_TST_EQ:
            return ConditionCode::CC_TST_EQ;
        case ConditionCode::CC_TST_NE:
            return ConditionCode::CC_TST_NE;

        default:
            UNREACHABLE();
    }
}

bool IsSignedConditionCode(ConditionCode code)
{
    switch (code) {
        case ConditionCode::CC_LT:
        case ConditionCode::CC_LE:
        case ConditionCode::CC_GT:
        case ConditionCode::CC_GE:
            return true;

        case ConditionCode::CC_EQ:
        case ConditionCode::CC_NE:
        case ConditionCode::CC_B:
        case ConditionCode::CC_BE:
        case ConditionCode::CC_A:
        case ConditionCode::CC_AE:
        case ConditionCode::CC_TST_EQ:
        case ConditionCode::CC_TST_NE:
            return false;

        default:
            UNREACHABLE();
    }
}

ConditionCode SwapOperandsConditionCode(ConditionCode code)
{
    switch (code) {
        case ConditionCode::CC_EQ:
        case ConditionCode::CC_NE:
            return code;

        case ConditionCode::CC_LT:
            return ConditionCode::CC_GT;
        case ConditionCode::CC_LE:
            return ConditionCode::CC_GE;
        case ConditionCode::CC_GT:
            return ConditionCode::CC_LT;
        case ConditionCode::CC_GE:
            return ConditionCode::CC_LE;

        case ConditionCode::CC_B:
            return ConditionCode::CC_A;
        case ConditionCode::CC_BE:
            return ConditionCode::CC_AE;
        case ConditionCode::CC_A:
            return ConditionCode::CC_B;
        case ConditionCode::CC_AE:
            return ConditionCode::CC_BE;

        case ConditionCode::CC_TST_EQ:
        case ConditionCode::CC_TST_NE:
            return code;

        default:
            UNREACHABLE();
    }
}

bool IsVolatileMemInst(const Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::LoadObject:
            return inst->CastToLoadObject()->GetVolatile();
        case Opcode::LoadObjectPair:
            return inst->CastToLoadObjectPair()->GetVolatile();
        case Opcode::StoreObject:
            return inst->CastToStoreObject()->GetVolatile();
        case Opcode::StoreObjectPair:
            return inst->CastToStoreObjectPair()->GetVolatile();
        case Opcode::LoadStatic:
            return inst->CastToLoadStatic()->GetVolatile();
        case Opcode::StoreStatic:
            return inst->CastToStoreStatic()->GetVolatile();
        case Opcode::UnresolvedStoreStatic:
        case Opcode::LoadResolvedObjectFieldStatic:
        case Opcode::StoreResolvedObjectFieldStatic:
            return true;
        default:
            return false;
    }
}

const ObjectTypeInfo ObjectTypeInfo::INVALID {};
const ObjectTypeInfo ObjectTypeInfo::UNKNOWN {1};

void Inst::ReserveInputs(size_t capacity)
{
    ASSERT(IsOperandsDynamic());
    GetDynamicOperands()->Reallocate(capacity);
}

Inst *User::GetInst()
{
    if (UNLIKELY(IsDynamic())) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *reinterpret_cast<Inst **>(this + GetIndex() + 1);
    }
    auto p = reinterpret_cast<uintptr_t>(this);
    p += (GetIndex() + 1) * sizeof(User);

    auto inputsCount {SizeField::Decode(properties_)};
    p += (inputsCount + Input::GetPadding(RUNTIME_ARCH, inputsCount)) * sizeof(Input);
    return reinterpret_cast<Inst *>(p);
}

void Inst::InsertBefore(Inst *inst)
{
    ASSERT(bb_ != nullptr);
    bb_->InsertBefore(inst, this);
}

void Inst::InsertAfter(Inst *inst)
{
    ASSERT(bb_ != nullptr);
    bb_->InsertAfter(inst, this);
}

uint32_t Inst::GetInliningDepth() const
{
    auto ss = GetSaveState();
    return ss == nullptr ? 0 : ss->GetInliningDepth();
}

void DynamicOperands::Reallocate([[maybe_unused]] size_t newCapacity /* =0 */)
{
    if (newCapacity == 0) {
        constexpr auto IMM_2 = 2;
        newCapacity = (((capacity_ != 0U) ? capacity_ : 1U) << 1U) + IMM_2;
    } else if (newCapacity <= capacity_) {
        return;
    }
    // NOLINTNEXTLINE(bugprone-sizeof-expression)
    auto size = newCapacity * (sizeof(User) + sizeof(Inst *)) + sizeof(Inst *);
    auto newStor = reinterpret_cast<uintptr_t>(allocator_->Alloc(size));

    auto ownerInst {GetOwnerInst()};
    // Set pointer to owned instruction into new storage NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT(reinterpret_cast<User *>(newStor) != nullptr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *reinterpret_cast<Inst **>(reinterpret_cast<User *>(newStor) + newCapacity) = ownerInst;

    if (users_ == nullptr) {
        users_ = reinterpret_cast<User *>(newStor);
        capacity_ = newCapacity;
        return;
    }
    Input *oldInputs = Inputs();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *newInputs = reinterpret_cast<Input *>(newStor + sizeof(User) * newCapacity) + 1;

    for (size_t i = 0; i < size_; i++) {
        Inst *oldInput = oldInputs[i].GetInst();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT(oldInput);
        // Initialize new User in container. Since users are placed from end of array, i.e. zero index element
        // will be at the end of array, we need to add capacity and substitute index.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        User *newUser = new (reinterpret_cast<User *>(newStor) + newCapacity - i - 1) User(false, i, newCapacity);
        auto oldUser {GetUser(i)};
        if (ownerInst->IsSaveState()) {
            newUser->SetVirtualRegister(oldUser->GetVirtualRegister());
        } else if (ownerInst->IsPhi()) {
            newUser->SetBbNum(oldUser->GetBbNum());
        }
        oldInput->RemoveUser(oldUser);
        oldInput->AddUser(newUser);
        newInputs[i] = Input(oldInput);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    capacity_ = newCapacity;
    users_ = reinterpret_cast<User *>(newStor);
}

unsigned DynamicOperands::Append(Inst *inst)
{
    ASSERT(capacity_ >= size_);
    if (capacity_ == size_) {
        Reallocate();
    }
    ASSERT(capacity_ > size_);
    SetInput(size_, Input(inst));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    new (users_ + capacity_ - size_ - 1) User(false, size_, capacity_);
    auto user {GetUser(size_)};
    if (GetOwnerInst()->IsPhi()) {
        user->SetBbNum(size_);
    }
    ASSERT(inst != nullptr);
    inst->AddUser(user);
    return size_++;
}

void DynamicOperands::Remove(unsigned index)
{
    size_--;
    auto *currInput = GetInput(index)->GetInst();
    if (currInput->GetBasicBlock() != nullptr && currInput->HasUsers()) {
        currInput->RemoveUser(GetUser(index));
    }

    auto bbNum {GetUser(index)->GetBbNum()};
    auto ownerInst {GetOwnerInst()};

    if (index != size_) {
        auto *lastInput = GetInput(size_)->GetInst();
        if (lastInput->HasUsers()) {
            lastInput->RemoveUser(GetUser(size_));
            lastInput->AddUser(GetUser(index));
        }
        SetInput(index, *GetInput(size_));
        if (ownerInst->IsSaveState()) {
            GetUser(index)->SetVirtualRegister(GetUser(size_)->GetVirtualRegister());
        } else if (ownerInst->IsPhi()) {
            GetUser(index)->SetBbNum(GetUser(size_)->GetBbNum());
        }
    }

    if (ownerInst->IsPhi()) {
        for (size_t i {0}; i < size_; ++i) {
            if (GetUser(i)->GetBbNum() == size_) {
                GetUser(i)->SetBbNum(bbNum);
                break;
            }
        }
    }
}

void Inst::AppendOpcodeAndTypeToVnObject(VnObject *vnObj) const
{
    vnObj->Add(static_cast<VnObject::HalfObjType>(GetOpcode()), static_cast<VnObject::HalfObjType>(GetType()));
}

void Inst::AppendInputsToVnObject(VnObject *vnObj) const
{
    if (IsCommutative() && !DataType::IsFloatType(GetType())) {
        // 2 : Exactly 2 data inputs expected for commutative operations
        ASSERT((GetInputsCount() == 2U && !RequireState()) || (GetInputsCount() == 3U && RequireState()));
        auto input0 = GetDataFlowInput(GetInput(0).GetInst());
        auto input1 = GetDataFlowInput(GetInput(1).GetInst());
        ASSERT(!input0->IsSaveState() && !input1->IsSaveState() &&
               "SaveState is unexpected as the first or second input of commutative operations");
        ASSERT(input0->GetVN() != INVALID_VN && "VN of inputs must be known before current inst");
        ASSERT(input1->GetVN() != INVALID_VN && "VN of inputs must be known before current inst");
        if (input0->GetId() > input1->GetId()) {
            vnObj->Add(input0->GetVN());
            vnObj->Add(input1->GetVN());
        } else {
            vnObj->Add(input1->GetVN());
            vnObj->Add(input0->GetVN());
        }
        return;
    }
    for (auto input : GetInputs()) {
        auto inputInst = GetDataFlowInput(input.GetInst());
        if (inputInst->IsSaveState()) {
            continue;
        }
        auto vn = inputInst->GetVN();
        ASSERT(vn != INVALID_VN && "VN of inputs must be known before current inst");
        vnObj->Add(vn);
    }
}

void FieldMixin::AppendObjFieldToVnObject(VnObject *vnObj) const
{
    ASSERT(GetObjField() != nullptr);
    vnObj->Add(reinterpret_cast<VnObject::DoubleObjType>(GetObjField()));
}

void MethodDataMixin::AppendMethodIdToVnObject(VnObject *vnObj) const
{
    vnObj->Add(GetCallMethodId());
}

void TypeIdMixin::AppendTypeIdAndMethodToVnObject(VnObject *vnObj) const
{
    vnObj->Add(GetTypeId());
    vnObj->Add(reinterpret_cast<VnObject::DoubleObjType>(GetMethod()));
}

void CompareAnyTypeInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<VnObject::ObjType>(GetAnyType()));
}

void GetAnyTypeNameInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<VnObject::ObjType>(GetAnyType()));
}

void BinaryImmOperation::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(GetImm());
}

void BinaryShiftedRegisterOperation::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(GetImm());
    vnObj->Add(static_cast<uint32_t>(GetShiftType()));
}

void UnaryShiftedRegisterOperation::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(GetImm());
    vnObj->Add(static_cast<uint32_t>(GetShiftType()));
}

void CompareInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<uint32_t>(GetCc()));
}

void SelectInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<uint32_t>(GetCc()));
}

void SelectTransformInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<VnObject::HalfObjType>(GetCc()),
               static_cast<VnObject::HalfObjType>(GetSelectTransformType()));
}

void SelectImmInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(GetImm());
    vnObj->Add(static_cast<VnObject::ObjType>(GetCc()));
}

void SelectImmTransformInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(GetImm());
    vnObj->Add(static_cast<VnObject::HalfObjType>(GetCc()),
               static_cast<VnObject::HalfObjType>(GetSelectTransformType()));
}

void IfInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<uint32_t>(GetCc()));
}

void IfImmInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<uint32_t>(GetCc()));
}

void UnaryOperation::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    if (GetOpcode() == Opcode::Cast) {
        vnObj->Add(static_cast<uint32_t>(GetInput(0).GetInst()->GetType()));
    }
}

void CmpInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    if (DataType::IsFloatType(GetOperandsType())) {
        vnObj->Add(static_cast<uint32_t>(IsFcmpg()));
    }
    vnObj->Add(static_cast<uint32_t>(GetInputType(0)));
}

void LoadFromPoolDynamic::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(GetTypeId());
}

void CastInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<uint32_t>(GetInputType(0)));
}

void LoadImmediateInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<uint64_t>(GetObjectType()));
    vnObj->Add(reinterpret_cast<uint64_t>(GetObject()));
}

void CastAnyTypeValueInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<VnObject::ObjType>(GetAnyType()));
}

void CastValueToAnyTypeInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<VnObject::ObjType>(GetAnyType()));
}

void ClassInst::SetVnObject(VnObject *vnObj) const
{
    vnObj->Add(static_cast<VnObject::ObjType>(Opcode::InitClass));
    auto klass = GetClass();
    if (klass == nullptr) {
        vnObj->Add(GetTypeId());
    } else {
        vnObj->Add(reinterpret_cast<VnObject::DoubleObjType>(klass));
    }
}

void ExtractBitfieldInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<VnObject::HalfObjType>((sourceBit_ << 1U) | static_cast<unsigned>(signExt_)),
               static_cast<VnObject::HalfObjType>(width_));
}

void GlobalVarInst::SetVnObject(VnObject *vnObj) const
{
    vnObj->Add(static_cast<VnObject::ObjType>(Opcode::GetGlobalVarAddress));
    vnObj->Add(GetTypeId());
}

void IntrinsicInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<VnObject::ObjType>(GetIntrinsicId()));
}

void LoadObjectInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    AppendObjFieldToVnObject(vnObj);
}

void LoadStaticInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    AppendObjFieldToVnObject(vnObj);
}

void ResolveObjectFieldInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    AppendTypeIdAndMethodToVnObject(vnObj);
}

void ResolveObjectFieldStaticInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    AppendTypeIdAndMethodToVnObject(vnObj);
}

void ResolveStaticInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    AppendMethodIdToVnObject(vnObj);
}

void ResolveVirtualInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    AppendMethodIdToVnObject(vnObj);
}

void RuntimeClassInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(reinterpret_cast<uint64_t>(GetClass()));
}

void LoadObjFromConstInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<uint64_t>(GetObjPtr()));
}

void FunctionImmediateInst::SetVnObject(VnObject *vnObj) const
{
    AppendOpcodeAndTypeToVnObject(vnObj);
    AppendInputsToVnObject(vnObj);
    vnObj->Add(static_cast<uint64_t>(GetFunctionPtr()));
}

bool CastInst::IsDynamicCast() const
{
    return DataType::IsFloatType(GetInputType(0U)) && DataType::GetCommonType(GetType()) == DataType::INT64 &&
           GetBasicBlock()->GetGraph()->IsDynamicMethod();
}

BasicBlock *PhiInst::GetPhiInputBb(unsigned index)
{
    ASSERT(index < GetInputsCount());

    auto bbNum {GetPhiInputBbNum(index)};
    ASSERT(bbNum < GetBasicBlock()->GetPredsBlocks().size());
    return GetBasicBlock()->GetPredsBlocks()[bbNum];
}

Inst *PhiInst::GetPhiInput(BasicBlock *bb)
{
    auto index = GetPredBlockIndex(bb);
    ASSERT(index < GetInputs().size());
    return GetInput(index).GetInst();
}

Inst *PhiInst::GetPhiDataflowInput(BasicBlock *bb)
{
    auto index = GetPredBlockIndex(bb);
    ASSERT(index < GetInputs().size());
    return GetDataFlowInput(index);
}

size_t PhiInst::GetPredBlockIndex(const BasicBlock *block) const
{
    for (size_t i {0}; i < GetInputsCount(); ++i) {
        if (GetPhiInputBb(i) == block) {
            return i;
        }
    }
    UNREACHABLE();
}

template <Opcode OPC, size_t INPUT_IDX>
Inst *SkipInstructions(Inst *inputInst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (Opcode opcode = inputInst->GetOpcode(); opcode == OPC; opcode = inputInst->GetOpcode()) {
        inputInst = inputInst->GetInput(INPUT_IDX).GetInst();
    }
    return inputInst;
}
/*
 * For instructions LoadArray, StoreArray, LoadArrayPair, StoreArrayPair, LoadArrayI, StoreArrayI, LoadArrayPairI,
 * StoreArrayPairI, LenArray, LoadObject, StoreObject, CallVirtual, Monitor, LoadObjectPair, StoreObjectPair with
 * NullCheck input the dataflow user is object, which is the first input of NullCheck instruction.
 * For instructions LoadArray, StoreArray, LoadArrayPair, StoreArrayPair with BoundsCheck input the dataflow user is
 * array index, which is the second input of BoundsCheck instruction
 * For instructions Div and Mod with ZeroCheck input the dataflow user is the first input of ZeroCheck
 */
Inst *Inst::GetDataFlowInput(Inst *inputInst)
{
    auto opcode = inputInst->GetOpcode();
    if (opcode == Opcode::NullCheck) {
        return SkipInstructions<Opcode::NullCheck, 0>(inputInst);
    }
    if (opcode == Opcode::BoundsCheck) {
        return SkipInstructions<Opcode::BoundsCheck, 1>(inputInst);
    }
    if (opcode == Opcode::BoundsCheckI) {
        return SkipInstructions<Opcode::BoundsCheckI, 0>(inputInst);
    }
    if (opcode == Opcode::ZeroCheck) {
        return SkipInstructions<Opcode::ZeroCheck, 0>(inputInst);
    }
    if (opcode == Opcode::NegativeCheck) {
        return SkipInstructions<Opcode::NegativeCheck, 0>(inputInst);
    }
    if (opcode == Opcode::NotPositiveCheck) {
        return SkipInstructions<Opcode::NotPositiveCheck, 0>(inputInst);
    }
    if (opcode == Opcode::AnyTypeCheck) {
        return SkipInstructions<Opcode::AnyTypeCheck, 0>(inputInst);
    }
    if (opcode == Opcode::ObjByIndexCheck) {
        return SkipInstructions<Opcode::ObjByIndexCheck, 0>(inputInst);
    }
    if (opcode == Opcode::HclassCheck) {
        inputInst = SkipInstructions<Opcode::HclassCheck, 0>(inputInst);
        return SkipInstructions<Opcode::LoadObject, 0>(inputInst);
    }
    if (opcode == Opcode::RefTypeCheck) {
        inputInst = SkipInstructions<Opcode::RefTypeCheck, 1>(inputInst);
        if (inputInst->GetOpcode() == Opcode::NullCheck) {
            return SkipInstructions<Opcode::NullCheck, 0>(inputInst);
        }
        return inputInst;
    }
    return inputInst;
}

bool Inst::IsPrecedingInSameBlock(const Inst *other) const
{
    ASSERT(other != nullptr && GetBasicBlock() == other->GetBasicBlock());
    if (this == other) {
        return true;
    }
    auto next = GetNext();
    while (next != nullptr) {
        if (next == other) {
            return true;
        }
        next = next->GetNext();
    }
    return false;
}

bool Inst::IsDominate(const Inst *other) const
{
    ASSERT(other != nullptr);
    if (this == other) {
        return true;
    }
    auto thisBb = GetBasicBlock();
    auto otherBb = other->GetBasicBlock();
    return thisBb == otherBb ? IsPrecedingInSameBlock(other) : thisBb->IsDominate(otherBb);
}

bool Inst::InSameBlockOrDominate(const Inst *other) const
{
    return GetBasicBlock() == other->GetBasicBlock() || IsDominate(other);
}

Inst *Inst::Clone(const Graph *targetGraph) const
{
    ASSERT(targetGraph != nullptr);
    auto clone = targetGraph->CreateInst(GetOpcode());
    clone->bitFields_ = GetAllFields();
    clone->pc_ = GetPc();
#ifndef NDEBUG
    clone->SetDstReg(GetDstReg());
#endif
    if (IsOperandsDynamic()) {
        clone->ReserveInputs(GetInputsCount());
    }
#ifdef PANDA_COMPILER_DEBUG_INFO
    clone->SetCurrentMethod(GetCurrentMethod());
#endif
    return clone;
}

template <size_t N>
Inst *FixedInputsInst<N>::Clone(const Graph *targetGraph) const
{
    auto clone = static_cast<FixedInputsInst *>(Inst::Clone(targetGraph));
#ifndef NDEBUG
    for (size_t i = 0; i < INPUT_COUNT; ++i) {
        clone->SetSrcReg(i, GetSrcReg(i));
    }
#endif
    return clone;
}

template class FixedInputsInst<0>;
template class FixedInputsInst<1>;
template class FixedInputsInst<2U>;
template class FixedInputsInst<3U>;
template class FixedInputsInst<4U>;

Inst *ExtractBitfieldInst::Clone(const Graph *targetGraph) const
{
    auto clone = static_cast<ExtractBitfieldInst *>(FixedInputsInst<1>::Clone(targetGraph));
    clone->sourceBit_ = this->sourceBit_;
    clone->width_ = this->width_;
    clone->signExt_ = this->signExt_;
    return clone;
}

Inst *CallInst::Clone(const Graph *targetGraph) const
{
    ASSERT(targetGraph != nullptr);
    auto instClone = Inst::Clone(targetGraph);
    auto callClone = static_cast<CallInst *>(instClone);
    callClone->SetCallMethodId(GetCallMethodId());
    callClone->SetCallMethod(GetCallMethod());
    callClone->SetIsNative(GetIsNative());
    callClone->SetCanNativeException(GetCanNativeException());
    CloneTypes(targetGraph->GetAllocator(), callClone);
    return instClone;
}

Inst *CallIndirectInst::Clone(const Graph *targetGraph) const
{
    auto clone = Inst::Clone(targetGraph)->CastToCallIndirect();
    CloneTypes(targetGraph->GetAllocator(), clone);
    return clone;
}

Inst *IntrinsicInst::Clone(const Graph *targetGraph) const
{
    ASSERT(targetGraph != nullptr);
    auto intrinsicClone = (GetOpcode() == Opcode::Intrinsic ? Inst::Clone(targetGraph)->CastToIntrinsic()
                                                            : Inst::Clone(targetGraph)->CastToBuiltin());
    intrinsicClone->intrinsicId_ = GetIntrinsicId();
    CloneTypes(targetGraph->GetAllocator(), intrinsicClone);
    if (HasImms()) {
        for (auto imm : GetImms()) {
            intrinsicClone->AddImm(targetGraph->GetAllocator(), imm);
        }
    }
    intrinsicClone->SetMethod(GetMethod());
    return intrinsicClone;
}

Inst *ConstantInst::Clone(const Graph *targetGraph) const
{
    Inst *newCnst = nullptr;
    bool isSupportInt32 = GetBasicBlock()->GetGraph()->IsBytecodeOptimizer();
    switch (GetType()) {
        case DataType::INT32:
            newCnst = targetGraph->CreateInstConstant(static_cast<int32_t>(GetIntValue()), isSupportInt32);
            break;
        case DataType::INT64:
            newCnst = targetGraph->CreateInstConstant(GetIntValue(), isSupportInt32);
            break;
        case DataType::FLOAT32:
            newCnst = targetGraph->CreateInstConstant(GetFloatValue(), isSupportInt32);
            break;
        case DataType::FLOAT64:
            newCnst = targetGraph->CreateInstConstant(GetDoubleValue(), isSupportInt32);
            break;
        case DataType::ANY:
            newCnst = targetGraph->CreateInstConstant(GetRawValue(), isSupportInt32);
            newCnst->SetType(DataType::ANY);
            break;
        default:
            UNREACHABLE();
    }
#ifndef NDEBUG
    newCnst->SetDstReg(GetDstReg());
#endif
    return newCnst;
}

Inst *ParameterInst::Clone(const Graph *targetGraph) const
{
    auto clone = FixedInputsInst::Clone(targetGraph)->CastToParameter();
    clone->SetArgNumber(GetArgNumber());
    clone->SetLocationData(GetLocationData());
    return clone;
}

Inst *SaveStateInst::Clone(const Graph *targetGraph) const
{
    auto clone = static_cast<SaveStateInst *>(Inst::Clone(targetGraph));
    if (GetImmediatesCount() > 0) {
        clone->AllocateImmediates(targetGraph->GetAllocator(), GetImmediatesCount());
        std::copy(immediates_->begin(), immediates_->end(), clone->immediates_->begin());
    }
    clone->method_ = method_;
    clone->callerInst_ = callerInst_;
    clone->inliningDepth_ = inliningDepth_;
    return clone;
}

Inst *BinaryShiftedRegisterOperation::Clone(const Graph *targetGraph) const
{
    auto clone = static_cast<BinaryShiftedRegisterOperation *>(FixedInputsInst::Clone(targetGraph));
    clone->SetImm(GetImm());
    clone->SetShiftType(GetShiftType());
    return clone;
}

Inst *UnaryShiftedRegisterOperation::Clone(const Graph *targetGraph) const
{
    auto clone = static_cast<UnaryShiftedRegisterOperation *>(FixedInputsInst::Clone(targetGraph));
    clone->SetImm(GetImm());
    clone->SetShiftType(GetShiftType());
    return clone;
}

void SaveStateInst::AppendImmediate(uint64_t imm, uint16_t vreg, DataType::Type type, VRegType vregType)
{
    if (immediates_ == nullptr) {
        ASSERT(GetBasicBlock() != nullptr);
        AllocateImmediates(GetBasicBlock()->GetGraph()->GetAllocator(), 0);
    }
    ASSERT(immediates_ != nullptr);
    immediates_->emplace_back(SaveStateImm {imm, vreg, type, vregType});
}

void SaveStateInst::AllocateImmediates(ArenaAllocator *allocator, size_t size)
{
    immediates_ = allocator->New<ArenaVector<SaveStateImm>>(allocator->Adapter());
    ASSERT(immediates_ != nullptr);
    immediates_->resize(size);
}

bool SaveStateInst::GetInputsWereDeletedRec() const
{
    if (GetInputsWereDeleted()) {
        return true;
    }
    if (callerInst_ != nullptr) {
        auto *saveState = callerInst_->GetSaveState();
        ASSERT(saveState != nullptr);
        return saveState->GetInputsWereDeletedRec();
    }
    return false;
}

void TryInst::AppendCatchTypeId(uint32_t id, uint32_t catchEdgeIndex)
{
    if (catchTypeIds_ == nullptr) {
        ASSERT(catchEdgeIndexes_ == nullptr);
        ASSERT(GetBasicBlock() != nullptr);
        auto allocator = GetBasicBlock()->GetGraph()->GetAllocator();
        catchTypeIds_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
        catchEdgeIndexes_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
    }
    catchTypeIds_->push_back(id);
    ASSERT(catchEdgeIndexes_ != nullptr);
    catchEdgeIndexes_->push_back(catchEdgeIndex);
}

void CatchPhiInst::AppendThrowableInst(const Inst *inst)
{
    if (throwInsts_ == nullptr) {
        ASSERT(GetBasicBlock() != nullptr);
        auto allocator = GetBasicBlock()->GetGraph()->GetAllocator();
        throwInsts_ = allocator->New<ArenaVector<const Inst *>>(allocator->Adapter());
    }
    ASSERT(throwInsts_ != nullptr);
    throwInsts_->push_back(inst);
}

void CatchPhiInst::ReplaceThrowableInst(const Inst *oldInst, const Inst *newInst)
{
    auto index = GetThrowableInstIndex(oldInst);
    throwInsts_->at(index) = newInst;
}

void CatchPhiInst::RemoveInput(unsigned index)
{
    Inst::RemoveInput(index);
    if (throwInsts_ != nullptr) {
        throwInsts_->at(index) = throwInsts_->back();
        throwInsts_->pop_back();
    }
}

Inst *TryInst::Clone(const Graph *targetGraph) const
{
    auto clone = FixedInputsInst::Clone(targetGraph)->CastToTry();
    if (auto idsCount = this->GetCatchTypeIdsCount(); idsCount > 0) {
        if (clone->catchTypeIds_ == nullptr) {
            auto allocator = targetGraph->GetAllocator();
            clone->catchTypeIds_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
            clone->catchEdgeIndexes_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
        }
        clone->catchTypeIds_->resize(idsCount);
        clone->catchEdgeIndexes_->resize(idsCount);
        std::copy(this->catchTypeIds_->begin(), this->catchTypeIds_->end(), clone->catchTypeIds_->begin());
        std::copy(this->catchEdgeIndexes_->begin(), this->catchEdgeIndexes_->end(), clone->catchEdgeIndexes_->begin());
    }
    return clone;
}

BasicBlock *IfImmInst::GetEdgeIfInputTrue()
{
    return GetBasicBlock()->GetSuccessor(GetTrueInputEdgeIdx());
}

BasicBlock *IfImmInst::GetEdgeIfInputFalse()
{
    return GetBasicBlock()->GetSuccessor(1 - GetTrueInputEdgeIdx());
}

/**
 * NB! Can be called before Lowering pass only
 * Return if_imm's block successor index when input is true
 */
size_t IfImmInst::GetTrueInputEdgeIdx()
{
    ASSERT(GetBasicBlock() != nullptr);
    ASSERT(GetBasicBlock()->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    ASSERT(GetCc() == ConditionCode::CC_NE || GetCc() == ConditionCode::CC_EQ);
    ASSERT(GetImm() == 0);
    return GetCc() == CC_NE ? 0 : 1;
}

bool Inst::IsPropagateLiveness() const
{
    return (CanThrow() && GetBasicBlock()->IsTry()) || CanDeoptimize();
}

bool Inst::RequireRegMap() const
{
    if (GetOpcode() == Opcode::SafePoint) {
        return g_options.IsCompilerSafePointsRequireRegMap();
    }
    return GetOpcode() == Opcode::SaveStateOsr || IsPropagateLiveness();
}

bool Inst::IsZeroRegInst() const
{
    ASSERT(GetBasicBlock() != nullptr);
    ASSERT(GetBasicBlock()->GetGraph() != nullptr);
    return GetBasicBlock()->GetGraph()->GetZeroReg() != GetInvalidReg() && IsZeroConstantOrNullPtr(this) &&
           !IsReferenceForNativeApiCall();
}

bool Inst::IsAccRead() const
{
    return GetFlag(inst_flags::ACC_READ);
}

bool Inst::IsAccWrite() const
{
    if (GetBasicBlock()->GetGraph()->IsDynamicMethod() && IsConst()) {
        return true;
    }
    return GetFlag(inst_flags::ACC_WRITE);
}

// Returns true if instruction result can be object
bool Inst::IsReferenceOrAny() const
{
    if (GetType() == DataType::ANY) {
        switch (opcode_) {
            // GetAnyTypeName always return movable string
            case Opcode::GetAnyTypeName:
            // We conservative decide that phi with ANY type is always reference,
            // because for phi we can speculate incorrect any_type
            case Opcode::Phi:
                return true;
            default:
                break;
        }
        auto anyType = GetAnyType();
        if (anyType == AnyBaseType::UNDEFINED_TYPE) {
            return true;
        }
        auto dataType = AnyBaseTypeToDataType(anyType);
        return dataType == DataType::REFERENCE;
    }
    return GetType() == DataType::REFERENCE;
}

bool IsMovableObjectRec(Inst *inst, Marker visitedMrk)
{
    if (inst->SetMarker(visitedMrk)) {
        return false;
    }

    if (inst->IsPhi() || inst->IsOneOf(Opcode::Select, Opcode::SelectImm)) {
        for (size_t i = 0U; i < inst->GetInputsCount(); ++i) {
            if (IsMovableObjectRec(inst->GetDataFlowInput(i), visitedMrk)) {
                return true;
            }
        }
        return false;
    }

    return inst->IsMovableObject();
}

// Returns true if instruction result can be moved by GC
// Returns false for checks because their result is equal to input
bool Inst::IsMovableObject()
{
    if (IsCheck() || !IsReferenceOrAny()) {
        return false;
    }
    switch (opcode_) {
        case Opcode::NullPtr:
        case Opcode::LoadClass:
        case Opcode::InitClass:
        case Opcode::LoadAndInitClass:
        case Opcode::UnresolvedLoadAndInitClass:
        case Opcode::LoadImmediate:
        case Opcode::GetInstanceClass:
        case Opcode::GetGlobalVarAddress:
        case Opcode::ResolveObjectFieldStatic:
        case Opcode::Constant:
        case Opcode::LoadConstantPool:
        case Opcode::LoadRuntimeClass:
        case Opcode::LoadUniqueObject:
            // The result of these instructions can't be moved by GC.
            return false;
        case Opcode::LoadObject:
            // Classes in non moveble space.
            return this->CastToLoadObject()->GetObjectType() != ObjectType::MEM_DYN_CLASS &&
                   this->CastToLoadObject()->GetObjectType() != ObjectType::MEM_DYN_HCLASS;
        case Opcode::Phi:
        case Opcode::Select:
        case Opcode::SelectImm: {
            MarkerHolder marker {GetBasicBlock()->GetGraph()};
            return IsMovableObjectRec(this, marker.GetMarker());
        }
        case Opcode::Intrinsic:
            return CastToIntrinsic()->GetIntrinsicId() !=
                   RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_NATIVE_METHOD_MANAGED_CLASS;
        default:
            return true;
    }
}

TryInst *GetTryBeginInst(const BasicBlock *tryBeginBb)
{
    ASSERT(tryBeginBb != nullptr && tryBeginBb->IsTryBegin());
    for (auto inst : tryBeginBb->AllInsts()) {
        if (inst->GetOpcode() == Opcode::Try) {
            return inst->CastToTry();
        }
    }
    UNREACHABLE();
    return nullptr;
}

/**
 * Regalloc's helper to checks if intrinsic's arguments should be located on the registers according to
 * calling-convention
 */
bool IntrinsicInst::IsNativeCall() const
{
    ASSERT(GetBasicBlock() != nullptr);
    ASSERT(GetBasicBlock()->GetGraph() != nullptr);
    if (IsFastpathIntrinsic(intrinsicId_)) {
        return false;
    }
#ifdef PANDA_WITH_IRTOC
    if (IsIrtocIntrinsic(intrinsicId_)) {
        return intrinsicId_ == RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY;
    }
#endif
    auto graph = GetBasicBlock()->GetGraph();
    auto arch = graph->GetArch();
    auto runtime = graph->GetRuntime();
    return !EncodesBuiltin(runtime, intrinsicId_, arch) || IsRuntimeCall();
}

DeoptimizeType AnyTypeCheckInst::GetDeoptimizeType() const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto customDeoptimize = graph->IsAotMode() || graph->GetRuntime()->GetMethodProfile(graph->GetMethod(), true) !=
                                                      profiling::INVALID_PROFILE;
    if (!customDeoptimize) {
        return DeoptimizeType::ANY_TYPE_CHECK;
    }
    switch (AnyBaseTypeToDataType(GetAnyType())) {
        case DataType::Type::INT32:
            return DeoptimizeType::NOT_SMALL_INT;
        case DataType::Type::FLOAT64:
            if (IsIntegerWasSeen()) {
                return DeoptimizeType::NOT_NUMBER;
            }
            return DeoptimizeType::DOUBLE_WITH_INT;
        default:
            return DeoptimizeType::ANY_TYPE_CHECK;
    }
}

void HclassCheckInst::ExtendFlags(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::HclassCheck);
    auto check = inst->CastToHclassCheck();
    if (check->GetCheckFunctionIsNotClassConstructor()) {
        SetCheckFunctionIsNotClassConstructor(true);
    }
    if (check->GetCheckIsFunction()) {
        SetCheckIsFunction(true);
    }
}

}  // namespace ark::compiler
