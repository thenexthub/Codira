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
#include "graph_visitor.h"
#include "optimizer/optimizations/vn.h"

namespace panda::compiler {

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

    auto inputs_count {SizeField::Decode(properties_)};
    p += (inputs_count + Input::GetPadding(RUNTIME_ARCH, inputs_count)) * sizeof(Input);
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

void DynamicOperands::Reallocate([[maybe_unused]] size_t new_capacity /* =0 */)
{
    if (new_capacity == 0) {
        constexpr auto IMM_2 = 2;
        new_capacity = (((capacity_ != 0U) ? capacity_ : 1U) << 1U) + IMM_2;
    } else if (new_capacity <= capacity_) {
        return;
    }
    auto size = new_capacity * (sizeof(User) + sizeof(Inst *)) + sizeof(Inst *);
    auto new_stor = reinterpret_cast<uintptr_t>(allocator_->Alloc(size));
    CHECK(new_stor != 0);

    auto owner_inst {GetOwnerInst()};
    // Set pointer to owned instruction into new storage NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *reinterpret_cast<Inst **>(reinterpret_cast<User *>(new_stor) + new_capacity) = owner_inst;

    if (users_ == nullptr) {
        users_ = reinterpret_cast<User *>(new_stor);
        capacity_ = new_capacity;
        return;
    }
    Input *old_inputs = Inputs();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *new_inputs = reinterpret_cast<Input *>(new_stor + sizeof(User) * new_capacity) + 1;

    for (size_t i = 0; i < size_; i++) {
        Inst *old_input = old_inputs[i].GetInst();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT(old_input);
        // Initialize new User in container. Since users are placed from end of array, i.e. zero index element
        // will be at the end of array, we need to add capacity and substitute index.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        User *new_user = new (reinterpret_cast<User *>(new_stor) + new_capacity - i - 1) User(false, i, new_capacity);
        auto old_user {GetUser(i)};
        if (owner_inst->IsSaveState()) {
            new_user->SetVirtualRegister(old_user->GetVirtualRegister());
        } else if (owner_inst->IsPhi()) {
            new_user->SetBbNum(old_user->GetBbNum());
        }
        old_input->RemoveUser(old_user);
        old_input->AddUser(new_user);
        new_inputs[i] = Input(old_input);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    capacity_ = new_capacity;
    users_ = reinterpret_cast<User *>(new_stor);
}

unsigned DynamicOperands::Append(Inst *inst)
{
    ASSERT(capacity_ >= size_);
    if (capacity_ == size_) {
        Reallocate();
    }
    SetInput(size_, Input(inst));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    new (users_ + capacity_ - size_ - 1) User(false, size_, capacity_);
    auto user {GetUser(size_)};
    if (GetOwnerInst()->IsPhi()) {
        user->SetBbNum(size_);
    }
    inst->AddUser(user);
    return size_++;
}

void DynamicOperands::Remove(unsigned index)
{
    size_--;
    auto *curr_input = GetInput(index)->GetInst();
    if (curr_input->GetBasicBlock() != nullptr && curr_input->HasUsers()) {
        curr_input->RemoveUser(GetUser(index));
    }

    auto bb_num {GetUser(index)->GetBbNum()};
    auto owner_inst {GetOwnerInst()};

    if (index != size_) {
        auto *last_input = GetInput(size_)->GetInst();
        if (last_input->HasUsers()) {
            last_input->RemoveUser(GetUser(size_));
            last_input->AddUser(GetUser(index));
        }
        SetInput(index, *GetInput(size_));
        if (owner_inst->IsSaveState()) {
            GetUser(index)->SetVirtualRegister(GetUser(size_)->GetVirtualRegister());
        } else if (owner_inst->IsPhi()) {
            GetUser(index)->SetBbNum(GetUser(size_)->GetBbNum());
        }
    }

    if (owner_inst->IsPhi()) {
        for (size_t i {0}; i < size_; ++i) {
            if (GetUser(i)->GetBbNum() == size_) {
                GetUser(i)->SetBbNum(bb_num);
                break;
            }
        }
    }
}

void CompareInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetCc()));
}

void IfInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetCc()));
}

void IfImmInst::SetVnObject(VnObject *vn_obj)
{
    vn_obj->Add(static_cast<uint32_t>(GetCc()));
}

void CmpInst::SetVnObject(VnObject *vn_obj)
{
    if (DataType::IsFloatType(GetOperandsType())) {
        vn_obj->Add(static_cast<uint32_t>(IsFcmpg()));
    }
    vn_obj->Add(static_cast<uint32_t>(GetInputType(0)));
}

BasicBlock *PhiInst::GetPhiInputBb(unsigned index)
{
    ASSERT(index < GetInputsCount());

    auto bb_num {GetPhiInputBbNum(index)};
    ASSERT(bb_num < GetBasicBlock()->GetPredsBlocks().size());
    return GetBasicBlock()->GetPredsBlocks()[bb_num];
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

template <Opcode opc, size_t input_idx>
Inst *SkipInstructions(Inst *input_inst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (Opcode opcode = input_inst->GetOpcode(); opcode == opc; opcode = input_inst->GetOpcode()) {
        input_inst = input_inst->GetInput(input_idx).GetInst();
    }
    return input_inst;
}
/*
 * For instructions LoadArray, StoreArray, LoadArrayPair, StoreArrayPair, LoadArrayI, StoreArrayI, LoadArrayPairI,
 * StoreArrayPairI, LenArray, LoadObject, StoreObject, CallVirtual, Monitor with NullCheck input the dataflow user
 * is object, which is the first input of NullCheck instruction.
 * For instructions LoadArray, StoreArray, LoadArrayPair, StoreArrayPair with BoundsCheck input the dataflow user is
 * array index, which is the second input of BoundsCheck instruction
 * For instructions Div and Mod with ZeroCheck input the dataflow user is the first input of ZeroCheck
 */
Inst *Inst::GetDataFlowInput(Inst *input_inst) const
{
    return input_inst;
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
    auto this_bb = GetBasicBlock();
    auto other_bb = other->GetBasicBlock();
    return this_bb == other_bb ? IsPrecedingInSameBlock(other) : this_bb->IsDominate(other_bb);
}

bool Inst::InSameBlockOrDominate(const Inst *other) const
{
    return GetBasicBlock() == other->GetBasicBlock() || IsDominate(other);
}

Inst *Inst::Clone(const Graph *targetGraph) const
{
    ASSERT(targetGraph != nullptr);
    auto clone = targetGraph->CreateInst(GetOpcode());
    CHECK_NOT_NULL(clone);
    clone->bit_fields_ = GetAllFields();
    clone->pc_ = GetPc();
#ifndef NDEBUG
    clone->SetDstReg(GetDstReg());
#endif
    if (IsOperandsDynamic()) {
        clone->ReserveInputs(GetInputsCount());
    }
    return clone;
}

#if PANDA_TARGET_MACOS
template class FixedInputsInst<0>;
template class FixedInputsInst<1>;
template class FixedInputsInst<2U>;
#endif

Inst *SpillFillInst::Clone(const Graph *targetGraph) const
{
    auto clone = FixedInputsInst::Clone(targetGraph)->CastToSpillFill();
    for (auto spill_fill : spill_fills_) {
        clone->AddSpillFill(spill_fill);
    }
    return clone;
}

Inst *CompareInst::Clone(const Graph *targetGraph) const
{
    auto clone = FixedInputsInst::Clone(targetGraph);
    clone->CastToCompare()->SetCc(GetCc());
    clone->CastToCompare()->SetOperandsType(GetOperandsType());
    return clone;
}

Inst *CmpInst::Clone(const Graph *targetGraph) const
{
    auto clone = FixedInputsInst::Clone(targetGraph);
    clone->CastToCmp()->SetOperandsType(GetOperandsType());
    return clone;
}

Inst *IfInst::Clone(const Graph *targetGraph) const
{
    auto clone = FixedInputsInst::Clone(targetGraph);
    static_cast<IfInst *>(clone)->SetCc(GetCc());
    static_cast<IfInst *>(clone)->SetOperandsType(GetOperandsType());
    static_cast<IfInst *>(clone)->SetMethod(GetMethod());
    return clone;
}

Inst *IntrinsicInst::Clone(const Graph *targetGraph) const
{
    ASSERT(targetGraph != nullptr);
    auto intrinsicClone = Inst::Clone(targetGraph)->CastToIntrinsic();
    intrinsicClone->SetIntrinsicId(GetIntrinsicId());
    CloneTypes(targetGraph->GetAllocator(), intrinsicClone);
    if (HasImms()) {
        for (auto imm : GetImms()) {
            intrinsicClone->AddImm(targetGraph->GetAllocator(), imm);
        }
    }
    return intrinsicClone;
}

Inst *ConstantInst::Clone(const Graph *targetGraph) const
{
    Inst *new_cnst = nullptr;
    bool is_support_int32 = GetBasicBlock()->GetGraph()->IsBytecodeOptimizer();
    switch (GetType()) {
        case DataType::INT32:
            new_cnst = targetGraph->CreateInstConstant(static_cast<int32_t>(GetIntValue()), is_support_int32);
            break;
        case DataType::INT64:
            new_cnst = targetGraph->CreateInstConstant(GetIntValue(), is_support_int32);
            break;
        case DataType::FLOAT32:
            new_cnst = targetGraph->CreateInstConstant(GetFloatValue(), is_support_int32);
            break;
        case DataType::FLOAT64:
            new_cnst = targetGraph->CreateInstConstant(GetDoubleValue(), is_support_int32);
            break;
        case DataType::ANY:
            new_cnst = targetGraph->CreateInstConstant(GetRawValue(), is_support_int32);
            new_cnst->SetType(DataType::ANY);
            break;
        default:
            UNREACHABLE();
    }
#ifndef NDEBUG
    new_cnst->SetDstReg(GetDstReg());
#endif
    return new_cnst;
}

Inst *ParameterInst::Clone(const Graph *targetGraph) const
{
    auto clone = Inst::Clone(targetGraph)->CastToParameter();
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
    return clone;
}

void SaveStateInst::AppendImmediate(uint64_t imm, uint16_t vreg, DataType::Type type, bool is_acc)
{
    if (immediates_ == nullptr) {
        ASSERT(GetBasicBlock() != nullptr);
        AllocateImmediates(GetBasicBlock()->GetGraph()->GetAllocator(), 0);
    }
    immediates_->emplace_back(SaveStateImm {imm, vreg, type, is_acc});
}

void SaveStateInst::AllocateImmediates(ArenaAllocator *allocator, size_t size)
{
    immediates_ = allocator->New<ArenaVector<SaveStateImm>>(allocator->Adapter());
    CHECK_NOT_NULL(immediates_);
    immediates_->resize(size);
}

void TryInst::AppendCatchTypeId(uint32_t id, uint32_t catch_edge_index)
{
    if (catch_type_ids_ == nullptr) {
        ASSERT(catch_edge_indexes_ == nullptr);
        ASSERT(GetBasicBlock() != nullptr);
        auto allocator = GetBasicBlock()->GetGraph()->GetAllocator();
        catch_type_ids_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
        catch_edge_indexes_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
        CHECK_NOT_NULL(catch_type_ids_);
        CHECK_NOT_NULL(catch_edge_indexes_);
    }
    catch_type_ids_->push_back(id);
    catch_edge_indexes_->push_back(catch_edge_index);
}

void CatchPhiInst::AppendThrowableInst(const Inst *inst)
{
    if (throw_insts_ == nullptr) {
        ASSERT(GetBasicBlock() != nullptr);
        auto allocator = GetBasicBlock()->GetGraph()->GetAllocator();
        throw_insts_ = allocator->New<ArenaVector<const Inst *>>(allocator->Adapter());
        CHECK_NOT_NULL(throw_insts_);
    }
    throw_insts_->push_back(inst);
}

void CatchPhiInst::ReplaceThrowableInst(const Inst *old_inst, const Inst *new_inst)
{
    auto index = GetThrowableInstIndex(old_inst);
    throw_insts_->at(index) = new_inst;
}

void CatchPhiInst::RemoveInput(unsigned index)
{
    Inst::RemoveInput(index);
    if (throw_insts_ != nullptr) {
        throw_insts_->at(index) = throw_insts_->back();
        throw_insts_->pop_back();
    }
}

Inst *TryInst::Clone(const Graph *targetGraph) const
{
    auto clone = FixedInputsInst::Clone(targetGraph)->CastToTry();
    if (auto ids_count = this->GetCatchTypeIdsCount(); ids_count > 0) {
        if (clone->catch_type_ids_ == nullptr) {
            auto allocator = targetGraph->GetAllocator();
            clone->catch_type_ids_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
            clone->catch_edge_indexes_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
            CHECK_NOT_NULL(clone->catch_edge_indexes_);
        }
        clone->catch_type_ids_->resize(ids_count);
        clone->catch_edge_indexes_->resize(ids_count);
        std::copy(this->catch_type_ids_->begin(), this->catch_type_ids_->end(), clone->catch_type_ids_->begin());
        std::copy(this->catch_edge_indexes_->begin(), this->catch_edge_indexes_->end(),
                  clone->catch_edge_indexes_->begin());
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
    return CanThrow() && GetBasicBlock()->IsTry();
}

bool Inst::IsZeroRegInst() const
{
    ASSERT(GetBasicBlock() != nullptr);
    ASSERT(GetBasicBlock()->GetGraph() != nullptr);
    return false;
}

bool Inst::IsAccRead() const
{
    return GetFlag(inst_flags::ACC_READ);
}

bool Inst::IsAccWrite() const
{
    if (IsConst()) {
        return true;
    }
    return GetFlag(inst_flags::ACC_WRITE);
}

TryInst *GetTryBeginInst(const BasicBlock *try_begin_bb)
{
    ASSERT(try_begin_bb != nullptr && try_begin_bb->IsTryBegin());
    for (auto inst : try_begin_bb->AllInsts()) {
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
    return false;
}

}  // namespace panda::compiler
