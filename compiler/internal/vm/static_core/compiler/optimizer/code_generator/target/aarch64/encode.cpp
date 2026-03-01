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

#include <aarch64/macro-assembler-aarch64.h>
#include <cstddef>
#include "compiler/optimizer/code_generator/target/aarch64/target.h"
#include "compiler/optimizer/code_generator/encode.h"
#include "compiler/optimizer/code_generator/fast_divisor.h"
#include "scoped_tmp_reg.h"
#include "compiler/optimizer/code_generator/relocations.h"

#if defined(USE_VIXL_ARM64) && !defined(PANDA_MINIMAL_VIXL)
#include "aarch64/disasm-aarch64.h"
#endif

#include <iomanip>

#include "lib_helpers.inl"

#ifndef PANDA_TARGET_MACOS
#include "elf.h"
#endif  // PANDA_TARGET_MACOS

namespace ark::compiler::aarch64 {
using vixl::aarch64::CPURegister;
using vixl::aarch64::MemOperand;

/// Converters
static vixl::aarch64::Condition Convert(const Condition cc)
{
    switch (cc) {
        case Condition::EQ:
            return vixl::aarch64::Condition::eq;
        case Condition::NE:
            return vixl::aarch64::Condition::ne;
        case Condition::LT:
            return vixl::aarch64::Condition::lt;
        case Condition::GT:
            return vixl::aarch64::Condition::gt;
        case Condition::LE:
            return vixl::aarch64::Condition::le;
        case Condition::GE:
            return vixl::aarch64::Condition::ge;
        case Condition::LO:
            return vixl::aarch64::Condition::lo;
        case Condition::LS:
            return vixl::aarch64::Condition::ls;
        case Condition::HI:
            return vixl::aarch64::Condition::hi;
        case Condition::HS:
            return vixl::aarch64::Condition::hs;
        // NOTE(igorban) : Remove them
        case Condition::MI:
            return vixl::aarch64::Condition::mi;
        case Condition::PL:
            return vixl::aarch64::Condition::pl;
        case Condition::VS:
            return vixl::aarch64::Condition::vs;
        case Condition::VC:
            return vixl::aarch64::Condition::vc;
        case Condition::AL:
            return vixl::aarch64::Condition::al;
        case Condition::NV:
            return vixl::aarch64::Condition::nv;
        default:
            UNREACHABLE();
            return vixl::aarch64::Condition::eq;
    }
}

static vixl::aarch64::Condition ConvertTest(const Condition cc)
{
    ASSERT(cc == Condition::TST_EQ || cc == Condition::TST_NE);
    return cc == Condition::TST_EQ ? vixl::aarch64::Condition::eq : vixl::aarch64::Condition::ne;
}

static vixl::aarch64::Shift Convert(const ShiftType type)
{
    switch (type) {
        case ShiftType::LSL:
            return vixl::aarch64::Shift::LSL;
        case ShiftType::LSR:
            return vixl::aarch64::Shift::LSR;
        case ShiftType::ASR:
            return vixl::aarch64::Shift::ASR;
        case ShiftType::ROR:
            return vixl::aarch64::Shift::ROR;
        default:
            UNREACHABLE();
    }
}

static vixl::aarch64::VRegister VixlVReg(Reg reg)
{
    ASSERT(reg.IsValid());
    auto vixlVreg = vixl::aarch64::VRegister(reg.GetId(), reg.GetSize());
    ASSERT(vixlVreg.IsValid());
    return vixlVreg;
}

static vixl::aarch64::Operand VixlShift(Shift shift)
{
    Reg reg = shift.GetBase();
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        ASSERT(reg.IsScalar());
        size_t regSize = reg.GetSize();
        if (regSize < WORD_SIZE) {
            regSize = WORD_SIZE;
        }
        auto vixlReg = vixl::aarch64::Register(reg.GetId(), regSize);
        ASSERT(vixlReg.IsValid());

        return vixl::aarch64::Operand(vixlReg, Convert(shift.GetType()), shift.GetScale());
    }

    // Invalid register type
    UNREACHABLE();
}

static vixl::aarch64::MemOperand ConvertMem(MemRef mem, vixl::aarch64::AddrMode mode = vixl::aarch64::Offset)
{
    bool base = mem.HasBase() && (mem.GetBase().GetId() != vixl::aarch64::xzr.GetCode());
    bool hasIndex = mem.HasIndex();
    bool shift = mem.HasScale();
    bool offset = mem.HasDisp();
    auto baseReg = Reg(mem.GetBase().GetId(), INT64_TYPE);
    if (base && !hasIndex && !shift) {
        // Memory address = x_reg(base) + imm(offset)
        // Pre-indexing:  [base, offset]!
        // Post-indexing: [base], offset
        if (mem.GetDisp() != 0) {
            auto disp = mem.GetDisp();
            return vixl::aarch64::MemOperand(VixlReg(baseReg), VixlImm(disp), mode);
        }
        // Memory address = x_reg(base)
        // mode has no effect.
        return vixl::aarch64::MemOperand(VixlReg(mem.GetBase(), DOUBLE_WORD_SIZE));
    }
    if (base && hasIndex && !offset && mode == vixl::aarch64::Offset) {
        auto scale = mem.GetScale();
        auto indexReg = mem.GetIndex();
        // Memory address = x_reg(base) + (SXTW(w_reg(index)) << scale)
        if (indexReg.GetSize() == WORD_SIZE) {
            // Sign-extend and shift w-register in offset-position (signed because index always has signed type)
            return vixl::aarch64::MemOperand(VixlReg(baseReg), VixlReg(indexReg), vixl::aarch64::Extend::SXTW, scale);
        }
        // Memory address = x_reg(base) + (x_reg(index) << scale)
        if (scale != 0) {
            ASSERT(indexReg.GetSize() == DOUBLE_WORD_SIZE);
            return vixl::aarch64::MemOperand(VixlReg(baseReg), VixlReg(indexReg), vixl::aarch64::LSL, scale);
        }
        // Memory address = x_reg(base) + x_reg(index)
        return vixl::aarch64::MemOperand(VixlReg(baseReg), VixlReg(indexReg));
    }
    // Wrong memRef or AddrMode
    // Return invalid memory operand
    auto tmp = vixl::aarch64::MemOperand();
    ASSERT(!tmp.IsValid());
    return tmp;
}

static Reg Promote(Reg reg)
{
    if (reg.GetType() == INT8_TYPE) {
        return Reg(reg.GetId(), INT16_TYPE);
    }
    return reg;
}

Aarch64LabelHolder::LabelId Aarch64LabelHolder::CreateLabel()
{
    ++id_;
    auto allocator = GetEncoder()->GetAllocator();
    auto *label = allocator->New<LabelType>(allocator);
    labels_.push_back(label);
    ASSERT(labels_.size() == id_);
    return id_ - 1;
}

void Aarch64LabelHolder::CreateLabels(LabelId size)
{
    for (LabelId i = 0; i <= size; ++i) {
        CreateLabel();
    }
}

void Aarch64LabelHolder::BindLabel(LabelId id)
{
    static_cast<Aarch64Encoder *>(GetEncoder())->GetMasm()->Bind(labels_[id]);
}

Aarch64LabelHolder::LabelType *Aarch64LabelHolder::GetLabel(LabelId id) const
{
    ASSERT(labels_.size() > id);
    return labels_[id];
}

Aarch64LabelHolder::LabelId Aarch64LabelHolder::Size()
{
    return labels_.size();
}

Aarch64Encoder::Aarch64Encoder(ArenaAllocator *allocator) : Encoder(allocator, Arch::AARCH64)
{
    labels_ = allocator->New<Aarch64LabelHolder>(this);
    if (labels_ == nullptr) {
        SetFalseResult();
    }
    // We enable LR tmp reg by default in Aarch64
    EnableLrAsTempReg(true);
}

Aarch64Encoder::~Aarch64Encoder()
{
    auto labels = static_cast<Aarch64LabelHolder *>(GetLabels())->labels_;
    for (auto label : labels) {
        label->~Label();
    }
    if (masm_ != nullptr) {
        masm_->~MacroAssembler();
        masm_ = nullptr;
    }
}

LabelHolder *Aarch64Encoder::GetLabels() const
{
    ASSERT(labels_ != nullptr);
    return labels_;
}

bool Aarch64Encoder::IsValid() const
{
    return true;
}

constexpr auto Aarch64Encoder::GetTarget()
{
    return ark::compiler::Target(Arch::AARCH64);
}

void Aarch64Encoder::SetMaxAllocatedBytes(size_t size)
{
    GetMasm()->GetBuffer()->SetMmapMaxBytes(size);
}

bool Aarch64Encoder::InitMasm()
{
    if (masm_ == nullptr) {
        // Initialize Masm
        masm_ = GetAllocator()->New<vixl::aarch64::MacroAssembler>(GetAllocator());
        if (masm_ == nullptr || !masm_->IsValid()) {
            SetFalseResult();
            return false;
        }
        ASSERT(GetMasm());

        // Make sure that the compiler uses the same scratch registers as the assembler
        CHECK_EQ(RegMask(GetMasm()->GetScratchRegisterList()->GetList()), GetTarget().GetTempRegsMask());
        CHECK_EQ(RegMask(GetMasm()->GetScratchVRegisterList()->GetList()), GetTarget().GetTempVRegsMask());
    }
    return true;
}

void Aarch64Encoder::Finalize()
{
    GetMasm()->FinalizeCode();
}

void Aarch64Encoder::EncodeJump(LabelHolder::LabelId id)
{
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label);
}

void Aarch64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    if (src1.GetId() == GetRegfile()->GetZeroReg().GetId()) {
        EncodeJump(id, src0, cc);
        return;
    }

    if (src0.IsScalar()) {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    } else {
        GetMasm()->Fcmp(VixlVReg(src0), VixlVReg(src1));
    }

    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    auto value = imm.GetAsInt();
    if (value == 0) {
        EncodeJump(id, src, cc);
        return;
    }

    if (value < 0) {
        GetMasm()->Cmn(VixlReg(src), VixlImm(-value));
    } else {  // if (value > 0)
        GetMasm()->Cmp(VixlReg(src), VixlImm(value));
    }

    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsScalar() && src1.IsScalar());

    GetMasm()->Tst(VixlReg(src0), VixlReg(src1));
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, ConvertTest(cc));
}

void Aarch64Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    ASSERT(src.IsScalar());

    auto value = imm.GetAsInt();
    if (CanEncodeImmLogical(value, src.GetSize() > WORD_SIZE ? DOUBLE_WORD_SIZE : WORD_SIZE)) {
        GetMasm()->Tst(VixlReg(src), VixlImm(value));
        auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
        GetMasm()->B(label, ConvertTest(cc));
    } else {
        ScopedTmpReg tmpReg(this, src.GetType());
        EncodeMov(tmpReg, imm);
        EncodeJumpTest(id, src, tmpReg, cc);
    }
}

void Aarch64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Condition cc)
{
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    ASSERT(src.IsScalar());
    auto rzero = Reg(GetRegfile()->GetZeroReg().GetId(), src.GetType());

    switch (cc) {
        case Condition::LO:
            // Always false
            return;
        case Condition::HS:
            // Always true
            GetMasm()->B(label);
            return;
        case Condition::EQ:
        case Condition::LS:
            if (src.GetId() == rzero.GetId()) {
                GetMasm()->B(label);
                return;
            }
            // True only when zero
            GetMasm()->Cbz(VixlReg(src), label);
            return;
        case Condition::NE:
        case Condition::HI:
            if (src.GetId() == rzero.GetId()) {
                // Do nothing
                return;
            }
            // True only when non-zero
            GetMasm()->Cbnz(VixlReg(src), label);
            return;
        default:
            break;
    }

    ASSERT(rzero.IsValid());
    GetMasm()->Cmp(VixlReg(src), VixlReg(rzero));
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeJump(Reg dst)
{
    GetMasm()->Br(VixlReg(dst));
}

void Aarch64Encoder::EncodeJump([[maybe_unused]] RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    auto buffer = GetMasm()->GetBuffer();
    relocation->offset = GetCursorOffset();
    relocation->addend = 0;
    relocation->type = R_AARCH64_CALL26;
    static constexpr uint32_t CALL_WITH_ZERO_OFFSET = 0x14000000;
    buffer->Emit32(CALL_WITH_ZERO_OFFSET);
#endif
}

void Aarch64Encoder::EncodeBitTestAndBranch(LabelHolder::LabelId id, compiler::Reg reg, uint32_t bitPos, bool bitValue)
{
    ASSERT(reg.IsScalar() && reg.GetSize() > bitPos);
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    if (bitValue) {
        GetMasm()->Tbnz(VixlReg(reg), bitPos, label);
    } else {
        GetMasm()->Tbz(VixlReg(reg), bitPos, label);
    }
}

void Aarch64Encoder::EncodeNop()
{
    GetMasm()->Nop();
}

void Aarch64Encoder::MakeCall([[maybe_unused]] compiler::RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    auto buffer = GetMasm()->GetBuffer();
    relocation->offset = GetCursorOffset();
    relocation->addend = 0;
    relocation->type = R_AARCH64_CALL26;
    static constexpr uint32_t CALL_WITH_ZERO_OFFSET = 0x94000000;
    buffer->Emit32(CALL_WITH_ZERO_OFFSET);
#endif
}

void Aarch64Encoder::MakeCall(const void *entryPoint)
{
    ScopedTmpReg tmp(this, true);
    EncodeMov(tmp, Imm(reinterpret_cast<uintptr_t>(entryPoint)));
    GetMasm()->Blr(VixlReg(tmp));
}

void Aarch64Encoder::MakeCall(MemRef entryPoint)
{
    ScopedTmpReg tmp(this, true);
    EncodeLdr(tmp, false, entryPoint);
    GetMasm()->Blr(VixlReg(tmp));
}

void Aarch64Encoder::MakeCall(Reg reg)
{
    GetMasm()->Blr(VixlReg(reg));
}

void Aarch64Encoder::MakeCall(LabelHolder::LabelId id)
{
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->Bl(label);
}

void Aarch64Encoder::LoadPcRelative(Reg reg, intptr_t offset, Reg regAddr)
{
    ASSERT(GetCodeOffset() != Encoder::INVALID_OFFSET);
    ASSERT(reg.IsValid() || regAddr.IsValid());

    if (!regAddr.IsValid()) {
        regAddr = reg.As(INT64_TYPE);
    }

    if (vixl::IsInt21(offset)) {
        GetMasm()->adr(VixlReg(regAddr), offset);
        if (reg != INVALID_REGISTER) {
            EncodeLdr(reg, false, MemRef(regAddr));
        }
    } else {
        size_t pc = GetCodeOffset() + GetCursorOffset();
        size_t addr;
        if (auto res = static_cast<intptr_t>(helpers::ToSigned(pc) + offset); res < 0) {
            // Make both, pc and addr, positive
            ssize_t extend = RoundUp(std::abs(res), vixl::aarch64::kPageSize);
            addr = static_cast<size_t>(res + extend);
            pc += static_cast<size_t>(extend);
        } else {
            addr = res;
        }

        ssize_t adrpImm = (addr >> vixl::aarch64::kPageSizeLog2) - (pc >> vixl::aarch64::kPageSizeLog2);

        GetMasm()->adrp(VixlReg(regAddr), adrpImm);

        offset = ark::helpers::ToUnsigned(addr) & (vixl::aarch64::kPageSize - 1);
        if (reg.GetId() != regAddr.GetId()) {
            EncodeAdd(regAddr, regAddr, Imm(offset));
            if (reg != INVALID_REGISTER) {
                EncodeLdr(reg, true, MemRef(regAddr));
            }
        } else {
            EncodeLdr(reg, true, MemRef(regAddr, offset));
        }
    }
}

void Aarch64Encoder::MakeCallAot(intptr_t offset)
{
    ScopedTmpReg tmp(this, true);
    LoadPcRelative(tmp, offset);
    GetMasm()->Blr(VixlReg(tmp));
}

bool Aarch64Encoder::CanMakeCallByOffset(intptr_t offset)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    auto off = (static_cast<uintptr_t>(offset) >> vixl::aarch64::kInstructionSizeLog2);
    return vixl::aarch64::Instruction::IsValidImmPCOffset(vixl::aarch64::ImmBranchType::UncondBranchType, off);
}

void Aarch64Encoder::MakeCallByOffset(intptr_t offset)
{
    GetMasm()->Bl(offset);
}

void Aarch64Encoder::MakeLoadAotTable(intptr_t offset, Reg reg)
{
    LoadPcRelative(reg, offset);
}

void Aarch64Encoder::MakeLoadAotTableAddr(intptr_t offset, Reg addr, Reg val)
{
    LoadPcRelative(val, offset, addr);
}

void Aarch64Encoder::EncodeAbort()
{
    GetMasm()->Brk();
}

void Aarch64Encoder::EncodeReturn()
{
    GetMasm()->Ret();
}

void Aarch64Encoder::EncodeMul([[maybe_unused]] Reg unused1, [[maybe_unused]] Reg unused2, [[maybe_unused]] Imm unused3)
{
    SetFalseResult();
}

void Aarch64Encoder::EncodeMov(Reg dst, Reg src)
{
    if (dst == src) {
        return;
    }
    if (src.IsFloat() && dst.IsFloat()) {
        if (src.GetSize() != dst.GetSize()) {
            GetMasm()->Fcvt(VixlVReg(dst), VixlVReg(src));
            return;
        }
        GetMasm()->Fmov(VixlVReg(dst), VixlVReg(src));
        return;
    }
    if (src.IsFloat() && !dst.IsFloat()) {
        GetMasm()->Fmov(VixlReg(dst, src.GetSize()), VixlVReg(src));
        return;
    }
    if (dst.IsFloat()) {
        ASSERT(src.IsScalar());
        GetMasm()->Fmov(VixlVReg(dst), VixlReg(src));
        return;
    }
    // DiscardForSameWReg below means we would drop "mov w0, w0", but it is guarded by "dst == src" above anyway.
    // NOTE: "mov w0, w0" is not equal "nop", as it clears upper bits of x0.
    // Keeping the option here helps to generate nothing when e.g. src is x0 and dst is w0.
    // Probably, a better solution here is to system-wide checking register size on Encoder level.
    if (src.GetSize() != dst.GetSize()) {
        auto srcReg = Reg(src.GetId(), dst.GetType());
        GetMasm()->Mov(VixlReg(dst), VixlReg(srcReg), vixl::aarch64::DiscardMoveMode::kDiscardForSameWReg);
        return;
    }
    GetMasm()->Mov(VixlReg(dst), VixlReg(src), vixl::aarch64::DiscardMoveMode::kDiscardForSameWReg);
}

void Aarch64Encoder::EncodeNeg(Reg dst, Reg src)
{
    if (dst.IsFloat()) {
        GetMasm()->Fneg(VixlVReg(dst), VixlVReg(src));
        return;
    }
    GetMasm()->Neg(VixlReg(dst), VixlReg(src));
}

void Aarch64Encoder::EncodeAbs(Reg dst, Reg src)
{
    if (dst.IsFloat()) {
        GetMasm()->Fabs(VixlVReg(dst), VixlVReg(src));
        return;
    }

    ASSERT(!GetRegfile()->IsZeroReg(dst));
    if (GetRegfile()->IsZeroReg(src)) {
        EncodeMov(dst, src);
        return;
    }

    if (src.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src), vixl::aarch64::xzr);
    } else {
        GetMasm()->Cmp(VixlReg(src), vixl::aarch64::wzr);
    }
    GetMasm()->Cneg(VixlReg(Promote(dst)), VixlReg(Promote(src)), vixl::aarch64::Condition::lt);
}

void Aarch64Encoder::EncodeSqrt(Reg dst, Reg src)
{
    ASSERT(dst.IsFloat());
    GetMasm()->Fsqrt(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeIsInf(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());

    if (src.GetSize() == WORD_SIZE) {
        constexpr uint32_t INF_MASK = 0xff000000;

        ScopedTmpRegU32 tmpReg(this);
        auto tmp = VixlReg(tmpReg);
        GetMasm()->Fmov(tmp, VixlVReg(src));
        GetMasm()->Mov(VixlReg(dst).W(), INF_MASK);
        GetMasm()->Lsl(tmp, tmp, 1);
        GetMasm()->Cmp(tmp, VixlReg(dst, WORD_SIZE));
    } else {
        constexpr uint64_t INF_MASK = 0xffe0000000000000;

        ScopedTmpRegU64 tmpReg(this);
        auto tmp = VixlReg(tmpReg);
        GetMasm()->Fmov(tmp, VixlVReg(src));
        GetMasm()->Mov(VixlReg(dst).X(), INF_MASK);
        GetMasm()->Lsl(tmp, tmp, 1);
        GetMasm()->Cmp(tmp, VixlReg(dst, DOUBLE_WORD_SIZE));
    }

    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::eq);
}

void Aarch64Encoder::EncodeCmpFracWithZero(Reg src)
{
    ASSERT(src.IsFloat());
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    // Encode (fabs(src - trunc(src)) <= DELTA)
    if (src.GetSize() == WORD_SIZE) {
        ScopedTmpRegF32 tmp(this);
        GetMasm()->Frintz(VixlVReg(tmp), VixlVReg(src));
        EncodeSub(tmp, src, tmp);
        EncodeAbs(tmp, tmp);
        GetMasm()->Fcmp(VixlVReg(tmp), 0.0);
    } else {
        ScopedTmpRegF64 tmp(this);
        GetMasm()->Frintz(VixlVReg(tmp), VixlVReg(src));
        EncodeSub(tmp, src, tmp);
        EncodeAbs(tmp, tmp);
        GetMasm()->Fcmp(VixlVReg(tmp), 0.0);
    }
}

void Aarch64Encoder::EncodeIsInteger(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    auto labelExit = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelInfOrNan = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    EncodeCmpFracWithZero(src);
    GetMasm()->B(labelInfOrNan, vixl::aarch64::Condition::vs);  // Inf or NaN
    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::le);
    GetMasm()->B(labelExit);

    // IsInteger returns false if src is Inf or NaN
    GetMasm()->Bind(labelInfOrNan);
    EncodeMov(dst, Imm(false));

    GetMasm()->Bind(labelExit);
}

void Aarch64Encoder::EncodeIsSafeInteger(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    auto labelExit = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelFalse = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    // Check if IsInteger
    EncodeCmpFracWithZero(src);
    GetMasm()->B(labelFalse, vixl::aarch64::Condition::vs);  // Inf or NaN
    GetMasm()->B(labelFalse, vixl::aarch64::Condition::gt);

    // Check if it is safe, i.e. src can be represented in float/double without losing precision
    if (src.GetSize() == WORD_SIZE) {
        ScopedTmpRegF32 tmp(this);
        EncodeAbs(tmp, src);
        GetMasm()->Fcmp(VixlVReg(tmp), MaxIntAsExactFloat());
    } else {
        ScopedTmpRegF64 tmp(this);
        EncodeAbs(tmp, src);
        GetMasm()->Fcmp(VixlVReg(tmp), MaxIntAsExactDouble());
    }
    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::le);
    GetMasm()->B(labelExit);

    // Return false if src !IsInteger
    GetMasm()->Bind(labelFalse);
    EncodeMov(dst, Imm(false));

    GetMasm()->Bind(labelExit);
}

/* NaN values are needed to be canonicalized */
void Aarch64Encoder::EncodeFpToBits(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(dst.GetSize() == WORD_SIZE || dst.GetSize() == DOUBLE_WORD_SIZE);

    if (dst.GetSize() == WORD_SIZE) {
        ASSERT(src.GetSize() == WORD_SIZE);

        constexpr auto FNAN = 0x7fc00000;

        ScopedTmpRegU32 tmp(this);

        GetMasm()->Fcmp(VixlVReg(src), VixlVReg(src));
        GetMasm()->Mov(VixlReg(tmp), FNAN);
        GetMasm()->Umov(VixlReg(dst), VixlVReg(src), 0);
        GetMasm()->Csel(VixlReg(dst), VixlReg(tmp), VixlReg(dst), vixl::aarch64::Condition::ne);
    } else {
        ASSERT(src.GetSize() == DOUBLE_WORD_SIZE);

        constexpr auto DNAN = 0x7ff8000000000000;

        ScopedTmpRegU64 tmpReg(this);
        auto tmp = VixlReg(tmpReg);

        GetMasm()->Fcmp(VixlVReg(src), VixlVReg(src));
        GetMasm()->Mov(tmp, DNAN);
        GetMasm()->Umov(VixlReg(dst), VixlVReg(src), 0);
        GetMasm()->Csel(VixlReg(dst), tmp, VixlReg(dst), vixl::aarch64::Condition::ne);
    }
}

void Aarch64Encoder::EncodeMoveBitsRaw(Reg dst, Reg src)
{
    ASSERT((dst.IsFloat() && src.IsScalar()) || (src.IsFloat() && dst.IsScalar()));
    if (dst.IsScalar()) {
        ASSERT(src.GetSize() == dst.GetSize());
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Umov(VixlReg(dst).W(), VixlVReg(src).S(), 0);
        } else {
            GetMasm()->Umov(VixlReg(dst), VixlVReg(src), 0);
        }
    } else {
        ASSERT(dst.GetSize() == src.GetSize());
        ScopedTmpReg tmpReg(this, src.GetType());
        auto srcReg = src;
        auto rzero = GetRegfile()->GetZeroReg();
        if (src.GetId() == rzero.GetId()) {
            EncodeMov(tmpReg, Imm(0));
            srcReg = tmpReg;
        }

        if (srcReg.GetSize() == WORD_SIZE) {
            GetMasm()->Fmov(VixlVReg(dst).S(), VixlReg(srcReg).W());
        } else {
            GetMasm()->Fmov(VixlVReg(dst), VixlReg(srcReg));
        }
    }
}

void Aarch64Encoder::EncodeReverseBytes(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (src.GetId() == rzero.GetId()) {
        EncodeMov(dst, Imm(0));
        return;
    }

    ASSERT(src.GetSize() > BYTE_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());

    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Rev16(VixlReg(dst), VixlReg(src));
        GetMasm()->Sxth(VixlReg(dst), VixlReg(dst));
    } else {
        GetMasm()->Rev(VixlReg(dst), VixlReg(src));
    }
}

void Aarch64Encoder::EncodeBitCount(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (src.GetId() == rzero.GetId()) {
        EncodeMov(dst, Imm(0));
        return;
    }

    ASSERT(dst.GetSize() == WORD_SIZE);

    ScopedTmpRegF64 tmpReg0(this);
    vixl::aarch64::VRegister tmpReg;
    if (src.GetSize() == DOUBLE_WORD_SIZE) {
        tmpReg = VixlVReg(tmpReg0).D();
    } else {
        tmpReg = VixlVReg(tmpReg0).S();
    }

    if (src.GetSize() < WORD_SIZE) {
        int64_t cutValue = (1ULL << src.GetSize()) - 1;
        EncodeAnd(src, src, Imm(cutValue));
    }

    GetMasm()->Fmov(tmpReg, VixlReg(src));
    GetMasm()->Cnt(tmpReg.V8B(), tmpReg.V8B());
    GetMasm()->Addv(tmpReg.B(), tmpReg.V8B());
    EncodeMov(dst, tmpReg0);
}

/* Since only ROR is supported on AArch64 we do
 * left rotaion as ROR(v, -count) */
void Aarch64Encoder::EncodeRotate(Reg dst, Reg src1, Reg src2, bool isRor)
{
    ASSERT(src1.GetSize() == WORD_SIZE || src1.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(src1.GetSize() == dst.GetSize());
    auto rzero = GetRegfile()->GetZeroReg();
    if (rzero.GetId() == src2.GetId() || rzero.GetId() == src1.GetId()) {
        EncodeMov(dst, src1);
        return;
    }
    /* as the second parameters is always 32-bits long we have to
     * adjust the counter register for the 64-bits first operand case */
    if (isRor) {
        auto count = (dst.GetSize() == WORD_SIZE ? VixlReg(src2) : VixlReg(src2).X());
        GetMasm()->Ror(VixlReg(dst), VixlReg(src1), count);
    } else {
        ScopedTmpReg tmp(this);
        auto cnt = (dst.GetId() == src1.GetId() ? tmp : dst);
        auto count = (dst.GetSize() == WORD_SIZE ? VixlReg(cnt).W() : VixlReg(cnt).X());
        auto source2 = (dst.GetSize() == WORD_SIZE ? VixlReg(src2).W() : VixlReg(src2).X());
        GetMasm()->Neg(count, source2);
        GetMasm()->Ror(VixlReg(dst), VixlReg(src1), count);
    }
}

void Aarch64Encoder::EncodeSignum(Reg dst, Reg src)
{
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    ScopedTmpRegU32 tmp(this);
    auto sign = (dst.GetId() == src.GetId() ? tmp : dst);

    GetMasm()->Cmp(VixlReg(src), VixlImm(0));
    GetMasm()->Cset(VixlReg(sign), vixl::aarch64::Condition::gt);

    constexpr auto SHIFT_WORD_BITS = 31;
    constexpr auto SHIFT_DWORD_BITS = 63;

    /* The operation below is "sub dst, dst, src, lsr #reg_size-1"
     * however, we can only encode as many as 32 bits in lsr field, so
     * for 64-bits cases we cannot avoid having a separate lsr instruction */
    if (src.GetSize() == WORD_SIZE) {
        auto shift = Shift(src, LSR, SHIFT_WORD_BITS);
        EncodeSub(dst, sign, shift);
    } else {
        ScopedTmpRegU64 shift(this);
        sign = Reg(sign.GetId(), INT64_TYPE);
        EncodeShr(shift, src, Imm(SHIFT_DWORD_BITS));
        EncodeSub(dst, sign, shift);
    }
}

void Aarch64Encoder::EncodeCountLeadingZeroBits(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (rzero.GetId() == src.GetId()) {
        EncodeMov(dst, Imm(src.GetSize()));
        return;
    }
    GetMasm()->Clz(VixlReg(dst), VixlReg(src));
}

void Aarch64Encoder::EncodeCountTrailingZeroBits(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (rzero.GetId() == src.GetId()) {
        EncodeMov(dst, Imm(src.GetSize()));
        return;
    }
    GetMasm()->Rbit(VixlReg(dst), VixlReg(src));
    GetMasm()->Clz(VixlReg(dst), VixlReg(dst));
}

void Aarch64Encoder::EncodeCeil(Reg dst, Reg src)
{
    GetMasm()->Frintp(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeFloor(Reg dst, Reg src)
{
    GetMasm()->Frintm(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeRint(Reg dst, Reg src)
{
    GetMasm()->Frintn(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeTrunc(Reg dst, Reg src)
{
    GetMasm()->Frintz(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeRoundAway(Reg dst, Reg src)
{
    GetMasm()->Frinta(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeRoundToPInfReturnScalar(Reg dst, Reg src)
{
    auto done = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    ScopedTmpReg tmp(this, src.GetType());
    // round to nearest integer, ties away from zero
    GetMasm()->Fcvtas(VixlReg(dst), VixlVReg(src));
    // for positive values, zero and NaN inputs rounding is done
    GetMasm()->Tbz(VixlReg(dst), dst.GetSize() - 1, done);
    // if input is negative but not a tie, round to nearest is valid
    // if input is a negative tie, dst += 1
    GetMasm()->Frinta(VixlVReg(tmp), VixlVReg(src));
    GetMasm()->Fsub(VixlVReg(tmp), VixlVReg(src), VixlVReg(tmp));
    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->Fcmp(VixlVReg(tmp), 0.5F);
    GetMasm()->Cinc(VixlReg(dst), VixlReg(dst), vixl::aarch64::Condition::eq);
    GetMasm()->Bind(done);
}

void Aarch64Encoder::EncodeRoundToPInfReturnFloat(Reg dst, Reg src)
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

    auto skipZero = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto negZeroCase = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto done = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Fmov(VixlVReg(ceil), NEG_HALF);
    GetMasm()->Fcmp(VixlVReg(src), VixlVReg(ceil));
    GetMasm()->B(vixl::aarch64::lt, skipZero);

    GetMasm()->Fmov(VixlVReg(ceil), HALF);
    GetMasm()->Fcmp(VixlVReg(src), VixlVReg(ceil));
    GetMasm()->B(vixl::aarch64::ge, skipZero);

    GetMasm()->Fmov(VixlVReg(ceil), ZERO);
    GetMasm()->Fcmp(VixlVReg(src), VixlVReg(ceil));
    GetMasm()->B(vixl::aarch64::lt, negZeroCase);

    GetMasm()->Fmov(VixlVReg(dst), ZERO);
    GetMasm()->B(done);

    GetMasm()->Bind(negZeroCase);
    GetMasm()->Fmov(VixlVReg(dst), NEG_ZERO);
    GetMasm()->B(done);

    GetMasm()->Bind(skipZero);

    // calculate ceil(val)
    GetMasm()->Frintp(VixlVReg(ceil), VixlVReg(src));

    // compare ceil(val) - val with 0.5
    GetMasm()->Fsub(VixlVReg(dst), VixlVReg(ceil), VixlVReg(src));
    GetMasm()->Fcmp(VixlVReg(dst), HALF);

    // calculate ceil(val) - 1
    GetMasm()->Fmov(VixlVReg(dst), ONE);
    GetMasm()->Fsub(VixlVReg(dst), VixlVReg(ceil), VixlVReg(dst));

    // select final value based on comparison result
    GetMasm()->Fcsel(VixlVReg(dst), VixlVReg(dst), VixlVReg(ceil), vixl::aarch64::Condition::gt);
    GetMasm()->Bind(done);
}

void Aarch64Encoder::EncodeCrc32Update(Reg dst, Reg crcReg, Reg valReg)
{
    auto tmp = dst.GetId() != crcReg.GetId() && dst.GetId() != valReg.GetId() ? dst : ScopedTmpReg(this, dst.GetType());
    GetMasm()->Mvn(VixlReg(tmp), VixlReg(crcReg));
    GetMasm()->Crc32b(VixlReg(tmp), VixlReg(tmp), VixlReg(valReg));
    GetMasm()->Mvn(VixlReg(dst), VixlReg(tmp));
}

void Aarch64Encoder::EncodeCompressEightUtf16ToUtf8CharsUsingSimd(Reg srcAddr, Reg dstAddr)
{
    ScopedTmpReg tmp1(this, FLOAT64_TYPE);
    ScopedTmpReg tmp2(this, FLOAT64_TYPE);
    auto vixlVreg1 = vixl::aarch64::VRegister(tmp1.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat8B);
    ASSERT(vixlVreg1.IsValid());
    auto vixlVreg2 = vixl::aarch64::VRegister(tmp2.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat8B);
    ASSERT(vixlVreg2.IsValid());
    auto src = vixl::aarch64::MemOperand(VixlReg(srcAddr));
    auto dst = vixl::aarch64::MemOperand(VixlReg(dstAddr));
    GetMasm()->Ld2(vixlVreg1, vixlVreg2, src);
    GetMasm()->St1(vixlVreg1, dst);
}

void Aarch64Encoder::EncodeCompressSixteenUtf16ToUtf8CharsUsingSimd(Reg srcAddr, Reg dstAddr)
{
    ScopedTmpReg tmp1(this, FLOAT64_TYPE);
    ScopedTmpReg tmp2(this, FLOAT64_TYPE);
    auto vixlVreg1 = vixl::aarch64::VRegister(tmp1.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    ASSERT(vixlVreg1.IsValid());
    auto vixlVreg2 = vixl::aarch64::VRegister(tmp2.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    ASSERT(vixlVreg2.IsValid());
    auto src = vixl::aarch64::MemOperand(VixlReg(srcAddr));
    auto dst = vixl::aarch64::MemOperand(VixlReg(dstAddr));
    GetMasm()->Ld2(vixlVreg1, vixlVreg2, src);
    GetMasm()->St1(vixlVreg1, dst);
}

void Aarch64Encoder::EncodeMemCharU8X32UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp)
{
    ScopedTmpReg vTmp0(this, FLOAT64_TYPE);
    ScopedTmpReg vTmp1(this, FLOAT64_TYPE);
    auto vReg0 = vixl::aarch64::VRegister(vTmp0.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    auto vReg1 = vixl::aarch64::VRegister(vTmp1.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    auto vReg2 = vixl::aarch64::VRegister(tmp.GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    auto xReg0 = vixl::aarch64::Register(dst.GetId(), vixl::aarch64::kXRegSize);
    auto labelReturn = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelFound = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelSecond16B = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCheckV0D1 = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCheckV1D1 = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Ld1(vReg0, vReg1, vixl::aarch64::MemOperand(VixlReg(srcAddr)));
    GetMasm()->Dup(vReg2, VixlReg(ch));
    GetMasm()->Cmeq(vReg0, vReg0, vReg2);
    GetMasm()->Cmeq(vReg1, vReg1, vReg2);
    // Give up if char is not there
    GetMasm()->Addp(vReg2, vReg0, vReg1);
    GetMasm()->Addp(vReg2.V2D(), vReg2.V2D(), vReg2.V2D());
    GetMasm()->Mov(xReg0, vReg2.D(), 0);
    GetMasm()->Cbz(xReg0, labelReturn);
    // Inspect the first 16-byte block
    GetMasm()->Mov(xReg0, vReg0.D(), 0);
    GetMasm()->Cbz(xReg0, labelCheckV0D1);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->B(labelFound);
    GetMasm()->Bind(labelCheckV0D1);
    GetMasm()->Mov(xReg0, vReg0.D(), 1U);
    GetMasm()->Cbz(xReg0, labelSecond16B);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(BITS_PER_UINT64));
    GetMasm()->B(labelFound);
    // Inspect the second 16-byte block
    GetMasm()->Bind(labelSecond16B);
    GetMasm()->Mov(xReg0, vReg1.D(), 0);
    GetMasm()->Cbz(xReg0, labelCheckV1D1);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(2U * BITS_PER_UINT64));
    GetMasm()->B(labelFound);
    GetMasm()->Bind(labelCheckV1D1);
    GetMasm()->Mov(xReg0, vReg1.D(), 1U);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(3U * BITS_PER_UINT64));

    GetMasm()->Bind(labelFound);
    GetMasm()->Lsr(xReg0, xReg0, 3U);
    GetMasm()->Add(xReg0, xReg0, VixlReg(srcAddr));
    GetMasm()->Bind(labelReturn);
}

void Aarch64Encoder::EncodeMemCharU16X16UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp)
{
    ScopedTmpReg vTmp0(this, FLOAT64_TYPE);
    ScopedTmpReg vTmp1(this, FLOAT64_TYPE);
    auto vReg0 = vixl::aarch64::VRegister(vTmp0.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat8H);
    auto vReg1 = vixl::aarch64::VRegister(vTmp1.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat8H);
    auto vReg2 = vixl::aarch64::VRegister(tmp.GetId(), vixl::aarch64::VectorFormat::kFormat8H);
    auto xReg0 = vixl::aarch64::Register(dst.GetId(), vixl::aarch64::kXRegSize);
    auto labelReturn = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelFound = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelSecond16B = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCheckV0D1 = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCheckV1D1 = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Ld1(vReg0, vReg1, vixl::aarch64::MemOperand(VixlReg(srcAddr)));
    GetMasm()->Dup(vReg2, VixlReg(ch));
    GetMasm()->Cmeq(vReg0, vReg0, vReg2);
    GetMasm()->Cmeq(vReg1, vReg1, vReg2);
    // Give up if char is not there
    GetMasm()->Addp(vReg2, vReg0, vReg1);
    GetMasm()->Addp(vReg2.V2D(), vReg2.V2D(), vReg2.V2D());
    GetMasm()->Mov(xReg0, vReg2.D(), 0);
    GetMasm()->Cbz(xReg0, labelReturn);
    // Inspect the first 16-byte block
    GetMasm()->Mov(xReg0, vReg0.D(), 0);
    GetMasm()->Cbz(xReg0, labelCheckV0D1);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->B(labelFound);
    GetMasm()->Bind(labelCheckV0D1);
    GetMasm()->Mov(xReg0, vReg0.D(), 1U);
    GetMasm()->Cbz(xReg0, labelSecond16B);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(BITS_PER_UINT64));
    GetMasm()->B(labelFound);
    // Inspect the second 16-byte block
    GetMasm()->Bind(labelSecond16B);
    GetMasm()->Mov(xReg0, vReg1.D(), 0);
    GetMasm()->Cbz(xReg0, labelCheckV1D1);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(2U * BITS_PER_UINT64));
    GetMasm()->B(labelFound);
    GetMasm()->Bind(labelCheckV1D1);
    GetMasm()->Mov(xReg0, vReg1.D(), 1U);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(3U * BITS_PER_UINT64));

    GetMasm()->Bind(labelFound);
    GetMasm()->Lsr(xReg0, xReg0, 4U);
    GetMasm()->Lsl(xReg0, xReg0, 1U);
    GetMasm()->Add(xReg0, xReg0, VixlReg(srcAddr));
    GetMasm()->Bind(labelReturn);
}

void Aarch64Encoder::EncodeMemCharU8X16UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp)
{
    ScopedTmpReg vTmp0(this, FLOAT64_TYPE);
    ScopedTmpReg vTmp1(this, FLOAT64_TYPE);
    auto vReg0 = vixl::aarch64::VRegister(vTmp0.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    auto vReg1 = vixl::aarch64::VRegister(tmp.GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    auto xReg0 = vixl::aarch64::Register(dst.GetId(), vixl::aarch64::kXRegSize);
    auto labelReturn = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelFound = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCheckV0D1 = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Ld1(vReg0, vixl::aarch64::MemOperand(VixlReg(srcAddr)));
    GetMasm()->Dup(vReg1, VixlReg(ch));
    GetMasm()->Cmeq(vReg0, vReg0, vReg1);
    // Give up if char is not there
    GetMasm()->Addp(vReg1.V2D(), vReg0.V2D(), vReg0.V2D());
    GetMasm()->Mov(xReg0, vReg1.D(), 0);
    GetMasm()->Cbz(xReg0, labelReturn);
    // Compute a pointer to the char
    GetMasm()->Mov(xReg0, vReg0.D(), 0);
    GetMasm()->Cbz(xReg0, labelCheckV0D1);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->B(labelFound);
    GetMasm()->Bind(labelCheckV0D1);
    GetMasm()->Mov(xReg0, vReg0.D(), 1U);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(BITS_PER_UINT64));
    GetMasm()->Bind(labelFound);
    GetMasm()->Lsr(xReg0, xReg0, 3U);  // number of 8-bit chars
    GetMasm()->Add(xReg0, xReg0, VixlReg(srcAddr));
    GetMasm()->Bind(labelReturn);
}

void Aarch64Encoder::EncodeMemCharU16X8UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp)
{
    ScopedTmpReg vTmp0(this, FLOAT64_TYPE);
    ScopedTmpReg vTmp1(this, FLOAT64_TYPE);
    auto vReg0 = vixl::aarch64::VRegister(vTmp0.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat8H);
    auto vReg1 = vixl::aarch64::VRegister(tmp.GetId(), vixl::aarch64::VectorFormat::kFormat8H);
    auto xReg0 = vixl::aarch64::Register(dst.GetId(), vixl::aarch64::kXRegSize);
    auto labelReturn = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelFound = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCheckV0D1 = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Ld1(vReg0, vixl::aarch64::MemOperand(VixlReg(srcAddr)));
    GetMasm()->Dup(vReg1, VixlReg(ch));
    GetMasm()->Cmeq(vReg0, vReg0, vReg1);
    // Give up if char is not there
    GetMasm()->Addp(vReg1.V2D(), vReg0.V2D(), vReg0.V2D());
    GetMasm()->Mov(xReg0, vReg1.D(), 0);
    GetMasm()->Cbz(xReg0, labelReturn);
    // Compute a pointer to the char
    GetMasm()->Mov(xReg0, vReg0.D(), 0);
    GetMasm()->Cbz(xReg0, labelCheckV0D1);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->B(labelFound);
    GetMasm()->Bind(labelCheckV0D1);
    GetMasm()->Mov(xReg0, vReg0.D(), 1U);
    GetMasm()->Rev(xReg0, xReg0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(BITS_PER_UINT64));
    GetMasm()->Bind(labelFound);
    GetMasm()->Lsr(xReg0, xReg0, 4U);  // number of 16-bit chars
    GetMasm()->Lsl(xReg0, xReg0, 1U);  // number of bytes
    GetMasm()->Add(xReg0, xReg0, VixlReg(srcAddr));
    GetMasm()->Bind(labelReturn);
}

void Aarch64Encoder::EncodeMemLastCharU16X8UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp)
{
    ScopedTmpReg vTmp0(this, FLOAT64_TYPE);
    ScopedTmpReg vTmp1(this, FLOAT64_TYPE);
    auto vReg0 = vixl::aarch64::VRegister(vTmp0.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat8H);
    auto vReg1 = vixl::aarch64::VRegister(tmp.GetId(), vixl::aarch64::VectorFormat::kFormat8H);
    auto xReg0 = vixl::aarch64::Register(dst.GetId(), vixl::aarch64::kXRegSize);
    auto labelReturn = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelFound = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCheckV0D0 = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    // We use 'Ld1' to load the string into a vector register containing eight 16-bit elements (kFormat8H).
    //
    // 'Ld1' transfer vector elements as follows:
    //   - the order of the elements in the register is the same regardless of the endianness
    //   - the order of bytes in the elements depends on the endianness
    //
    // Suppose that we are looking for '零' in the following u16 string: '一零二三四五六七'
    // 'Ld1' loads eight u16 chars with the lowest indexed element fetched from the lowest address.
    // The order of bytes in the elements does not matter in this particular case.
    //
    // 1. Load eight 16-bit chars into vReg0:
    //       7    6    5    4    3    2    1    0
    //     | 七 | 六 | 五 | 四 | 三 | 二 | 零 | 一 |
    //
    // 2. Fill vReg1 with eight '零':
    //     | 零 | 零 | 零 | 零 | 零 | 零 | 零 | 零 |
    //
    // 3. Compare vReg0 with vReg1 using 'Cmeq':
    //     |0000|0000|0000|0000|0000|0000|FFFF|0000|
    //
    // 4. Compute an offset (in u16 chars) to the char of interest (if any).
    //    It would be better to use 'Ctz' instruction but it is supported iff FEAT_CSSC is in effect.
    //    As FEAT_CSSC is optional from Armv8.7 and mandatory from Armv8.9.
    //    So now we make use of 'Clz' and compute the offset as follows: (7 - (Clz(vReg0) / 16)).
    //    For this particular example it is as follows: (7 - 96/16) = 1
    //
    // 5. Compute an address of the char if it is found
    //
    GetMasm()->Ld1(vReg0, vixl::aarch64::MemOperand(VixlReg(srcAddr)));
    GetMasm()->Dup(vReg1, VixlReg(ch));
    GetMasm()->Cmeq(vReg0, vReg0, vReg1);
    // Give up if char is not there
    GetMasm()->Addp(vReg1.V2D(), vReg0.V2D(), vReg0.V2D());
    GetMasm()->Mov(xReg0, vReg1.D(), 0);
    GetMasm()->Cbz(xReg0, labelReturn);
    // Compute a pointer to the char
    // Check D1 first as we are looking for the last occurrence of 'ch'
    GetMasm()->Mov(xReg0, vReg0.D(), 1U);
    GetMasm()->Cbz(xReg0, labelCheckV0D0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->B(labelFound);
    GetMasm()->Bind(labelCheckV0D0);
    GetMasm()->Mov(xReg0, vReg0.D(), 0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(BITS_PER_UINT64));
    GetMasm()->Bind(labelFound);
    GetMasm()->Lsr(xReg0, xReg0, 4U);  // number of 16-bit chars
    GetMasm()->Neg(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(7U));
    GetMasm()->Lsl(xReg0, xReg0, 1U);  // number of bytes
    GetMasm()->Add(xReg0, xReg0, VixlReg(srcAddr));
    GetMasm()->Bind(labelReturn);
}

void Aarch64Encoder::EncodeMemLastCharU8X16UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp)
{
    ScopedTmpReg vTmp0(this, FLOAT64_TYPE);
    ScopedTmpReg vTmp1(this, FLOAT64_TYPE);
    auto vReg0 = vixl::aarch64::VRegister(vTmp0.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    auto vReg1 = vixl::aarch64::VRegister(tmp.GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    auto xReg0 = vixl::aarch64::Register(dst.GetId(), vixl::aarch64::kXRegSize);
    auto labelReturn = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelFound = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCheckV0D0 = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Ld1(vReg0, vixl::aarch64::MemOperand(VixlReg(srcAddr)));
    GetMasm()->Dup(vReg1, VixlReg(ch));
    GetMasm()->Cmeq(vReg0, vReg0, vReg1);
    // Give up if char is not there
    GetMasm()->Addp(vReg1.V2D(), vReg0.V2D(), vReg0.V2D());
    GetMasm()->Mov(xReg0, vReg1.D(), 0);
    GetMasm()->Cbz(xReg0, labelReturn);
    // Compute a pointer to the char
    // Check D1 first as we are looking for the last occurrence of 'ch'
    GetMasm()->Mov(xReg0, vReg0.D(), 1U);
    GetMasm()->Cbz(xReg0, labelCheckV0D0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->B(labelFound);
    GetMasm()->Bind(labelCheckV0D0);
    GetMasm()->Mov(xReg0, vReg0.D(), 0);
    GetMasm()->Clz(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(BITS_PER_UINT64));
    GetMasm()->Bind(labelFound);
    GetMasm()->Lsr(xReg0, xReg0, 3U);  // number of 8-bit chars
    GetMasm()->Neg(xReg0, xReg0);
    GetMasm()->Add(xReg0, xReg0, VixlImm(15U));  // NOLINT(readability-magic-numbers)
    GetMasm()->Add(xReg0, xReg0, VixlReg(srcAddr));
    GetMasm()->Bind(labelReturn);
}

template <uint8_t BEGIN, uint8_t END, bool EIGHT_BYTES>
static void EncodeCharsToCaseU8UsingSimd(Aarch64Encoder *encoder, Reg srcAddr, Reg dstAddr, Reg tmp)
{
    ScopedTmpReg vTmp0(encoder, FLOAT64_TYPE);
    ScopedTmpReg vTmp1(encoder, FLOAT64_TYPE);
    auto vRegSrc = vixl::aarch64::VRegister(vTmp0.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    auto vRegDst = vixl::aarch64::VRegister(vTmp1.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat16B);
    constexpr uint8_t CASE_CONVERSION_FLIP_BIT = 1U << 5U;     // CC-OFF(G.NAM.03-CPP) project code style
    constexpr uint8_t INVERTED_BEGIN = 0xFFU - BEGIN + 0x01U;  // CC-OFF(G.NAM.03-CPP) project code style
    auto vRegFlip = vixl::aarch64::VRegister(tmp.GetId(), vixl::aarch64::VectorFormat::kFormat16B);

    if constexpr (EIGHT_BYTES) {
        encoder->GetMasm()->Ldr(vRegSrc.V1D(), vixl::aarch64::MemOperand(VixlReg(srcAddr)));
    } else {
        encoder->GetMasm()->Ldr(vRegSrc, vixl::aarch64::MemOperand(VixlReg(srcAddr)));
    }
    encoder->GetMasm()->Movi(vRegDst, INVERTED_BEGIN);
    encoder->GetMasm()->Add(vRegDst, vRegDst, vRegSrc);
    encoder->GetMasm()->Movi(vRegFlip, END - BEGIN);
    encoder->GetMasm()->Cmhs(vRegDst, vRegFlip, vRegDst);
    encoder->GetMasm()->Movi(vRegFlip, CASE_CONVERSION_FLIP_BIT);
    encoder->GetMasm()->Eor(vRegFlip, vRegSrc, vRegFlip);
    encoder->GetMasm()->Bsl(vRegDst, vRegFlip, vRegSrc);
    if constexpr (EIGHT_BYTES) {
        encoder->GetMasm()->Str(vRegDst.V1D(), vixl::aarch64::MemOperand(VixlReg(dstAddr)));
    } else {
        encoder->GetMasm()->Str(vRegDst, vixl::aarch64::MemOperand(VixlReg(dstAddr)));
    }
}

void Aarch64Encoder::EncodeCharsToUpperCaseU8X16UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    EncodeCharsToCaseU8UsingSimd<0x61U, 0x7aU, false>(this, srcAddr, dstAddr, tmp);
}

void Aarch64Encoder::EncodeCharsToUpperCaseU8X8UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    EncodeCharsToCaseU8UsingSimd<0x61U, 0x7aU, true>(this, srcAddr, dstAddr, tmp);
}

void Aarch64Encoder::EncodeCharsToLowerCaseU8X16UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    EncodeCharsToCaseU8UsingSimd<0x41U, 0x5aU, false>(this, srcAddr, dstAddr, tmp);
}

void Aarch64Encoder::EncodeCharsToLowerCaseU8X8UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    EncodeCharsToCaseU8UsingSimd<0x41U, 0x5aU, true>(this, srcAddr, dstAddr, tmp);
}

void Aarch64Encoder::EncodeUnsignedExtendBytesToShorts(Reg dst, Reg src)
{
    GetMasm()->Uxtl(VixlVReg(dst).V8H(), VixlVReg(src).V8B());
}

void Aarch64Encoder::EncodeReverseHalfWords(Reg dst, Reg src)
{
    ASSERT(src.GetSize() == dst.GetSize());

    GetMasm()->rev64(VixlVReg(dst).V4H(), VixlVReg(src).V4H());
}

bool Aarch64Encoder::CanEncodeBitCount()
{
    return true;
}

bool Aarch64Encoder::CanEncodeCompressedStringCharAt()
{
    return true;
}

bool Aarch64Encoder::CanEncodeCompressedStringCharAtI()
{
    return true;
}

bool Aarch64Encoder::CanEncodeMAdd()
{
    return true;
}

bool Aarch64Encoder::CanEncodeMSub()
{
    return true;
}

bool Aarch64Encoder::CanEncodeMNeg()
{
    return true;
}

bool Aarch64Encoder::CanEncodeOrNot()
{
    return true;
}

bool Aarch64Encoder::CanEncodeAndNot()
{
    return true;
}

bool Aarch64Encoder::CanEncodeXorNot()
{
    return true;
}

size_t Aarch64Encoder::GetCursorOffset() const
{
    return GetMasm()->GetBuffer()->GetCursorOffset();
}

void Aarch64Encoder::SetCursorOffset(size_t offset)
{
    GetMasm()->GetBuffer()->Rewind(offset);
}

/* return the power of 2 for the size of the type */
void Aarch64Encoder::EncodeGetTypeSize(Reg size, Reg type)
{
    auto sreg = VixlReg(type);
    auto dreg = VixlReg(size);
    constexpr uint8_t I16 = 0x5;
    constexpr uint8_t I32 = 0x7;
    constexpr uint8_t F64 = 0xa;
    constexpr uint8_t REF = 0xd;
    constexpr uint8_t SMALLREF = ark::OBJECT_POINTER_SIZE < sizeof(uint64_t) ? 1 : 0;
    auto end = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Mov(dreg, VixlImm(0));
    GetMasm()->Cmp(sreg, VixlImm(I16));
    GetMasm()->Cinc(dreg, dreg, vixl::aarch64::Condition::ge);
    GetMasm()->Cmp(sreg, VixlImm(I32));
    GetMasm()->Cinc(dreg, dreg, vixl::aarch64::Condition::ge);
    GetMasm()->Cmp(sreg, VixlImm(F64));
    GetMasm()->Cinc(dreg, dreg, vixl::aarch64::Condition::ge);
    GetMasm()->Cmp(sreg, VixlImm(REF));
    GetMasm()->B(end, vixl::aarch64::Condition::ne);
    GetMasm()->Sub(dreg, dreg, VixlImm(SMALLREF));
    GetMasm()->Bind(end);
}

void Aarch64Encoder::EncodeReverseBits(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (rzero.GetId() == src.GetId()) {
        EncodeMov(dst, Imm(0));
        return;
    }
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());

    GetMasm()->Rbit(VixlReg(dst), VixlReg(src));
}

void Aarch64Encoder::EncodeCompressedStringCharAt(ArgsCompressedStringCharAt &&args)
{
    auto [dst, str, idx, length, tmp, dataOffset, shift] = args;
    ASSERT(dst.GetSize() == HALF_SIZE);

    auto labelNotCompressed = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCharLoaded = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto vixlTmp = VixlReg(tmp, DOUBLE_WORD_SIZE);
    auto vixlDst = VixlReg(dst);

    GetMasm()->Tbnz(VixlReg(length), 0, labelNotCompressed);
    EncodeAdd(tmp, str, idx);
    GetMasm()->ldrb(vixlDst, MemOperand(vixlTmp, dataOffset));
    GetMasm()->B(labelCharLoaded);
    GetMasm()->Bind(labelNotCompressed);
    EncodeAdd(tmp, str, Shift(idx, shift));
    GetMasm()->ldrh(vixlDst, MemOperand(vixlTmp, dataOffset));
    GetMasm()->Bind(labelCharLoaded);
}

void Aarch64Encoder::EncodeCompressedStringCharAtI(ArgsCompressedStringCharAtI &&args)
{
    auto [dst, str, length, dataOffset, index, shift] = args;
    ASSERT(dst.GetSize() == HALF_SIZE);

    auto labelNotCompressed = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto labelCharLoaded = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto vixlStr = VixlReg(str);
    auto vixlDst = VixlReg(dst);

    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (str.GetId() == rzero) {
        return;
    }
    GetMasm()->Tbnz(VixlReg(length), 0, labelNotCompressed);
    GetMasm()->Ldrb(vixlDst, MemOperand(vixlStr, dataOffset + index));
    GetMasm()->B(labelCharLoaded);
    GetMasm()->Bind(labelNotCompressed);
    GetMasm()->Ldrh(vixlDst, MemOperand(vixlStr, dataOffset + (index << shift)));
    GetMasm()->Bind(labelCharLoaded);
}

/* Unsafe builtins implementation */
void Aarch64Encoder::EncodeCompareAndSwap(Reg dst, Reg obj, Reg offset, Reg val, Reg newval)
{
    /* Modeled according to the following logic:
      .L2:
      ldaxr   cur, [addr]
      cmp     cur, old
      bne     .L3
      stlxr   res, new, [addr]
      cbnz    res, .L2
      .L3:
      cset    w0, eq
    */
    ScopedTmpReg addr(this, true); /* LR is used */
    ScopedTmpReg cur(this, val.GetType());
    ScopedTmpReg res(this, val.GetType());
    auto loop = CreateLabel();
    auto exit = CreateLabel();

    /* ldaxr wants [reg]-form of memref (no offset or disp) */
    EncodeAdd(addr, obj, offset);

    BindLabel(loop);
    EncodeLdrExclusive(cur, addr, true);
    EncodeJump(exit, cur, val, Condition::NE);
    cur.Release();
    EncodeStrExclusive(res, newval, addr, true);
    EncodeJump(loop, res, Imm(0), Condition::NE);
    BindLabel(exit);

    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::eq);
}

void Aarch64Encoder::EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val)
{
    auto cur = ScopedTmpReg(this, val.GetType());
    auto last = ScopedTmpReg(this, val.GetType());
    auto addr = ScopedTmpReg(this, true); /* LR is used */
    auto mem = MemRef(addr);
    auto restart = CreateLabel();
    auto retryLdaxr = CreateLabel();

    /* ldaxr wants [reg]-form of memref (no offset or disp) */
    EncodeAdd(addr, obj, offset);

    /* Since GetAndSet is defined as a non-faulting operation we
     * have to cover two possible faulty cases:
     *      1. stlxr failed, we have to retry ldxar
     *      2. the value we got via ldxar was not the value we initially
     *         loaded, we have to start from the very beginning */
    BindLabel(restart);
    EncodeLdrAcquire(last, false, mem);

    BindLabel(retryLdaxr);
    EncodeLdrExclusive(cur, addr, true);
    EncodeJump(restart, cur, last, Condition::NE);
    last.Release();
    EncodeStrExclusive(dst, val, addr, true);
    EncodeJump(retryLdaxr, dst, Imm(0), Condition::NE);

    EncodeMov(dst, cur);
}

void Aarch64Encoder::EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, Reg tmp)
{
    ScopedTmpReg cur(this, val.GetType());
    ScopedTmpReg last(this, val.GetType());
    auto newval = Reg(tmp.GetId(), val.GetType());

    auto restart = CreateLabel();
    auto retryLdaxr = CreateLabel();

    /* addr_reg aliases obj, obj reg will be restored bedore exit */
    auto addr = Reg(obj.GetId(), INT64_TYPE);

    /* ldaxr wants [reg]-form of memref (no offset or disp) */
    auto mem = MemRef(addr);
    EncodeAdd(addr, obj, offset);

    /* Since GetAndAdd is defined as a non-faulting operation we
     * have to cover two possible faulty cases:
     *      1. stlxr failed, we have to retry ldxar
     *      2. the value we got via ldxar was not the value we initially
     *         loaded, we have to start from the very beginning */
    BindLabel(restart);
    EncodeLdrAcquire(last, false, mem);
    EncodeAdd(newval, last, val);

    BindLabel(retryLdaxr);
    EncodeLdrExclusive(cur, addr, true);
    EncodeJump(restart, cur, last, Condition::NE);
    last.Release();
    EncodeStrExclusive(dst, newval, addr, true);
    EncodeJump(retryLdaxr, dst, Imm(0), Condition::NE);

    EncodeSub(obj, addr, offset); /* restore the original value */
    EncodeMov(dst, cur);
}

void Aarch64Encoder::EncodeMemoryBarrier(memory_order::Order order)
{
    switch (order) {
        case memory_order::ACQUIRE: {
            GetMasm()->Dmb(vixl::aarch64::InnerShareable, vixl::aarch64::BarrierReads);
            break;
        }
        case memory_order::RELEASE: {
            GetMasm()->Dmb(vixl::aarch64::InnerShareable, vixl::aarch64::BarrierWrites);
            break;
        }
        case memory_order::FULL: {
            GetMasm()->Dmb(vixl::aarch64::InnerShareable, vixl::aarch64::BarrierAll);
            break;
        }
        default:
            break;
    }
}

void Aarch64Encoder::EncodeNot(Reg dst, Reg src)
{
    GetMasm()->Mvn(VixlReg(dst), VixlReg(src));
}

void Aarch64Encoder::EncodeCastFloat(Reg dst, bool dstSigned, Reg src, bool srcSigned)
{
    // We DON'T support casts from float32/64 to int8/16 and bool, because this caste is not declared anywhere
    // in other languages and architecture, we do not know what the behavior should be.
    // But there is one implementation in other function: "EncodeCastFloatWithSmallDst". Call it in the "EncodeCast"
    // function instead of "EncodeCastFloat". It works as follows: cast from float32/64 to int32, moving sign bit from
    // int32 to dst type, then extend number from dst type to int32 (a necessary condition for an isa). All work in dst
    // register.
    ASSERT(dst.GetSize() >= WORD_SIZE);

    if (src.IsFloat() && dst.IsScalar()) {
        if (dstSigned) {
            GetMasm()->Fcvtzs(VixlReg(dst), VixlVReg(src));
        } else {
            GetMasm()->Fcvtzu(VixlReg(dst), VixlVReg(src));
        }
        return;
    }
    if (src.IsScalar() && dst.IsFloat()) {
        auto rzero = GetRegfile()->GetZeroReg().GetId();
        if (src.GetId() == rzero) {
            if (dst.GetSize() == WORD_SIZE) {
                GetMasm()->Fmov(VixlVReg(dst), 0.0F);
            } else {
                GetMasm()->Fmov(VixlVReg(dst), 0.0);
            }
        } else if (srcSigned) {
            GetMasm()->Scvtf(VixlVReg(dst), VixlReg(src));
        } else {
            GetMasm()->Ucvtf(VixlVReg(dst), VixlReg(src));
        }
        return;
    }
    if (src.IsFloat() && dst.IsFloat()) {
        if (src.GetSize() != dst.GetSize()) {
            GetMasm()->Fcvt(VixlVReg(dst), VixlVReg(src));
            return;
        }
        GetMasm()->Fmov(VixlVReg(dst), VixlVReg(src));
        return;
    }
    UNREACHABLE();
}

void Aarch64Encoder::EncodeCastFloatWithSmallDst(Reg dst, bool dstSigned, Reg src, bool srcSigned)
{
    // Dst bool type don't supported!

    if (src.IsFloat() && dst.IsScalar()) {
        if (dstSigned) {
            GetMasm()->Fcvtzs(VixlReg(dst), VixlVReg(src));
            if (dst.GetSize() < WORD_SIZE) {
                constexpr uint32_t TEST_BIT = (1U << (static_cast<uint32_t>(WORD_SIZE) - 1));
                ScopedTmpReg tmpReg1(this, dst.GetType());
                auto tmp1 = VixlReg(tmpReg1);
                ScopedTmpReg tmpReg2(this, dst.GetType());
                auto tmp2 = VixlReg(tmpReg2);

                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                int32_t setBit = (dst.GetSize() == BYTE_SIZE) ? (1UL << (BYTE_SIZE - 1)) : (1UL << (HALF_SIZE - 1));
                int32_t remBit = setBit - 1;
                GetMasm()->Ands(tmp1, VixlReg(dst), TEST_BIT);

                GetMasm()->Orr(tmp1, VixlReg(dst), setBit);
                GetMasm()->And(tmp2, VixlReg(dst), remBit);
                // Select result - if zero set - tmp2, else tmp1
                GetMasm()->Csel(VixlReg(dst), tmp2, tmp1, vixl::aarch64::eq);
                EncodeCastScalar(Reg(dst.GetId(), INT32_TYPE), dstSigned, dst, dstSigned);
            }
            return;
        }
        GetMasm()->Fcvtzu(VixlReg(dst), VixlVReg(src));
        if (dst.GetSize() < WORD_SIZE) {
            EncodeCastScalar(Reg(dst.GetId(), INT32_TYPE), dstSigned, dst, dstSigned);
        }
        return;
    }
    if (src.IsScalar() && dst.IsFloat()) {
        if (srcSigned) {
            GetMasm()->Scvtf(VixlVReg(dst), VixlReg(src));
        } else {
            GetMasm()->Ucvtf(VixlVReg(dst), VixlReg(src));
        }
        return;
    }
    if (src.IsFloat() && dst.IsFloat()) {
        if (src.GetSize() != dst.GetSize()) {
            GetMasm()->Fcvt(VixlVReg(dst), VixlVReg(src));
            return;
        }
        GetMasm()->Fmov(VixlVReg(dst), VixlVReg(src));
        return;
    }
    UNREACHABLE();
}

void Aarch64Encoder::EncodeCastSigned(Reg dst, Reg src)
{
    size_t srcSize = src.GetSize();
    size_t dstSize = dst.GetSize();
    auto srcR = Reg(src.GetId(), dst.GetType());
    // Else signed extend
    if (srcSize > dstSize) {
        srcSize = dstSize;
    }
    switch (srcSize) {
        case BYTE_SIZE:
            GetMasm()->Sxtb(VixlReg(dst), VixlReg(srcR));
            break;
        case HALF_SIZE:
            GetMasm()->Sxth(VixlReg(dst), VixlReg(srcR));
            break;
        case WORD_SIZE:
            GetMasm()->Sxtw(VixlReg(dst), VixlReg(srcR));
            break;
        case DOUBLE_WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(srcR));
            break;
        default:
            SetFalseResult();
            break;
    }
}

void Aarch64Encoder::EncodeCastUnsigned(Reg dst, Reg src)
{
    size_t srcSize = src.GetSize();
    size_t dstSize = dst.GetSize();
    auto srcR = Reg(src.GetId(), dst.GetType());
    if (srcSize > dstSize && dstSize < WORD_SIZE) {
        // We need to cut the number, if it is less, than 32-bit. It is by ISA agreement.
        int64_t cutValue = (1ULL << dstSize) - 1;
        GetMasm()->And(VixlReg(dst), VixlReg(src), VixlImm(cutValue));
        return;
    }
    // Else unsigned extend
    switch (srcSize) {
        case BYTE_SIZE:
            GetMasm()->Uxtb(VixlReg(dst), VixlReg(srcR));
            return;
        case HALF_SIZE:
            GetMasm()->Uxth(VixlReg(dst), VixlReg(srcR));
            return;
        case WORD_SIZE:
            GetMasm()->Uxtw(VixlReg(dst), VixlReg(srcR));
            return;
        case DOUBLE_WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(srcR));
            return;
        default:
            SetFalseResult();
            return;
    }
}

void Aarch64Encoder::EncodeCastScalar(Reg dst, bool dstSigned, Reg src, bool srcSigned)
{
    size_t srcSize = src.GetSize();
    size_t dstSize = dst.GetSize();
    // In our ISA minimal type is 32-bit, so type less then 32-bit
    // we should extend to 32-bit. So we can have 2 cast
    // (For examble, i8->u16 will work as i8->u16 and u16->u32)
    if (dstSize < WORD_SIZE) {
        if (srcSize > dstSize) {
            if (dstSigned) {
                EncodeCastSigned(dst, src);
            } else {
                EncodeCastUnsigned(dst, src);
            }
            return;
        }
        if (srcSize == dstSize) {
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (!(srcSigned || dstSigned) || (srcSigned && dstSigned)) {
                return;
            }
            if (dstSigned) {
                EncodeCastSigned(Reg(dst.GetId(), INT32_TYPE), dst);
            } else {
                EncodeCastUnsigned(Reg(dst.GetId(), INT32_TYPE), dst);
            }
            return;
        }
        if (srcSigned) {
            EncodeCastSigned(dst, src);
            if (!dstSigned) {
                EncodeCastUnsigned(Reg(dst.GetId(), INT32_TYPE), dst);
            }
        } else {
            EncodeCastUnsigned(dst, src);
            if (dstSigned) {
                EncodeCastSigned(Reg(dst.GetId(), INT32_TYPE), dst);
            }
        }
    } else {
        if (srcSize == dstSize) {
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            return;
        }
        if (srcSigned) {
            EncodeCastSigned(dst, src);
        } else {
            EncodeCastUnsigned(dst, src);
        }
    }
}

void Aarch64Encoder::EncodeFastPathDynamicCast(Reg dst, Reg src, LabelHolder::LabelId slow)
{
    ASSERT(src.IsFloat() && dst.IsScalar());

    CHECK_EQ(src.GetSize(), BITS_PER_UINT64);
    CHECK_EQ(dst.GetSize(), BITS_PER_UINT32);

    // We use slow path, because in general JS double -> int32 cast is complex and we check only few common cases here
    // and move other checks in slow path. In case CPU supports special JS double -> int32 instruction we do not need
    // slow path.
    if (!IsLabelValid(slow)) {
        // use special JS aarch64 instruction
#ifndef NDEBUG
        vixl::CPUFeaturesScope scope(GetMasm(), vixl::CPUFeatures::kFP, vixl::CPUFeatures::kJSCVT);
#endif
        GetMasm()->Fjcvtzs(VixlReg(dst), VixlVReg(src));
        return;
    }

    // infinite and big numbers will overflow here to INT64_MIN or INT64_MAX, but NaN casts to 0
    GetMasm()->Fcvtzs(VixlReg(dst, DOUBLE_WORD_SIZE), VixlVReg(src));
    // check INT64_MIN
    GetMasm()->Cmp(VixlReg(dst, DOUBLE_WORD_SIZE), VixlImm(1));
    // check INT64_MAX
    GetMasm()->Ccmp(VixlReg(dst, DOUBLE_WORD_SIZE), VixlImm(-1), vixl::aarch64::StatusFlags::VFlag,
                    vixl::aarch64::Condition::vc);
    auto slowLabel {static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(slow)};
    // jump to slow path in case of overflow
    GetMasm()->B(slowLabel, vixl::aarch64::Condition::vs);
}

void Aarch64Encoder::EncodeJsDoubleToCharCast(Reg dst, Reg src)
{
    ASSERT(src.IsFloat() && dst.IsScalar());

    CHECK_EQ(src.GetSize(), BITS_PER_UINT64);
    CHECK_EQ(dst.GetSize(), BITS_PER_UINT32);

    // use special JS aarch64 instruction
#ifndef NDEBUG
    vixl::CPUFeaturesScope scope(GetMasm(), vixl::CPUFeatures::kFP, vixl::CPUFeatures::kJSCVT);
#endif
    GetMasm()->Fjcvtzs(VixlReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeJsDoubleToCharCast(Reg dst, Reg src, Reg tmp, uint32_t failureResult)
{
    ASSERT(src.IsFloat() && dst.IsScalar());

    CHECK_EQ(src.GetSize(), BITS_PER_UINT64);
    CHECK_EQ(dst.GetSize(), BITS_PER_UINT32);

    // infinite and big numbers will overflow here to INT64_MIN or INT64_MAX, but NaN casts to 0
    GetMasm()->Fcvtzs(VixlReg(dst, DOUBLE_WORD_SIZE), VixlVReg(src));
    // check INT64_MIN
    GetMasm()->Cmp(VixlReg(dst, DOUBLE_WORD_SIZE), VixlImm(1));
    // check INT64_MAX
    GetMasm()->Ccmp(VixlReg(dst, DOUBLE_WORD_SIZE), VixlImm(-1), vixl::aarch64::StatusFlags::VFlag,
                    vixl::aarch64::Condition::vc);
    // 'And' with 0xffff
    constexpr uint32_t UTF16_CHAR_MASK = 0xffff;
    GetMasm()->And(VixlReg(dst), VixlReg(dst), VixlImm(UTF16_CHAR_MASK));
    // 'And' and 'Mov' change no flags so we may conditionally move failure result in case of overflow at old checking
    // for INT64_MAX
    GetMasm()->mov(VixlReg(tmp), failureResult);
    GetMasm()->csel(VixlReg(dst), VixlReg(tmp), VixlReg(dst), vixl::aarch64::Condition::vs);
}

void Aarch64Encoder::EncodeCast(Reg dst, bool dstSigned, Reg src, bool srcSigned)
{
    if (src.IsFloat() || dst.IsFloat()) {
        EncodeCastFloat(dst, dstSigned, src, srcSigned);
        return;
    }

    ASSERT(src.IsScalar() && dst.IsScalar());
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (src.GetId() == rzero) {
        ASSERT(dst.GetId() != rzero);
        EncodeMov(dst, Imm(0));
        return;
    }
    // Scalar part
    EncodeCastScalar(dst, dstSigned, src, srcSigned);
}

void Aarch64Encoder::EncodeCastToBool(Reg dst, Reg src)
{
    // In ISA says that we only support casts:
    // i32tou1, i64tou1, u32tou1, u64tou1
    ASSERT(src.IsScalar());
    ASSERT(dst.IsScalar());

    GetMasm()->Cmp(VixlReg(src), VixlImm(0));
    // In our ISA minimal type is 32-bit, so bool in 32bit
    GetMasm()->Cset(VixlReg(Reg(dst.GetId(), INT32_TYPE)), vixl::aarch64::Condition::ne);
}

void Aarch64Encoder::EncodeAdd(Reg dst, Reg src0, Shift src1)
{
    if (dst.IsFloat()) {
        UNREACHABLE();
    }
    ASSERT(src0.GetSize() <= dst.GetSize());
    if (src0.GetSize() < dst.GetSize()) {
        auto src0Reg = Reg(src0.GetId(), dst.GetType());
        auto src1Reg = Reg(src1.GetBase().GetId(), dst.GetType());
        GetMasm()->Add(VixlReg(dst), VixlReg(src0Reg), VixlShift(Shift(src1Reg, src1.GetType(), src1.GetScale())));
        return;
    }
    GetMasm()->Add(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeAdd(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fadd(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    /* if any of the operands has 64-bits size,
     * forcibly do the 64-bits wide operation */
    if ((src0.GetSize() | src1.GetSize() | dst.GetSize()) >= DOUBLE_WORD_SIZE) {
        GetMasm()->Add(VixlReg(dst).X(), VixlReg(src0).X(), VixlReg(src1).X());
    } else {
        /* Otherwise do 32-bits operation as any lesser
         * sizes have to be upcasted to 32-bits anyway */
        GetMasm()->Add(VixlReg(dst).W(), VixlReg(src0).W(), VixlReg(src1).W());
    }
}

void Aarch64Encoder::EncodeSub(Reg dst, Reg src0, Shift src1)
{
    ASSERT(dst.IsScalar());
    GetMasm()->Sub(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeSub(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fsub(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    /* if any of the operands has 64-bits size,
     * forcibly do the 64-bits wide operation */
    if ((src0.GetSize() | src1.GetSize() | dst.GetSize()) >= DOUBLE_WORD_SIZE) {
        GetMasm()->Sub(VixlReg(dst).X(), VixlReg(src0).X(), VixlReg(src1).X());
    } else {
        /* Otherwise do 32-bits operation as any lesser
         * sizes have to be upcasted to 32-bits anyway */
        GetMasm()->Sub(VixlReg(dst).W(), VixlReg(src0).W(), VixlReg(src1).W());
    }
}

void Aarch64Encoder::EncodeMul(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fmul(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (src0.GetId() == rzero || src1.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }
    GetMasm()->Mul(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAddOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(!dst.IsFloat() && !src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::VS || cc == Condition::VC);
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Adds(VixlReg(dst).X(), VixlReg(src0).X(), VixlReg(src1).X());
    } else {
        /* Otherwise do 32-bits operation as any lesser
         * sizes have to be upcasted to 32-bits anyway */
        GetMasm()->Adds(VixlReg(dst).W(), VixlReg(src0).W(), VixlReg(src1).W());
    }
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeSubOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(!dst.IsFloat() && !src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::VS || cc == Condition::VC);
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Subs(VixlReg(dst).X(), VixlReg(src0).X(), VixlReg(src1).X());
    } else {
        /* Otherwise do 32-bits operation as any lesser
         * sizes have to be upcasted to 32-bits anyway */
        GetMasm()->Subs(VixlReg(dst).W(), VixlReg(src0).W(), VixlReg(src1).W());
    }
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeNegOverflowAndZero(compiler::LabelHolder::LabelId id, Reg dst, Reg src)
{
    ASSERT(!dst.IsFloat() && !src.IsFloat());
    // NOLINTNEXTLINE(readability-magic-numbers)
    EncodeJumpTest(id, src, Imm(0x7fffffff), Condition::TST_EQ);
    GetMasm()->Neg(VixlReg(dst).W(), VixlReg(src).W());
}

void Aarch64Encoder::EncodeDiv(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fdiv(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (src1.GetId() == rzero || src0.GetId() == rzero) {
        ScopedTmpReg tmpReg(this, src1.GetType());
        EncodeMov(tmpReg, Imm(0));
        // Denominator is zero-reg
        if (src1.GetId() == rzero) {
            // Encode Abort
            GetMasm()->Udiv(VixlReg(dst), VixlReg(tmpReg), VixlReg(tmpReg));
            return;
        }

        // But src1 still may be zero
        if (src1.GetId() != src0.GetId()) {
            if (dstSigned) {
                GetMasm()->Sdiv(VixlReg(dst), VixlReg(tmpReg), VixlReg(src1));
            } else {
                GetMasm()->Udiv(VixlReg(dst), VixlReg(tmpReg), VixlReg(src1));
            }
            return;
        }
        UNREACHABLE();
    }
    if (dstSigned) {
        GetMasm()->Sdiv(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    } else {
        GetMasm()->Udiv(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    }
}

void Aarch64Encoder::EncodeMod(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        auto rzero = GetRegfile()->GetZeroReg().GetId();
        if (src1.GetId() == rzero || src0.GetId() == rzero) {
            ScopedTmpReg tmpReg(this, src1.GetType());
            EncodeMov(tmpReg, Imm(0));
            // Denominator is zero-reg
            if (src1.GetId() == rzero) {
                // Encode Abort
                GetMasm()->Udiv(VixlReg(dst), VixlReg(tmpReg), VixlReg(tmpReg));
                return;
            }

            if (src1.GetId() == src0.GetId()) {
                SetFalseResult();
                return;
            }
            // But src1 still may be zero
            ScopedTmpRegU64 tmpRegUd(this);
            if (dst.GetSize() < DOUBLE_WORD_SIZE) {
                tmpRegUd.ChangeType(INT32_TYPE);
            }
            auto tmp = VixlReg(tmpRegUd);
            if (!dstSigned) {
                GetMasm()->Udiv(tmp, VixlReg(tmpReg), VixlReg(src1));
                GetMasm()->Msub(VixlReg(dst), tmp, VixlReg(src1), VixlReg(tmpReg));
                return;
            }
            GetMasm()->Sdiv(tmp, VixlReg(tmpReg), VixlReg(src1));
            GetMasm()->Msub(VixlReg(dst), tmp, VixlReg(src1), VixlReg(tmpReg));
            return;
        }

        ScopedTmpRegU64 tmpReg(this);
        if (dst.GetSize() < DOUBLE_WORD_SIZE) {
            tmpReg.ChangeType(INT32_TYPE);
        }
        auto tmp = VixlReg(tmpReg);

        if (!dstSigned) {
            GetMasm()->Udiv(tmp, VixlReg(src0), VixlReg(src1));
            GetMasm()->Msub(VixlReg(dst), tmp, VixlReg(src1), VixlReg(src0));
            return;
        }
        GetMasm()->Sdiv(tmp, VixlReg(src0), VixlReg(src1));
        GetMasm()->Msub(VixlReg(dst), tmp, VixlReg(src1), VixlReg(src0));
        return;
    }

    EncodeFMod(dst, src0, src1);
}

void Aarch64Encoder::EncodeFMod(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsFloat());

    if (dst.GetType() == FLOAT32_TYPE) {
        using Fp = float (*)(float, float);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<Fp>(fmodf)));
    } else {
        using Fp = double (*)(double, double);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<Fp>(fmod)));
    }
}

void Aarch64Encoder::EncodeSignedDiv(Reg dst, Reg src0, Imm imm)
{
    int64_t divisor = imm.GetAsInt();

    FastConstSignedDivisor fastDivisor(divisor, dst.GetSize());
    int64_t magic = fastDivisor.GetMagic();

    ScopedTmpReg tmp(this, dst.GetType());
    Reg tmp64 = tmp.GetReg().As(INT64_TYPE);
    EncodeMov(tmp, Imm(magic));

    int64_t extraShift = 0;
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Smulh(VixlReg(tmp), VixlReg(src0), VixlReg(tmp));
    } else {
        GetMasm()->Smull(VixlReg(tmp64), VixlReg(src0), VixlReg(tmp));
        extraShift = WORD_SIZE;
    }

    bool useSignFlag = false;
    if (divisor > 0 && magic < 0) {
        GetMasm()->Adds(VixlReg(tmp64), VixlReg(tmp64), VixlShift(Shift(src0.As(INT64_TYPE), extraShift)));
        useSignFlag = true;
    } else if (divisor < 0 && magic > 0) {
        GetMasm()->Subs(VixlReg(tmp64), VixlReg(tmp64), VixlShift(Shift(src0.As(INT64_TYPE), extraShift)));
        useSignFlag = true;
    }

    int64_t shift = fastDivisor.GetShift();
    EncodeAShr(dst.As(INT64_TYPE), tmp64, Imm(shift + extraShift));

    // result = (result < 0 ? result + 1 : result)
    if (useSignFlag) {
        GetMasm()->Cinc(VixlReg(dst), VixlReg(dst), vixl::aarch64::Condition::mi);
    } else {
        GetMasm()->Add(VixlReg(dst), VixlReg(dst), VixlShift(Shift(dst, ShiftType::LSR, dst.GetSize() - 1U)));
    }
}

void Aarch64Encoder::EncodeUnsignedDiv(Reg dst, Reg src0, Imm imm)
{
    auto divisor = bit_cast<uint64_t>(imm.GetAsInt());

    FastConstUnsignedDivisor fastDivisor(divisor, dst.GetSize());
    uint64_t magic = fastDivisor.GetMagic();

    ScopedTmpReg tmp(this, dst.GetType());
    Reg tmp64 = tmp.GetReg().As(INT64_TYPE);
    EncodeMov(tmp, Imm(magic));

    uint64_t extraShift = 0;
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Umulh(VixlReg(tmp), VixlReg(src0), VixlReg(tmp));
    } else {
        GetMasm()->Umull(VixlReg(tmp64), VixlReg(src0), VixlReg(tmp));
        extraShift = WORD_SIZE;
    }

    uint64_t shift = fastDivisor.GetShift();
    if (!fastDivisor.GetAdd()) {
        EncodeShr(dst.As(INT64_TYPE), tmp64, Imm(shift + extraShift));
    } else {
        ASSERT(shift >= 1U);
        if (extraShift > 0U) {
            EncodeShr(tmp64, tmp64, Imm(extraShift));
        }
        EncodeSub(dst, src0, tmp);
        GetMasm()->Add(VixlReg(dst), VixlReg(tmp), VixlShift(Shift(dst, ShiftType::LSR, 1U)));
        EncodeShr(dst, dst, Imm(shift - 1U));
    }
}

void Aarch64Encoder::EncodeDiv(Reg dst, Reg src0, Imm imm, bool isSigned)
{
    ASSERT(dst.IsScalar() && dst.GetSize() >= WORD_SIZE);
    ASSERT(CanOptimizeImmDivMod(bit_cast<uint64_t>(imm.GetAsInt()), isSigned));
    if (isSigned) {
        EncodeSignedDiv(dst, src0, imm);
    } else {
        EncodeUnsignedDiv(dst, src0, imm);
    }
}

void Aarch64Encoder::EncodeMod(Reg dst, Reg src0, Imm imm, bool isSigned)
{
    ASSERT(dst.IsScalar() && dst.GetSize() >= WORD_SIZE);
    ASSERT(CanOptimizeImmDivMod(bit_cast<uint64_t>(imm.GetAsInt()), isSigned));
    // dst = src0 - imm * (src0 / imm)
    ScopedTmpReg tmp(this, dst.GetType());
    EncodeDiv(tmp, src0, imm, isSigned);

    ScopedTmpReg immReg(this, dst.GetType());
    EncodeMov(immReg, imm);

    GetMasm()->Msub(VixlReg(dst), VixlReg(immReg), VixlReg(tmp), VixlReg(src0));
}

void Aarch64Encoder::EncodeMin(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fmin(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }
    if (dstSigned) {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), vixl::aarch64::Condition::lt);
        return;
    }
    GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), vixl::aarch64::Condition::ls);
}

void Aarch64Encoder::EncodeMax(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fmax(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }
    if (dstSigned) {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), vixl::aarch64::Condition::gt);
        return;
    }
    GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), vixl::aarch64::Condition::hi);
}

void Aarch64Encoder::EncodeShl(Reg dst, Reg src0, Reg src1)
{
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src0.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }
    if (src1.GetId() == rzero) {
        EncodeMov(dst, src0);
    }
    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }
    GetMasm()->Lsl(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeShr(Reg dst, Reg src0, Reg src1)
{
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src0.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }
    if (src1.GetId() == rzero) {
        EncodeMov(dst, src0);
    }

    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }

    GetMasm()->Lsr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAShr(Reg dst, Reg src0, Reg src1)
{
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src0.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }
    if (src1.GetId() == rzero) {
        EncodeMov(dst, src0);
    }

    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }
    GetMasm()->Asr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAnd(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->And(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAnd(Reg dst, Reg src0, Shift src1)
{
    GetMasm()->And(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeOr(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->Orr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeOr(Reg dst, Reg src0, Shift src1)
{
    GetMasm()->Orr(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeXor(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->Eor(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeXor(Reg dst, Reg src0, Shift src1)
{
    GetMasm()->Eor(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeAdd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "UNIMPLEMENTED");
    ASSERT(dst.GetSize() >= src.GetSize());
    if (dst.GetSize() != src.GetSize()) {
        auto srcReg = Reg(src.GetId(), dst.GetType());
        GetMasm()->Add(VixlReg(dst), VixlReg(srcReg), VixlImm(imm));
        return;
    }
    GetMasm()->Add(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeSub(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "UNIMPLEMENTED");
    GetMasm()->Sub(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeShl(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }

    GetMasm()->Lsl(VixlReg(dst), VixlReg(src), imm.GetAsInt());
}

void Aarch64Encoder::EncodeShr(Reg dst, Reg src, Imm imm)
{
    int64_t immValue = static_cast<uint64_t>(imm.GetAsInt()) & (dst.GetSize() - 1);

    ASSERT(dst.IsScalar() && "Invalid operand type");
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }

    GetMasm()->Lsr(VixlReg(dst), VixlReg(src), immValue);
}

void Aarch64Encoder::EncodeAShr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Asr(VixlReg(dst), VixlReg(src), imm.GetAsInt());
}

void Aarch64Encoder::EncodeAnd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->And(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeOr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Orr(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeXor(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Eor(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeMov(Reg dst, Imm src)
{
    if (dst.IsFloat()) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Fmov(VixlVReg(dst), src.GetAsFloat());
        } else {
            GetMasm()->Fmov(VixlVReg(dst), src.GetAsDouble());
        }
        return;
    }
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(VixlReg(dst), VixlImm(src));
    } else {
        GetMasm()->Mov(VixlReg(dst), VixlImm(static_cast<int32_t>(src.GetAsInt())));
    }
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
static void EncodeLdrImpl(Aarch64Encoder *self, Reg dst, bool dstSigned, MemRef mem, vixl::aarch64::AddrMode mode)
{
    auto rzero = self->GetRegfile()->GetZeroReg().GetId();
    auto convertedMem = ConvertMem(mem, mode);
    if (!convertedMem.IsValid() || (dst.GetId() == rzero && dst.IsScalar())) {
        // Check: invalid conversion is not due to unsupported mode.
        ASSERT(mode == vixl::aarch64::Offset);

        // Try move zero reg to dst (for do not create temp-reg)
        // Check: dst not vector, dst not index, dst not rzero
        [[maybe_unused]] auto baseReg = mem.GetBase();
        auto indexReg = mem.GetIndex();

        // Invalid == base is rzero or invalid
        ASSERT(baseReg.GetId() == rzero || !baseReg.IsValid());
        // checks for use dst-register
        if (dst.IsScalar() && dst.IsValid() &&    // not float
            (indexReg.GetId() != dst.GetId()) &&  // not index
            (dst.GetId() != rzero)) {             // not rzero
            // May use dst like rzero
            self->EncodeMov(dst, Imm(0));

            auto fixMem = MemRef(dst, indexReg, mem.GetScale(), mem.GetDisp());
            ASSERT(ConvertMem(fixMem).IsValid());
            self->EncodeLdr(dst, dstSigned, fixMem);
        } else {
            // Use tmp-reg
            ScopedTmpReg tmpReg(self);
            self->EncodeMov(tmpReg, Imm(0));

            auto fixMem = MemRef(tmpReg, indexReg, mem.GetScale(), mem.GetDisp());
            ASSERT(ConvertMem(fixMem).IsValid());
            // Used for zero-dst
            self->EncodeLdr(tmpReg, dstSigned, fixMem);
        }
        return;
    }
    ASSERT(convertedMem.IsValid());
    if (dst.IsFloat()) {
        self->GetMasm()->Ldr(VixlVReg(dst), convertedMem);
        return;
    }
    if (dstSigned) {
        if (dst.GetSize() == BYTE_SIZE) {
            self->GetMasm()->Ldrsb(VixlReg(dst, DOUBLE_WORD_SIZE), convertedMem);
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            self->GetMasm()->Ldrsh(VixlReg(dst), convertedMem);
            return;
        }
    } else {
        if (dst.GetSize() == BYTE_SIZE) {
            self->GetMasm()->Ldrb(VixlReg(dst, WORD_SIZE), convertedMem);
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            self->GetMasm()->Ldrh(VixlReg(dst), convertedMem);
            return;
        }
    }
    self->GetMasm()->Ldr(VixlReg(dst), convertedMem);
}

void Aarch64Encoder::EncodeLdr(Reg dst, bool dstSigned, MemRef mem)
{
    EncodeLdrImpl(this, dst, dstSigned, mem, vixl::aarch64::Offset);
}

void Aarch64Encoder::EncodeLdrPreIndex(Reg dst, bool dstSigned, Reg srcBase, Imm offset)
{
    EncodeLdrImpl(this, dst, dstSigned, MemRef {srcBase, offset.GetAsInt()}, vixl::aarch64::PreIndex);
}

void Aarch64Encoder::EncodeLdrPostIndex(Reg dst, bool dstSigned, Reg srcBase, Imm offset)
{
    EncodeLdrImpl(this, dst, dstSigned, MemRef {srcBase, offset.GetAsInt()}, vixl::aarch64::PostIndex);
}

void Aarch64Encoder::EncodeLdrAcquireInvalid(Reg dst, bool dstSigned, MemRef mem)
{
    // Try move zero reg to dst (for do not create temp-reg)
    // Check: dst not vector, dst not index, dst not rzero
    [[maybe_unused]] auto baseReg = mem.GetBase();
    auto rzero = GetRegfile()->GetZeroReg().GetId();

    auto indexReg = mem.GetIndex();

    // Invalid == base is rzero or invalid
    ASSERT(baseReg.GetId() == rzero || !baseReg.IsValid());
    // checks for use dst-register
    if (dst.IsScalar() && dst.IsValid() &&    // not float
        (indexReg.GetId() != dst.GetId()) &&  // not index
        (dst.GetId() != rzero)) {             // not rzero
        // May use dst like rzero
        EncodeMov(dst, Imm(0));

        auto fixMem = MemRef(dst, indexReg, mem.GetScale(), mem.GetDisp());
        ASSERT(ConvertMem(fixMem).IsValid());
        EncodeLdrAcquire(dst, dstSigned, fixMem);
    } else {
        // Use tmp-reg
        ScopedTmpReg tmpReg(this);
        EncodeMov(tmpReg, Imm(0));

        auto fixMem = MemRef(tmpReg, indexReg, mem.GetScale(), mem.GetDisp());
        ASSERT(ConvertMem(fixMem).IsValid());
        // Used for zero-dst
        EncodeLdrAcquire(tmpReg, dstSigned, fixMem);
    }
}

void Aarch64Encoder::EncodeLdrAcquireScalar(Reg dst, bool dstSigned, MemRef mem)
{
#ifndef NDEBUG
    CheckAlignment(mem, dst.GetSize());
#endif  // NDEBUG
    if (dstSigned) {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldarb(VixlReg(dst), ConvertMem(mem));
            GetMasm()->Sxtb(VixlReg(dst), VixlReg(dst));
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldarh(VixlReg(dst), ConvertMem(mem));
            GetMasm()->Sxth(VixlReg(dst), VixlReg(dst));
            return;
        }
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Ldar(VixlReg(dst), ConvertMem(mem));
            GetMasm()->Sxtw(VixlReg(dst), VixlReg(dst));
            return;
        }
    } else {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldarb(VixlReg(dst, WORD_SIZE), ConvertMem(mem));
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldarh(VixlReg(dst), ConvertMem(mem));
            return;
        }
    }
    GetMasm()->Ldar(VixlReg(dst), ConvertMem(mem));
}

void Aarch64Encoder::CheckAlignment(MemRef mem, size_t size)
{
    ASSERT(size == WORD_SIZE || size == BYTE_SIZE || size == HALF_SIZE || size == DOUBLE_WORD_SIZE);
    if (size == BYTE_SIZE) {
        return;
    }
    size_t alignmentMask = (size >> 3U) - 1;
    ASSERT(!mem.HasIndex() && !mem.HasScale());
    if (mem.HasDisp()) {
        // We need additional tmp register for check base + offset.
        // The case when separately the base and the offset are not aligned, but in sum there are aligned very rarely.
        // Therefore, the alignment check for base and offset takes place separately
        [[maybe_unused]] auto offset = static_cast<size_t>(mem.GetDisp());
        ASSERT((offset & alignmentMask) == 0);
    }
    auto baseReg = mem.GetBase();
    auto end = CreateLabel();
    EncodeJumpTest(end, baseReg, Imm(alignmentMask), Condition::TST_EQ);
    EncodeAbort();
    BindLabel(end);
}

void Aarch64Encoder::EncodeLdrAcquire(Reg dst, bool dstSigned, MemRef mem)
{
    if (mem.HasIndex()) {
        ScopedTmpRegU64 tmpReg(this);
        if (mem.HasScale()) {
            EncodeAdd(tmpReg, mem.GetBase(), Shift(mem.GetIndex(), mem.GetScale()));
        } else {
            EncodeAdd(tmpReg, mem.GetBase(), mem.GetIndex());
        }
        mem = MemRef(tmpReg, mem.GetDisp());
    }

    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (!ConvertMem(mem).IsValid() || (dst.GetId() == rzero && dst.IsScalar())) {
        EncodeLdrAcquireInvalid(dst, dstSigned, mem);
        return;
    }

    ASSERT(!mem.HasIndex() && !mem.HasScale());
    if (dst.IsFloat()) {
        ScopedTmpRegU64 tmpReg(this);
        auto memLdar = mem;
        if (mem.HasDisp()) {
            if (vixl::aarch64::Assembler::IsImmAddSub(mem.GetDisp())) {
                EncodeAdd(tmpReg, mem.GetBase(), Imm(mem.GetDisp()));
            } else {
                EncodeMov(tmpReg, Imm(mem.GetDisp()));
                EncodeAdd(tmpReg, mem.GetBase(), tmpReg);
            }
            memLdar = MemRef(tmpReg);
        }
#ifndef NDEBUG
        CheckAlignment(memLdar, dst.GetSize());
#endif  // NDEBUG
        auto tmp = VixlReg(tmpReg, dst.GetSize());
        GetMasm()->Ldar(tmp, ConvertMem(memLdar));
        GetMasm()->Fmov(VixlVReg(dst), tmp);
        return;
    }

    if (!mem.HasDisp()) {
        EncodeLdrAcquireScalar(dst, dstSigned, mem);
        return;
    }

    Reg dst64(dst.GetId(), INT64_TYPE);
    if (vixl::aarch64::Assembler::IsImmAddSub(mem.GetDisp())) {
        EncodeAdd(dst64, mem.GetBase(), Imm(mem.GetDisp()));
    } else {
        EncodeMov(dst64, Imm(mem.GetDisp()));
        EncodeAdd(dst64, mem.GetBase(), dst64);
    }
    EncodeLdrAcquireScalar(dst, dstSigned, MemRef(dst64));
}

static void EncodeStrImpl(Aarch64Encoder *self, Reg src, MemRef mem, vixl::aarch64::AddrMode mode)
{
    auto convertedMem = ConvertMem(mem, mode);
    if (!convertedMem.IsValid()) {
        // Check: invalid conversion is not due to unsupported mode.
        ASSERT(mode == vixl::aarch64::Offset);

        auto indexReg = mem.GetIndex();
        auto rzero = self->GetRegfile()->GetZeroReg().GetId();
        // Invalid == base is rzero or invalid
        ASSERT(mem.GetBase().GetId() == rzero || !mem.GetBase().IsValid());
        // Use tmp-reg
        ScopedTmpReg tmpReg(self);
        self->EncodeMov(tmpReg, Imm(0));

        auto fixMem = MemRef(tmpReg, indexReg, mem.GetScale(), mem.GetDisp());
        ASSERT(ConvertMem(fixMem).IsValid());
        if (src.GetId() != rzero) {
            self->EncodeStr(src, fixMem);
        } else {
            self->EncodeStr(tmpReg, fixMem);
        }
        return;
    }
    ASSERT(convertedMem.IsValid());
    if (src.IsFloat()) {
        self->GetMasm()->Str(VixlVReg(src), convertedMem);
        return;
    }
    if (src.GetSize() == BYTE_SIZE) {
        self->GetMasm()->Strb(VixlReg(src), convertedMem);
        return;
    }
    if (src.GetSize() == HALF_SIZE) {
        self->GetMasm()->Strh(VixlReg(src), convertedMem);
        return;
    }
    self->GetMasm()->Str(VixlReg(src), convertedMem);
}

void Aarch64Encoder::EncodeStr(Reg src, MemRef mem)
{
    EncodeStrImpl(this, src, mem, vixl::aarch64::Offset);
}

void Aarch64Encoder::EncodeStrPreIndex(Reg src, Reg dstBase, Imm offset)
{
    EncodeStrImpl(this, src, MemRef {dstBase, offset.GetAsInt()}, vixl::aarch64::PreIndex);
}

void Aarch64Encoder::EncodeStrPostIndex(Reg src, Reg dstBase, Imm offset)
{
    EncodeStrImpl(this, src, MemRef {dstBase, offset.GetAsInt()}, vixl::aarch64::PostIndex);
}

void Aarch64Encoder::EncodeStrRelease(Reg src, MemRef mem)
{
    ScopedTmpRegLazy base(this);
    MemRef fixedMem;
    bool memWasFixed = false;
    if (mem.HasDisp()) {
        if (vixl::aarch64::Assembler::IsImmAddSub(mem.GetDisp())) {
            base.AcquireIfInvalid();
            EncodeAdd(base, mem.GetBase(), Imm(mem.GetDisp()));
        } else {
            base.AcquireIfInvalid();
            EncodeMov(base, Imm(mem.GetDisp()));
            EncodeAdd(base, mem.GetBase(), base);
        }
        memWasFixed = true;
    }
    if (mem.HasIndex()) {
        base.AcquireIfInvalid();
        if (mem.HasScale()) {
            EncodeAdd(base, memWasFixed ? base : mem.GetBase(), Shift(mem.GetIndex(), mem.GetScale()));
        } else {
            EncodeAdd(base, memWasFixed ? base : mem.GetBase(), mem.GetIndex());
        }
        memWasFixed = true;
    }

    if (memWasFixed) {
        fixedMem = MemRef(base);
    } else {
        fixedMem = mem;
    }

#ifndef NDEBUG
    CheckAlignment(fixedMem, src.GetSize());
#endif  // NDEBUG
    if (src.IsFloat()) {
        ScopedTmpRegU64 tmpReg(this);
        auto tmp = VixlReg(tmpReg, src.GetSize());
        GetMasm()->Fmov(tmp, VixlVReg(src));
        GetMasm()->Stlr(tmp, ConvertMem(fixedMem));
        return;
    }
    if (src.GetSize() == BYTE_SIZE) {
        GetMasm()->Stlrb(VixlReg(src), ConvertMem(fixedMem));
        return;
    }
    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Stlrh(VixlReg(src), ConvertMem(fixedMem));
        return;
    }
    GetMasm()->Stlr(VixlReg(src), ConvertMem(fixedMem));
}

void Aarch64Encoder::EncodeLdrExclusive(Reg dst, Reg addr, bool acquire)
{
    ASSERT(dst.IsScalar());
    auto dstReg = VixlReg(dst);
    auto memCvt = ConvertMem(MemRef(addr));
#ifndef NDEBUG
    CheckAlignment(MemRef(addr), dst.GetSize());
#endif  // NDEBUG
    if (dst.GetSize() == BYTE_SIZE) {
        if (acquire) {
            GetMasm()->Ldaxrb(dstReg, memCvt);
            return;
        }
        GetMasm()->Ldxrb(dstReg, memCvt);
        return;
    }
    if (dst.GetSize() == HALF_SIZE) {
        if (acquire) {
            GetMasm()->Ldaxrh(dstReg, memCvt);
            return;
        }
        GetMasm()->Ldxrh(dstReg, memCvt);
        return;
    }
    if (acquire) {
        GetMasm()->Ldaxr(dstReg, memCvt);
        return;
    }
    GetMasm()->Ldxr(dstReg, memCvt);
}

void Aarch64Encoder::EncodeStrExclusive(Reg dst, Reg src, Reg addr, bool release)
{
    ASSERT(dst.IsScalar() && src.IsScalar());

    bool copyDst = dst.GetId() == src.GetId() || dst.GetId() == addr.GetId();
    ScopedTmpReg tmp(this);
    auto srcReg = VixlReg(src);
    auto memCvt = ConvertMem(MemRef(addr));
    auto dstReg = copyDst ? VixlReg(tmp) : VixlReg(dst);
#ifndef NDEBUG
    CheckAlignment(MemRef(addr), src.GetSize());
#endif  // NDEBUG

    if (src.GetSize() == BYTE_SIZE) {
        if (release) {
            GetMasm()->Stlxrb(dstReg, srcReg, memCvt);
        } else {
            GetMasm()->Stxrb(dstReg, srcReg, memCvt);
        }
    } else if (src.GetSize() == HALF_SIZE) {
        if (release) {
            GetMasm()->Stlxrh(dstReg, srcReg, memCvt);
        } else {
            GetMasm()->Stxrh(dstReg, srcReg, memCvt);
        }
    } else {
        if (release) {
            GetMasm()->Stlxr(dstReg, srcReg, memCvt);
        } else {
            GetMasm()->Stxr(dstReg, srcReg, memCvt);
        }
    }
    if (copyDst) {
        EncodeMov(dst, tmp);
    }
}

void Aarch64Encoder::EncodeStrz(Reg src, MemRef mem)
{
    if (!ConvertMem(mem).IsValid()) {
        EncodeStr(src, mem);
        return;
    }
    ASSERT(ConvertMem(mem).IsValid());
    // Upper half of registers must be zeroed by-default
    if (src.IsFloat()) {
        EncodeStr(src.As(FLOAT64_TYPE), mem);
        return;
    }
    if (src.GetSize() < WORD_SIZE) {
        EncodeCast(src, false, src.As(INT64_TYPE), false);
    }
    GetMasm()->Str(VixlReg(src.As(INT64_TYPE)), ConvertMem(mem));
}

void Aarch64Encoder::EncodeSti(int64_t src, uint8_t srcSizeBytes, MemRef mem)
{
    if (mem.IsValid() && mem.IsOffsetMem() && src == 0 && srcSizeBytes == 1) {
        auto rzero = GetRegfile()->GetZeroReg();
        GetMasm()->Strb(VixlReg(rzero), ConvertMem(mem));
        return;
    }
    if (!ConvertMem(mem).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        EncodeStr(rzero, mem);
        return;
    }

    ScopedTmpRegU64 tmpReg(this);
    auto tmp = VixlReg(tmpReg);
    GetMasm()->Mov(tmp, VixlImm(src));
    if (srcSizeBytes == 1U) {
        GetMasm()->Strb(tmp, ConvertMem(mem));
        return;
    }
    if (srcSizeBytes == HALF_WORD_SIZE_BYTES) {
        GetMasm()->Strh(tmp, ConvertMem(mem));
        return;
    }
    ASSERT((srcSizeBytes == WORD_SIZE_BYTES) || (srcSizeBytes == DOUBLE_WORD_SIZE_BYTES));
    GetMasm()->Str(tmp, ConvertMem(mem));
}

void Aarch64Encoder::EncodeSti(float src, MemRef mem)
{
    if (!ConvertMem(mem).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        EncodeStr(rzero, mem);
        return;
    }
    ScopedTmpRegF32 tmpReg(this);
    GetMasm()->Fmov(VixlVReg(tmpReg).S(), src);
    EncodeStr(tmpReg, mem);
}

void Aarch64Encoder::EncodeSti(double src, MemRef mem)
{
    if (!ConvertMem(mem).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        EncodeStr(rzero, mem);
        return;
    }
    ScopedTmpRegF64 tmpReg(this);
    GetMasm()->Fmov(VixlVReg(tmpReg).D(), src);
    EncodeStr(tmpReg, mem);
}

void Aarch64Encoder::EncodeMemCopy(MemRef memFrom, MemRef memTo, size_t size)
{
    if (!ConvertMem(memFrom).IsValid() || !ConvertMem(memTo).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        if (!ConvertMem(memFrom).IsValid()) {
            // Encode one load - will fix inside
            EncodeLdr(rzero, false, memFrom);
        } else {
            ASSERT(!ConvertMem(memTo).IsValid());
            // Encode one store - will fix inside
            EncodeStr(rzero, memTo);
        }
        return;
    }
    ASSERT(ConvertMem(memFrom).IsValid());
    ASSERT(ConvertMem(memTo).IsValid());
    ScopedTmpRegU64 tmpReg(this);
    auto tmp = VixlReg(tmpReg, std::min(size, static_cast<size_t>(DOUBLE_WORD_SIZE)));
    if (size == BYTE_SIZE) {
        GetMasm()->Ldrb(tmp, ConvertMem(memFrom));
        GetMasm()->Strb(tmp, ConvertMem(memTo));
    } else if (size == HALF_SIZE) {
        GetMasm()->Ldrh(tmp, ConvertMem(memFrom));
        GetMasm()->Strh(tmp, ConvertMem(memTo));
    } else {
        ASSERT(size == WORD_SIZE || size == DOUBLE_WORD_SIZE);
        GetMasm()->Ldr(tmp, ConvertMem(memFrom));
        GetMasm()->Str(tmp, ConvertMem(memTo));
    }
}

void Aarch64Encoder::EncodeMemCopyz(MemRef memFrom, MemRef memTo, size_t size)
{
    if (!ConvertMem(memFrom).IsValid() || !ConvertMem(memTo).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        if (!ConvertMem(memFrom).IsValid()) {
            // Encode one load - will fix inside
            EncodeLdr(rzero, false, memFrom);
        } else {
            ASSERT(!ConvertMem(memTo).IsValid());
            // Encode one store - will fix inside
            EncodeStr(rzero, memTo);
        }
        return;
    }
    ASSERT(ConvertMem(memFrom).IsValid());
    ASSERT(ConvertMem(memTo).IsValid());
    ScopedTmpRegU64 tmpReg(this);
    auto tmp = VixlReg(tmpReg, std::min(size, static_cast<size_t>(DOUBLE_WORD_SIZE)));
    auto zero = VixlReg(GetRegfile()->GetZeroReg(), WORD_SIZE);
    if (size == BYTE_SIZE) {
        GetMasm()->Ldrb(tmp, ConvertMem(memFrom));
        GetMasm()->Stp(tmp, zero, ConvertMem(memTo));
    } else if (size == HALF_SIZE) {
        GetMasm()->Ldrh(tmp, ConvertMem(memFrom));
        GetMasm()->Stp(tmp, zero, ConvertMem(memTo));
    } else {
        ASSERT(size == WORD_SIZE || size == DOUBLE_WORD_SIZE);
        GetMasm()->Ldr(tmp, ConvertMem(memFrom));
        if (size == WORD_SIZE) {
            GetMasm()->Stp(tmp, zero, ConvertMem(memTo));
        } else {
            GetMasm()->Str(tmp, ConvertMem(memTo));
        }
    }
}

void Aarch64Encoder::EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsFloat() == src1.IsFloat());
    if (src0.IsFloat()) {
        GetMasm()->Fcmp(VixlVReg(src0), VixlVReg(src1));
    } else {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    }
    GetMasm()->Cset(VixlReg(dst), Convert(cc));
}

void Aarch64Encoder::EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsScalar() && src1.IsScalar());

    GetMasm()->Tst(VixlReg(src0), VixlReg(src1));
    GetMasm()->Cset(VixlReg(dst), ConvertTest(cc));
}

void Aarch64Encoder::EncodeAtomicByteOr(Reg addr, Reg value, bool fastEncoding)
{
    if (fastEncoding) {
#ifndef NDEBUG
        vixl::CPUFeaturesScope scope(GetMasm(), vixl::CPUFeatures::kAtomics);
#endif
        GetMasm()->Stsetb(VixlReg(value, BYTE_SIZE), MemOperand(VixlReg(addr)));
        return;
    }

    // Slow encoding, should not be used in production code!!!
    auto linkReg = GetTarget().GetLinkReg();
    auto frameReg = GetTarget().GetFrameReg();
    static constexpr size_t PAIR_OFFSET = 2 * DOUBLE_WORD_SIZE_BYTES;

    ScopedTmpRegLazy tmp1(this);
    ScopedTmpRegLazy tmp2(this);
    Reg orValue;
    Reg storeResult;
    bool hasTemps = GetScratchRegistersWithLrCount() >= 2U;
    if (hasTemps) {
        tmp1.AcquireWithLr();
        tmp2.AcquireWithLr();
        orValue = tmp1.GetReg().As(INT32_TYPE);
        storeResult = tmp2.GetReg().As(INT32_TYPE);
    } else {
        GetMasm()->stp(VixlReg(frameReg), VixlReg(linkReg),
                       MemOperand(vixl::aarch64::sp, -PAIR_OFFSET, vixl::aarch64::AddrMode::PreIndex));
        orValue = frameReg.As(INT32_TYPE);
        storeResult = linkReg.As(INT32_TYPE);
    }

    auto *loop = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    GetMasm()->Bind(loop);
    GetMasm()->Ldxrb(VixlReg(orValue), MemOperand(VixlReg(addr)));
    GetMasm()->Orr(VixlReg(orValue), VixlReg(orValue), VixlReg(value, WORD_SIZE));
    GetMasm()->Stxrb(VixlReg(storeResult), VixlReg(orValue), MemOperand(VixlReg(addr)));
    GetMasm()->Cbnz(VixlReg(storeResult), loop);
    if (!hasTemps) {
        GetMasm()->ldp(VixlReg(frameReg), VixlReg(linkReg),
                       MemOperand(vixl::aarch64::sp, PAIR_OFFSET, vixl::aarch64::AddrMode::PostIndex));
    }
}

void Aarch64Encoder::EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc)
{
    if (src0.IsFloat()) {
        ASSERT(src1.IsFloat());
        ASSERT(cc == Condition::MI || cc == Condition::LT);
        GetMasm()->Fcmp(VixlVReg(src0), VixlVReg(src1));
    } else {
        ASSERT(src0.IsScalar() && src1.IsScalar());
        ASSERT(cc == Condition::LO || cc == Condition::LT);
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    }
    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::ne);
    GetMasm()->Cneg(VixlReg(Promote(dst)), VixlReg(Promote(dst)), Convert(cc));
}

static void EncodeCmpForSelect(Aarch64Encoder *enc, Reg src2, Reg src3)
{
    if (src2.IsScalar()) {
        enc->GetMasm()->Cmp(VixlReg(src2), VixlReg(src3));
    } else {
        enc->GetMasm()->Fcmp(VixlVReg(src2), VixlVReg(src3));
    }
}

static void EncodeCmpForSelectImm(Aarch64Encoder *enc, Reg src2, Imm imm)
{
    if (src2.IsScalar()) {
        enc->GetMasm()->Cmp(VixlReg(src2), VixlImm(imm));
    } else {
        enc->GetMasm()->Fcmp(VixlVReg(src2), imm.GetAsDouble());
    }
}

static void EncodeTstForSelect(Aarch64Encoder *enc, Reg src2, Reg src3)
{
    ASSERT(!src2.IsFloat() && !src3.IsFloat());
    enc->GetMasm()->Tst(VixlReg(src2), VixlReg(src3));
}

static void EncodeTstForSelectImm(Aarch64Encoder *enc, Reg src2, Imm imm)
{
    ASSERT(!src2.IsFloat());
    ASSERT(enc->CanEncodeImmLogical(imm.GetAsInt(), src2.GetSize() > WORD_SIZE ? DOUBLE_WORD_SIZE : WORD_SIZE));
    enc->GetMasm()->Tst(VixlReg(src2), VixlImm(imm));
}

static void EncodeCselForSelect(Aarch64Encoder *enc, Reg dst, Reg src0, Reg src1, Condition cc)
{
    auto ccConverted = IsTestCc(cc) ? ConvertTest(cc) : Convert(cc);
    if (dst.IsFloat()) {
        enc->GetMasm()->Fcsel(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1), ccConverted);
    } else {
        enc->GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), ccConverted);
    }
}

// CC-OFFNXT(G.FUN.01-CPP) Internal helper function
static void EncodeCselTransformForSelect(Aarch64Encoder *enc, Reg dst, Reg src0, Reg src1, Condition cc,
                                         SelectTransformType transform)
{
    ASSERT(!dst.IsFloat());
    auto encodeFn = [transform]() {
        switch (transform) {
            case SelectTransformType::INC:
                return &vixl::aarch64::MacroAssembler::Csinc;
            case SelectTransformType::INV:
                return &vixl::aarch64::MacroAssembler::Csinv;
            case SelectTransformType::NEG:
                return &vixl::aarch64::MacroAssembler::Csneg;
            default:
                UNREACHABLE();
        }
    }();
    auto ccConverted = IsTestCc(cc) ? ConvertTest(cc) : Convert(cc);
    (enc->GetMasm()->*encodeFn)(VixlReg(dst), VixlReg(src0), VixlReg(src1), ccConverted);
}

void Aarch64Encoder::EncodeSelect(const ArgsSelect &args)
{
    auto [dst, src0, src1, src2, src3, cc] = args;
    EncodeCmpForSelect(this, src2, src3);
    EncodeCselForSelect(this, dst, src0, src1, cc);
}

void Aarch64Encoder::EncodeSelect(const ArgsSelectImm &args)
{
    auto [dst, src0, src1, src2, imm, cc] = args;
    EncodeCmpForSelectImm(this, src2, imm);
    EncodeCselForSelect(this, dst, src0, src1, cc);
}

void Aarch64Encoder::EncodeSelectTransform(const ArgsSelectTransform &args)
{
    auto [dst, src0, src1, src2, src3, cc, transform] = args;
    EncodeCmpForSelect(this, src2, src3);
    EncodeCselTransformForSelect(this, dst, src0, src1, cc, transform);
}

void Aarch64Encoder::EncodeSelectTransform(const ArgsSelectImmTransform &args)
{
    auto [dst, src0, src1, src2, imm, cc, transform] = args;
    EncodeCmpForSelectImm(this, src2, imm);
    EncodeCselTransformForSelect(this, dst, src0, src1, cc, transform);
}

void Aarch64Encoder::EncodeSelectTest(const ArgsSelect &args)
{
    auto [dst, src0, src1, src2, src3, cc] = args;
    EncodeTstForSelect(this, src2, src3);
    EncodeCselForSelect(this, dst, src0, src1, cc);
}

void Aarch64Encoder::EncodeSelectTest(const ArgsSelectImm &args)
{
    auto [dst, src0, src1, src2, imm, cc] = args;
    EncodeTstForSelectImm(this, src2, imm);
    EncodeCselForSelect(this, dst, src0, src1, cc);
}

void Aarch64Encoder::EncodeSelectTestTransform(const ArgsSelectTransform &args)
{
    auto [dst, src0, src1, src2, src3, cc, transform] = args;
    EncodeTstForSelect(this, src2, src3);
    EncodeCselTransformForSelect(this, dst, src0, src1, cc, transform);
}

void Aarch64Encoder::EncodeSelectTestTransform(const ArgsSelectImmTransform &args)
{
    auto [dst, src0, src1, src2, imm, cc, transform] = args;
    EncodeTstForSelectImm(this, src2, imm);
    EncodeCselTransformForSelect(this, dst, src0, src1, cc, transform);
}

static void EncodeLdpImpl(Aarch64Encoder *self, Reg dst0, Reg dst1, bool dstSigned, MemRef mem,
                          vixl::aarch64::AddrMode mode)
{
    ASSERT(dst0.IsFloat() == dst1.IsFloat());
    ASSERT(dst0.GetSize() == dst1.GetSize());

    auto convertedMem = ConvertMem(mem, mode);
    if (!convertedMem.IsValid()) {
        // Encode one Ldr - will fix inside
        self->EncodeLdr(dst0, dstSigned, mem);
        return;
    }

    if (dst0.IsFloat()) {
        self->GetMasm()->Ldp(VixlVReg(dst0), VixlVReg(dst1), convertedMem);
        return;
    }
    if (dstSigned && dst0.GetSize() == WORD_SIZE) {
        self->GetMasm()->Ldpsw(VixlReg(dst0, DOUBLE_WORD_SIZE), VixlReg(dst1, DOUBLE_WORD_SIZE), convertedMem);
        return;
    }
    self->GetMasm()->Ldp(VixlReg(dst0), VixlReg(dst1), convertedMem);
}

void Aarch64Encoder::EncodeLdp(Reg dst0, Reg dst1, bool dstSigned, MemRef mem)
{
    EncodeLdpImpl(this, dst0, dst1, dstSigned, mem, vixl::aarch64::Offset);
}

void Aarch64Encoder::EncodeLdpPreIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, Imm offset)
{
    EncodeLdpImpl(this, dst0, dst1, dstSigned, MemRef {srcBase, offset.GetAsInt()}, vixl::aarch64::PreIndex);
}

void Aarch64Encoder::EncodeLdpPostIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, Imm offset)
{
    EncodeLdpImpl(this, dst0, dst1, dstSigned, MemRef {srcBase, offset.GetAsInt()}, vixl::aarch64::PostIndex);
}

static void EncodeStpImpl(Aarch64Encoder *self, Reg src0, Reg src1, MemRef mem, vixl::aarch64::AddrMode mode)
{
    ASSERT(src0.IsFloat() == src1.IsFloat());
    ASSERT(src0.GetSize() == src1.GetSize());

    auto convertedMem = ConvertMem(mem, mode);
    if (!convertedMem.IsValid()) {
        // Encode one Str - will fix inside
        self->EncodeStr(src0, mem);
        return;
    }

    if (src0.IsFloat()) {
        self->GetMasm()->Stp(VixlVReg(src0), VixlVReg(src1), convertedMem);
        return;
    }
    self->GetMasm()->Stp(VixlReg(src0), VixlReg(src1), convertedMem);
}

void Aarch64Encoder::EncodeStp(Reg src0, Reg src1, MemRef mem)
{
    EncodeStpImpl(this, src0, src1, mem, vixl::aarch64::Offset);
}

void Aarch64Encoder::EncodeStpPreIndex(Reg src0, Reg src1, Reg dstBase, Imm offset)
{
    EncodeStpImpl(this, src0, src1, MemRef {dstBase, offset.GetAsInt()}, vixl::aarch64::PreIndex);
}

void Aarch64Encoder::EncodeStpPostIndex(Reg src0, Reg src1, Reg dstBase, Imm offset)
{
    EncodeStpImpl(this, src0, src1, MemRef {dstBase, offset.GetAsInt()}, vixl::aarch64::PostIndex);
}

void Aarch64Encoder::EncodeMAdd(Reg dst, Reg src0, Reg src1, Reg src2)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize() && dst.GetSize() == src2.GetSize());
    ASSERT(dst.IsScalar() == src0.IsScalar() && dst.IsScalar() == src1.IsScalar() && dst.IsScalar() == src2.IsScalar());

    ASSERT(!GetRegfile()->IsZeroReg(dst));

    if (GetRegfile()->IsZeroReg(src0) || GetRegfile()->IsZeroReg(src1)) {
        EncodeMov(dst, src2);
        return;
    }

    if (GetRegfile()->IsZeroReg(src2)) {
        EncodeMul(dst, src0, src1);
        return;
    }

    if (dst.IsScalar()) {
        GetMasm()->Madd(VixlReg(dst), VixlReg(src0), VixlReg(src1), VixlReg(src2));
    } else {
        GetMasm()->Fmadd(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1), VixlVReg(src2));
    }
}

void Aarch64Encoder::EncodeMSub(Reg dst, Reg src0, Reg src1, Reg src2)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize() && dst.GetSize() == src2.GetSize());
    ASSERT(dst.IsScalar() == src0.IsScalar() && dst.IsScalar() == src1.IsScalar() && dst.IsScalar() == src2.IsScalar());

    ASSERT(!GetRegfile()->IsZeroReg(dst));

    if (GetRegfile()->IsZeroReg(src0) || GetRegfile()->IsZeroReg(src1)) {
        EncodeMov(dst, src2);
        return;
    }

    if (GetRegfile()->IsZeroReg(src2)) {
        EncodeMNeg(dst, src0, src1);
        return;
    }

    if (dst.IsScalar()) {
        GetMasm()->Msub(VixlReg(dst), VixlReg(src0), VixlReg(src1), VixlReg(src2));
    } else {
        GetMasm()->Fmsub(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1), VixlVReg(src2));
    }
}

void Aarch64Encoder::EncodeMNeg(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize());
    ASSERT(dst.IsScalar() == src0.IsScalar() && dst.IsScalar() == src1.IsScalar());

    ASSERT(!GetRegfile()->IsZeroReg(dst));

    if (GetRegfile()->IsZeroReg(src0) || GetRegfile()->IsZeroReg(src1)) {
        EncodeMov(dst, Imm(0U));
        return;
    }

    if (dst.IsScalar()) {
        GetMasm()->Mneg(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    } else {
        GetMasm()->Fnmul(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
    }
}

void Aarch64Encoder::EncodeOrNot(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.IsScalar());
    GetMasm()->Orn(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeOrNot(Reg dst, Reg src0, Shift src1)
{
    ASSERT(dst.GetSize() == src0.GetSize() && dst.GetSize() == src1.GetBase().GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.GetBase().IsScalar());
    GetMasm()->Orn(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeExtractBits(Reg dst, Reg src0, Imm imm1, Imm imm2, bool signExt)
{
    if (signExt) {
        GetMasm()->Sbfx(VixlReg(dst), VixlReg(src0), imm1.GetAsInt(), imm2.GetAsInt());
    } else {
        GetMasm()->Ubfx(VixlReg(dst), VixlReg(src0), imm1.GetAsInt(), imm2.GetAsInt());
    }
}

void Aarch64Encoder::EncodeAndNot(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.IsScalar());
    GetMasm()->Bic(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAndNot(Reg dst, Reg src0, Shift src1)
{
    ASSERT(dst.GetSize() == src0.GetSize() && dst.GetSize() == src1.GetBase().GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.GetBase().IsScalar());
    GetMasm()->Bic(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeXorNot(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.IsScalar());
    GetMasm()->Eon(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeXorNot(Reg dst, Reg src0, Shift src1)
{
    ASSERT(dst.GetSize() == src0.GetSize() && dst.GetSize() == src1.GetBase().GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.GetBase().IsScalar());
    GetMasm()->Eon(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeNeg(Reg dst, Shift src)
{
    ASSERT(dst.GetSize() == src.GetBase().GetSize());
    ASSERT(dst.IsScalar() && src.GetBase().IsScalar());
    GetMasm()->Neg(VixlReg(dst), VixlShift(src));
}

void Aarch64Encoder::EncodeStackOverflowCheck(ssize_t offset)
{
    ScopedTmpReg tmp(this);
    EncodeAdd(tmp, GetTarget().GetStackReg(), Imm(offset));
    EncodeLdr(tmp, false, MemRef(tmp));
}

void Aarch64Encoder::EncodeGetCurrentPc(Reg dst)
{
    ASSERT(dst.GetType() == INT64_TYPE);

    auto currentPc = CreateLabel();
    BindLabel(currentPc);

    auto *labelHolder = static_cast<Aarch64LabelHolder *>(GetLabels());
    GetMasm()->Adr(VixlReg(dst), labelHolder->GetLabel(currentPc));
}

bool Aarch64Encoder::CanEncodeImmAddSubCmp(int64_t imm, [[maybe_unused]] uint32_t size,
                                           [[maybe_unused]] bool signedCompare)
{
    if (imm == INT64_MIN) {
        return false;
    }
    if (imm < 0) {
        imm = -imm;
    }
    return vixl::aarch64::Assembler::IsImmAddSub(imm);
}

bool Aarch64Encoder::CanEncodeImmLogical(uint64_t imm, uint32_t size)
{
#ifndef NDEBUG
    if (size < DOUBLE_WORD_SIZE) {
        // Test if the highest part is consistent:
        ASSERT(((imm >> size) == 0) || (((~imm) >> size) == 0));
    }
#endif  // NDEBUG
    return vixl::aarch64::Assembler::IsImmLogical(imm, size);
}

bool Aarch64Encoder::CanOptimizeImmDivMod(uint64_t imm, bool isSigned) const
{
    return CanOptimizeImmDivModCommon(imm, isSigned);
}

/*
 * From aarch64 instruction set
 *
 * ========================================================
 * Syntax
 *
 * LDR  Wt, [Xn|SP, Rm{, extend {amount}}]    ; 32-bit general registers
 *
 * LDR  Xt, [Xn|SP, Rm{, extend {amount}}]    ; 64-bit general registers
 *
 * amount
 * Is the index shift amount, optional and defaulting to #0 when extend is not LSL:
 *
 * 32-bit general registers
 * Can be one of #0 or #2.
 *
 * 64-bit general registers
 * Can be one of #0 or #3.
 * ========================================================
 * Syntax
 *
 * LDRH  Wt, [Xn|SP, Rm{, extend {amount}}]
 *
 * amount
 * Is the index shift amount, optional and defaulting to #0 when extend is not LSL, and can be either #0 or #1.
 * ========================================================
 *
 * Scale can be 0 or 1 for half load, 2 for word load, 3 for double word load
 */
bool Aarch64Encoder::CanEncodeScale(uint64_t imm, uint32_t size)
{
    return (imm == 0) || ((1U << imm) == (size >> 3U));
}

bool Aarch64Encoder::CanEncodeShiftedOperand(ShiftOpcode opcode, ShiftType shiftType)
{
    switch (opcode) {
        case ShiftOpcode::NEG_SR:
        case ShiftOpcode::ADD_SR:
        case ShiftOpcode::SUB_SR:
            return shiftType == ShiftType::LSL || shiftType == ShiftType::LSR || shiftType == ShiftType::ASR;
        case ShiftOpcode::AND_SR:
        case ShiftOpcode::OR_SR:
        case ShiftOpcode::XOR_SR:
        case ShiftOpcode::AND_NOT_SR:
        case ShiftOpcode::OR_NOT_SR:
        case ShiftOpcode::XOR_NOT_SR:
            return shiftType != ShiftType::INVALID_SHIFT;
        default:
            return false;
    }
}

bool Aarch64Encoder::CanEncodeFloatSelect()
{
    return true;
}

bool Aarch64Encoder::CanEncodeSelectTransformFor(DataType::Type type)
{
    return type == DataType::INT32 || type == DataType::INT64 || type == DataType::UINT32 || type == DataType::UINT64;
}

bool Aarch64Encoder::CanEncodeBitfieldExtractionFor(DataType::Type type)
{
    return type == DataType::INT32 || type == DataType::UINT32 || type == DataType::INT64 || type == DataType::UINT64;
}

Reg Aarch64Encoder::AcquireScratchRegister(TypeInfo type)
{
    ASSERT(GetMasm()->GetCurrentScratchRegisterScope() == nullptr);
    auto reg = type.IsFloat() ? GetMasm()->GetScratchVRegisterList()->PopLowestIndex()
                              : GetMasm()->GetScratchRegisterList()->PopLowestIndex();
    ASSERT(reg.IsValid());
    return Reg(reg.GetCode(), type);
}

void Aarch64Encoder::AcquireScratchRegister(Reg reg)
{
    ASSERT(GetMasm()->GetCurrentScratchRegisterScope() == nullptr);
    if (reg == GetTarget().GetLinkReg()) {
        ASSERT_PRINT(!lrAcquired_, "Trying to acquire LR, which hasn't been released before");
        lrAcquired_ = true;
        return;
    }
    auto type = reg.GetType();
    auto regId = reg.GetId();

    if (type.IsFloat()) {
        ASSERT(GetMasm()->GetScratchVRegisterList()->IncludesAliasOf(VixlVReg(reg)));
        GetMasm()->GetScratchVRegisterList()->Remove(regId);
    } else {
        ASSERT(GetMasm()->GetScratchRegisterList()->IncludesAliasOf(VixlReg(reg)));
        GetMasm()->GetScratchRegisterList()->Remove(regId);
    }
}

void Aarch64Encoder::ReleaseScratchRegister(Reg reg)
{
    if (reg == GetTarget().GetLinkReg()) {
        ASSERT_PRINT(lrAcquired_, "Trying to release LR, which hasn't been acquired before");
        lrAcquired_ = false;
    } else if (reg.IsFloat()) {
        GetMasm()->GetScratchVRegisterList()->Combine(reg.GetId());
    } else if (reg.GetId() != GetTarget().GetLinkReg().GetId()) {
        GetMasm()->GetScratchRegisterList()->Combine(reg.GetId());
    }
}

bool Aarch64Encoder::IsScratchRegisterReleased(Reg reg) const
{
    if (reg == GetTarget().GetLinkReg()) {
        return !lrAcquired_;
    }
    if (reg.IsFloat()) {
        return GetMasm()->GetScratchVRegisterList()->IncludesAliasOf(VixlVReg(reg));
    }
    return GetMasm()->GetScratchRegisterList()->IncludesAliasOf(VixlReg(reg));
}

RegMask Aarch64Encoder::GetScratchRegistersMask() const
{
    return RegMask(GetMasm()->GetScratchRegisterList()->GetList());
}

RegMask Aarch64Encoder::GetScratchFpRegistersMask() const
{
    return RegMask(GetMasm()->GetScratchVRegisterList()->GetList());
}

RegMask Aarch64Encoder::GetAvailableScratchRegisters() const
{
    return RegMask(GetMasm()->GetScratchRegisterList()->GetList());
}

VRegMask Aarch64Encoder::GetAvailableScratchFpRegisters() const
{
    return VRegMask(GetMasm()->GetScratchVRegisterList()->GetList());
}

TypeInfo Aarch64Encoder::GetRefType()
{
    return INT64_TYPE;
}

void *Aarch64Encoder::BufferData() const
{
    return GetMasm()->GetBuffer()->GetStartAddress<void *>();
}

size_t Aarch64Encoder::BufferSize() const
{
    return GetMasm()->GetBuffer()->GetSizeInBytes();
}

void Aarch64Encoder::MakeLibCall(Reg dst, Reg src0, Reg src1, const void *entryPoint)
{
    if (!dst.IsFloat()) {
        SetFalseResult();
        return;
    }
    if (dst.GetType() == FLOAT32_TYPE) {
        if (!src0.IsFloat() || !src1.IsFloat()) {
            SetFalseResult();
            return;
        }

        if (src0.GetId() != vixl::aarch64::s0.GetCode() || src1.GetId() != vixl::aarch64::s1.GetCode()) {
            ScopedTmpRegF32 tmp(this);
            GetMasm()->Fmov(VixlVReg(tmp), VixlVReg(src1));
            GetMasm()->Fmov(vixl::aarch64::s0, VixlVReg(src0));
            GetMasm()->Fmov(vixl::aarch64::s1, VixlVReg(tmp));
        }

        MakeCall(entryPoint);

        if (dst.GetId() != vixl::aarch64::s0.GetCode()) {
            GetMasm()->Fmov(VixlVReg(dst), vixl::aarch64::s0);
        }
    } else if (dst.GetType() == FLOAT64_TYPE) {
        if (!src0.IsFloat() || !src1.IsFloat()) {
            SetFalseResult();
            return;
        }

        if (src0.GetId() != vixl::aarch64::d0.GetCode() || src1.GetId() != vixl::aarch64::d1.GetCode()) {
            ScopedTmpRegF64 tmp(this);
            GetMasm()->Fmov(VixlVReg(tmp), VixlVReg(src1));

            GetMasm()->Fmov(vixl::aarch64::d0, VixlVReg(src0));
            GetMasm()->Fmov(vixl::aarch64::d1, VixlVReg(tmp));
        }

        MakeCall(entryPoint);

        if (dst.GetId() != vixl::aarch64::d0.GetCode()) {
            GetMasm()->Fmov(VixlVReg(dst), vixl::aarch64::d0);
        }
    } else {
        UNREACHABLE();
    }
}

template <bool IS_STORE>
void Aarch64Encoder::LoadStoreRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp)
{
    if (registers.none()) {
        return;
    }
    auto lastReg = static_cast<int32_t>(registers.size() - 1);
    for (; lastReg >= 0; --lastReg) {
        if (registers.test(lastReg)) {
            break;
        }
    }
    // Construct single add for big offset
    size_t spOffset;
    auto lastOffset = (slot + lastReg - static_cast<ssize_t>(startReg)) * static_cast<ssize_t>(DOUBLE_WORD_SIZE_BYTES);

    if (!vixl::aarch64::Assembler::IsImmLSPair(lastOffset, vixl::aarch64::kXRegSizeInBytesLog2)) {
        ScopedTmpReg lrReg(this, true);
        auto tmp = VixlReg(lrReg);
        spOffset = static_cast<size_t>(slot * DOUBLE_WORD_SIZE_BYTES);
        slot = 0;
        if (vixl::aarch64::Assembler::IsImmAddSub(spOffset)) {
            GetMasm()->Add(tmp, vixl::aarch64::sp, VixlImm(spOffset));
        } else {
            GetMasm()->Mov(tmp, VixlImm(spOffset));
            GetMasm()->Add(tmp, vixl::aarch64::sp, tmp);
        }
        LoadStoreRegistersLoop<IS_STORE>(registers, slot, startReg, isFp, tmp);
    } else {
        LoadStoreRegistersLoop<IS_STORE>(registers, slot, startReg, isFp, vixl::aarch64::sp);
    }
}

template <bool IS_STORE>
static void LoadStorePair(vixl::aarch64::MacroAssembler *masm, CPURegister lastReg, CPURegister reg, Reg base,
                          int32_t idx)
{
    auto baseReg = VixlReg(base);
    static constexpr int32_t OFFSET = 2;
    if constexpr (IS_STORE) {  // NOLINT
        masm->Stp(lastReg, reg, MemOperand(baseReg, (idx - OFFSET) * DOUBLE_WORD_SIZE_BYTES));
    } else {  // NOLINT
        masm->Ldp(lastReg, reg, MemOperand(baseReg, (idx - OFFSET) * DOUBLE_WORD_SIZE_BYTES));
    }
}

template <bool IS_STORE>
static void LoadStoreReg(vixl::aarch64::MacroAssembler *masm, CPURegister lastReg, Reg base, int32_t idx)
{
    auto baseReg = VixlReg(base);
    if constexpr (IS_STORE) {  // NOLINT
        masm->Str(lastReg, MemOperand(baseReg, (idx - 1) * DOUBLE_WORD_SIZE_BYTES));
    } else {  // NOLINT
        masm->Ldr(lastReg, MemOperand(baseReg, (idx - 1) * DOUBLE_WORD_SIZE_BYTES));
    }
}

template <bool IS_STORE>
void Aarch64Encoder::LoadStoreRegistersMainLoop(RegMask registers, bool isFp, int32_t slot, Reg base, RegMask mask)
{
    bool hasMask = mask.any();
    int32_t index = hasMask ? static_cast<int32_t>(mask.GetMinRegister()) : 0;
    int32_t lastIndex = -1;
    ssize_t lastId = -1;

    slot -= index;
    for (ssize_t id = index; id < helpers::ToSigned(registers.size()); id++) {
        if (hasMask) {
            if (!mask.test(id)) {
                continue;
            }
            index++;
        }
        if (!registers.test(id)) {
            continue;
        }
        if (!hasMask) {
            index++;
        }
        if (lastId == -1) {
            lastId = id;
            lastIndex = index;
            continue;
        }

        auto lastReg =
            CPURegister(lastId, vixl::aarch64::kXRegSize, isFp ? CPURegister::kVRegister : CPURegister::kRegister);
        if (!hasMask || lastId + 1 == id) {
            auto reg =
                CPURegister(id, vixl::aarch64::kXRegSize, isFp ? CPURegister::kVRegister : CPURegister::kRegister);
            LoadStorePair<IS_STORE>(GetMasm(), lastReg, reg, base, slot + index);
            lastId = -1;
        } else {
            LoadStoreReg<IS_STORE>(GetMasm(), lastReg, base, slot + lastIndex);
            lastId = id;
            lastIndex = index;
        }
    }
    if (lastId != -1) {
        auto lastReg =
            CPURegister(lastId, vixl::aarch64::kXRegSize, isFp ? CPURegister::kVRegister : CPURegister::kRegister);
        LoadStoreReg<IS_STORE>(GetMasm(), lastReg, base, slot + lastIndex);
    }
}

template <bool IS_STORE>
void Aarch64Encoder::LoadStoreRegisters(RegMask registers, bool isFp, int32_t slot, Reg base, RegMask mask)
{
    if (registers.none()) {
        return;
    }

    int32_t maxOffset = (slot + helpers::ToSigned(registers.GetMaxRegister())) * DOUBLE_WORD_SIZE_BYTES;
    int32_t minOffset = (slot + helpers::ToSigned(registers.GetMinRegister())) * DOUBLE_WORD_SIZE_BYTES;

    ScopedTmpRegLazy tmpReg(this, true);
    // Construct single add for big offset
    if (!vixl::aarch64::Assembler::IsImmLSPair(minOffset, vixl::aarch64::kXRegSizeInBytesLog2) ||
        !vixl::aarch64::Assembler::IsImmLSPair(maxOffset, vixl::aarch64::kXRegSizeInBytesLog2)) {
        tmpReg.AcquireWithLr();
        auto lrReg = VixlReg(tmpReg);
        ssize_t spOffset = slot * DOUBLE_WORD_SIZE_BYTES;
        if (vixl::aarch64::Assembler::IsImmAddSub(spOffset)) {
            GetMasm()->Add(lrReg, VixlReg(base), VixlImm(spOffset));
        } else {
            GetMasm()->Mov(lrReg, VixlImm(spOffset));
            GetMasm()->Add(lrReg, VixlReg(base), lrReg);
        }
        // Adjust new values for slot and base register
        slot = 0;
        base = tmpReg;
    }

    LoadStoreRegistersMainLoop<IS_STORE>(registers, isFp, slot, base, mask);
}

template <bool IS_STORE>
void Aarch64Encoder::LoadStoreRegistersLoop(RegMask registers, ssize_t slot, size_t startReg, bool isFp,
                                            const vixl::aarch64::Register &baseReg)
{
    size_t i = 0;
    const auto getNextReg = [&registers, &i, isFp]() {
        for (; i < registers.size(); i++) {
            if (registers.test(i)) {
                return CPURegister(i++, vixl::aarch64::kXRegSize,
                                   isFp ? CPURegister::kVRegister : CPURegister::kRegister);
            }
        }
        return CPURegister();
    };

    for (CPURegister nextReg = getNextReg(); nextReg.IsValid();) {
        const CPURegister currReg = nextReg;
        nextReg = getNextReg();
        if (nextReg.IsValid() && (nextReg.GetCode() - 1 == currReg.GetCode())) {
            if constexpr (IS_STORE) {  // NOLINT
                GetMasm()->Stp(currReg, nextReg,
                               MemOperand(baseReg, (slot + currReg.GetCode() - startReg) * DOUBLE_WORD_SIZE_BYTES));
            } else {  // NOLINT
                GetMasm()->Ldp(currReg, nextReg,
                               MemOperand(baseReg, (slot + currReg.GetCode() - startReg) * DOUBLE_WORD_SIZE_BYTES));
            }
            nextReg = getNextReg();
        } else {
            if constexpr (IS_STORE) {  // NOLINT
                GetMasm()->Str(currReg,
                               MemOperand(baseReg, (slot + currReg.GetCode() - startReg) * DOUBLE_WORD_SIZE_BYTES));
            } else {  // NOLINT
                GetMasm()->Ldr(currReg,
                               MemOperand(baseReg, (slot + currReg.GetCode() - startReg) * DOUBLE_WORD_SIZE_BYTES));
            }
        }
    }
}

void Aarch64Encoder::SaveRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp)
{
    LoadStoreRegisters<true>(registers, slot, startReg, isFp);
}

void Aarch64Encoder::LoadRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp)
{
    LoadStoreRegisters<false>(registers, slot, startReg, isFp);
}

void Aarch64Encoder::SaveRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask)
{
    LoadStoreRegisters<true>(registers, isFp, slot, base, mask);
}

void Aarch64Encoder::LoadRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask)
{
    LoadStoreRegisters<false>(registers, isFp, slot, base, mask);
}

void Aarch64Encoder::PushRegisters(RegMask registers, bool isFp)
{
    static constexpr size_t PAIR_OFFSET = 2 * DOUBLE_WORD_SIZE_BYTES;
    Register lastReg = INVALID_REG;
    for (size_t i = 0; i < registers.size(); i++) {
        if (registers[i]) {
            if (lastReg == INVALID_REG) {
                lastReg = i;
                continue;
            }
            if (isFp) {
                GetMasm()->stp(vixl::aarch64::VRegister(lastReg, DOUBLE_WORD_SIZE),
                               vixl::aarch64::VRegister(i, DOUBLE_WORD_SIZE),
                               MemOperand(vixl::aarch64::sp, -PAIR_OFFSET, vixl::aarch64::AddrMode::PreIndex));
            } else {
                GetMasm()->stp(vixl::aarch64::Register(lastReg, DOUBLE_WORD_SIZE),
                               vixl::aarch64::Register(i, DOUBLE_WORD_SIZE),
                               MemOperand(vixl::aarch64::sp, -PAIR_OFFSET, vixl::aarch64::AddrMode::PreIndex));
            }
            lastReg = INVALID_REG;
        }
    }
    if (lastReg != INVALID_REG) {
        if (isFp) {
            GetMasm()->str(vixl::aarch64::VRegister(lastReg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, -PAIR_OFFSET, vixl::aarch64::AddrMode::PreIndex));
        } else {
            GetMasm()->str(vixl::aarch64::Register(lastReg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, -PAIR_OFFSET, vixl::aarch64::AddrMode::PreIndex));
        }
    }
}

void Aarch64Encoder::PopRegisters(RegMask registers, bool isFp)
{
    static constexpr size_t PAIR_OFFSET = 2 * DOUBLE_WORD_SIZE_BYTES;
    Register lastReg;
    if ((registers.count() & 1U) != 0) {
        lastReg = registers.GetMaxRegister();
        if (isFp) {
            GetMasm()->ldr(vixl::aarch64::VRegister(lastReg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, PAIR_OFFSET, vixl::aarch64::AddrMode::PostIndex));
        } else {
            GetMasm()->ldr(vixl::aarch64::Register(lastReg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, PAIR_OFFSET, vixl::aarch64::AddrMode::PostIndex));
        }
        registers.reset(lastReg);
    }
    lastReg = INVALID_REG;
    for (auto i = static_cast<ssize_t>(registers.size() - 1); i >= 0; i--) {
        if (registers[i]) {
            if (lastReg == INVALID_REG) {
                lastReg = i;
                continue;
            }
            if (isFp) {
                GetMasm()->ldp(vixl::aarch64::VRegister(i, DOUBLE_WORD_SIZE),
                               vixl::aarch64::VRegister(lastReg, DOUBLE_WORD_SIZE),
                               MemOperand(vixl::aarch64::sp, PAIR_OFFSET, vixl::aarch64::AddrMode::PostIndex));
            } else {
                GetMasm()->ldp(vixl::aarch64::Register(i, DOUBLE_WORD_SIZE),
                               vixl::aarch64::Register(lastReg, DOUBLE_WORD_SIZE),
                               MemOperand(vixl::aarch64::sp, PAIR_OFFSET, vixl::aarch64::AddrMode::PostIndex));
            }
            lastReg = INVALID_REG;
        }
    }
}

vixl::aarch64::MacroAssembler *Aarch64Encoder::GetMasm() const
{
    ASSERT(masm_ != nullptr);
    return masm_;
}

size_t Aarch64Encoder::GetLabelAddress(LabelHolder::LabelId label)
{
    auto plabel = labels_->GetLabel(label);
    ASSERT(plabel->IsBound());
    return GetMasm()->GetLabelAddress<size_t>(plabel);
}

bool Aarch64Encoder::LabelHasLinks(LabelHolder::LabelId label)
{
    auto plabel = labels_->GetLabel(label);
    return plabel->IsLinked();
}

#ifndef PANDA_MINIMAL_VIXL
vixl::aarch64::Decoder &Aarch64Encoder::GetDecoder() const
{
    if (!decoder_) {
        decoder_.emplace(GetAllocator());
        decoder_->visitors()->push_back(&GetDisasm());
    }
    return *decoder_;
}

vixl::aarch64::Disassembler &Aarch64Encoder::GetDisasm() const
{
    if (!disasm_) {
        disasm_.emplace(GetAllocator());
    }
    return *disasm_;
}
#endif

size_t Aarch64Encoder::DisasmInstr([[maybe_unused]] std::ostream &stream, size_t pc,
                                   [[maybe_unused]] ssize_t codeOffset) const
{
#ifndef PANDA_MINIMAL_VIXL
    auto bufferStart = GetMasm()->GetBuffer()->GetOffsetAddress<uintptr_t>(0);
    auto instr = GetMasm()->GetBuffer()->GetOffsetAddress<vixl::aarch64::Instruction *>(pc);
    GetDecoder().Decode(instr);
    if (codeOffset < 0) {
        stream << GetDisasm().GetOutput();
    } else {
        stream << std::setw(0x4) << std::right << std::setfill('0') << std::hex
               << reinterpret_cast<uintptr_t>(instr) - bufferStart + static_cast<size_t>(codeOffset) << ": "
               << GetDisasm().GetOutput() << std::setfill(' ') << std::dec;
    }

#endif
    return pc + vixl::aarch64::kInstructionSize;
}
}  // namespace ark::compiler::aarch64
