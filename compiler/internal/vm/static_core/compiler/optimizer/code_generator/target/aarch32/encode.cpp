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
/*
Encoder (implementation of math and mem Low-level emitters)
*/

#include <aarch32/disasm-aarch32.h>
#include "compiler/optimizer/code_generator/relocations.h"
#include "encode.h"
#include "scoped_tmp_reg.h"
#include "operands.h"
#include "target/aarch32/target.h"
#include "lib_helpers.inl"
#include "type_info.h"

#ifndef PANDA_TARGET_MACOS
#include "elf.h"
#endif  // PANDA_TARGET_MACOS

#include <iomanip>

namespace ark::compiler::aarch32 {

/// Converters
static vixl::aarch32::Condition Convert(const Condition cc)
{
    switch (cc) {
        case Condition::EQ:
            return vixl::aarch32::eq;
        case Condition::NE:
            return vixl::aarch32::ne;
        case Condition::LT:
            return vixl::aarch32::lt;
        case Condition::GT:
            return vixl::aarch32::gt;
        case Condition::LE:
            return vixl::aarch32::le;
        case Condition::GE:
            return vixl::aarch32::ge;
        case Condition::LO:
            return vixl::aarch32::lo;
        case Condition::LS:
            return vixl::aarch32::ls;
        case Condition::HI:
            return vixl::aarch32::hi;
        case Condition::HS:
            return vixl::aarch32::hs;
        // NOTE(igorban) : Remove them
        case Condition::MI:
            return vixl::aarch32::mi;
        case Condition::PL:
            return vixl::aarch32::pl;
        case Condition::VS:
            return vixl::aarch32::vs;
        case Condition::VC:
            return vixl::aarch32::vc;
        case Condition::AL:
            return vixl::aarch32::al;
        default:
            UNREACHABLE();
            return vixl::aarch32::eq;
    }
}

/// Converters
static vixl::aarch32::Condition ConvertTest(const Condition cc)
{
    ASSERT(cc == Condition::TST_EQ || cc == Condition::TST_NE);
    return cc == Condition::TST_EQ ? vixl::aarch32::eq : vixl::aarch32::ne;
}

static bool IsConditionSigned(Condition cc)
{
    switch (cc) {
        case Condition::LT:
        case Condition::LE:
        case Condition::GT:
        case Condition::GE:
            return true;

        default:
            return false;
    }
}

static vixl::aarch32::DataType Convert(const TypeInfo info, const bool isSigned = false)
{
    if (!isSigned) {
        if (info.IsFloat()) {
            if (info.GetSize() == WORD_SIZE) {
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::F32);
            }
            ASSERT(info.GetSize() == DOUBLE_WORD_SIZE);
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::F64);
        }
        switch (info.GetSize()) {
            case BYTE_SIZE:
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::I8);
            case HALF_SIZE:
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::I16);
            case WORD_SIZE:
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::I32);
            case DOUBLE_WORD_SIZE:
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::I64);
            default:
                break;
        }
    }
    ASSERT(!info.IsFloat());

    switch (info.GetSize()) {
        case BYTE_SIZE:
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::S8);
        case HALF_SIZE:
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::S16);
        case WORD_SIZE:
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::S32);
        case DOUBLE_WORD_SIZE:
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::S64);
        default:
            break;
    }
    return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::kDataTypeValueInvalid);
}

Reg Aarch32Encoder::AcquireScratchRegister(TypeInfo type)
{
    ASSERT(GetMasm()->GetCurrentScratchRegisterScope() == nullptr);
    if (type.IsFloat()) {
        if (type == FLOAT32_TYPE) {
            auto reg = GetMasm()->GetScratchVRegisterList()->GetFirstAvailableSRegister();
            ASSERT(reg.IsValid());
            GetMasm()->GetScratchVRegisterList()->Remove(reg);
            return Reg(reg.GetCode(), type);
        }
        // Get 2 float registers instead one double - to save agreement about hi-regs in VixlVReg
        auto reg1 = GetMasm()->GetScratchVRegisterList()->GetFirstAvailableSRegister();
        auto reg2 = GetMasm()->GetScratchVRegisterList()->GetFirstAvailableSRegister();
        ASSERT(reg1.IsValid());
        ASSERT(reg2.IsValid());
        GetMasm()->GetScratchVRegisterList()->Remove(reg1);
        GetMasm()->GetScratchVRegisterList()->Remove(reg2);
        return Reg(reg1.GetCode(), type);
    }
    auto reg = GetMasm()->GetScratchRegisterList()->GetFirstAvailableRegister();
    ASSERT(reg.IsValid());
    GetMasm()->GetScratchRegisterList()->Remove(reg);
    return Reg(reg.GetCode(), type);
}

LabelHolder *Aarch32Encoder::GetLabels() const
{
    return labels_;
}

constexpr auto Aarch32Encoder::GetTarget()
{
    return ark::compiler::Target(Arch::AARCH32);
}

void Aarch32Encoder::AcquireScratchRegister(Reg reg)
{
    if (reg == GetTarget().GetLinkReg()) {
        ASSERT_PRINT(!lrAcquired_, "Trying to acquire LR, which hasn't been released before");
        lrAcquired_ = true;
    } else if (reg.IsFloat()) {
        ASSERT(GetMasm()->GetScratchVRegisterList()->IncludesAliasOf(vixl::aarch32::SRegister(reg.GetId())));
        GetMasm()->GetScratchVRegisterList()->Remove(vixl::aarch32::SRegister(reg.GetId()));
    } else {
        ASSERT(GetMasm()->GetScratchRegisterList()->Includes(vixl::aarch32::Register(reg.GetId())));
        GetMasm()->GetScratchRegisterList()->Remove(vixl::aarch32::Register(reg.GetId()));
    }
}

void Aarch32Encoder::ReleaseScratchRegister(Reg reg)
{
    if (reg == GetTarget().GetLinkReg()) {
        ASSERT_PRINT(lrAcquired_, "Trying to release LR, which hasn't been acquired before");
        lrAcquired_ = false;
    } else if (reg.IsFloat()) {
        GetMasm()->GetScratchVRegisterList()->Combine(vixl::aarch32::SRegister(reg.GetId()));
    } else {
        GetMasm()->GetScratchRegisterList()->Combine(vixl::aarch32::Register(reg.GetId()));
    }
}

bool Aarch32Encoder::IsScratchRegisterReleased(Reg reg) const
{
    if (reg == GetTarget().GetLinkReg()) {
        return !lrAcquired_;
    }
    if (reg.IsFloat()) {
        return GetMasm()->GetScratchVRegisterList()->IncludesAliasOf(vixl::aarch32::SRegister(reg.GetId()));
    }
    return GetMasm()->GetScratchRegisterList()->Includes(vixl::aarch32::Register(reg.GetId()));
}

void Aarch32Encoder::SetRegister(RegMask *mask, VRegMask *vmask, Reg reg, bool val) const
{
    if (!reg.IsValid()) {
        return;
    }
    if (reg.IsScalar()) {
        ASSERT(mask != nullptr);
        mask->set(reg.GetId(), val);
        if (reg.GetSize() > WORD_SIZE) {
            mask->set(reg.GetId() + 1, val);
        }
    } else {
        ASSERT(vmask != nullptr);
        ASSERT(reg.IsFloat());
        vmask->set(reg.GetId(), val);
        if (reg.GetSize() > WORD_SIZE) {
            vmask->set(reg.GetId() + 1, val);
        }
    }
}
vixl::aarch32::MemOperand Aarch32Encoder::ConvertMem(MemRef mem)
{
    bool hasIndex = mem.HasIndex();
    bool hasShift = mem.HasScale();
    bool hasOffset = mem.HasDisp();
    auto baseReg = VixlReg(mem.GetBase());
    if (hasIndex) {
        // MemRef with index and offser isn't supported
        ASSERT(!hasOffset);
        auto indexReg = mem.GetIndex();
        if (hasShift) {
            auto shift = mem.GetScale();
            return vixl::aarch32::MemOperand(baseReg, VixlReg(indexReg), vixl::aarch32::LSL, shift);
        }
        return vixl::aarch32::MemOperand(baseReg, VixlReg(indexReg));
    }
    if (hasOffset) {
        auto offset = mem.GetDisp();
        return vixl::aarch32::MemOperand(baseReg, offset);
    }
    return vixl::aarch32::MemOperand(baseReg);
}

size_t Aarch32Encoder::GetCursorOffset() const
{
    return GetMasm()->GetBuffer()->GetCursorOffset();
}

void Aarch32Encoder::SetCursorOffset(size_t offset)
{
    GetMasm()->GetBuffer()->Rewind(offset);
}

RegMask Aarch32Encoder::GetScratchRegistersMask() const
{
    return RegMask(GetMasm()->GetScratchRegisterList()->GetList());
}

RegMask Aarch32Encoder::GetScratchFpRegistersMask() const
{
    return RegMask(GetMasm()->GetScratchVRegisterList()->GetList());
}

RegMask Aarch32Encoder::GetAvailableScratchRegisters() const
{
    return RegMask(GetMasm()->GetScratchRegisterList()->GetList());
}

VRegMask Aarch32Encoder::GetAvailableScratchFpRegisters() const
{
    return VRegMask(GetMasm()->GetScratchVRegisterList()->GetList());
}

LabelHolder::LabelId Aarch32LabelHolder::CreateLabel()
{
    ++id_;
    auto allocator = GetEncoder()->GetAllocator();
    auto *label = allocator->New<LabelType>(allocator);
    labels_.push_back(label);
    ASSERT(labels_.size() == id_);
    return id_ - 1;
}

void Aarch32LabelHolder::CreateLabels(LabelId size)
{
    for (LabelId i = 0; i <= size; ++i) {
        CreateLabel();
    }
}

void Aarch32LabelHolder::BindLabel(LabelId id)
{
    static_cast<Aarch32Encoder *>(GetEncoder())->GetMasm()->Bind(labels_[id]);
}

Aarch32LabelHolder::LabelType *Aarch32LabelHolder::GetLabel(LabelId id)
{
    ASSERT(labels_.size() > id);
    return labels_[id];
}

LabelHolder::LabelId Aarch32LabelHolder::Size()
{
    return labels_.size();
}

TypeInfo Aarch32Encoder::GetRefType()
{
    return INT32_TYPE;
}

void *Aarch32Encoder::BufferData() const
{
    return GetMasm()->GetBuffer()->GetStartAddress<void *>();
}

size_t Aarch32Encoder::BufferSize() const
{
    return GetMasm()->GetBuffer()->GetSizeInBytes();
}

void Aarch32Encoder::SaveRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp)
{
    LoadStoreRegisters<true>(registers, slot, startReg, isFp);
}
void Aarch32Encoder::LoadRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp)
{
    LoadStoreRegisters<false>(registers, slot, startReg, isFp);
}

void Aarch32Encoder::SaveRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask)
{
    LoadStoreRegisters<true>(registers, slot, base, mask, isFp);
}
void Aarch32Encoder::LoadRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask)
{
    LoadStoreRegisters<false>(registers, slot, base, mask, isFp);
}

vixl::aarch32::MacroAssembler *Aarch32Encoder::GetMasm() const
{
    ASSERT(masm_ != nullptr);
    return masm_;
}

size_t Aarch32Encoder::GetLabelAddress(LabelHolder::LabelId label)
{
    auto plabel = labels_->GetLabel(label);
    ASSERT(plabel->IsBound());
    return GetMasm()->GetBuffer()->GetOffsetAddress<size_t>(plabel->GetLocation());
}

bool Aarch32Encoder::LabelHasLinks(LabelHolder::LabelId label)
{
    auto plabel = labels_->GetLabel(label);
    return plabel->IsReferenced();
}

bool Aarch32Encoder::IsValid() const
{
    return true;
}

void Aarch32Encoder::SetMaxAllocatedBytes(size_t size)
{
    GetMasm()->GetBuffer()->SetMmapMaxBytes(size);
}
Aarch32Encoder::Aarch32Encoder(ArenaAllocator *allocator) : Encoder(allocator, Arch::AARCH32)
{
    labels_ = allocator->New<Aarch32LabelHolder>(this);
    if (labels_ == nullptr) {
        SetFalseResult();
    }
    EnableLrAsTempReg(true);
}

Aarch32Encoder::~Aarch32Encoder()
{
    auto labels = static_cast<Aarch32LabelHolder *>(GetLabels())->labels_;
    for (auto label : labels) {
        label->~Label();
    }
    if (masm_ != nullptr) {
        masm_->~MacroAssembler();
        masm_ = nullptr;
    }
}

bool Aarch32Encoder::InitMasm()
{
    if (masm_ == nullptr) {
        auto allocator = GetAllocator();

        // Initialize Masm
        masm_ = allocator->New<vixl::aarch32::MacroAssembler>(allocator);
        if (masm_ == nullptr || !masm_->IsValid()) {
            SetFalseResult();
            return false;
        }

        ASSERT(masm_);
        for (auto regCode : AARCH32_TMP_REG) {
            masm_->GetScratchRegisterList()->Combine(vixl::aarch32::Register(regCode));
        }
        for (auto vregCode : AARCH32_TMP_VREG) {
            masm_->GetScratchVRegisterList()->Combine(vixl::aarch32::SRegister(vregCode));
        }

        // Make sure that the compiler uses the same scratch registers as the assembler
        CHECK_EQ(RegMask(masm_->GetScratchRegisterList()->GetList()), GetTarget().GetTempRegsMask());
        CHECK_EQ(RegMask(masm_->GetScratchVRegisterList()->GetList()), GetTarget().GetTempVRegsMask());
    }
    return true;
}

void Aarch32Encoder::Finalize()
{
    GetMasm()->FinalizeCode();
}

void Aarch32Encoder::EncodeJump(LabelHolder::LabelId id)
{
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label);
}

void Aarch32Encoder::EncodeJump(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    CompareHelper(src0, src1, &cc);
    GetMasm()->B(Convert(cc), label);
}

void Aarch32Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    TestHelper(src0, src1, cc);
    GetMasm()->B(ConvertTest(cc), label);
}

void Aarch32Encoder::EncodeBitTestAndBranch(LabelHolder::LabelId id, Reg reg, uint32_t bitPos, bool bitValue)
{
    ASSERT(reg.IsScalar() && reg.GetSize() > bitPos);
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    if (reg.GetSize() == DOUBLE_WORD_SIZE) {
        if (bitPos < WORD_SIZE) {
            GetMasm()->tst(VixlReg(reg), VixlImm(1U << bitPos));
        } else {
            GetMasm()->tst(VixlRegU(reg), VixlImm(1U << (bitPos - WORD_SIZE)));
        }
    } else {
        GetMasm()->tst(VixlReg(reg), VixlImm(1U << bitPos));
    }
    if (bitValue) {
        GetMasm()->B(Convert(Condition::NE), label);
    } else {
        GetMasm()->B(Convert(Condition::EQ), label);
    }
}

bool Aarch32Encoder::CompareImmHelper(Reg src, int64_t imm, Condition *cc)
{
    ASSERT(src.IsScalar());
    ASSERT(imm != 0);
    ASSERT(-static_cast<int64_t>(UINT32_MAX) <= imm && imm <= UINT32_MAX);
    ASSERT(CanEncodeImmAddSubCmp(imm, src.GetSize(), IsConditionSigned(*cc)));

    return imm < 0 ? CompareNegImmHelper(src, imm, cc) : ComparePosImmHelper(src, imm, cc);
}

void Aarch32Encoder::TestImmHelper(Reg src, Imm imm, [[maybe_unused]] Condition cc)
{
    auto value = imm.GetAsInt();
    ASSERT(src.IsScalar());
    ASSERT(cc == Condition::TST_EQ || cc == Condition::TST_NE);
    ASSERT(CanEncodeImmLogical(value, src.GetSize()));

    if (src.GetSize() <= WORD_SIZE) {
        GetMasm()->Tst(VixlReg(src), VixlImm(value));
    } else {
        GetMasm()->Tst(VixlRegU(src), VixlImm(0x0));
        GetMasm()->Tst(Convert(Condition::EQ), VixlReg(src), VixlImm(value));
    }
}

bool Aarch32Encoder::CompareNegImmHelper(Reg src, int64_t value, const Condition *cc)
{
    if (src.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmn(VixlReg(src), VixlImm(-value));
    } else {
        if (!IsConditionSigned(*cc)) {
            GetMasm()->Cmn(VixlRegU(src), VixlImm(0x1));
            GetMasm()->Cmn(Convert(Condition::EQ), VixlReg(src), VixlImm(-value));
        } else {
            // There are no effective implementation in this case
            // Can't get here because of logic behind CanEncodeImmAddSubCmp
            UNREACHABLE();
            SetFalseResult();
            return false;
        }
    }
    return true;
}

Condition Aarch32Encoder::TrySwapCc(Condition cc, bool *swap)
{
    switch (cc) {
        case Condition::GT:
            *swap = true;
            return Condition::LT;
        case Condition::LE:
            *swap = true;
            return Condition::GE;
        case Condition::GE:
        case Condition::LT:
            return cc;
        default:
            UNREACHABLE();
    }
}

bool Aarch32Encoder::ComparePosImmHelper(Reg src, int64_t value, Condition *cc)
{
    if (src.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src), VixlImm(value));
    } else {
        if (!IsConditionSigned(*cc)) {
            GetMasm()->Cmp(VixlRegU(src), VixlImm(0x0));
            GetMasm()->Cmp(Convert(Condition::EQ), VixlReg(src), VixlImm(value));
        } else {
            bool swap = false;
            *cc = TrySwapCc(*cc, &swap);

            ScopedTmpRegU32 tmpReg(this);
            if (swap) {
                GetMasm()->Rsbs(VixlReg(tmpReg), VixlReg(src), VixlImm(value));
                GetMasm()->Rscs(VixlReg(tmpReg), VixlRegU(src), vixl::aarch32::Operand(0x0));
            } else {
                GetMasm()->Cmp(VixlReg(src), VixlImm(value));
                GetMasm()->Sbcs(VixlReg(tmpReg), VixlRegU(src), vixl::aarch32::Operand(0x0));
            }
        }
    }
    return true;
}

void Aarch32Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    auto value = imm.GetAsInt();
    if (value == 0) {
        EncodeJump(id, src, cc);
        return;
    }

    if (!CompareImmHelper(src, value, &cc)) {
        return;
    }

    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(Convert(cc), label);
}

void Aarch32Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    ASSERT(src.IsScalar());

    TestImmHelper(src, imm, cc);
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(ConvertTest(cc), label);
}

void Aarch32Encoder::CompareZeroHelper(Reg src, Condition *cc)
{
    ASSERT(src.IsScalar());
    if (src.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src), vixl::aarch32::Operand(0x0));
    } else {
        ScopedTmpRegU32 tmpReg(this);
        uint32_t imm = 0x0;

        switch (*cc) {
            case Condition::EQ:
            case Condition::NE:
                GetMasm()->Orrs(VixlReg(tmpReg), VixlReg(src), VixlRegU(src));
                break;
            case Condition::LE:
                imm = 0x1;
                *cc = Condition::LT;
                /* fallthrough */
                [[fallthrough]];
            case Condition::LT:
                GetMasm()->Cmp(VixlReg(src), vixl::aarch32::Operand(imm));
                GetMasm()->Sbcs(VixlReg(tmpReg), VixlRegU(src), vixl::aarch32::Operand(0x0));
                break;
            case Condition::GT:
                imm = 0x1;
                *cc = Condition::GE;
                /* fallthrough */
                [[fallthrough]];
            case Condition::GE:
                GetMasm()->Cmp(VixlReg(src), vixl::aarch32::Operand(imm));
                GetMasm()->Sbcs(VixlReg(tmpReg), VixlRegU(src), vixl::aarch32::Operand(0x0));
                break;
            default:
                UNREACHABLE();
        }
    }
}

void Aarch32Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Condition cc)
{
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    ASSERT(src.IsScalar());

    switch (cc) {
        case Condition::LO:
            // LO is always false
            return;
        case Condition::HS:
            // HS is always true
            GetMasm()->B(label);
            return;
        case Condition::HI:
            // HI is same as NE
            cc = Condition::NE;
            break;
        case Condition::LS:
            // LS is same as EQ
            cc = Condition::EQ;
            break;
        default:
            break;
    }

    CompareZeroHelper(src, &cc);

    GetMasm()->B(Convert(cc), label);
}

void Aarch32Encoder::EncodeJump(Reg dst)
{
    GetMasm()->Bx(VixlReg(dst));
}

void Aarch32Encoder::EncodeJump([[maybe_unused]] RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    auto buffer = GetMasm()->GetBuffer();
    relocation->offset = GetCursorOffset();
    relocation->addend = 0;
    relocation->type = R_ARM_CALL;
    static constexpr uint32_t CALL_WITH_ZERO_OFFSET = 0xeafffffe;
    buffer->Emit32(CALL_WITH_ZERO_OFFSET);
#endif
}

void Aarch32Encoder::EncodeNop()
{
    GetMasm()->Nop();
}

void Aarch32Encoder::MakeCall([[maybe_unused]] compiler::RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    auto buffer = GetMasm()->GetBuffer();
    relocation->offset = GetCursorOffset();
    relocation->addend = 0;
    relocation->type = R_ARM_CALL;
    static constexpr uint32_t CALL_WITH_ZERO_OFFSET = 0xebfffffe;
    buffer->Emit32(CALL_WITH_ZERO_OFFSET);
#endif
}

void Aarch32Encoder::MakeCall(const void *entryPoint)
{
    ScopedTmpRegU32 tmpReg(this);

    auto entry = static_cast<int32_t>(reinterpret_cast<int64_t>(entryPoint));
    EncodeMov(tmpReg, Imm(entry));
    GetMasm()->Blx(VixlReg(tmpReg));
}

void Aarch32Encoder::MakeCall(MemRef entryPoint)
{
    ScopedTmpRegU32 tmpReg(this);

    EncodeLdr(tmpReg, false, entryPoint);
    GetMasm()->Blx(VixlReg(tmpReg));
}

void Aarch32Encoder::MakeCall(Reg reg)
{
    GetMasm()->Blx(VixlReg(reg));
}

void Aarch32Encoder::MakeCallAot([[maybe_unused]] intptr_t offset)
{
    // Unimplemented
    SetFalseResult();
}

void Aarch32Encoder::MakeCallByOffset([[maybe_unused]] intptr_t offset)
{
    // Unimplemented
    SetFalseResult();
}

void Aarch32Encoder::MakeLoadAotTable([[maybe_unused]] intptr_t offset, [[maybe_unused]] Reg reg)
{
    // Unimplemented
    SetFalseResult();
}

void Aarch32Encoder::MakeLoadAotTableAddr([[maybe_unused]] intptr_t offset, [[maybe_unused]] Reg addr,
                                          [[maybe_unused]] Reg val)
{
    // Unimplemented
    SetFalseResult();
}

void Aarch32Encoder::EncodeReturn()
{
    GetMasm()->Bx(vixl::aarch32::lr);
}

void Aarch32Encoder::EncodeAbort()
{
    GetMasm()->Udf(0);
}

void Aarch32Encoder::EncodeMul([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src, [[maybe_unused]] Imm imm)
{
    SetFalseResult();
}

/**
 * The function construct additional instruction for encode memory instructions and returns MemOperand for ldr/str
 * LDR/STR with immediate offset(for A32)
 * |  mem type  | offset size |
 * | ---------- | ----------- |
 * |word or byte|-4095 to 4095|
 * |   others   | -255 to 255 |
 *
 * LDR/STR with register offset(for A32)
 * |  mem type  |    shift    |
 * | ---------- | ----------- |
 * |word or byte|    0 to 31  |
 * |   others   |      --     |
 *
 * VLDR and VSTR has base and offset. The offset must be a multiple of 4, and lie in the range -1020 to +1020.
 */
/* static */
bool Aarch32Encoder::IsNeedToPrepareMemLdS(MemRef mem, const TypeInfo &memType, bool isSigned)
{
    bool hasIndex = mem.HasIndex();
    bool hasShift = mem.HasScale();
    bool hasOffset = mem.HasDisp();
    ASSERT(mem.HasBase());
    // VLDR and VSTR has base and offset. The offset must be a multiple of 4, and lie in the range -1020 to +1020.
    if (memType.IsFloat()) {
        if (hasIndex) {
            return true;
        }
        if (hasOffset) {
            auto offset = mem.GetDisp();
            constexpr int32_t IMM_4 = 4;
            if (!((offset >= -VMEM_OFFSET) && (offset <= VMEM_OFFSET) && ((offset % IMM_4) == 0))) {
                return true;
            }
        }
        return false;
    }
    int32_t maxOffset = MEM_SMALL_OFFSET;
    bool hasLsl = false;
    if (!isSigned && (memType == INT32_TYPE || memType == INT8_TYPE)) {
        hasLsl = true;
        maxOffset = MEM_BIG_OFFSET;
    }
    // LDR/STR with register offset(for A32)
    // |  mem type  |    shift    |
    // | ---------- | ----------- |
    // |word or byte|    0 to 31  |
    // |   others   |      --     |
    //
    if (hasIndex) {
        // MemRef with index and offser isn't supported
        ASSERT(!hasOffset);
        if (hasShift) {
            ASSERT(mem.GetScale() != 0);
            [[maybe_unused]] constexpr int32_t MAX_SHIFT = 3;
            ASSERT(mem.GetScale() <= MAX_SHIFT);
            if (!hasLsl) {
                return true;
            }
        }
        return false;
    }
    // LDR/STR with immediate offset(for A32)
    // |  mem type  | offset size |
    // | ---------- | ----------- |
    // |word or byte|-4095 to 4095|
    // |   others   | -255 to 255 |
    //
    if (hasOffset) {
        ASSERT(mem.GetDisp() != 0);
        [[maybe_unused]] auto offset = mem.GetDisp();
        if (!((offset >= -maxOffset) && (offset <= maxOffset))) {
            return true;
        }
    }
    return false;
}

/**
 * The function construct additional instruction for encode memory instructions and returns MemOperand for ldr/str
 * LDR/STR with immediate offset(for A32)
 * |  mem type  | offset size |
 * | ---------- | ----------- |
 * |word or byte|-4095 to 4095|
 * |   others   | -255 to 255 |
 *
 * LDR/STR with register offset(for A32)
 * |  mem type  |    shift    |
 * | ---------- | ----------- |
 * |word or byte|    0 to 31  |
 * |   others   |      --     |
 *
 * VLDR and VSTR has base and offset. The offset must be a multiple of 4, and lie in the range -1020 to +1020.
 */
vixl::aarch32::MemOperand Aarch32Encoder::PrepareMemLdS(MemRef mem, const TypeInfo &memType,
                                                        vixl::aarch32::Register tmp, bool isSigned, bool copySp)
{
    bool hasIndex = mem.HasIndex();
    bool hasShift = mem.HasScale();
    bool hasOffset = mem.HasDisp();
    ASSERT(mem.HasBase());
    auto baseReg = VixlReg(mem.GetBase());
    if (copySp) {
        if (baseReg.IsSP()) {
            GetMasm()->Mov(tmp, baseReg);
            baseReg = tmp;
        }
    }
    // VLDR and VSTR has base and offset. The offset must be a multiple of 4, and lie in the range -1020 to +1020.
    if (memType.IsFloat()) {
        return PrepareMemLdSForFloat(mem, tmp);
    }
    int32_t maxOffset = MEM_SMALL_OFFSET;
    bool hasLsl = false;
    if (!isSigned && (memType == INT32_TYPE || memType == INT8_TYPE)) {
        hasLsl = true;
        maxOffset = MEM_BIG_OFFSET;
    }
    // LDR/STR with register offset(for A32)
    // |  mem type  |    shift    |
    // | ---------- | ----------- |
    // |word or byte|    0 to 31  |
    // |   others   |      --     |
    //
    if (hasIndex) {
        // MemRef with index and offser isn't supported
        ASSERT(!hasOffset);
        auto indexReg = mem.GetIndex();
        if (hasShift) {
            ASSERT(mem.GetScale() != 0);
            auto shift = mem.GetScale();
            [[maybe_unused]] constexpr int32_t MAX_SHIFT = 3;
            ASSERT(mem.GetScale() <= MAX_SHIFT);
            if (hasLsl) {
                return vixl::aarch32::MemOperand(baseReg, VixlReg(indexReg), vixl::aarch32::LSL, shift);
            }
            // from:
            //   mem: base, index, scale
            // to:
            //   add tmp, base, index, scale
            //   mem tmp
            GetMasm()->Add(tmp, baseReg, vixl::aarch32::Operand(VixlReg(indexReg), vixl::aarch32::LSL, shift));
            return vixl::aarch32::MemOperand(tmp);
        }
        return vixl::aarch32::MemOperand(baseReg, VixlReg(indexReg));
    }
    // LDR/STR with immediate offset(for A32):
    // |  mem type  | offset size |
    // | ---------- | ----------- |
    // |word or byte|-4095 to 4095|
    // |   others   | -255 to 255 |
    //
    if (hasOffset) {
        ASSERT(mem.GetDisp() != 0);
        auto offset = mem.GetDisp();
        if ((offset >= -maxOffset) && (offset <= maxOffset)) {
            return vixl::aarch32::MemOperand(baseReg, offset);
        }
        // from:
        //   mem: base, offset
        // to:
        //   add tmp, base, offset
        //   mem tmp
        GetMasm()->Add(tmp, baseReg, VixlImm(offset));
        baseReg = tmp;
    }
    return vixl::aarch32::MemOperand(baseReg);
}

vixl::aarch32::MemOperand Aarch32Encoder::PrepareMemLdSForFloat(MemRef mem, vixl::aarch32::Register tmp)
{
    bool hasIndex = mem.HasIndex();
    bool hasShift = mem.HasScale();
    bool hasOffset = mem.HasDisp();
    auto baseReg = VixlReg(mem.GetBase());
    if (hasIndex) {
        auto indexReg = mem.GetIndex();
        auto scale = mem.GetScale();
        // from:
        //   vmem: base, index, scale, offset
        // to:
        //   add tmp, base, index, scale
        //   vmem tmp, offset
        if (hasShift) {
            ASSERT(scale != 0);
            [[maybe_unused]] constexpr int32_t MAX_SHIFT = 3;
            ASSERT(scale > 0 && scale <= MAX_SHIFT);
            GetMasm()->Add(tmp, baseReg, vixl::aarch32::Operand(VixlReg(indexReg), vixl::aarch32::LSL, scale));
        } else {
            ASSERT(scale == 0);
            GetMasm()->Add(tmp, baseReg, VixlReg(indexReg));
        }
        baseReg = tmp;
    }
    if (hasOffset) {
        ASSERT(mem.GetDisp() != 0);
        auto offset = mem.GetDisp();
        constexpr int32_t IMM_4 = 4;
        if ((offset >= -VMEM_OFFSET) && (offset <= VMEM_OFFSET) && ((offset % IMM_4) == 0)) {
            return vixl::aarch32::MemOperand(baseReg, offset);
        }
        // from:
        //   vmem: base, offset
        // to:
        //   add tmp, base, offset
        //   vmem tmp
        GetMasm()->Add(tmp, baseReg, VixlImm(offset));
        baseReg = tmp;
    }
    return vixl::aarch32::MemOperand(baseReg);
}

void Aarch32Encoder::EncodeFpToBits(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    if (dst.GetSize() == WORD_SIZE) {
        ASSERT(src.GetSize() == WORD_SIZE);

        constexpr uint32_t NANF = 0x7fc00000U;

        GetMasm()->Vcmp(VixlVReg(src), VixlVReg(src));
        GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
        GetMasm()->Vmov(VixlReg(dst), VixlVReg(src).S());
        GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(NANF));
    } else {
        ASSERT(src.GetSize() == DOUBLE_WORD_SIZE);

        constexpr uint32_t NAND_HIGH = 0x7ff80000U;

        GetMasm()->Vcmp(VixlVReg(src), VixlVReg(src));
        GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
        GetMasm()->Vmov(VixlReg(dst), VixlRegU(dst), VixlVReg(src).D());
        GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(0));
        GetMasm()->Mov(Convert(Condition::NE), VixlRegU(dst), VixlImm(NAND_HIGH));
    }
}

void Aarch32Encoder::EncodeMoveBitsRaw(Reg dst, Reg src)
{
    ASSERT((dst.IsFloat() && src.IsScalar()) || (src.IsFloat() && dst.IsScalar()));
    if (dst.IsScalar()) {
        ASSERT(src.GetSize() == dst.GetSize());
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(VixlReg(dst), VixlVReg(src).S());
        } else {
            GetMasm()->Vmov(VixlReg(dst), VixlRegU(dst), VixlVReg(src).D());
        }
    } else {
        ASSERT(dst.GetSize() == src.GetSize());
        if (src.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(VixlVReg(dst).S(), VixlReg(src));
        } else {
            GetMasm()->Vmov(VixlVReg(dst).D(), VixlReg(src), VixlRegU(src));
        }
    }
}
void Aarch32Encoder::EncodeMov(Reg dst, Reg src)
{
    if (src.GetSize() <= WORD_SIZE && dst.GetSize() == DOUBLE_WORD_SIZE && !src.IsFloat() && !dst.IsFloat()) {
        SetFalseResult();
        return;
    }
    if (dst == src) {
        return;
    }
    if (dst.IsFloat()) {
        if (src.GetType().IsScalar()) {
            if (src.GetSize() == DOUBLE_WORD_SIZE) {
                GetMasm()->Vmov(VixlVReg(dst).D(), VixlReg(src), VixlRegU(src));
                return;
            }
            GetMasm()->Vmov(VixlVReg(dst).S(), VixlReg(src));
            return;
        }
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(VixlVReg(dst).S(), VixlVReg(src));
        } else {
            GetMasm()->Vmov(VixlVReg(dst).D(), VixlVReg(src));
        }
        return;
    }
    ASSERT(dst.IsScalar());
    if (src.IsFloat()) {
        if (src.GetSize() == DOUBLE_WORD_SIZE) {
            GetMasm()->Vmov(VixlReg(dst), VixlRegU(dst), VixlVReg(src).D());
            return;
        }
        GetMasm()->Vmov(VixlReg(dst), VixlVReg(src).S());
        return;
    }
    ASSERT(src.IsScalar());
    GetMasm()->Mov(VixlReg(dst), VixlReg(src));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(VixlRegU(dst), VixlRegU(src));
    }
}

void Aarch32Encoder::EncodeNeg(Reg dst, Reg src)
{
    if (dst.IsFloat()) {
        GetMasm()->Vneg(VixlVReg(dst), VixlVReg(src));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Rsb(VixlReg(dst), VixlReg(src), VixlImm(0x0));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Rsbs(VixlReg(dst), VixlReg(src), VixlImm(0x0));
    GetMasm()->Rsc(VixlRegU(dst), VixlRegU(src), VixlImm(0x0));
}

void Aarch32Encoder::EncodeAbs(Reg dst, Reg src)
{
    if (dst.IsFloat()) {
        GetMasm()->Vabs(VixlVReg(dst), VixlVReg(src));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src), VixlImm(0x0));
        GetMasm()->Rsb(Convert(Condition::MI), VixlReg(dst), VixlReg(src), VixlImm(0x0));
        GetMasm()->Mov(Convert(Condition::PL), VixlReg(dst), VixlReg(src));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 loReg(this);
    ScopedTmpRegU32 hiReg(this);

    GetMasm()->Rsbs(VixlReg(loReg), VixlReg(src), VixlImm(0x0));
    GetMasm()->Rsc(VixlReg(hiReg), VixlRegU(src), VixlImm(0x0));
    GetMasm()->Cmp(VixlRegU(src), VixlImm(0x0));
    GetMasm()->Mov(Convert(Condition::PL), VixlReg(loReg), VixlReg(src));
    GetMasm()->Mov(Convert(Condition::PL), VixlReg(hiReg), VixlRegU(src));

    GetMasm()->Mov(VixlReg(dst), VixlReg(loReg));
    GetMasm()->Mov(VixlRegU(dst), VixlReg(hiReg));
}

void Aarch32Encoder::EncodeSqrt(Reg dst, Reg src)
{
    ASSERT(dst.IsFloat());
    GetMasm()->Vsqrt(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeNot([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    GetMasm()->Mvn(VixlReg(dst), VixlReg(src));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Mvn(VixlRegU(dst), VixlRegU(src));
    }
}

void Aarch32Encoder::EncodeIsInf(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());

    if (src.GetSize() == WORD_SIZE) {
        constexpr uint32_t INF_MASK = 0xff000000;

        ScopedTmpRegU32 tmp(this);

        GetMasm()->Vmov(VixlReg(tmp), VixlVReg(src).S());
        GetMasm()->Lsl(VixlReg(tmp), VixlReg(tmp), 1); /* 0xff000000 if Infinity */
        GetMasm()->Mov(VixlReg(dst), INF_MASK);
        GetMasm()->Cmp(VixlReg(dst), VixlReg(tmp));
    } else {
        constexpr uint32_t INF_MASK = 0xffe00000;

        ScopedTmpRegU32 tmp(this);
        ScopedTmpRegU32 tmp1(this);

        GetMasm()->Vmov(VixlReg(tmp), VixlReg(tmp1), VixlVReg(src).D());
        GetMasm()->Lsl(VixlReg(tmp1), VixlReg(tmp1), 1); /* 0xffe00000 if Infinity */
        GetMasm()->Mov(VixlReg(dst), INF_MASK);
        GetMasm()->Cmp(VixlReg(dst), VixlReg(tmp1));
    }

    GetMasm()->Mov(VixlReg(dst), VixlImm(0));
    GetMasm()->Mov(Convert(Condition::EQ), VixlReg(dst), VixlImm(1));
}

void Aarch32Encoder::EncodeCmpFracWithZero(Reg src)
{
    ASSERT(src.IsFloat());
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    // Encode (fabs(src - trunc(src)) <= DELTA)
    if (src.GetSize() == WORD_SIZE) {
        ScopedTmpRegF32 tmp(this);
        vixl::aarch32::SRegister tmp1(tmp.GetReg().GetId() + 1);
        GetMasm()->Vrintz(VixlVReg(tmp).S(), VixlVReg(src).S());
        EncodeSub(tmp, src, tmp);
        EncodeAbs(tmp, tmp);
        GetMasm()->Vmov(tmp1, 0.0);
        GetMasm()->Vcmp(VixlVReg(tmp).S(), tmp1);
    } else {
        ScopedTmpRegF64 tmp(this);
        vixl::aarch32::DRegister tmp1(tmp.GetReg().GetId() + 1);
        GetMasm()->Vrintz(VixlVReg(tmp).D(), VixlVReg(src).D());
        EncodeSub(tmp, src, tmp);
        EncodeAbs(tmp, tmp);
        GetMasm()->Vmov(tmp1, 0.0);
        GetMasm()->Vcmp(VixlVReg(tmp).D(), tmp1);
    }
    GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
}

void Aarch32Encoder::EncodeIsInteger(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    auto labelExit = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelInfOrNan = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    EncodeCmpFracWithZero(src);
    GetMasm()->B(vixl::aarch32::vs, labelInfOrNan);  // Inf or NaN
    GetMasm()->Mov(vixl::aarch32::le, VixlReg(dst), 0x1);
    GetMasm()->Mov(vixl::aarch32::gt, VixlReg(dst), 0x0);
    GetMasm()->B(labelExit);

    // IsInteger returns false if src is Inf or NaN
    GetMasm()->Bind(labelInfOrNan);
    EncodeMov(dst, Imm(false));

    GetMasm()->Bind(labelExit);
}

void Aarch32Encoder::EncodeIsSafeInteger(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    auto labelExit = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelFalse = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    // Check if IsInteger
    EncodeCmpFracWithZero(src);
    GetMasm()->B(vixl::aarch32::vs, labelFalse);  // Inf or NaN
    GetMasm()->B(vixl::aarch32::gt, labelFalse);

    // Check if it is safe, i.e. src can be represented in float/double without losing precision
    if (src.GetSize() == WORD_SIZE) {
        ScopedTmpRegF32 tmp1(this);
        vixl::aarch32::SRegister tmp2(tmp1.GetReg().GetId() + 1);
        EncodeAbs(tmp1, src);
        GetMasm()->Vmov(tmp2, MaxIntAsExactFloat());
        GetMasm()->Vcmp(VixlVReg(tmp1).S(), tmp2);
    } else {
        ScopedTmpRegF64 tmp1(this);
        vixl::aarch32::DRegister tmp2(tmp1.GetReg().GetId() + 1);
        EncodeAbs(tmp1, src);
        GetMasm()->Vmov(tmp2, MaxIntAsExactDouble());
        GetMasm()->Vcmp(VixlVReg(tmp1).D(), tmp2);
    }
    GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
    GetMasm()->Mov(vixl::aarch32::le, VixlReg(dst), 0x1);
    GetMasm()->Mov(vixl::aarch32::gt, VixlReg(dst), 0x0);
    GetMasm()->B(labelExit);

    // Return false if src !IsInteger
    GetMasm()->Bind(labelFalse);
    EncodeMov(dst, Imm(false));

    GetMasm()->Bind(labelExit);
}

void Aarch32Encoder::EncodeReverseBytes(Reg dst, Reg src)
{
    ASSERT(src.GetSize() > BYTE_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());

    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Rev16(VixlReg(dst), VixlReg(src));
        GetMasm()->Sxth(VixlReg(dst), VixlReg(dst));
    } else if (src.GetSize() == WORD_SIZE) {
        GetMasm()->Rev(VixlReg(dst), VixlReg(src));
    } else {
        if (src == dst) {
            ScopedTmpRegU32 tmpReg(this);
            GetMasm()->Mov(VixlReg(tmpReg), VixlReg(src));
            GetMasm()->Rev(VixlReg(dst), VixlRegU(src));
            GetMasm()->Rev(VixlRegU(dst), VixlReg(tmpReg));
        } else {
            GetMasm()->Rev(VixlRegU(dst), VixlReg(src));
            GetMasm()->Rev(VixlReg(dst), VixlRegU(src));
        }
    }
}

void Aarch32Encoder::EncodeBitCount([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Aarch32Encoder::EncodeCountLeadingZeroBits(Reg dst, Reg src)
{
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);
    if (src.GetSize() == WORD_SIZE) {
        GetMasm()->Clz(VixlReg(dst), VixlReg(src));
        return;
    }

    auto low = CreateLabel();
    auto end = CreateLabel();
    auto highBits = Reg(src.GetId() + 1, INT32_TYPE);
    EncodeJump(low, highBits, Condition::EQ);
    GetMasm()->Clz(VixlReg(dst), VixlReg(highBits));
    EncodeJump(end);

    BindLabel(low);
    GetMasm()->Clz(VixlReg(dst), VixlReg(src));
    GetMasm()->Adds(VixlReg(dst), VixlReg(dst), VixlImm(WORD_SIZE));

    BindLabel(end);
}

void Aarch32Encoder::EncodeCeil(Reg dst, Reg src)
{
    GetMasm()->Vrintp(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeFloor(Reg dst, Reg src)
{
    GetMasm()->Vrintm(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeRint(Reg dst, Reg src)
{
    GetMasm()->Vrintn(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeTrunc(Reg dst, Reg src)
{
    GetMasm()->Vrintz(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeRoundAway(Reg dst, Reg src)
{
    GetMasm()->Vrinta(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeRoundToPInfReturnScalar(Reg dst, Reg src)
{
    ScopedTmpRegF64 temp(this);
    vixl::aarch32::SRegister temp1(temp.GetReg().GetId());
    vixl::aarch32::SRegister temp2(temp.GetReg().GetId() + 1);

    auto done = CreateLabel();
    // round to nearest integer, ties away from zero
    GetMasm()->Vcvta(vixl::aarch32::DataTypeValue::S32, vixl::aarch32::DataTypeValue::F32, temp1, VixlVReg(src).S());
    GetMasm()->Vmov(VixlReg(dst), temp1);
    // for positive, zero and NaN inputs, rounding is done
    GetMasm()->Cmp(VixlReg(dst), 0);
    GetMasm()->B(vixl::aarch32::ge, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(done));
    // if input is negative but not a tie, round to nearest is valid.
    // if input is a negative tie, change rounding direction to positive infinity, dst += 1.
    GetMasm()->Vrinta(vixl::aarch32::DataTypeValue::F32, temp1, VixlVReg(src).S());
    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->Vmov(temp2, 0.5F);
    GetMasm()->Vsub(vixl::aarch32::DataTypeValue::F32, temp1, VixlVReg(src).S(), temp1);
    GetMasm()->Vcmp(vixl::aarch32::DataTypeValue::F32, temp1, temp2);
    GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
    GetMasm()->add(vixl::aarch32::eq, VixlReg(dst), VixlReg(dst), 1);

    BindLabel(done);
}

void Aarch32Encoder::EncodeRoundToPInfReturnFloat(Reg dst, Reg src)
{
    ASSERT(src.GetType() == FLOAT64_TYPE);
    ASSERT(dst.GetType() == FLOAT64_TYPE);

    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double HALF = 0.5;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double NEG_HALF = -0.5;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double ONE = 1.0;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double ZERO = 0.0;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double NEG_ZERO = -0.0;

    ScopedTmpRegF64 ceil(this);
    vixl::aarch32::DRegister tmp(ceil.GetReg().GetId() + 1);

    auto skipZero = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto negZeroCase = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto done = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Vmov(tmp, NEG_HALF);
    GetMasm()->Vcmp(VixlVReg(src), tmp);
    GetMasm()->Vmrs(vixl::aarch32::APSR_nzcv, vixl::aarch32::FPSCR);
    GetMasm()->B(vixl::aarch32::lt, skipZero);

    GetMasm()->Vmov(tmp, HALF);
    GetMasm()->Vcmp(VixlVReg(src), tmp);
    GetMasm()->Vmrs(vixl::aarch32::APSR_nzcv, vixl::aarch32::FPSCR);
    GetMasm()->B(vixl::aarch32::ge, skipZero);

    GetMasm()->Vmov(tmp, NEG_HALF);
    GetMasm()->Vcmp(VixlVReg(src), tmp);
    GetMasm()->Vmrs(vixl::aarch32::APSR_nzcv, vixl::aarch32::FPSCR);
    GetMasm()->B(vixl::aarch32::lt, negZeroCase);

    GetMasm()->Vmov(tmp, ZERO);
    GetMasm()->Vmov(VixlVReg(dst), tmp);
    GetMasm()->B(done);

    GetMasm()->Bind(negZeroCase);
    GetMasm()->Vmov(tmp, NEG_ZERO);
    GetMasm()->Vmov(VixlVReg(dst), tmp);
    GetMasm()->B(done);

    GetMasm()->Bind(skipZero);

    // calculate ceil(val)
    GetMasm()->Vrintp(VixlVReg(ceil), VixlVReg(src));

    // compare ceil(val) - val with 0.5
    GetMasm()->Vsub(VixlVReg(dst), VixlVReg(ceil), VixlVReg(src));
    GetMasm()->Vmov(tmp, HALF);
    GetMasm()->Vcmp(VixlVReg(dst), tmp);
    GetMasm()->Vmrs(vixl::aarch32::APSR_nzcv, vixl::aarch32::FPSCR);

    // calculate ceil(val) - 1
    GetMasm()->Vmov(tmp, ONE);
    GetMasm()->Vsub(VixlVReg(dst), VixlVReg(ceil), tmp);

    // final value is ceil(val) if le
    GetMasm()->Vmov(Convert(Condition::LE), VixlVReg(dst), VixlVReg(ceil));  // ceil(val)
    GetMasm()->Bind(done);
}

void Aarch32Encoder::EncodeReverseBits(Reg dst, Reg src)
{
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());

    if (src.GetSize() == WORD_SIZE) {
        GetMasm()->Rbit(VixlReg(dst), VixlReg(src));
        return;
    }

    if (src == dst) {
        ScopedTmpRegU32 tmpReg(this);
        GetMasm()->Mov(VixlReg(tmpReg), VixlReg(src));
        GetMasm()->Rbit(VixlReg(dst), VixlRegU(src));
        GetMasm()->Rbit(VixlRegU(dst), VixlReg(tmpReg));
        return;
    }

    GetMasm()->Rbit(VixlRegU(dst), VixlReg(src));
    GetMasm()->Rbit(VixlReg(dst), VixlRegU(src));
}

void Aarch32Encoder::EncodeCastToBool(Reg dst, Reg src)
{
    // In ISA says that we only support casts:
    // i32tou1, i64tou1, u32tou1, u64tou1
    ASSERT(src.IsScalar());
    ASSERT(dst.IsScalar());

    GetMasm()->Cmp(VixlReg(src), VixlImm(0x0));
    GetMasm()->Mov(Convert(Condition::EQ), VixlReg(dst), VixlImm(0x0));
    GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(0x1));
    if (src.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Cmp(VixlRegU(src), VixlImm(0x0));
        GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(0x1));
    }
}

void Aarch32Encoder::EncodeCast(Reg dst, bool dstSigned, Reg src, bool srcSigned)
{
    // float/double -> float/double
    if (dst.IsFloat() && src.IsFloat()) {
        EncodeCastFloatToFloat(dst, src);
        return;
    }

    // uint/int -> float/double
    if (dst.IsFloat() && src.IsScalar()) {
        EncodeCastScalarToFloat(dst, src, srcSigned);
        return;
    }

    // float/double -> uint/int
    if (dst.IsScalar() && src.IsFloat()) {
        EncodeCastFloatToScalar(dst, dstSigned, src);
        return;
    }

    // uint/int -> uint/int
    ASSERT(dst.IsScalar() && src.IsScalar());
    EncodeCastScalar(dst, dstSigned, src, srcSigned);
}

void Aarch32Encoder::EncodeCastScalar(Reg dst, bool dstSigned, Reg src, bool srcSigned)
{
    size_t srcSize = src.GetSize();
    size_t dstSize = dst.GetSize();
    // In our ISA minimal type is 32-bit, so type less then 32-bit
    // we should extend to 32-bit. So we can have 2 cast
    // (For examble, i8->u16 will work as i8->u16 and u16->u32)
    if (dstSize < WORD_SIZE) {
        if (srcSize > dstSize) {
            if (dstSigned) {
                EncodeCastScalarFromSignedScalar(dst, src);
            } else {
                EncodeCastScalarFromUnsignedScalar(dst, src);
            }
            return;
        }
        if (srcSize == dstSize) {
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (srcSigned == dstSigned) {
                return;
            }
            if (dstSigned) {
                EncodeCastScalarFromSignedScalar(Reg(dst.GetId(), INT32_TYPE), dst);
            } else {
                EncodeCastScalarFromUnsignedScalar(Reg(dst.GetId(), INT32_TYPE), dst);
            }
            return;
        }
        if (srcSigned) {
            EncodeCastScalarFromSignedScalar(dst, src);
            if (!dstSigned) {
                EncodeCastScalarFromUnsignedScalar(Reg(dst.GetId(), INT32_TYPE), dst);
            }
        } else {
            EncodeCastScalarFromUnsignedScalar(dst, src);
            if (dstSigned) {
                EncodeCastScalarFromSignedScalar(Reg(dst.GetId(), INT32_TYPE), dst);
            }
        }
    } else {
        if (srcSize == dstSize) {
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (srcSize == DOUBLE_WORD_SIZE) {
                GetMasm()->Mov(VixlRegU(dst), VixlRegU(src));
            }
            return;
        }

        if (srcSigned) {
            EncodeCastScalarFromSignedScalar(dst, src);
        } else {
            EncodeCastScalarFromUnsignedScalar(dst, src);
        }
    }
}

void Aarch32Encoder::EncodeCastFloatToFloat(Reg dst, Reg src)
{
    // float/double -> float/double
    if (dst.GetSize() == src.GetSize()) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(VixlVReg(dst).S(), VixlVReg(src));
        } else {
            GetMasm()->Vmov(VixlVReg(dst).D(), VixlVReg(src));
        }
        return;
    }

    // double -> float
    if (dst.GetSize() == WORD_SIZE) {
        GetMasm()->Vcvt(vixl::aarch32::DataTypeValue::F32, vixl::aarch32::DataTypeValue::F64, VixlVReg(dst).S(),
                        VixlVReg(src).D());
    } else {
        // float -> double
        GetMasm()->Vcvt(vixl::aarch32::DataTypeValue::F64, vixl::aarch32::DataTypeValue::F32, VixlVReg(dst).D(),
                        VixlVReg(src).S());
    }
}

void Aarch32Encoder::EncodeCastScalarToFloat(Reg dst, Reg src, bool srcSigned)
{
    // uint/int -> float/double
    switch (src.GetSize()) {
        case BYTE_SIZE:
        case HALF_SIZE:
        case WORD_SIZE: {
            ScopedTmpRegF32 tmpReg(this);

            GetMasm()->Vmov(VixlVReg(tmpReg).S(), VixlReg(src));
            auto dataType = srcSigned ? vixl::aarch32::DataTypeValue::S32 : vixl::aarch32::DataTypeValue::U32;
            if (dst.GetSize() == WORD_SIZE) {
                GetMasm()->Vcvt(vixl::aarch32::DataTypeValue::F32, dataType, VixlVReg(dst).S(), VixlVReg(tmpReg).S());
            } else {
                GetMasm()->Vcvt(vixl::aarch32::DataTypeValue::F64, dataType, VixlVReg(dst).D(), VixlVReg(tmpReg).S());
            }
            break;
        }
        case DOUBLE_WORD_SIZE: {
            if (dst.GetSize() == WORD_SIZE) {
                if (srcSigned) {
                    // int64 -> float
                    MakeLibCall(dst, src, reinterpret_cast<void *>(AEABIl2f));
                } else {
                    // uint64 -> float
                    MakeLibCall(dst, src, reinterpret_cast<void *>(AEABIul2f));
                }
            } else {
                if (srcSigned) {
                    // int64 -> double
                    MakeLibCall(dst, src, reinterpret_cast<void *>(AEABIl2d));
                } else {
                    // uint64 -> double
                    MakeLibCall(dst, src, reinterpret_cast<void *>(AEABIul2d));
                }
            }
            break;
        }
        default:
            SetFalseResult();
            break;
    }
}

void Aarch32Encoder::EncodeCastFloatToScalar(Reg dst, bool dstSigned, Reg src)
{
    // We DON'T support casts from float32/64 to int8/16 and bool, because this caste is not declared anywhere
    // in other languages and architecture, we do not know what the behavior should be.
    // But there is one implementation in other function: "EncodeCastFloatToScalarWithSmallDst". Call it in the
    // "EncodeCast" function instead of "EncodeCastFloat". It works as follows: cast from float32/64 to int32, moving
    // sign bit from int32 to dst type, then extend number from dst type to int32 (a necessary condition for an isa).
    // All work in dst register.
    ASSERT(dst.GetSize() >= WORD_SIZE);

    switch (dst.GetSize()) {
        case WORD_SIZE: {
            ScopedTmpRegF32 tmpReg(this);

            auto dataType = dstSigned ? vixl::aarch32::DataTypeValue::S32 : vixl::aarch32::DataTypeValue::U32;
            if (src.GetSize() == WORD_SIZE) {
                GetMasm()->Vcvt(dataType, vixl::aarch32::DataTypeValue::F32, VixlVReg(tmpReg).S(), VixlVReg(src).S());
            } else {
                GetMasm()->Vcvt(dataType, vixl::aarch32::DataTypeValue::F64, VixlVReg(tmpReg).S(), VixlVReg(src).D());
            }

            GetMasm()->Vmov(VixlReg(dst), VixlVReg(tmpReg).S());
            break;
        }
        case DOUBLE_WORD_SIZE: {
            EncodeCastToDoubleWord(dst, dstSigned, src);
            break;
        }
        default:
            SetFalseResult();
            break;
    }
}

void Aarch32Encoder::EncodeCastToDoubleWord(Reg dst, bool dstSigned, Reg src)
{
    if (src.GetSize() == WORD_SIZE) {
        if (dstSigned) {
            // float -> int64
            EncodeCastFloatToInt64(dst, src);
        } else {
            // float -> uint64
            MakeLibCall(dst, src, reinterpret_cast<void *>(AEABIf2ulz));
        }
    } else {
        if (dstSigned) {
            // double -> int64
            EncodeCastDoubleToInt64(dst, src);
        } else {
            // double -> uint64
            MakeLibCall(dst, src, reinterpret_cast<void *>(AEABId2ulz));
        }
    }
}

void Aarch32Encoder::EncodeCastFloatToScalarWithSmallDst(Reg dst, bool dstSigned, Reg src)
{
    switch (dst.GetSize()) {
        case BYTE_SIZE:
        case HALF_SIZE:
        case WORD_SIZE: {
            ScopedTmpRegF32 tmpReg(this);

            auto dataType = dstSigned ? vixl::aarch32::DataTypeValue::S32 : vixl::aarch32::DataTypeValue::U32;
            if (src.GetSize() == WORD_SIZE) {
                GetMasm()->Vcvt(dataType, vixl::aarch32::DataTypeValue::F32, VixlVReg(tmpReg).S(), VixlVReg(src).S());
            } else {
                GetMasm()->Vcvt(dataType, vixl::aarch32::DataTypeValue::F64, VixlVReg(tmpReg).S(), VixlVReg(src).D());
            }

            GetMasm()->Vmov(VixlReg(dst), VixlVReg(tmpReg).S());
            if (dst.GetSize() < WORD_SIZE) {
                EncoderCastExtendFromInt32(dst, dstSigned);
            }
            break;
        }
        case DOUBLE_WORD_SIZE: {
            EncodeCastToDoubleWord(dst, dstSigned, src);
            break;
        }
        default:
            SetFalseResult();
            break;
    }
}

void Aarch32Encoder::EncoderCastExtendFromInt32(Reg dst, bool dstSigned)
{
    if (dstSigned) {
        constexpr uint32_t TEST_BIT = (1U << (static_cast<uint32_t>(WORD_SIZE) - 1));
        ScopedTmpReg tmpReg(this, dst.GetType());

        uint32_t setBit = (dst.GetSize() == BYTE_SIZE) ? (1U << static_cast<uint32_t>(BYTE_SIZE - 1))
                                                       : (1U << static_cast<uint32_t>(HALF_SIZE - 1));
        uint32_t remBit = setBit - 1;
        GetMasm()->And(VixlReg(tmpReg), VixlReg(dst), TEST_BIT);
        auto labelSkip = CreateLabel();
        auto labelEndIf = CreateLabel();
        EncodeJump(labelSkip, tmpReg, Condition::EQ);
        // Set signed bit in dst reg (accordingly destination type)
        // If signed bit == 1
        GetMasm()->Orr(VixlReg(dst), VixlReg(dst), setBit);
        EncodeJump(labelEndIf);
        BindLabel(labelSkip);
        // If signed bit == 0
        GetMasm()->And(VixlReg(dst), VixlReg(dst), remBit);
        BindLabel(labelEndIf);
    }
    EncodeCastScalar(Reg(dst.GetId(), INT32_TYPE), dstSigned, dst, dstSigned);
}

void Aarch32Encoder::EncodeCastDoubleToInt64(Reg dst, Reg src)
{
    auto labelCheckNan = CreateLabel();
    auto labelNotNan = CreateLabel();
    auto labelExit = CreateLabel();
    ScopedTmpReg tmpReg1(this, INT32_TYPE);

    // Mov double to 2x reg to storage double in hex format and work with them
    EncodeMov(dst, src);
    // See the exponent of number
    GetMasm()->Ubfx(VixlReg(tmpReg1), VixlRegU(dst), START_EXP_DOUBLE, SIZE_EXP_DOUBLE);
    // Max exponent that we can load in int64
    // Check that x > MIN_INT64 & x < MAX_INT64, else jump
    GetMasm()->Cmp(VixlReg(tmpReg1), VixlImm(POSSIBLE_EXP_DOUBLE));
    // If greater than or equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::hs, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(labelCheckNan));
    MakeLibCall(dst, src, reinterpret_cast<void *>(AEABId2lz));
    EncodeJump(labelExit);

    BindLabel(labelCheckNan);
    // Form of nan number
    GetMasm()->Cmp(VixlReg(tmpReg1), VixlImm(UP_BITS_NAN_DOUBLE));
    // If not equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::ne, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(labelNotNan));

    GetMasm()->Orrs(VixlReg(tmpReg1), VixlReg(dst),
                    vixl::aarch32::Operand(VixlRegU(dst), vixl::aarch32::LSL, SHIFT_BITS_DOUBLE));
    auto addrLabel = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(labelNotNan);
    GetMasm()->B(vixl::aarch32::eq, addrLabel);
    GetMasm()->Mov(VixlReg(dst), VixlImm(0));
    GetMasm()->Mov(VixlRegU(dst), VixlImm(0));
    EncodeJump(labelExit);

    BindLabel(labelNotNan);
    GetMasm()->Adds(VixlRegU(dst), VixlRegU(dst), VixlRegU(dst));
    GetMasm()->Mov(VixlReg(dst), VixlImm(UINT32_MAX));
    GetMasm()->Mov(VixlRegU(dst), VixlImm(INT32_MAX));
    GetMasm()->Adc(VixlReg(dst), VixlReg(dst), VixlImm(0));
    // If exponent negative, transform maxint64 to minint64
    GetMasm()->Adc(VixlRegU(dst), VixlRegU(dst), VixlImm(0));

    BindLabel(labelExit);
}

void Aarch32Encoder::EncodeCastFloatToInt64(Reg dst, Reg src)
{
    auto labelCheckNan = CreateLabel();
    auto labelNotNan = CreateLabel();
    auto labelExit = CreateLabel();

    ScopedTmpReg tmpReg(this, INT32_TYPE);
    ScopedTmpReg movedSrc(this, INT32_TYPE);
    EncodeMov(movedSrc, src);
    // See the exponent of number
    GetMasm()->Ubfx(VixlReg(tmpReg), VixlReg(movedSrc), START_EXP_FLOAT, SIZE_EXP_FLOAT);
    // Max exponent that we can load in int64
    // Check that x > MIN_INT64 & x < MAX_INT64, else jump
    GetMasm()->Cmp(VixlReg(tmpReg), VixlImm(POSSIBLE_EXP_FLOAT));
    // If greater than or equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::hs, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(labelCheckNan));
    MakeLibCall(dst, src, reinterpret_cast<void *>(AEABIf2lz));
    EncodeJump(labelExit);

    BindLabel(labelCheckNan);
    // Form of nan number
    GetMasm()->Cmp(VixlReg(tmpReg), VixlImm(UP_BITS_NAN_FLOAT));
    // If not equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::ne, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(labelNotNan));

    GetMasm()->Lsls(VixlReg(tmpReg), VixlReg(movedSrc), vixl::aarch32::Operand(SHIFT_BITS_FLOAT));
    // If equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::eq, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(labelNotNan));
    GetMasm()->Mov(VixlReg(dst), VixlImm(0));
    GetMasm()->Mov(VixlRegU(dst), VixlImm(0));
    EncodeJump(labelExit);

    BindLabel(labelNotNan);
    GetMasm()->Adds(VixlReg(movedSrc), VixlReg(movedSrc), VixlReg(movedSrc));
    GetMasm()->Mov(VixlReg(dst), VixlImm(UINT32_MAX));
    GetMasm()->Mov(VixlRegU(dst), VixlImm(INT32_MAX));
    GetMasm()->Adc(VixlReg(dst), VixlReg(dst), VixlImm(0));
    // If exponent negative, transform maxint64 to minint64
    GetMasm()->Adc(VixlRegU(dst), VixlRegU(dst), VixlImm(0));

    BindLabel(labelExit);
}

void Aarch32Encoder::EncodeCastScalarFromSignedScalar(Reg dst, Reg src)
{
    size_t srcSize = src.GetSize();
    size_t dstSize = dst.GetSize();
    if (srcSize > dstSize) {
        srcSize = dstSize;
    }
    switch (srcSize) {
        case BYTE_SIZE:
            GetMasm()->Sxtb(VixlReg(dst), VixlReg(src));
            break;
        case HALF_SIZE:
            GetMasm()->Sxth(VixlReg(dst), VixlReg(src));
            break;
        case WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            break;
        case DOUBLE_WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (dstSize == DOUBLE_WORD_SIZE) {
                GetMasm()->Mov(VixlReg(dst), VixlReg(src));
                return;
            }
            break;
        default:
            SetFalseResult();
            break;
    }
    if (dstSize == DOUBLE_WORD_SIZE) {
        GetMasm()->Asr(VixlRegU(dst), VixlReg(dst), VixlImm(WORD_SIZE - 1));
    }
}

void Aarch32Encoder::EncodeCastScalarFromUnsignedScalar(Reg dst, Reg src)
{
    size_t srcSize = src.GetSize();
    size_t dstSize = dst.GetSize();
    if (srcSize > dstSize && dstSize < WORD_SIZE) {
        // We need to cut the number, if it is less, than 32-bit. It is by ISA agreement.
        int64_t cutValue = (1ULL << dstSize) - 1;
        GetMasm()->And(VixlReg(dst), VixlReg(src), VixlImm(cutValue));
        return;
    }
    // Else unsigned extend
    switch (srcSize) {
        case BYTE_SIZE:
            GetMasm()->Uxtb(VixlReg(dst), VixlReg(src));
            break;
        case HALF_SIZE:
            GetMasm()->Uxth(VixlReg(dst), VixlReg(src));
            break;
        case WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            break;
        case DOUBLE_WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (dstSize == DOUBLE_WORD_SIZE) {
                GetMasm()->Mov(VixlReg(dst), VixlReg(src));
                return;
            }
            break;
        default:
            SetFalseResult();
            break;
    }
    if (dstSize == DOUBLE_WORD_SIZE) {
        GetMasm()->Mov(VixlRegU(dst), VixlImm(0x0));
    }
}

void Aarch32Encoder::EncodeAdd(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        ASSERT(src0.IsFloat() && src1.IsFloat());
        GetMasm()->Vadd(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Add(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Adds(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    GetMasm()->Adc(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
}

void Aarch32Encoder::EncodeSub(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Vsub(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Sub(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Subs(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    GetMasm()->Sbc(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
}

void Aarch32Encoder::EncodeMul(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Vmul(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Mul(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 loReg(this);
    ScopedTmpRegU32 hiReg(this);

    GetMasm()->Umull(VixlReg(loReg), VixlReg(hiReg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Mla(VixlReg(hiReg), VixlRegU(src0), VixlReg(src1), VixlReg(hiReg));
    GetMasm()->Mla(VixlReg(hiReg), VixlReg(src0), VixlRegU(src1), VixlReg(hiReg));

    GetMasm()->Mov(VixlReg(dst), VixlReg(loReg));
    GetMasm()->Mov(VixlRegU(dst), VixlReg(hiReg));
}

void Aarch32Encoder::MakeLibCall(Reg dst, Reg src0, Reg src1, void *entryPoint, bool secondValue)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        MakeLibCallWithFloatResult(dst, src0, src1, entryPoint, secondValue);
        return;
    }

    if (dst.GetType() == FLOAT64_TYPE) {
        MakeLibCallWithDoubleResult(dst, src0, src1, entryPoint, secondValue);
        return;
    }

    if (dst.GetType() == INT64_TYPE) {
        MakeLibCallWithInt64Result(dst, src0, src1, entryPoint, secondValue);
        return;
    }

    ASSERT(dst.GetSize() < DOUBLE_WORD_SIZE);

    if (src1.GetId() == vixl::aarch32::r0.GetCode() || src0 == src1) {
        ScopedTmpRegU32 tmp(this);
        GetMasm()->Mov(VixlReg(tmp), VixlReg(src1));
        GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src0));
        GetMasm()->Mov(vixl::aarch32::r1, VixlReg(tmp));
    } else {
        GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src0));
        GetMasm()->Mov(vixl::aarch32::r1, VixlReg(src1));
    }

    // Call lib-method
    MakeCall(entryPoint);

    auto dstRegister = secondValue ? vixl::aarch32::r1 : vixl::aarch32::r0;

    if (dst.GetId() <= vixl::aarch32::r3.GetCode()) {
        ScopedTmpRegU32 tmp(this);
        GetMasm()->Mov(VixlReg(tmp), dstRegister);
        GetMasm()->Mov(VixlReg(dst), VixlReg(tmp));
    } else {
        GetMasm()->Mov(VixlReg(dst), dstRegister);
    }
}

void Aarch32Encoder::MakeLibCallWithFloatResult(Reg dst, Reg src0, Reg src1, void *entryPoint, bool secondValue)
{
#if (PANDA_TARGET_ARM32_ABI_HARD)
    // gnueabihf
    // use double parameters
    if (src1.GetId() == vixl::aarch32::s0.GetCode() || src0 == src1) {
        ScopedTmpRegF32 tmp(this);
        GetMasm()->Vmov(VixlVReg(tmp), VixlVReg(src1));
        GetMasm()->Vmov(vixl::aarch32::s0, VixlVReg(src0));
        GetMasm()->Vmov(vixl::aarch32::s1, VixlVReg(tmp));
    } else {
        GetMasm()->Vmov(vixl::aarch32::s0, VixlVReg(src0));
        GetMasm()->Vmov(vixl::aarch32::s1, VixlVReg(src1));
    }

    MakeCall(entryPoint);

    auto dstRegister = secondValue ? vixl::aarch32::s1 : vixl::aarch32::s0;
    if (dst.GetId() <= vixl::aarch32::s1.GetCode()) {
        ScopedTmpRegF32 tmp(this);
        GetMasm()->Vmov(VixlVReg(tmp).S(), dstRegister);
        GetMasm()->Vmov(VixlVReg(dst).S(), VixlVReg(tmp).S());
    } else {
        GetMasm()->Vmov(VixlVReg(dst).S(), dstRegister);
    }
#else
    // gnueabi
    // use scalar parameters
    GetMasm()->Vmov(vixl::aarch32::r0, VixlVReg(src0).S());
    GetMasm()->Vmov(vixl::aarch32::r1, VixlVReg(src1).S());

    MakeCall(entryPoint);

    auto dstRegister = secondValue ? vixl::aarch32::r1 : vixl::aarch32::r0;
    GetMasm()->Vmov(VixlVReg(dst).S(), dstRegister);
#endif  // PANDA_TARGET_ARM32_ABI_HARD
}

void Aarch32Encoder::MakeLibCallWithDoubleResult(Reg dst, Reg src0, Reg src1, void *entryPoint, bool secondValue)
{
#if (PANDA_TARGET_ARM32_ABI_HARD)
    // Scope for temp
    if (src1.GetId() == vixl::aarch32::d0.GetCode() || src0 == src1) {
        ScopedTmpRegF64 tmp(this);
        GetMasm()->Vmov(VixlVReg(tmp), VixlVReg(src1));
        GetMasm()->Vmov(vixl::aarch32::d0, VixlVReg(src0));
        GetMasm()->Vmov(vixl::aarch32::d1, VixlVReg(tmp));
    } else {
        GetMasm()->Vmov(vixl::aarch32::d0, VixlVReg(src0));
        GetMasm()->Vmov(vixl::aarch32::d1, VixlVReg(src1));
    }
    MakeCall(entryPoint);
    auto dstRegister = secondValue ? vixl::aarch32::d1 : vixl::aarch32::d0;

    if (dst.GetId() <= vixl::aarch32::d1.GetCode()) {
        ScopedTmpRegF64 tmp(this);
        GetMasm()->Vmov(VixlVReg(tmp), dstRegister);
        GetMasm()->Vmov(VixlVReg(dst), VixlVReg(tmp));
    } else {
        GetMasm()->Vmov(VixlVReg(dst), dstRegister);
    }

    // use double parameters
#else
    // use scalar parameters
    GetMasm()->Vmov(vixl::aarch32::r0, vixl::aarch32::r1, VixlVReg(src0).D());
    GetMasm()->Vmov(vixl::aarch32::r2, vixl::aarch32::r3, VixlVReg(src1).D());

    MakeCall(entryPoint);

    auto dstRegister1 = secondValue ? vixl::aarch32::r2 : vixl::aarch32::r0;
    auto dstRegister2 = secondValue ? vixl::aarch32::r3 : vixl::aarch32::r1;

    GetMasm()->Vmov(VixlVReg(dst).D(), dstRegister1, dstRegister2);
#endif  // PANDA_TARGET_ARM32_ABI_HARD
}

void Aarch32Encoder::MakeLibCallWithInt64Result(Reg dst, Reg src0, Reg src1, void *entryPoint, bool secondValue)
{
    // Here I look only for this case, because src - is mapped on two regs.
    // (INT64_TYPE), and src0 can't be rewrited
    // NOTE(igorban) If src0==src1 - the result will be 1 or UB(if src0 = 0)
    // It is better to check them in optimizations
    // ASSERT(src0 != src1); - do not enable for tests

    if (src1.GetId() == vixl::aarch32::r0.GetCode() || src0 == src1) {
        ScopedTmpRegU32 tmp1(this);
        ScopedTmpRegU32 tmp2(this);
        GetMasm()->Mov(VixlReg(tmp1), VixlReg(src1));
        GetMasm()->Mov(VixlReg(tmp2), VixlRegU(src1));
        GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src0));
        GetMasm()->Mov(vixl::aarch32::r1, VixlRegU(src0));
        GetMasm()->Mov(vixl::aarch32::r2, VixlReg(tmp1));
        GetMasm()->Mov(vixl::aarch32::r3, VixlReg(tmp2));
    } else {
        GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src0));
        GetMasm()->Mov(vixl::aarch32::r1, VixlRegU(src0));
        GetMasm()->Mov(vixl::aarch32::r2, VixlReg(src1));
        GetMasm()->Mov(vixl::aarch32::r3, VixlRegU(src1));
    }

    // Call lib-method
    MakeCall(entryPoint);

    auto dstRegister1 = secondValue ? vixl::aarch32::r2 : vixl::aarch32::r0;
    auto dstRegister2 = secondValue ? vixl::aarch32::r3 : vixl::aarch32::r1;

    if (dst.GetId() <= vixl::aarch32::r3.GetCode()) {
        ScopedTmpRegU32 tmp1(this);
        ScopedTmpRegU32 tmp2(this);
        GetMasm()->Mov(VixlReg(tmp1), dstRegister1);
        GetMasm()->Mov(VixlReg(tmp2), dstRegister2);
        GetMasm()->Mov(VixlReg(dst), VixlReg(tmp1));
        GetMasm()->Mov(VixlRegU(dst), VixlReg(tmp2));
    } else {
        GetMasm()->Mov(VixlReg(dst), dstRegister1);
        GetMasm()->Mov(VixlRegU(dst), dstRegister2);
    }
}

void Aarch32Encoder::MakeLibCallFromScalarToFloat(Reg dst, Reg src, void *entryPoint)
{
    if (src.GetSize() != DOUBLE_WORD_SIZE) {
        SetFalseResult();
        return;
    }

    bool saveR1 {src.GetId() != vixl::aarch32::r0.GetCode() || dst.GetType() == FLOAT64_TYPE};

    GetMasm()->Push(vixl::aarch32::r0);
    if (saveR1) {
        GetMasm()->Push(vixl::aarch32::r1);
    }

    if (src.GetId() != vixl::aarch32::r0.GetCode()) {
        GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src));
        GetMasm()->Mov(vixl::aarch32::r1, VixlRegU(src));
    }

    MakeCall(entryPoint);

    if (dst.GetType() == FLOAT64_TYPE) {
        GetMasm()->Vmov(VixlVReg(dst).D(), vixl::aarch32::r0, vixl::aarch32::r1);
    } else {
        GetMasm()->Vmov(VixlVReg(dst).S(), vixl::aarch32::r0);
    }

    if (saveR1) {
        GetMasm()->Pop(vixl::aarch32::r1);
    }
    GetMasm()->Pop(vixl::aarch32::r0);
}

void Aarch32Encoder::MakeLibCallFromFloatToScalar(Reg dst, Reg src, void *entryPoint)
{
    if (dst.GetSize() != DOUBLE_WORD_SIZE) {
        SetFalseResult();
        return;
    }

    bool saveR0R1 {dst.GetId() != vixl::aarch32::r0.GetCode()};

    if (saveR0R1) {
        GetMasm()->Push(vixl::aarch32::r0);
        GetMasm()->Push(vixl::aarch32::r1);
    }

    if (src.GetType() == FLOAT64_TYPE) {
        GetMasm()->Vmov(vixl::aarch32::r0, vixl::aarch32::r1, VixlVReg(src).D());
    } else {
        GetMasm()->Vmov(vixl::aarch32::r0, VixlVReg(src).S());
    }

    MakeCall(entryPoint);

    if (saveR0R1) {
        GetMasm()->Mov(VixlReg(dst), vixl::aarch32::r0);
        GetMasm()->Mov(VixlRegU(dst), vixl::aarch32::r1);

        GetMasm()->Pop(vixl::aarch32::r1);
        GetMasm()->Pop(vixl::aarch32::r0);
    }
}

void Aarch32Encoder::MakeLibCall(Reg dst, Reg src, void *entryPoint)
{
    if (dst.IsFloat() && src.IsScalar()) {
        MakeLibCallFromScalarToFloat(dst, src, entryPoint);
    } else if (dst.IsScalar() && src.IsFloat()) {
        MakeLibCallFromFloatToScalar(dst, src, entryPoint);
    } else {
        SetFalseResult();
        return;
    }
}

void Aarch32Encoder::EncodeDiv(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Vdiv(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        if (!dstSigned) {
            GetMasm()->Udiv(VixlReg(dst), VixlReg(src0), VixlReg(src1));
            return;
        }

        GetMasm()->Sdiv(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }
    if (dstSigned) {
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(AEABIldivmod));
    } else {
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(AEABIuldivmod));
    }
}

void Aarch32Encoder::EncodeMod(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        using Fp = float (*)(float, float);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<Fp>(fmodf)));
        return;
    }
    if (dst.GetType() == FLOAT64_TYPE) {
        using Fp = double (*)(double, double);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<Fp>(fmod)));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        if (dstSigned) {
            MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(AEABIidivmod), true);
        } else {
            MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(AEABIuidivmod), true);
        }

        // dst = -(tmp * src0) + src1
        if ((dst.GetSize() == BYTE_SIZE) && dstSigned) {
            GetMasm()->Sxtb(VixlReg(dst), VixlReg(dst));
        }
        if ((dst.GetSize() == HALF_SIZE) && dstSigned) {
            GetMasm()->Sxth(VixlReg(dst), VixlReg(dst));
        }
        return;
    }

    // Call lib-method
    if (dstSigned) {
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(AEABIldivmod), true);
    } else {
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(AEABIuldivmod), true);
    }
}

void Aarch32Encoder::EncodeMin(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        EncodeMinMaxFp<false>(dst, src0, src1);
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        if (dstSigned) {
            GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
            GetMasm()->Mov(Convert(Condition::LE), VixlReg(dst), VixlReg(src0));
            GetMasm()->Mov(Convert(Condition::GT), VixlReg(dst), VixlReg(src1));
            return;
        }
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        GetMasm()->Mov(Convert(Condition::LS), VixlReg(dst), VixlReg(src0));
        GetMasm()->Mov(Convert(Condition::HI), VixlReg(dst), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 tmpReg(this);

    GetMasm()->Subs(VixlReg(tmpReg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Sbcs(VixlReg(tmpReg), VixlRegU(src0), VixlRegU(src1));

    auto cc = Convert(dstSigned ? Condition::LT : Condition::LO);
    GetMasm()->Mov(cc, VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(cc, VixlRegU(dst), VixlRegU(src0));
    GetMasm()->Mov(cc.Negate(), VixlReg(dst), VixlReg(src1));
    GetMasm()->Mov(cc.Negate(), VixlRegU(dst), VixlRegU(src1));
}

void Aarch32Encoder::EncodeMax(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        EncodeMinMaxFp<true>(dst, src0, src1);
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        if (dstSigned) {
            GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
            GetMasm()->Mov(Convert(Condition::GT), VixlReg(dst), VixlReg(src0));
            GetMasm()->Mov(Convert(Condition::LE), VixlReg(dst), VixlReg(src1));
            return;
        }
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        GetMasm()->Mov(Convert(Condition::HI), VixlReg(dst), VixlReg(src0));
        GetMasm()->Mov(Convert(Condition::LS), VixlReg(dst), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 tmpReg(this);

    GetMasm()->Subs(VixlReg(tmpReg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Sbcs(VixlReg(tmpReg), VixlRegU(src0), VixlRegU(src1));

    auto cc = Convert(dstSigned ? Condition::LT : Condition::LO);
    GetMasm()->Mov(cc, VixlReg(dst), VixlReg(src1));
    GetMasm()->Mov(cc, VixlRegU(dst), VixlRegU(src1));
    GetMasm()->Mov(cc.Negate(), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(cc.Negate(), VixlRegU(dst), VixlRegU(src0));
}

void Aarch32Encoder::EncodeExtractBits(Reg dst, Reg src, Imm imm1, Imm imm2, bool signExt)
{
    if (signExt) {
        GetMasm()->Sbfx(VixlReg(dst), VixlReg(src), imm1.GetAsInt(), imm2.GetAsInt());
    } else {
        GetMasm()->Ubfx(VixlReg(dst), VixlReg(src), imm1.GetAsInt(), imm2.GetAsInt());
    }
}

template <bool IS_MAX>
void Aarch32Encoder::EncodeMinMaxFp(Reg dst, Reg src0, Reg src1)
{
    Aarch32LabelHolder::LabelType notEqual(GetAllocator());
    Aarch32LabelHolder::LabelType gotNan(GetAllocator());
    Aarch32LabelHolder::LabelType end(GetAllocator());
    auto &srcA = dst.GetId() != src1.GetId() ? src0 : src1;
    auto &srcB = srcA.GetId() == src0.GetId() ? src1 : src0;
    GetMasm()->Vmov(VixlVReg(dst), VixlVReg(srcA));
    // Vcmp change flags:
    // NZCV
    // 0011 <- if any operand is NaN
    // 0110 <- operands are equals
    // 1000 <- operand0 < operand1
    // 0010 <- operand0 > operand1
    GetMasm()->Vcmp(VixlVReg(srcB), VixlVReg(srcA));
    GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
    GetMasm()->B(Convert(Condition::VS), &gotNan);
    GetMasm()->B(Convert(Condition::NE), &notEqual);

    // calculate result for positive/negative zero operands
    if (IS_MAX) {
        EncodeVand(dst, srcA, srcB);
    } else {
        EncodeVorr(dst, srcA, srcB);
    }
    GetMasm()->B(&end);
    GetMasm()->Bind(&gotNan);
    // if any operand is NaN result is NaN
    EncodeVorr(dst, srcA, srcB);
    GetMasm()->B(&end);
    GetMasm()->bind(&notEqual);
    // calculate min/max for other cases
    if (IS_MAX) {
        GetMasm()->B(Convert(Condition::MI), &end);
    } else {
        GetMasm()->B(Convert(Condition::HI), &end);
    }
    GetMasm()->Vmov(VixlVReg(dst), VixlVReg(srcB));
    GetMasm()->bind(&end);
}

void Aarch32Encoder::EncodeVorr(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF64 tmpReg(this);
        GetMasm()->Vmov(vixl::aarch32::SRegister(tmpReg.GetReg().GetId() + (src0.GetId() & 1U)), VixlVReg(src1).S());
        GetMasm()->Vorr(vixl::aarch32::DRegister(tmpReg.GetReg().GetId() / 2U),
                        vixl::aarch32::DRegister(src0.GetId() / 2U),
                        vixl::aarch32::DRegister(tmpReg.GetReg().GetId() / 2U));
        GetMasm()->Vmov(VixlVReg(dst).S(), vixl::aarch32::SRegister(tmpReg.GetReg().GetId() + (src0.GetId() & 1U)));
    } else {
        GetMasm()->Vorr(VixlVReg(dst).D(), VixlVReg(src0).D(), VixlVReg(src1).D());
    }
}

void Aarch32Encoder::EncodeVand(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF64 tmpReg(this);
        GetMasm()->Vmov(vixl::aarch32::SRegister(tmpReg.GetReg().GetId() + (src0.GetId() & 1U)), VixlVReg(src1).S());
        GetMasm()->Vand(vixl::aarch32::kDataTypeValueNone, vixl::aarch32::DRegister(tmpReg.GetReg().GetId() / 2U),
                        vixl::aarch32::DRegister(src0.GetId() / 2U),
                        vixl::aarch32::DRegister(tmpReg.GetReg().GetId() / 2U));
        GetMasm()->Vmov(VixlVReg(dst).S(), vixl::aarch32::SRegister(tmpReg.GetReg().GetId() + (src0.GetId() & 1U)));
    } else {
        GetMasm()->Vand(vixl::aarch32::kDataTypeValueNone, VixlVReg(dst).D(), VixlVReg(src0).D(), VixlVReg(src1).D());
    }
}

void Aarch32Encoder::EncodeShl(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Lsl(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 hiReg(this);
    ScopedTmpRegU32 tmpReg(this);

    GetMasm()->Rsb(VixlReg(tmpReg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsr(VixlReg(tmpReg), VixlReg(src0), VixlReg(tmpReg));
    GetMasm()->Orr(VixlReg(hiReg), VixlReg(tmpReg),
                   vixl::aarch32::Operand(VixlRegU(src0), vixl::aarch32::LSL, VixlReg(src1)));
    GetMasm()->Subs(VixlReg(tmpReg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsl(Convert(Condition::PL), VixlReg(hiReg), VixlReg(src0), VixlReg(tmpReg));
    GetMasm()->Mov(Convert(Condition::PL), VixlReg(dst), VixlImm(0x0));
    GetMasm()->Lsl(Convert(Condition::MI), VixlReg(dst), VixlReg(src0), VixlReg(src1));
    GetMasm()->Mov(VixlRegU(dst), VixlReg(hiReg));
}

void Aarch32Encoder::EncodeShr(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Lsr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 loReg(this);
    ScopedTmpRegU32 tmpReg(this);

    GetMasm()->Rsb(VixlReg(tmpReg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsr(VixlReg(loReg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Orr(VixlReg(loReg), VixlReg(loReg),
                   vixl::aarch32::Operand(VixlRegU(src0), vixl::aarch32::LSL, VixlReg(tmpReg)));
    GetMasm()->Subs(VixlReg(tmpReg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsr(Convert(Condition::PL), VixlReg(loReg), VixlRegU(src0), VixlReg(tmpReg));
    GetMasm()->Mov(Convert(Condition::PL), VixlRegU(dst), VixlImm(0x0));
    GetMasm()->Lsr(Convert(Condition::MI), VixlRegU(dst), VixlRegU(src0), VixlReg(src1));
    GetMasm()->Mov(VixlReg(dst), VixlReg(loReg));
}

void Aarch32Encoder::EncodeAShr(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Asr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 loReg(this);
    ScopedTmpRegU32 tmpReg(this);

    GetMasm()->Subs(VixlReg(tmpReg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsr(VixlReg(loReg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Rsb(VixlReg(tmpReg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Orr(VixlReg(loReg), VixlReg(loReg),
                   vixl::aarch32::Operand(VixlRegU(src0), vixl::aarch32::LSL, VixlReg(tmpReg)));
    GetMasm()->Rsb(Convert(Condition::PL), VixlReg(tmpReg), VixlReg(tmpReg), VixlImm(0x0));
    GetMasm()->Asr(Convert(Condition::PL), VixlReg(loReg), VixlRegU(src0), VixlReg(tmpReg));
    GetMasm()->Asr(Convert(Condition::PL), VixlRegU(dst), VixlRegU(src0), VixlImm(WORD_SIZE - 1));
    GetMasm()->Asr(Convert(Condition::MI), VixlRegU(dst), VixlRegU(src0), VixlReg(src1));
    GetMasm()->Mov(VixlReg(dst), VixlReg(loReg));
}

void Aarch32Encoder::EncodeAnd(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->And(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->And(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeOr(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->Orr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Orr(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeXor(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->Eor(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Eor(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeAdd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "UNIMPLEMENTED");
    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Add(VixlReg(dst), VixlReg(src), VixlImm(imm));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Adds(VixlReg(dst), VixlReg(src), VixlImm(imm));
    GetMasm()->Adc(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
}

void Aarch32Encoder::EncodeSub(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "UNIMPLEMENTED");
    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Sub(VixlReg(dst), VixlReg(src), VixlImm(imm));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Subs(VixlReg(dst), VixlReg(src), VixlImm(imm));
    GetMasm()->Sbc(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
}

void Aarch32Encoder::EncodeShl(Reg dst, Reg src, Imm imm)
{
    auto value = static_cast<uint32_t>(imm.GetAsInt());
    int32_t immValue = value & (dst.GetSize() - 1);

    ASSERT(dst.IsScalar() && "Invalid operand type");
    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Lsl(VixlReg(dst), VixlReg(src), VixlImm(immValue));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 hiReg(this);
    ScopedTmpRegU32 tmpReg(this);

    GetMasm()->Lsr(VixlReg(tmpReg), VixlReg(src), VixlImm(WORD_SIZE - immValue));
    GetMasm()->Mov(VixlReg(hiReg), VixlImm(immValue));
    GetMasm()->Orr(VixlReg(hiReg), VixlReg(tmpReg),
                   vixl::aarch32::Operand(VixlRegU(src), vixl::aarch32::LSL, VixlReg(hiReg)));
    GetMasm()->Movs(VixlReg(tmpReg), VixlImm(immValue - WORD_SIZE));
    GetMasm()->Lsl(Convert(Condition::PL), VixlReg(hiReg), VixlReg(src), VixlReg(tmpReg));
    GetMasm()->Mov(Convert(Condition::PL), VixlReg(dst), VixlImm(0x0));
    GetMasm()->Lsl(Convert(Condition::MI), VixlReg(dst), VixlReg(src), VixlImm(immValue));
    GetMasm()->Mov(VixlRegU(dst), VixlReg(hiReg));
}

void Aarch32Encoder::EncodeShr(Reg dst, Reg src, Imm imm)
{
    auto value = static_cast<uint32_t>(imm.GetAsInt());
    int32_t immValue = value & (dst.GetSize() - 1);

    ASSERT(dst.IsScalar() && "Invalid operand type");
    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Lsr(VixlReg(dst), VixlReg(src), immValue);
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 loReg(this);
    ScopedTmpRegU32 tmpReg(this);

    GetMasm()->Mov(VixlReg(tmpReg), VixlImm(WORD_SIZE - immValue));
    GetMasm()->Lsr(VixlReg(loReg), VixlReg(src), VixlImm(immValue));
    GetMasm()->Orr(VixlReg(loReg), VixlReg(loReg),
                   vixl::aarch32::Operand(VixlRegU(src), vixl::aarch32::LSL, VixlReg(tmpReg)));
    GetMasm()->Movs(VixlReg(tmpReg), VixlImm(immValue - WORD_SIZE));
    GetMasm()->Lsr(Convert(Condition::PL), VixlReg(loReg), VixlRegU(src), VixlReg(tmpReg));
    GetMasm()->Mov(Convert(Condition::PL), VixlRegU(dst), VixlImm(0x0));
    GetMasm()->Lsr(Convert(Condition::MI), VixlRegU(dst), VixlRegU(src), VixlImm(immValue));
    GetMasm()->Mov(VixlReg(dst), VixlReg(loReg));
}

void Aarch32Encoder::EncodeAShr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");

    auto value = static_cast<uint32_t>(imm.GetAsInt());
    int32_t immValue = value & (dst.GetSize() - 1);

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Asr(VixlReg(dst), VixlReg(src), immValue);
        return;
    }

    ScopedTmpRegU32 loReg(this);
    ScopedTmpRegU32 tmpReg(this);
    GetMasm()->Movs(VixlReg(tmpReg), VixlImm(immValue - WORD_SIZE));
    GetMasm()->Lsr(VixlReg(loReg), VixlReg(src), VixlImm(immValue));
    GetMasm()->Mov(VixlReg(tmpReg), VixlImm(WORD_SIZE - immValue));
    GetMasm()->Orr(VixlReg(loReg), VixlReg(loReg),
                   vixl::aarch32::Operand(VixlRegU(src), vixl::aarch32::LSL, VixlReg(tmpReg)));
    GetMasm()->Rsb(Convert(Condition::PL), VixlReg(tmpReg), VixlReg(tmpReg), VixlImm(0x0));
    GetMasm()->Asr(Convert(Condition::PL), VixlReg(loReg), VixlRegU(src), VixlReg(tmpReg));
    GetMasm()->Asr(Convert(Condition::PL), VixlRegU(dst), VixlRegU(src), VixlImm(WORD_SIZE - 1));
    GetMasm()->Asr(Convert(Condition::MI), VixlRegU(dst), VixlRegU(src), VixlImm(immValue));
    GetMasm()->Mov(VixlReg(dst), VixlReg(loReg));
}

void Aarch32Encoder::EncodeAnd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->And(VixlReg(dst), VixlReg(src), VixlImm(imm));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->And(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
    }
}

void Aarch32Encoder::EncodeOr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Orr(VixlReg(dst), VixlReg(src), VixlImm(imm));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Orr(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
    }
}

void Aarch32Encoder::EncodeXor(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Eor(VixlReg(dst), VixlReg(src), VixlImm(imm));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Eor(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
    }
}

void Aarch32Encoder::EncodeMov(Reg dst, Imm src)
{
    if (dst.IsFloat()) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(Convert(dst.GetType()), VixlVReg(dst).S(), VixlNeonImm(src.GetAsFloat()));
        } else {
            GetMasm()->Vmov(Convert(dst.GetType()), VixlVReg(dst).D(), VixlNeonImm(src.GetAsDouble()));
        }
        return;
    }

    GetMasm()->Mov(VixlReg(dst), VixlImm(src));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(VixlRegU(dst), VixlImmU(src));
    }
}

void Aarch32Encoder::EncodeLdr(Reg dst, bool dstSigned, const vixl::aarch32::MemOperand &vixlMem)
{
    if (dst.IsFloat()) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vldr(VixlVReg(dst).S(), vixlMem);
        } else {
            GetMasm()->Vldr(VixlVReg(dst).D(), vixlMem);
        }
        return;
    }
    if (dstSigned) {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldrsb(VixlReg(dst), vixlMem);
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldrsh(VixlReg(dst), vixlMem);
            return;
        }
    } else {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldrb(VixlReg(dst), vixlMem);
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldrh(VixlReg(dst), vixlMem);
            return;
        }
    }
    if (dst.GetSize() == WORD_SIZE) {
        GetMasm()->Ldr(VixlReg(dst), vixlMem);
        return;
    }
    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Ldrd(VixlReg(dst), VixlRegU(dst), vixlMem);
}

void Aarch32Encoder::EncodeLdr(Reg dst, bool dstSigned, MemRef mem)
{
    auto type = dst.GetType();
    if (IsNeedToPrepareMemLdS(mem, type, dstSigned)) {
        ScopedTmpRegU32 tmpReg(this);
        auto tmp = VixlReg(tmpReg);
        auto vixlMem = PrepareMemLdS(mem, type, tmp, dstSigned);
        EncodeLdr(dst, dstSigned, vixlMem);
    } else {
        auto vixlMem = ConvertMem(mem);
        EncodeLdr(dst, dstSigned, vixlMem);
    }
}

void Aarch32Encoder::EncodeLdrAcquire(Reg dst, bool dstSigned, MemRef mem)
{
    EncodeLdr(dst, dstSigned, mem);
    GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISH);
}

void Aarch32Encoder::EncodeMemoryBarrier(memory_order::Order order)
{
    switch (order) {
        case memory_order::ACQUIRE:
        case memory_order::RELEASE: {
            GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISH);
            break;
        }
        case memory_order::FULL: {
            GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISHST);
            break;
        }
        default:
            break;
    }
}

void Aarch32Encoder::EncodeStr(Reg src, const vixl::aarch32::MemOperand &vixlMem)
{
    if (src.IsFloat()) {
        if (src.GetSize() == WORD_SIZE) {
            GetMasm()->Vstr(VixlVReg(src).S(), vixlMem);
        } else {
            GetMasm()->Vstr(VixlVReg(src).D(), vixlMem);
        }
    } else if (src.GetSize() == BYTE_SIZE) {
        GetMasm()->Strb(VixlReg(src), vixlMem);
    } else if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Strh(VixlReg(src), vixlMem);
    } else if (src.GetSize() == WORD_SIZE) {
        GetMasm()->Str(VixlReg(src), vixlMem);
    } else {
        ASSERT(src.GetSize() == DOUBLE_WORD_SIZE);
        GetMasm()->Strd(VixlReg(src), VixlRegU(src), vixlMem);
    }
}

void Aarch32Encoder::EncodeStr(Reg src, MemRef mem)
{
    auto type = src.GetType();
    if (IsNeedToPrepareMemLdS(mem, type, false)) {
        ScopedTmpRegU32 tmpReg(this);
        auto tmp = VixlReg(tmpReg);
        auto vixlMem = PrepareMemLdS(mem, type, tmp, false);
        EncodeStr(src, vixlMem);
    } else {
        auto vixlMem = ConvertMem(mem);
        EncodeStr(src, vixlMem);
    }
}

void Aarch32Encoder::EncodeStrRelease(Reg src, MemRef mem)
{
    GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISH);
    EncodeStr(src, mem);
    GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISH);
}

void Aarch32Encoder::EncodeStrz(Reg src, MemRef mem)
{
    if (src.GetSize() <= WORD_SIZE) {
        EncodeSti(0, DOUBLE_WORD_SIZE_BYTES, mem);
    }
    EncodeStr(src, mem);
}

void Aarch32Encoder::EncodeStp(Reg src0, Reg src1, MemRef mem)
{
    ASSERT(src0.IsFloat() == src1.IsFloat());
    ASSERT(src0.GetSize() == src1.GetSize());
    EncodeStr(src0, mem);
    EncodeStr(src1, MemRef(mem.GetBase(), mem.GetIndex(), mem.GetScale(), mem.GetDisp() + WORD_SIZE_BYTES));
}

void Aarch32Encoder::EncodeLdrExclusive(Reg dst, Reg addr, bool acquire)
{
    ASSERT(dst.IsScalar());
    auto dstReg = VixlReg(dst);
    auto memCvt = ConvertMem(MemRef(addr));
    if (dst.GetSize() == BYTE_SIZE) {
        if (acquire) {
            GetMasm()->Ldaexb(dstReg, memCvt);
            return;
        }
        GetMasm()->Ldrexb(dstReg, memCvt);
        return;
    }
    if (dst.GetSize() == HALF_SIZE) {
        if (acquire) {
            GetMasm()->Ldaexh(dstReg, memCvt);
            return;
        }
        GetMasm()->Ldrexh(dstReg, memCvt);
        return;
    }
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        auto dstRegU = VixlRegU(dst);
        if (acquire) {
            GetMasm()->Ldaexd(dstReg, dstRegU, memCvt);
            return;
        }
        GetMasm()->Ldrexd(dstReg, dstRegU, memCvt);
        return;
    }
    if (acquire) {
        GetMasm()->Ldaex(dstReg, memCvt);
        return;
    }
    GetMasm()->Ldrex(dstReg, memCvt);
}

void Aarch32Encoder::EncodeStrExclusive(Reg dst, Reg src, Reg addr, bool release)
{
    ASSERT(dst.IsScalar() && src.IsScalar());
    ASSERT(dst.GetSize() != DOUBLE_WORD_SIZE);

    bool copyDst = dst.GetId() == src.GetId() || dst.GetId() == addr.GetId();
    ScopedTmpReg tmp(this);
    auto dstReg = copyDst ? VixlReg(tmp) : VixlReg(dst);
    auto srcReg = VixlReg(src);
    auto memCvt = ConvertMem(MemRef(addr));

    if (src.GetSize() == BYTE_SIZE) {
        if (release) {
            GetMasm()->Stlexb(dstReg, srcReg, memCvt);
        } else {
            GetMasm()->Strexb(dstReg, srcReg, memCvt);
        }
    } else if (src.GetSize() == HALF_SIZE) {
        if (release) {
            GetMasm()->Stlexh(dstReg, srcReg, memCvt);
        } else {
            GetMasm()->Strexh(dstReg, srcReg, memCvt);
        }
    } else if (src.GetSize() == DOUBLE_WORD_SIZE) {
        auto srcRegU = VixlRegU(src);
        if (release) {
            GetMasm()->Stlexd(dstReg, srcReg, srcRegU, memCvt);
        } else {
            GetMasm()->Strexd(dstReg, srcReg, srcRegU, memCvt);
        }
    } else {
        if (release) {
            GetMasm()->Stlex(dstReg, srcReg, memCvt);
        } else {
            GetMasm()->Strex(dstReg, srcReg, memCvt);
        }
    }

    if (copyDst) {
        EncodeMov(dst, tmp);
    }
}

static int32_t FindRegForMem(vixl::aarch32::MemOperand mem)
{
    int32_t baseRegId = mem.GetBaseRegister().GetCode();
    int32_t indexRegId = -1;
    if (mem.IsShiftedRegister()) {
        indexRegId = mem.GetOffsetRegister().GetCode();
    }
    // find regs for mem
    constexpr int32_t STEP = 2;
    for (int32_t i = 0; i < static_cast<int32_t>(BYTE_SIZE); i += STEP) {
        if (baseRegId == i || baseRegId == i + 1 || indexRegId == i || indexRegId == i + 1) {
            continue;
        }
        return i;
    }
    UNREACHABLE();
    return -1;
}

void Aarch32Encoder::EncodeSti(int64_t src, uint8_t srcSizeBytes, MemRef mem)
{
    ScopedTmpRegU32 tmpReg(this);
    auto tmp = VixlReg(tmpReg);
    auto type = TypeInfo::GetScalarTypeBySize(srcSizeBytes * BITS_PER_BYTE);
    if (srcSizeBytes <= WORD_SIZE_BYTES) {
        auto vixlMem = PrepareMemLdS(mem, type, tmp, false);
        if (vixlMem.GetBaseRegister().GetCode() == tmp.GetCode()) {
            ScopedTmpRegU32 tmp1Reg(this);
            tmp = VixlReg(tmp1Reg);
        }
        GetMasm()->Mov(tmp, VixlImm(src));
        if (srcSizeBytes == 1) {
            GetMasm()->Strb(tmp, vixlMem);
            return;
        }
        if (srcSizeBytes == HALF_WORD_SIZE_BYTES) {
            GetMasm()->Strh(tmp, vixlMem);
            return;
        }
        GetMasm()->Str(tmp, vixlMem);
        return;
    }

    auto vixlMem = PrepareMemLdS(mem, type, tmp, false, true);
    ASSERT(srcSizeBytes == DOUBLE_WORD_SIZE_BYTES);
    vixl::aarch32::Register tmpImm1;
    vixl::aarch32::Register tmpImm2;
    // if tmp isn't base reg and tmp is even and tmp+1 isn't SP we can use tmp and tmp + 1
    if (vixlMem.GetBaseRegister().GetCode() != tmp.GetCode() && (tmp.GetCode() % 2U == 0) &&
        tmp.GetCode() + 1 != vixl::aarch32::sp.GetCode()) {
        tmpImm1 = tmp;
        tmpImm2 = vixl::aarch32::Register(tmp.GetCode() + 1);
    } else {
        auto regId = FindRegForMem(vixlMem);
        ASSERT(regId != -1);
        tmpImm1 = vixl::aarch32::Register(regId);
        tmpImm2 = vixl::aarch32::Register(regId + 1);
        GetMasm()->Push(tmpImm1);
    }

    ASSERT(tmpImm1.IsValid() && tmpImm2.IsValid());
    GetMasm()->Push(tmpImm2);
    GetMasm()->Mov(tmpImm1, VixlImm(Imm(src)));
    GetMasm()->Mov(tmpImm2, VixlImmU(Imm(src)));
    GetMasm()->Strd(tmpImm1, tmpImm2, vixlMem);
    GetMasm()->Pop(tmpImm2);
    if (tmpImm1.GetCode() != tmp.GetCode()) {
        GetMasm()->Pop(tmpImm1);
    }
}

void Aarch32Encoder::EncodeSti(float src, MemRef mem)
{
    ScopedTmpRegF32 tmpReg(this);
    GetMasm()->Vmov(VixlVReg(tmpReg).S(), src);
    EncodeStr(tmpReg, mem);
}

void Aarch32Encoder::EncodeSti(double src, MemRef mem)
{
    ScopedTmpRegF64 tmpReg(this);
    GetMasm()->Vmov(VixlVReg(tmpReg).D(), src);
    EncodeStr(tmpReg, mem);
}

void Aarch32Encoder::EncodeMemCopy(MemRef memFrom, MemRef memTo, size_t size)
{
    if (size == DOUBLE_WORD_SIZE && memFrom.IsOffsetMem() && memTo.IsOffsetMem()) {
        EncodeMemCopy(memFrom, memTo, WORD_SIZE);
        constexpr int32_t STEP = 4;
        auto offsetFrom = memFrom.GetDisp() + STEP;
        auto offsetTo = memTo.GetDisp() + STEP;
        EncodeMemCopy(MemRef(memFrom.GetBase(), offsetFrom), MemRef(memTo.GetBase(), offsetTo), WORD_SIZE);

        return;
    }
    ScopedTmpRegU32 tmpReg(this);
    auto tmp = VixlReg(tmpReg);
    ScopedTmpRegU32 tmpReg1(this);
    auto tmp1 = VixlReg(tmpReg1);
    if (size == BYTE_SIZE) {
        GetMasm()->Ldrb(tmp, PrepareMemLdS(memFrom, INT8_TYPE, tmp, false));
        GetMasm()->Strb(tmp, PrepareMemLdS(memTo, INT8_TYPE, tmp1, false));
    } else if (size == HALF_SIZE) {
        GetMasm()->Ldrh(tmp, PrepareMemLdS(memFrom, INT16_TYPE, tmp, false));
        GetMasm()->Strh(tmp, PrepareMemLdS(memTo, INT16_TYPE, tmp1, false));
    } else if (size == WORD_SIZE) {
        GetMasm()->Ldr(tmp, PrepareMemLdS(memFrom, INT32_TYPE, tmp, false));
        GetMasm()->Str(tmp, PrepareMemLdS(memTo, INT32_TYPE, tmp1, false));
    } else {
        ASSERT(size == DOUBLE_WORD_SIZE);

        auto vixlMemFrom = PrepareMemLdS(memFrom, INT64_TYPE, tmp, false, true);
        auto vixlMemTo = PrepareMemLdS(memTo, INT64_TYPE, tmp1, false, true);
        auto regId = FindRegForMem(vixlMemTo);
        ASSERT(regId != -1);
        [[maybe_unused]] constexpr auto IMM_2 = 2;
        ASSERT(regId % IMM_2 == 0);
        vixl::aarch32::Register tmpCopy1(regId);
        vixl::aarch32::Register tmpCopy2(regId + 1);

        GetMasm()->Push(tmpCopy1);
        GetMasm()->Push(tmpCopy2);
        GetMasm()->Ldrd(tmpCopy1, tmpCopy2, vixlMemFrom);
        GetMasm()->Strd(tmpCopy1, tmpCopy2, vixlMemTo);
        GetMasm()->Pop(tmpCopy2);
        GetMasm()->Pop(tmpCopy1);
    }
}

void Aarch32Encoder::EncodeMemCopyz(MemRef memFrom, MemRef memTo, size_t size)
{
    ScopedTmpRegU32 tmpReg(this);
    auto tmp = VixlReg(tmpReg);
    ScopedTmpRegU32 tmpReg1(this);
    auto tmp1 = VixlReg(tmpReg1);

    auto type = TypeInfo::GetScalarTypeBySize(size);

    auto vixlMemFrom = PrepareMemLdS(memFrom, type, tmp, false, true);
    auto vixlMemTo = PrepareMemLdS(memTo, INT64_TYPE, tmp1, false, true);
    auto regId = FindRegForMem(vixlMemTo);
    ASSERT(regId != -1);
    [[maybe_unused]] constexpr auto IMM_2 = 2;
    ASSERT(regId % IMM_2 == 0);
    vixl::aarch32::Register tmpCopy1(regId);
    vixl::aarch32::Register tmpCopy2(regId + 1);

    GetMasm()->Push(tmpCopy1);
    GetMasm()->Push(tmpCopy2);
    if (size == BYTE_SIZE) {
        GetMasm()->Ldrb(tmpCopy1, vixlMemFrom);
        GetMasm()->Mov(tmpCopy2, VixlImm(0));
        GetMasm()->Strd(tmpCopy1, tmpCopy2, vixlMemTo);
    } else if (size == HALF_SIZE) {
        GetMasm()->Ldrh(tmpCopy1, vixlMemFrom);
        GetMasm()->Mov(tmpCopy2, VixlImm(0));
        GetMasm()->Strd(tmpCopy1, tmpCopy2, vixlMemTo);
    } else if (size == WORD_SIZE) {
        GetMasm()->Ldr(tmpCopy1, vixlMemFrom);
        GetMasm()->Mov(tmpCopy2, VixlImm(0));
        GetMasm()->Strd(tmpCopy1, tmpCopy2, vixlMemTo);
    } else {
        ASSERT(size == DOUBLE_WORD_SIZE);
        GetMasm()->Ldrd(tmpCopy1, tmpCopy2, vixlMemFrom);
        GetMasm()->Strd(tmpCopy1, tmpCopy2, vixlMemTo);
    }
    GetMasm()->Pop(tmpCopy2);
    GetMasm()->Pop(tmpCopy1);
}

void Aarch32Encoder::CompareHelper(Reg src0, Reg src1, Condition *cc)
{
    if (src0.IsFloat() && src1.IsFloat()) {
        GetMasm()->Vcmp(VixlVReg(src0), VixlVReg(src1));
        GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
    } else if (src0.GetSize() <= WORD_SIZE && src1.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    } else {
        if (!IsConditionSigned(*cc)) {
            GetMasm()->Cmp(VixlRegU(src0), VixlRegU(src1));
            GetMasm()->Cmp(Convert(Condition::EQ), VixlReg(src0), VixlReg(src1));
        } else {
            bool swap = false;
            *cc = TrySwapCc(*cc, &swap);

            Reg op0 = swap ? src1 : src0;
            Reg op1 = swap ? src0 : src1;
            ScopedTmpRegU32 tmpReg(this);
            GetMasm()->Cmp(VixlReg(op0), VixlReg(op1));
            GetMasm()->Sbcs(VixlReg(tmpReg), VixlRegU(op0), VixlRegU(op1));
        }
    }
}

void Aarch32Encoder::TestHelper(Reg src0, Reg src1, [[maybe_unused]] Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::TST_EQ || cc == Condition::TST_NE);

    if (src0.GetSize() <= WORD_SIZE && src1.GetSize() <= WORD_SIZE) {
        GetMasm()->Tst(VixlReg(src0), VixlReg(src1));
    } else {
        GetMasm()->Tst(VixlRegU(src0), VixlRegU(src1));
        GetMasm()->Tst(Convert(Condition::EQ), VixlReg(src0), VixlReg(src1));
    }
}

void Aarch32Encoder::EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc)
{
    CompareHelper(src0, src1, &cc);
    GetMasm()->Mov(Convert(cc), VixlReg(dst), 0x1);
    GetMasm()->Mov(Convert(cc).Negate(), VixlReg(dst), 0x0);
}

void Aarch32Encoder::EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc)
{
    TestHelper(src0, src1, cc);
    GetMasm()->Mov(ConvertTest(cc), VixlReg(dst), 0x1);
    GetMasm()->Mov(ConvertTest(cc).Negate(), VixlReg(dst), 0x0);
}

void Aarch32Encoder::EncodeAtomicByteOr(Reg addr, Reg value, [[maybe_unused]] bool fastEncoding)
{
    /**
     * .try:
     *   ldrexb  r2, [r0]
     *   orr     r2, r2, r1
     *   strexb  r3, r2, [r0]
     *   cmp     r3, #0
     *   bne     .try
     */

    auto labelCasFailed = CreateLabel();
    BindLabel(labelCasFailed);

    ScopedTmpReg tmpReg(this);
    ScopedTmpReg casResult(this);

    GetMasm()->Ldrexb(VixlReg(tmpReg), vixl::aarch32::MemOperand(VixlReg(addr)));
    GetMasm()->Orr(VixlReg(tmpReg), VixlReg(tmpReg), VixlReg(value));
    GetMasm()->Strexb(VixlReg(casResult), VixlReg(tmpReg), vixl::aarch32::MemOperand(VixlReg(addr)));
    GetMasm()->Cmp(VixlReg(casResult), VixlImm(0));
    GetMasm()->B(vixl::aarch32::ne, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(labelCasFailed));
}

void Aarch32Encoder::EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc)
{
    if (src0.IsFloat()) {
        ASSERT(src1.IsFloat());
        ASSERT(cc == Condition::MI || cc == Condition::LT);
        GetMasm()->Vcmp(VixlVReg(src0), VixlVReg(src1));
        GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
    } else {
        ASSERT(src0.IsScalar() && src1.IsScalar());
        ASSERT(cc == Condition::LO || cc == Condition::LT);
        if (src0.GetSize() <= WORD_SIZE && src1.GetSize() <= WORD_SIZE) {
            GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        } else {
            if (cc == Condition::LO) {
                GetMasm()->Cmp(VixlRegU(src0), VixlRegU(src1));
                GetMasm()->Cmp(Convert(Condition::EQ), VixlReg(src0), VixlReg(src1));
            } else if (cc == Condition::LT) {
                auto labelHolder = static_cast<Aarch32LabelHolder *>(GetLabels());
                auto endLabel = labelHolder->CreateLabel();
                ScopedTmpRegU32 tmpReg(this);

                GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
                GetMasm()->Sbcs(VixlReg(tmpReg), VixlRegU(src0), VixlRegU(src1));
                GetMasm()->Mov(Convert(Condition::LT), VixlReg(dst), VixlImm(-1));
                GetMasm()->B(Convert(Condition::LT), labelHolder->GetLabel(endLabel));

                GetMasm()->Cmp(VixlReg(src1), VixlReg(src0));
                GetMasm()->Sbcs(VixlReg(tmpReg), VixlRegU(src1), VixlRegU(src0));
                GetMasm()->Mov(Convert(Condition::LT), VixlReg(dst), VixlImm(1));
                GetMasm()->Mov(Convert(Condition::EQ), VixlReg(dst), VixlImm(0));

                labelHolder->BindLabel(endLabel);
                return;
            } else {
                UNREACHABLE();
            }
        }
    }

    GetMasm()->Mov(Convert(Condition::EQ), VixlReg(dst), VixlImm(0x0));
    GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(0x1));

    GetMasm()->Rsb(Convert(cc), VixlReg(dst), VixlReg(dst), VixlImm(0x0));
}

void Aarch32Encoder::EncodeStackOverflowCheck(ssize_t offset)
{
    ScopedTmpReg tmp(this);
    EncodeAdd(tmp, GetTarget().GetStackReg(), Imm(offset));
    EncodeLdr(tmp, false, MemRef(tmp));
}

void Aarch32Encoder::EncodeSelect(const ArgsSelect &args)
{
    auto [dst, src0, src1, src2, src3, cc] = args;
    ASSERT(!src0.IsFloat() && !src1.IsFloat());

    CompareHelper(src2, src3, &cc);

    GetMasm()->Mov(Convert(cc), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(Convert(cc).Negate(), VixlReg(dst), VixlReg(src1));

    if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(Convert(cc), VixlRegU(dst), VixlRegU(src0));
        GetMasm()->Mov(Convert(cc).Negate(), VixlRegU(dst), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeSelect(const ArgsSelectImm &args)
{
    auto [dst, src0, src1, src2, imm, cc] = args;
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());
    auto value = imm.GetAsInt();
    if (value == 0) {
        switch (cc) {
            case Condition::LO:
                // LO is always false, select src1
                GetMasm()->Mov(VixlReg(dst), VixlReg(src1));
                if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
                    GetMasm()->Mov(VixlRegU(dst), VixlRegU(src1));
                }
                return;
            case Condition::HS:
                // HS is always true, select src0
                GetMasm()->Mov(VixlReg(dst), VixlReg(src0));
                if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
                    GetMasm()->Mov(VixlRegU(dst), VixlRegU(src0));
                }
                return;
            case Condition::LS:
                // LS is same as EQ
                cc = Condition::EQ;
                break;
            case Condition::HI:
                // HI is same as NE
                cc = Condition::NE;
                break;
            default:
                break;
        }
        CompareZeroHelper(src2, &cc);
    } else {  // value != 0
        if (!CompareImmHelper(src2, value, &cc)) {
            return;
        }
    }

    GetMasm()->Mov(Convert(cc), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(Convert(cc).Negate(), VixlReg(dst), VixlReg(src1));

    if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(Convert(cc), VixlRegU(dst), VixlRegU(src0));
        GetMasm()->Mov(Convert(cc).Negate(), VixlRegU(dst), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeSelectTest(const ArgsSelect &args)
{
    auto [dst, src0, src1, src2, src3, cc] = args;
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    TestHelper(src2, src3, cc);

    GetMasm()->Mov(ConvertTest(cc), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(ConvertTest(cc).Negate(), VixlReg(dst), VixlReg(src1));

    if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(ConvertTest(cc), VixlRegU(dst), VixlRegU(src0));
        GetMasm()->Mov(ConvertTest(cc).Negate(), VixlRegU(dst), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeSelectTest(const ArgsSelectImm &args)
{
    auto [dst, src0, src1, src2, imm, cc] = args;
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    TestImmHelper(src2, imm, cc);
    GetMasm()->Mov(ConvertTest(cc), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(ConvertTest(cc).Negate(), VixlReg(dst), VixlReg(src1));

    if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(ConvertTest(cc), VixlRegU(dst), VixlRegU(src0));
        GetMasm()->Mov(ConvertTest(cc).Negate(), VixlRegU(dst), VixlRegU(src1));
    }
}

bool Aarch32Encoder::CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signedCompare)
{
    if (imm == INT64_MIN) {
        return false;
    }
    if (imm < 0) {
        imm = -imm;
        if (size > WORD_SIZE && signedCompare) {
            return false;
        }
    }
    // We don't support 64-bit immediate, even when both higher and lower parts are legal immediates
    if (imm > UINT32_MAX) {
        return false;
    }
    return vixl::aarch32::ImmediateA32::IsImmediateA32(imm);
}

bool Aarch32Encoder::CanEncodeImmLogical(uint64_t imm, uint32_t size)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    uint64_t high = imm >> WORD_SIZE;
    if (size == DOUBLE_WORD_SIZE) {
        if (high != 0U && high != UINT32_MAX) {
            return false;
        }
    }
#ifndef NDEBUG
    if (size < DOUBLE_WORD_SIZE) {
        // Test if the highest part is consistent:
        ASSERT(((imm >> size) == 0) || (((~imm) >> size) == 0));
    }
#endif  // NDEBUG
    return vixl::aarch32::ImmediateA32::IsImmediateA32(imm);
}

bool Aarch32Encoder::CanEncodeBitfieldExtractionFor(DataType::Type type)
{
    return type == DataType::INT32 || type == DataType::UINT32;
}

using vixl::aarch32::MemOperand;

template <bool IS_STORE>
void Aarch32Encoder::LoadStoreRegistersMainLoop(RegMask registers, ssize_t slot, Reg base, RegMask mask, bool isFp)
{
    bool hasMask = mask.any();
    int32_t index = hasMask ? static_cast<int32_t>(mask.GetMinRegister()) : 0;
    slot -= index;
    vixl::aarch32::Register baseReg = VixlReg(base);
    for (size_t i = index; i < registers.size(); i++) {
        if (hasMask) {
            if (!mask.test(i)) {
                continue;
            }
            index++;
        }
        if (!registers.test(i)) {
            continue;
        }

        if (!hasMask) {
            index++;
        }
        auto mem = MemOperand(baseReg, (slot + index - 1) * WORD_SIZE_BYTES);
        ConstructLdrStr<IS_STORE>(mem, i, isFp);
    }
}

template <bool IS_STORE>
void Aarch32Encoder::LoadStoreRegisters(RegMask registers, ssize_t slot, Reg base, RegMask mask, bool isFp)
{
    if (registers.none()) {
        return;
    }

    vixl::aarch32::Register baseReg = VixlReg(base);
    ssize_t maxOffset = (slot + helpers::ToSigned(registers.GetMaxRegister())) * WORD_SIZE_BYTES;

    ScopedTmpRegU32 tmpReg(this);
    auto tmp = VixlReg(tmpReg);
    // Construct single add for big offset
    ConstructAddForBigOffset(tmp, &baseReg, &slot, maxOffset, isFp);
    LoadStoreRegistersMainLoop<IS_STORE>(registers, slot, base, mask, isFp);
}

template <bool IS_STORE>
void Aarch32Encoder::ConstructLdrStr(vixl::aarch32::MemOperand mem, size_t i, bool isFp)
{
    if (isFp) {
        auto reg = vixl::aarch32::SRegister(i);
        if constexpr (IS_STORE) {  // NOLINT
            GetMasm()->Vstr(reg, mem);
        } else {  // NOLINT
            GetMasm()->Vldr(reg, mem);
        }
    } else {
        auto reg = vixl::aarch32::Register(i);
        if constexpr (IS_STORE) {  // NOLINT
            GetMasm()->Str(reg, mem);
        } else {  // NOLINT
            GetMasm()->Ldr(reg, mem);
        }
    }
}

void Aarch32Encoder::ConstructAddForBigOffset(vixl::aarch32::Register tmp, vixl::aarch32::Register *baseReg,
                                              ssize_t *slot, ssize_t maxOffset, bool isFp)
{
    if (isFp) {
        if ((maxOffset < -VMEM_OFFSET) || (maxOffset > VMEM_OFFSET)) {
            GetMasm()->Add(tmp, *baseReg, VixlImm((*slot) * WORD_SIZE_BYTES));
            *slot = 0;
            *baseReg = tmp;
        }
    } else {
        if ((maxOffset < -MEM_BIG_OFFSET) || (maxOffset > MEM_BIG_OFFSET)) {
            GetMasm()->Add(tmp, *baseReg, VixlImm((*slot) * WORD_SIZE_BYTES));
            *slot = 0;
            *baseReg = tmp;
        }
    }
}

template <bool IS_STORE>
void Aarch32Encoder::LoadStoreRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp)
{
    if (registers.none()) {
        return;
    }
    int32_t lastReg = registers.size() - 1;
    for (; lastReg >= 0; --lastReg) {
        if (registers.test(lastReg)) {
            break;
        }
    }
    vixl::aarch32::Register baseReg = vixl::aarch32::sp;
    auto maxOffset = (slot + lastReg) * WORD_SIZE_BYTES;
    ScopedTmpRegU32 tmpReg(this);
    auto tmp = VixlReg(tmpReg);
    // Construct single add for big offset
    ConstructAddForBigOffset(tmp, &baseReg, &slot, maxOffset, isFp);
    for (auto i = startReg; i < registers.size(); i++) {
        if (!registers.test(i)) {
            continue;
        }
        auto mem = MemOperand(baseReg, (slot + i - startReg) * WORD_SIZE_BYTES);
        ConstructLdrStr<IS_STORE>(mem, i, isFp);
    }
}

void Aarch32Encoder::PushRegisters(RegMask registers, bool isFp)
{
    (void)registers;
    (void)isFp;
    // NOTE(msherstennikov): Implement
}

void Aarch32Encoder::PopRegisters(RegMask registers, bool isFp)
{
    (void)registers;
    (void)isFp;
    // NOTE(msherstennikov): Implement
}

size_t Aarch32Encoder::DisasmInstr(std::ostream &stream, size_t pc, ssize_t codeOffset) const
{
    auto addr = GetMasm()->GetBuffer()->GetOffsetAddress<const uint32_t *>(pc);
    // Display pc is seted, because disassembler use pc
    // for upper bits (e.g. 0x40000000), when print one instruction.
    if (codeOffset < 0) {
        vixl::aarch32::PrintDisassembler disasm(GetAllocator(), stream);
        disasm.DisassembleA32Buffer(addr, vixl::aarch32::k32BitT32InstructionSizeInBytes);
    } else {
        const uint64_t displayPc = 0x10000000;
        vixl::aarch32::PrintDisassembler disasm(GetAllocator(), stream, displayPc + pc + codeOffset);
        disasm.DisassembleA32Buffer(addr, vixl::aarch32::k32BitT32InstructionSizeInBytes);

        stream << std::setfill(' ');
    }
    return pc + vixl::aarch32::k32BitT32InstructionSizeInBytes;
}
}  // namespace ark::compiler::aarch32
