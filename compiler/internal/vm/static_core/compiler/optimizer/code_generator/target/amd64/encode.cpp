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

#include <iomanip>

#include "libarkbase/utils/utils.h"
#include "compiler/optimizer/code_generator/relocations.h"
#include "compiler/optimizer/code_generator/fast_divisor.h"
#include "operands.h"
#include "scoped_tmp_reg.h"
#include "target/amd64/target.h"

#include "lib_helpers.inl"

#include "Zydis/Zydis.h"

#ifndef PANDA_TARGET_MACOS
#include "elf.h"
#endif  // PANDA_TARGET_MACOS

namespace ark::compiler::amd64 {

static auto ArchCcInt(Condition cc)
{
    switch (cc) {
        case Condition::EQ:
            return asmjit::x86::Condition::Code::kEqual;
        case Condition::NE:
            return asmjit::x86::Condition::Code::kNotEqual;
        case Condition::LT:
            return asmjit::x86::Condition::Code::kSignedLT;
        case Condition::GT:
            return asmjit::x86::Condition::Code::kSignedGT;
        case Condition::LE:
            return asmjit::x86::Condition::Code::kSignedLE;
        case Condition::GE:
            return asmjit::x86::Condition::Code::kSignedGE;
        case Condition::LO:
            return asmjit::x86::Condition::Code::kUnsignedLT;
        case Condition::LS:
            return asmjit::x86::Condition::Code::kUnsignedLE;
        case Condition::HI:
            return asmjit::x86::Condition::Code::kUnsignedGT;
        case Condition::HS:
            return asmjit::x86::Condition::Code::kUnsignedGE;
        // NOTE(igorban) : Remove them
        case Condition::MI:
            return asmjit::x86::Condition::Code::kNegative;
        case Condition::PL:
            return asmjit::x86::Condition::Code::kPositive;
        case Condition::VS:
            return asmjit::x86::Condition::Code::kOverflow;
        case Condition::VC:
            return asmjit::x86::Condition::Code::kNotOverflow;
        case Condition::AL:
        case Condition::NV:
        default:
            UNREACHABLE();
            return asmjit::x86::Condition::Code::kEqual;
    }
}
static auto ArchCcFloat(Condition cc)
{
    switch (cc) {
        case Condition::EQ:
            return asmjit::x86::Condition::Code::kEqual;
        case Condition::NE:
            return asmjit::x86::Condition::Code::kNotEqual;
        case Condition::LT:
            return asmjit::x86::Condition::Code::kUnsignedLT;
        case Condition::GT:
            return asmjit::x86::Condition::Code::kUnsignedGT;
        case Condition::LE:
            return asmjit::x86::Condition::Code::kUnsignedLE;
        case Condition::GE:
            return asmjit::x86::Condition::Code::kUnsignedGE;
        case Condition::LO:
            return asmjit::x86::Condition::Code::kUnsignedLT;
        case Condition::LS:
            return asmjit::x86::Condition::Code::kUnsignedLE;
        case Condition::HI:
            return asmjit::x86::Condition::Code::kUnsignedGT;
        case Condition::HS:
            return asmjit::x86::Condition::Code::kUnsignedGE;
        // NOTE(igorban) : Remove them
        case Condition::MI:
            return asmjit::x86::Condition::Code::kNegative;
        case Condition::PL:
            return asmjit::x86::Condition::Code::kPositive;
        case Condition::VS:
            return asmjit::x86::Condition::Code::kOverflow;
        case Condition::VC:
            return asmjit::x86::Condition::Code::kNotOverflow;
        case Condition::AL:
        case Condition::NV:
        default:
            UNREACHABLE();
            return asmjit::x86::Condition::Code::kEqual;
    }
}
/// Converters
static asmjit::x86::Condition::Code ArchCc(Condition cc, bool isFloat = false)
{
    return isFloat ? ArchCcFloat(cc) : ArchCcInt(cc);
}

static asmjit::x86::Condition::Code ArchCcTest(Condition cc)
{
    ASSERT(cc == Condition::TST_EQ || cc == Condition::TST_NE);
    return cc == Condition::TST_EQ ? asmjit::x86::Condition::Code::kEqual : asmjit::x86::Condition::Code::kNotEqual;
}

static bool CcMatchesNan(Condition cc)
{
    switch (cc) {
        case Condition::NE:
        case Condition::LT:
        case Condition::LE:
        case Condition::HI:
        case Condition::HS:
            return true;

        default:
            return false;
    }
}

/// Converters
static asmjit::x86::Gp ArchReg(Reg reg, uint8_t size = 0)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        size_t regSize = size == 0 ? reg.GetSize() : size;
        auto archId = ConvertRegNumber(reg.GetId());

        asmjit::x86::Gp archReg;
        switch (regSize) {
            case DOUBLE_WORD_SIZE:
                archReg = asmjit::x86::Gp(asmjit::x86::Gpq::kSignature, archId);
                break;
            case WORD_SIZE:
                archReg = asmjit::x86::Gp(asmjit::x86::Gpd::kSignature, archId);
                break;
            case HALF_SIZE:
                archReg = asmjit::x86::Gp(asmjit::x86::Gpw::kSignature, archId);
                break;
            case BYTE_SIZE:
                archReg = asmjit::x86::Gp(asmjit::x86::GpbLo::kSignature, archId);
                break;

            default:
                UNREACHABLE();
        }

        ASSERT(archReg.isValid());
        return archReg;
    }
    if (reg.GetId() == ConvertRegNumber(asmjit::x86::rsp.id())) {
        return asmjit::x86::rsp;
    }

    // Invalid register type
    UNREACHABLE();
    return asmjit::x86::rax;
}

static asmjit::x86::Xmm ArchVReg(Reg reg)
{
    ASSERT(reg.IsValid() && reg.IsFloat());
    auto archVreg = asmjit::x86::xmm(reg.GetId());
    return archVreg;
}

static asmjit::Imm ArchImm(Imm imm)
{
    ASSERT(imm.GetType() == INT64_TYPE);
    return asmjit::imm(imm.GetAsInt());
}

static uint64_t ImmToUnsignedInt(Imm imm)
{
    ASSERT(imm.GetType() == INT64_TYPE);
    return uint64_t(imm.GetAsInt());
}

static bool ImmFitsSize(int64_t imm, uint8_t size)
{
    if (size == DOUBLE_WORD_SIZE) {
        size = WORD_SIZE;
    }

    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
    int64_t max = (uint64_t(1) << (size - 1U)) - 1U;
    int64_t min = ~uint64_t(max);
    ASSERT(min < 0);
    ASSERT(max > 0);

    return imm >= min && imm <= max;
}

LabelHolder::LabelId Amd64LabelHolder::CreateLabel()
{
    ++id_;

    auto masm = (static_cast<Amd64Encoder *>(GetEncoder()))->GetMasm();
    auto label = masm->newLabel();

    auto allocator = GetEncoder()->GetAllocator();
    labels_.push_back(allocator->New<LabelType>(std::move(label)));
    ASSERT(labels_.size() == id_);
    return id_ - 1;
}

ArchMem::ArchMem(MemRef mem)
{
    bool base = mem.HasBase();
    bool regoffset = mem.HasIndex();
    bool shift = mem.HasScale();
    bool offset = mem.HasDisp();

    if (base && !regoffset && !shift) {
        // Default memory - base + offset
        mem_ = asmjit::x86::ptr(ArchReg(mem.GetBase()), mem.GetDisp());
    } else if (base && regoffset && !offset) {
        auto baseSize = mem.GetBase().GetSize();
        auto indexSize = mem.GetIndex().GetSize();

        ASSERT(baseSize >= indexSize);
        ASSERT(indexSize >= WORD_SIZE);

        if (baseSize > indexSize) {
            needExtendIndex_ = true;
        }

        if (mem.GetScale() == 0) {
            mem_ = asmjit::x86::ptr(ArchReg(mem.GetBase()), ArchReg(mem.GetIndex(), baseSize));
        } else {
            auto scale = mem.GetScale();
            if (scale <= 3U) {
                mem_ = asmjit::x86::ptr(ArchReg(mem.GetBase()), ArchReg(mem.GetIndex(), baseSize), scale);
            } else {
                mem_ = asmjit::x86::ptr(ArchReg(mem.GetBase()), ArchReg(mem.GetIndex(), baseSize));
                bigShift_ = scale;
            }
        }
    } else {
        // Wrong memRef
        UNREACHABLE();
    }
}

asmjit::x86::Mem ArchMem::Prepare(asmjit::x86::Assembler *masm)
{
    if (isPrepared_) {
        return mem_;
    }

    if (bigShift_ != 0) {
        ASSERT(!mem_.hasOffset() && mem_.hasIndex() && bigShift_ > 3U);
        masm->shl(mem_.indexReg().as<asmjit::x86::Gp>(), asmjit::imm(bigShift_));
    }

    if (needExtendIndex_) {
        ASSERT(mem_.hasIndex());
        auto qIndex = mem_.indexReg().as<asmjit::x86::Gp>();
        auto dIndex {qIndex};
        dIndex.setSignature(asmjit::x86::Gpd::kSignature);
        masm->movsxd(qIndex, dIndex);
    }

    isPrepared_ = true;
    return mem_;
}

AsmJitErrorHandler::AsmJitErrorHandler(Encoder *encoder) : encoder_(encoder)
{
    ASSERT(encoder != nullptr);
}

void AsmJitErrorHandler::handleError([[maybe_unused]] asmjit::Error err, [[maybe_unused]] const char *message,
                                     [[maybe_unused]] asmjit::BaseEmitter *origin)
{
    encoder_->SetFalseResult();
}

void Amd64LabelHolder::CreateLabels(LabelId max)
{
    for (LabelId i = 0; i < max; ++i) {
        CreateLabel();
    }
}

Amd64LabelHolder::LabelType *Amd64LabelHolder::GetLabel(LabelId id)
{
    ASSERT(labels_.size() > id);
    return labels_[id];
}

Amd64LabelHolder::LabelId Amd64LabelHolder::Size()
{
    return labels_.size();
}

void Amd64LabelHolder::BindLabel(LabelId id)
{
    static_cast<Amd64Encoder *>(GetEncoder())->GetMasm()->bind(*labels_[id]);
}

Amd64Encoder::Amd64Encoder(ArenaAllocator *allocator) : Encoder(allocator, Arch::X86_64, false) {}

Amd64Encoder::~Amd64Encoder()
{
    if (masm_ != nullptr) {
        masm_->~Assembler();
        masm_ = nullptr;
    }

    if (codeHolder_ != nullptr) {
        codeHolder_->~CodeHolder();
        codeHolder_ = nullptr;
    }

    if (errorHandler_ != nullptr) {
        errorHandler_->~ErrorHandler();
        errorHandler_ = nullptr;
    }

    if (labels_ != nullptr) {
        labels_->~Amd64LabelHolder();
        labels_ = nullptr;
    }
}

LabelHolder *Amd64Encoder::GetLabels() const
{
    ASSERT(labels_ != nullptr);
    return labels_;
}

bool Amd64Encoder::IsValid() const
{
    return true;
}

constexpr auto Amd64Encoder::GetTarget()
{
    return ark::compiler::Target(Arch::X86_64);
}

bool Amd64Encoder::InitMasm()
{
    if (masm_ == nullptr) {
        labels_ = GetAllocator()->New<Amd64LabelHolder>(this);
        if (labels_ == nullptr) {
            SetFalseResult();
            return false;
        }

        asmjit::Environment env;
        env.setArch(asmjit::Environment::kArchX64);

        codeHolder_ = GetAllocator()->New<asmjit::CodeHolder>(GetAllocator());
        if (codeHolder_ == nullptr) {
            SetFalseResult();
            return false;
        }
        codeHolder_->init(env, 0U);

        masm_ = GetAllocator()->New<asmjit::x86::Assembler>(codeHolder_);
        if (masm_ == nullptr) {
            SetFalseResult();
            return false;
        }

        // Enable strict validation.
        masm_->addValidationOptions(asmjit::BaseEmitter::kValidationOptionAssembler);
        errorHandler_ = GetAllocator()->New<AsmJitErrorHandler>(this);
        if (errorHandler_ == nullptr) {
            SetFalseResult();
            return false;
        }
        masm_->setErrorHandler(errorHandler_);

        // Make sure that the compiler uses the same scratch registers as the assembler
        CHECK_EQ(compiler::arch_info::x86_64::TEMP_REGS, GetTarget().GetTempRegsMask());
        CHECK_EQ(compiler::arch_info::x86_64::TEMP_FP_REGS, GetTarget().GetTempVRegsMask());
    }
    return true;
}

void Amd64Encoder::Finalize()
{
    auto code = GetMasm()->code();
    auto codeSize = code->codeSize();

    code->flatten();
    code->resolveUnresolvedLinks();

    auto codeBuffer = GetAllocator()->Alloc(codeSize);

    code->relocateToBase(reinterpret_cast<uintptr_t>(codeBuffer));
    code->copyFlattenedData(codeBuffer, codeSize, asmjit::CodeHolder::kCopyPadSectionBuffer);
}

void Amd64Encoder::EncodeJump(LabelHolder::LabelId id)
{
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->jmp(*label);
}

void Amd64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    if (src0.IsScalar()) {
        if (src0.GetSize() == src1.GetSize()) {
            GetMasm()->cmp(ArchReg(src0), ArchReg(src1));
        } else if (src0.GetSize() > src1.GetSize()) {
            ScopedTmpReg tmpReg(this, src0.GetType());
            EncodeCast(tmpReg, false, src1, false);
            GetMasm()->cmp(ArchReg(src0), ArchReg(tmpReg));
        } else {
            ScopedTmpReg tmpReg(this, src1.GetType());
            EncodeCast(tmpReg, false, src0, false);
            GetMasm()->cmp(ArchReg(tmpReg), ArchReg(src1));
        }
    } else if (src0.GetType() == FLOAT32_TYPE) {
        GetMasm()->comiss(ArchVReg(src0), ArchVReg(src1));
    } else {
        GetMasm()->comisd(ArchVReg(src0), ArchVReg(src1));
    }

    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    if (src0.IsScalar()) {
        GetMasm()->j(ArchCc(cc, src0.IsFloat()), *label);
        return;
    }

    if (CcMatchesNan(cc)) {
        GetMasm()->jp(*label);
        GetMasm()->j(ArchCc(cc, src0.IsFloat()), *label);
    } else {
        auto end = GetMasm()->newLabel();

        GetMasm()->jp(end);
        GetMasm()->j(ArchCc(cc, src0.IsFloat()), *label);
        GetMasm()->bind(end);
    }
}

void Amd64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    ASSERT(src.IsScalar());

    auto immVal = imm.GetAsInt();
    if (immVal == 0) {
        EncodeJump(id, src, cc);
        return;
    }

    if (ImmFitsSize(immVal, src.GetSize())) {
        auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);

        GetMasm()->cmp(ArchReg(src), asmjit::imm(immVal));
        GetMasm()->j(ArchCc(cc), *label);
    } else {
        ScopedTmpReg tmpReg(this, src.GetType());
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(immVal));
        EncodeJump(id, src, tmpReg, cc);
    }
}

void Amd64Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsScalar());
    if (src0.GetSize() == src1.GetSize()) {
        GetMasm()->test(ArchReg(src0), ArchReg(src1));
    } else if (src0.GetSize() > src1.GetSize()) {
        ScopedTmpReg tmpReg(this, src0.GetType());
        EncodeCast(tmpReg, false, src1, false);
        GetMasm()->test(ArchReg(src0), ArchReg(tmpReg));
    } else {
        ScopedTmpReg tmpReg(this, src1.GetType());
        EncodeCast(tmpReg, false, src0, false);
        GetMasm()->test(ArchReg(tmpReg), ArchReg(src1));
    }

    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->j(ArchCcTest(cc), *label);
}

void Amd64Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    ASSERT(src.IsScalar());

    auto immVal = imm.GetAsInt();
    if (ImmFitsSize(immVal, src.GetSize())) {
        auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);

        GetMasm()->test(ArchReg(src), asmjit::imm(immVal));
        GetMasm()->j(ArchCcTest(cc), *label);
    } else {
        ScopedTmpReg tmpReg(this, src.GetType());
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(immVal));
        EncodeJumpTest(id, src, tmpReg, cc);
    }
}

void Amd64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Condition cc)
{
    if (src.IsScalar()) {
        auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);

        GetMasm()->cmp(ArchReg(src), asmjit::imm(0));
        GetMasm()->j(ArchCc(cc), *label);
        return;
    }

    ScopedTmpReg tmpReg(this, src.GetType());
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->xorps(ArchVReg(tmpReg), ArchVReg(tmpReg));
    } else {
        GetMasm()->xorpd(ArchVReg(tmpReg), ArchVReg(tmpReg));
    }
    EncodeJump(id, src, tmpReg, cc);
}

void Amd64Encoder::EncodeJump(Reg dst)
{
    GetMasm()->jmp(ArchReg(dst));
}

void Amd64Encoder::EncodeJump(RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::array<uint8_t, 5U> data = {0xe9, 0, 0, 0, 0};
    GetMasm()->embed(data.data(), data.size());

    constexpr int ADDEND = 4;
    relocation->offset = GetCursorOffset() - ADDEND;
    relocation->addend = -ADDEND;
    relocation->type = R_X86_64_PLT32;
#endif
}

void Amd64Encoder::EncodeBitTestAndBranch(LabelHolder::LabelId id, compiler::Reg reg, uint32_t bitPos, bool bitValue)
{
    ASSERT(reg.IsScalar() && reg.GetSize() > bitPos);
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    if (reg.GetSize() == DOUBLE_WORD_SIZE) {
        ScopedTmpRegU64 tmpReg(this);
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(static_cast<uint64_t>(1) << bitPos));
        GetMasm()->test(ArchReg(reg), ArchReg(tmpReg));
    } else {
        GetMasm()->test(ArchReg(reg), asmjit::imm(1U << bitPos));
    }
    if (bitValue) {
        GetMasm()->j(ArchCc(Condition::NE), *label);
    } else {
        GetMasm()->j(ArchCc(Condition::EQ), *label);
    }
}

void Amd64Encoder::MakeCall([[maybe_unused]] compiler::RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::array<uint8_t, 5U> data = {0xe8, 0, 0, 0, 0};
    GetMasm()->embed(data.data(), data.size());

    relocation->offset = GetCursorOffset() - 4_I;
    relocation->addend = -4_I;
    relocation->type = R_X86_64_PLT32;
#endif
}

void Amd64Encoder::MakeCall(LabelHolder::LabelId id)
{
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->call(*label);
}

void Amd64Encoder::MakeCall(const void *entryPoint)
{
    ScopedTmpRegU64 tmpReg(this);
    GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(entryPoint));
    GetMasm()->call(ArchReg(tmpReg));
}

void Amd64Encoder::MakeCall(Reg reg)
{
    GetMasm()->call(ArchReg(reg));
}

void Amd64Encoder::MakeCall(MemRef entryPoint)
{
    ScopedTmpRegU64 tmpReg(this);
    EncodeLdr(tmpReg, false, entryPoint);
    GetMasm()->call(ArchReg(tmpReg));
}

template <typename Func>
void Amd64Encoder::EncodeRelativePcMov(Reg reg, intptr_t offset, Func encodeInstruction)
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    auto pos = GetMasm()->offset();
    encodeInstruction(reg, offset);
    // NOLINTNEXTLINE(readability-identifier-naming)
    offset -= (GetMasm()->offset() - pos);
    // NOLINTNEXTLINE(readability-identifier-naming)
    GetMasm()->setOffset(pos);
    encodeInstruction(reg, offset);
}

void Amd64Encoder::MakeCallAot(intptr_t offset)
{
    ScopedTmpRegU64 tmpReg(this);
    EncodeRelativePcMov(tmpReg, offset, [this](Reg reg, intptr_t offset) {
        GetMasm()->long_().mov(ArchReg(reg), asmjit::x86::ptr(asmjit::x86::rip, offset));
    });
    GetMasm()->call(ArchReg(tmpReg));
}

bool Amd64Encoder::CanMakeCallByOffset(intptr_t offset)
{
    return offset == static_cast<intptr_t>(static_cast<int32_t>(offset));
}

void Amd64Encoder::MakeCallByOffset(intptr_t offset)
{
    GetMasm()->call(GetCursorOffset() + int32_t(offset));
}

void Amd64Encoder::MakeLoadAotTable(intptr_t offset, Reg reg)
{
    EncodeRelativePcMov(reg, offset, [this](Reg reg, intptr_t offset) {
        GetMasm()->long_().mov(ArchReg(reg), asmjit::x86::ptr(asmjit::x86::rip, offset));
    });
}

void Amd64Encoder::MakeLoadAotTableAddr([[maybe_unused]] intptr_t offset, [[maybe_unused]] Reg addr,
                                        [[maybe_unused]] Reg val)
{
    EncodeRelativePcMov(addr, offset, [this](Reg reg, intptr_t offset) {
        GetMasm()->long_().lea(ArchReg(reg), asmjit::x86::ptr(asmjit::x86::rip, offset));
    });
    GetMasm()->mov(ArchReg(val), asmjit::x86::ptr(ArchReg(addr)));
}

void Amd64Encoder::EncodeAbort()
{
    GetMasm()->int3();
}

void Amd64Encoder::EncodeReturn()
{
    GetMasm()->ret();
}

void Amd64Encoder::EncodeMul([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src, [[maybe_unused]] Imm imm)
{
    SetFalseResult();
}

void Amd64Encoder::EncodeNop()
{
    GetMasm()->nop();
}

void Amd64Encoder::EncodeMov(Reg dst, Reg src)
{
    if (dst == src) {
        return;
    }

    if (dst.IsFloat() != src.IsFloat()) {
        ASSERT(src.GetSize() == dst.GetSize());
        if (dst.GetSize() == WORD_SIZE) {
            if (dst.IsFloat()) {
                GetMasm()->movd(ArchVReg(dst), ArchReg(src));
            } else {
                GetMasm()->movd(ArchReg(dst), ArchVReg(src));
            }
        } else {
            ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
            if (dst.IsFloat()) {
                GetMasm()->movq(ArchVReg(dst), ArchReg(src));
            } else {
                GetMasm()->movq(ArchReg(dst), ArchVReg(src));
            }
        }
        return;
    }

    if (dst.IsFloat()) {
        ASSERT(src.IsFloat());
        if (dst.GetType() == FLOAT32_TYPE) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src));
        } else {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        return;
    }

    if (dst.GetSize() < WORD_SIZE && dst.GetSize() == src.GetSize()) {
        GetMasm()->xor_(ArchReg(dst, WORD_SIZE), ArchReg(dst, WORD_SIZE));
    }

    if (dst.GetSize() == src.GetSize()) {
        GetMasm()->mov(ArchReg(dst), ArchReg(src));
    } else {
        EncodeCast(dst, false, src, false);
    }
}

void Amd64Encoder::EncodeNeg(Reg dst, Reg src)
{
    if (dst.IsScalar()) {
        EncodeMov(dst, src);
        GetMasm()->neg(ArchReg(dst));
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF32 tmp(this);
        CopyImmToXmm(tmp, -0.0F);

        if (dst.GetId() != src.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        GetMasm()->xorps(ArchVReg(dst), ArchVReg(tmp));
    } else {
        ScopedTmpRegF64 tmp(this);
        CopyImmToXmm(tmp, -0.0);

        if (dst.GetId() != src.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        GetMasm()->xorps(ArchVReg(dst), ArchVReg(tmp));
    }
}

void Amd64Encoder::EncodeAbs(Reg dst, Reg src)
{
    if (dst.IsScalar()) {
        auto size = std::max<uint8_t>(src.GetSize(), WORD_SIZE);

        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), ArchReg(src));
            GetMasm()->neg(ArchReg(dst));
            GetMasm()->cmovl(ArchReg(dst, size), ArchReg(src, size));
        } else if (GetScratchRegistersCount() > 0) {
            ScopedTmpReg tmpReg(this, dst.GetType());

            GetMasm()->mov(ArchReg(tmpReg), ArchReg(src));
            GetMasm()->neg(ArchReg(tmpReg));

            GetMasm()->cmovl(ArchReg(tmpReg, size), ArchReg(src, size));
            GetMasm()->mov(ArchReg(dst), ArchReg(tmpReg));
        } else {
            auto end = GetMasm()->newLabel();

            GetMasm()->test(ArchReg(dst), ArchReg(dst));
            GetMasm()->jns(end);

            GetMasm()->neg(ArchReg(dst));
            GetMasm()->bind(end);
        }
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF32 tmp(this);
        // NOLINTNEXTLINE(readability-magic-numbers)
        CopyImmToXmm(tmp, uint32_t(0x7fffffff));

        if (dst.GetId() != src.GetId()) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src));
        }
        GetMasm()->andps(ArchVReg(dst), ArchVReg(tmp));
    } else {
        ScopedTmpRegF64 tmp(this);
        // NOLINTNEXTLINE(readability-magic-numbers)
        CopyImmToXmm(tmp, uint64_t(0x7fffffffffffffff));

        if (dst.GetId() != src.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        GetMasm()->andps(ArchVReg(dst), ArchVReg(tmp));
    }
}

void Amd64Encoder::EncodeNot(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar());

    EncodeMov(dst, src);
    GetMasm()->not_(ArchReg(dst));
}

void Amd64Encoder::EncodeSqrt(Reg dst, Reg src)
{
    ASSERT(dst.IsFloat());
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->sqrtps(ArchVReg(dst), ArchVReg(src));
    } else {
        GetMasm()->sqrtpd(ArchVReg(dst), ArchVReg(src));
    }
}

void Amd64Encoder::EncodeCastFloatToScalar(Reg dst, bool dstSigned, Reg src)
{
    // We DON'T support casts from float32/64 to int8/16 and bool, because this caste is not declared anywhere
    // in other languages and architecture, we do not know what the behavior should be.
    ASSERT(dst.GetSize() >= WORD_SIZE);
    auto end = GetMasm()->newLabel();

    // if src is NaN, then dst = 0
    EncodeCastFloatCheckNan(dst, src, end);

    if (dstSigned) {
        EncodeCastFloatSignCheckRange(dst, src, end);
    } else {
        EncodeCastFloatUnsignCheckRange(dst, src, end);
    }

    if (src.GetType() == FLOAT32_TYPE) {
        if (dst.GetSize() == DOUBLE_WORD_SIZE) {
            EncodeCastFloat32ToUint64(dst, src);
        } else {
            GetMasm()->cvttss2si(ArchReg(dst, DOUBLE_WORD_SIZE), ArchVReg(src));
        }
    } else {
        if (dst.GetSize() == DOUBLE_WORD_SIZE) {
            EncodeCastFloat64ToUint64(dst, src);
        } else {
            GetMasm()->cvttsd2si(ArchReg(dst, DOUBLE_WORD_SIZE), ArchVReg(src));
        }
    }

    GetMasm()->bind(end);
}

void Amd64Encoder::EncodeCastFloat32ToUint64(Reg dst, Reg src)
{
    auto bigNumberLabel = GetMasm()->newLabel();
    auto endLabel = GetMasm()->newLabel();
    ScopedTmpReg tmpReg(this, src.GetType());
    ScopedTmpReg tmpNum(this, dst.GetType());

    // It is max number with max degree that we can load in sign int64
    // NOLINTNEXTLINE (readability-magic-numbers)
    GetMasm()->mov(ArchReg(dst, WORD_SIZE), asmjit::imm(0x5F000000));
    GetMasm()->movd(ArchVReg(tmpReg), ArchReg(dst, WORD_SIZE));
    GetMasm()->comiss(ArchVReg(src), ArchVReg(tmpReg));
    GetMasm()->jnb(bigNumberLabel);

    GetMasm()->cvttss2si(ArchReg(dst), ArchVReg(src));
    GetMasm()->jmp(endLabel);

    GetMasm()->bind(bigNumberLabel);
    GetMasm()->subss(ArchVReg(src), ArchVReg(tmpReg));
    GetMasm()->cvttss2si(ArchReg(dst), ArchVReg(src));
    // NOLINTNEXTLINE (readability-magic-numbers)
    GetMasm()->mov(ArchReg(tmpNum), asmjit::imm(0x8000000000000000));
    GetMasm()->xor_(ArchReg(dst), ArchReg(tmpNum));
    GetMasm()->bind(endLabel);
}

void Amd64Encoder::EncodeCastFloat64ToUint64(Reg dst, Reg src)
{
    auto bigNumberLabel = GetMasm()->newLabel();
    auto endLabel = GetMasm()->newLabel();
    ScopedTmpReg tmpReg(this, src.GetType());
    ScopedTmpReg tmpNum(this, dst.GetType());

    // It is max number with max degree that we can load in sign int64
    // NOLINTNEXTLINE (readability-magic-numbers)
    GetMasm()->mov(ArchReg(dst), asmjit::imm(0x43E0000000000000));
    GetMasm()->movq(ArchVReg(tmpReg), ArchReg(dst));
    GetMasm()->comisd(ArchVReg(src), ArchVReg(tmpReg));
    GetMasm()->jnb(bigNumberLabel);

    GetMasm()->cvttsd2si(ArchReg(dst), ArchVReg(src));
    GetMasm()->jmp(endLabel);

    GetMasm()->bind(bigNumberLabel);
    GetMasm()->subsd(ArchVReg(src), ArchVReg(tmpReg));
    GetMasm()->cvttsd2si(ArchReg(dst), ArchVReg(src));
    // NOLINTNEXTLINE (readability-magic-numbers)
    GetMasm()->mov(ArchReg(tmpNum), asmjit::imm(0x8000000000000000));
    GetMasm()->xor_(ArchReg(dst), ArchReg(tmpNum));
    GetMasm()->bind(endLabel);
}

void Amd64Encoder::EncodeCastFloatCheckNan(Reg dst, Reg src, const asmjit::Label &end)
{
    GetMasm()->xor_(ArchReg(dst, DOUBLE_WORD_SIZE), ArchReg(dst, DOUBLE_WORD_SIZE));
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->ucomiss(ArchVReg(src), ArchVReg(src));
    } else {
        GetMasm()->ucomisd(ArchVReg(src), ArchVReg(src));
    }
    GetMasm()->jp(end);
}

void Amd64Encoder::EncodeCastFloatSignCheckRange(Reg dst, Reg src, const asmjit::Label &end)
{
    // if src < INT_MIN, then dst = INT_MIN
    // if src >= (INT_MAX + 1), then dst = INT_MAX
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        EncodeCastFloatCheckRange(dst, src, end, INT64_MIN, INT64_MAX);
    } else {
        EncodeCastFloatCheckRange(dst, src, end, INT32_MIN, INT32_MAX);
    }
}

void Amd64Encoder::EncodeCastFloatCheckRange(Reg dst, Reg src, const asmjit::Label &end, const int64_t minValue,
                                             const uint64_t maxValue)
{
    ScopedTmpReg cmpReg(this, src.GetType());
    ScopedTmpReg tmpReg(this, src.GetType() == FLOAT64_TYPE ? INT64_TYPE : INT32_TYPE);

    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(minValue));
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(bit_cast<uint32_t>(float(minValue))));
        GetMasm()->movd(ArchVReg(cmpReg), ArchReg(tmpReg));
        GetMasm()->ucomiss(ArchVReg(src), ArchVReg(cmpReg));
    } else {
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(bit_cast<uint64_t>(double(minValue))));
        GetMasm()->movq(ArchVReg(cmpReg), ArchReg(tmpReg));
        GetMasm()->ucomisd(ArchVReg(src), ArchVReg(cmpReg));
    }
    GetMasm()->jb(end);

    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(maxValue));
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(bit_cast<uint32_t>(float(maxValue) + 1U)));
        GetMasm()->movd(ArchVReg(cmpReg), ArchReg(tmpReg));
        GetMasm()->ucomiss(ArchVReg(src), ArchVReg(cmpReg));
    } else {
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(bit_cast<uint64_t>(double(maxValue) + 1U)));
        GetMasm()->movq(ArchVReg(cmpReg), ArchReg(tmpReg));
        GetMasm()->ucomisd(ArchVReg(src), ArchVReg(cmpReg));
    }
    GetMasm()->jae(end);
}

void Amd64Encoder::EncodeCastFloatUnsignCheckRange(Reg dst, Reg src, const asmjit::Label &end)
{
    // if src < 0, then dst = 0
    // if src >= (UINT_MAX + 1), then dst = UINT_MAX
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        EncodeCastFloatCheckRange(dst, src, end, 0, UINT64_MAX);
    } else {
        EncodeCastFloatCheckRange(dst, src, end, 0, UINT32_MAX);
    }
}

void Amd64Encoder::EncodeCastScalarToFloatUnsignDouble(Reg dst, Reg src)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegU64 int1Reg(this);
        ScopedTmpRegU64 int2Reg(this);

        auto sgn = GetMasm()->newLabel();
        auto end = GetMasm()->newLabel();

        GetMasm()->test(ArchReg(src), ArchReg(src));
        GetMasm()->js(sgn);
        GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(src));
        GetMasm()->jmp(end);

        GetMasm()->bind(sgn);
        GetMasm()->mov(ArchReg(int1Reg), ArchReg(src));
        GetMasm()->mov(ArchReg(int2Reg), ArchReg(src));
        GetMasm()->shr(ArchReg(int2Reg), asmjit::imm(1));
        GetMasm()->and_(ArchReg(int1Reg, WORD_SIZE), asmjit::imm(1));
        GetMasm()->or_(ArchReg(int1Reg), ArchReg(int2Reg));
        GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(int1Reg));
        GetMasm()->addss(ArchVReg(dst), ArchVReg(dst));

        GetMasm()->bind(end);
    } else {
        static constexpr std::array<uint32_t, 4> ARR1 = {uint32_t(0x43300000), uint32_t(0x45300000), 0x0, 0x0};
        static constexpr std::array<uint64_t, 2> ARR2 = {uint64_t(0x4330000000000000), uint64_t(0x4530000000000000)};

        ScopedTmpReg float1Reg(this, dst.GetType());
        ScopedTmpRegF64 tmp(this);

        GetMasm()->movq(ArchVReg(float1Reg), ArchReg(src));
        CopyArrayToXmm(tmp, ARR1);
        GetMasm()->punpckldq(ArchVReg(float1Reg), ArchVReg(tmp));
        CopyArrayToXmm(tmp, ARR2);
        GetMasm()->subpd(ArchVReg(float1Reg), ArchVReg(tmp));
        GetMasm()->movapd(ArchVReg(dst), ArchVReg(float1Reg));
        GetMasm()->unpckhpd(ArchVReg(dst), ArchVReg(float1Reg));
        GetMasm()->addsd(ArchVReg(dst), ArchVReg(float1Reg));
    }
}

void Amd64Encoder::EncodeCastScalarToFloat(Reg dst, Reg src, bool srcSigned)
{
    if (!srcSigned && src.GetSize() == DOUBLE_WORD_SIZE) {
        EncodeCastScalarToFloatUnsignDouble(dst, src);
        return;
    }

    if (src.GetSize() < WORD_SIZE || (srcSigned && src.GetSize() == WORD_SIZE)) {
        if (dst.GetType() == FLOAT32_TYPE) {
            GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(src, WORD_SIZE));
        } else {
            GetMasm()->cvtsi2sd(ArchVReg(dst), ArchReg(src, WORD_SIZE));
        }
        return;
    }

    if (!srcSigned && src.GetSize() == WORD_SIZE) {
        ScopedTmpRegU64 int1Reg(this);

        GetMasm()->mov(ArchReg(int1Reg, WORD_SIZE), ArchReg(src, WORD_SIZE));
        if (dst.GetType() == FLOAT32_TYPE) {
            GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(int1Reg));
        } else {
            GetMasm()->cvtsi2sd(ArchVReg(dst), ArchReg(int1Reg));
        }
        return;
    }

    ASSERT(srcSigned && src.GetSize() == DOUBLE_WORD_SIZE);
    if (dst.GetType() == FLOAT32_TYPE) {
        GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(src));
    } else {
        GetMasm()->cvtsi2sd(ArchVReg(dst), ArchReg(src));
    }
}

void Amd64Encoder::EncodeCastToBool(Reg dst, Reg src)
{
    // In ISA says that we only support casts:
    // i32tou1, i64tou1, u32tou1, u64tou1
    ASSERT(src.IsScalar());
    ASSERT(dst.IsScalar());

    // In our ISA minimal type is 32-bit, so bool in 32bit
    GetMasm()->test(ArchReg(src), ArchReg(src));
    // One "mov" will be better, then 2 jump. Else other instructions will overwrite the flags.
    GetMasm()->mov(ArchReg(dst, WORD_SIZE), asmjit::imm(0));
    GetMasm()->setne(ArchReg(dst));
}

void Amd64Encoder::EncodeFastPathDynamicCast(Reg dst, Reg src, LabelHolder::LabelId slow)
{
    ASSERT(IsLabelValid(slow));
    ASSERT(src.IsFloat() && dst.IsScalar());

    CHECK_EQ(src.GetSize(), BITS_PER_UINT64);
    CHECK_EQ(dst.GetSize(), BITS_PER_UINT32);

    auto end {GetMasm()->newLabel()};

    // if src is NaN, then dst = 0
    EncodeCastFloatCheckNan(dst, src, end);

    // infinite and big numbers will overflow here to INT64_MIN
    GetMasm()->cvttsd2si(ArchReg(dst, DOUBLE_WORD_SIZE), ArchVReg(src));
    // check INT64_MIN
    GetMasm()->cmp(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(1));
    auto slowLabel {static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(slow)};
    // jump to slow path in case of overflow
    GetMasm()->jo(*slowLabel);

    GetMasm()->bind(end);
}

void Amd64Encoder::EncodeJsDoubleToCharCast(Reg dst, Reg src, Reg tmp, uint32_t failureResult)
{
    ASSERT(src.IsFloat() && dst.IsScalar());

    CHECK_EQ(src.GetSize(), BITS_PER_UINT64);
    CHECK_EQ(dst.GetSize(), BITS_PER_UINT32);

    // infinite and big numbers will overflow here to INT64_MIN. If src is NaN, cvttsd2si itself returns zero.
    GetMasm()->cvttsd2si(ArchReg(dst, DOUBLE_WORD_SIZE), ArchVReg(src));
    // save the result to tmp
    GetMasm()->mov(ArchReg(tmp, DOUBLE_WORD_SIZE), ArchReg(dst, DOUBLE_WORD_SIZE));
    // 'and' the result with 0xffff
    constexpr uint32_t UTF16_CHAR_MASK = 0xffff;
    GetMasm()->and_(ArchReg(dst), asmjit::imm(UTF16_CHAR_MASK));
    // check INT64_MIN
    GetMasm()->cmp(ArchReg(tmp, DOUBLE_WORD_SIZE), asmjit::imm(1));
    // 'mov' never affects the flags
    GetMasm()->mov(ArchReg(tmp, DOUBLE_WORD_SIZE), failureResult);
    // ... and we may move conditionally the failureResult into dst for overflow only
    GetMasm()->cmovo(ArchReg(dst), ArchReg(tmp));
}

void Amd64Encoder::EncodeCast(Reg dst, bool dstSigned, Reg src, bool srcSigned)
{
    if (src.IsFloat() && dst.IsScalar()) {
        EncodeCastFloatToScalar(dst, dstSigned, src);
        return;
    }

    if (src.IsScalar() && dst.IsFloat()) {
        EncodeCastScalarToFloat(dst, src, srcSigned);
        return;
    }

    if (src.IsFloat() && dst.IsFloat()) {
        if (src.GetSize() != dst.GetSize()) {
            if (src.GetType() == FLOAT32_TYPE) {
                GetMasm()->cvtss2sd(ArchVReg(dst), ArchVReg(src));
            } else {
                GetMasm()->cvtsd2ss(ArchVReg(dst), ArchVReg(src));
            }
            return;
        }

        if (src.GetType() == FLOAT32_TYPE) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src));
        } else {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        return;
    }

    ASSERT(src.IsScalar() && dst.IsScalar());
    EncodeCastScalar(dst, dstSigned, src, srcSigned);
}

void Amd64Encoder::EncodeCastScalar(Reg dst, bool dstSigned, Reg src, bool srcSigned)
{
    auto extendTo32bit = [this](Reg reg, bool isSigned) {
        if (reg.GetSize() < WORD_SIZE) {
            if (isSigned) {
                GetMasm()->movsx(ArchReg(reg, WORD_SIZE), ArchReg(reg));
            } else {
                GetMasm()->movzx(ArchReg(reg, WORD_SIZE), ArchReg(reg));
            }
        }
    };

    if (src.GetSize() >= dst.GetSize()) {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), ArchReg(src, dst.GetSize()));
        }
        extendTo32bit(dst, dstSigned);
        return;
    }

    if (srcSigned) {
        if (dst.GetSize() < DOUBLE_WORD_SIZE) {
            GetMasm()->movsx(ArchReg(dst), ArchReg(src));
            extendTo32bit(dst, dstSigned);
        } else if (src.GetSize() == WORD_SIZE) {
            GetMasm()->movsxd(ArchReg(dst), ArchReg(src));
        } else {
            GetMasm()->movsx(ArchReg(dst, WORD_SIZE), ArchReg(src));
            GetMasm()->movsxd(ArchReg(dst), ArchReg(dst, WORD_SIZE));
        }
        return;
    }

    if (src.GetSize() == WORD_SIZE) {
        GetMasm()->mov(ArchReg(dst, WORD_SIZE), ArchReg(src));
    } else if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->movzx(ArchReg(dst, WORD_SIZE), ArchReg(src));
    } else {
        GetMasm()->movzx(ArchReg(dst), ArchReg(src));
        extendTo32bit(dst, dstSigned);
    }
}

Reg Amd64Encoder::MakeShift(Shift shift)
{
    Reg reg = shift.GetBase();
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        ASSERT(shift.GetType() != ShiftType::INVALID_SHIFT);
        switch (shift.GetType()) {
            case ShiftType::LSL:
                GetMasm()->shl(ArchReg(reg), asmjit::imm(shift.GetScale()));
                break;
            case ShiftType::LSR:
                GetMasm()->shr(ArchReg(reg), asmjit::imm(shift.GetScale()));
                break;
            case ShiftType::ASR:
                GetMasm()->sar(ArchReg(reg), asmjit::imm(shift.GetScale()));
                break;
            case ShiftType::ROR:
                GetMasm()->ror(ArchReg(reg), asmjit::imm(shift.GetScale()));
                break;
            default:
                UNREACHABLE();
        }

        return reg;
    }

    // Invalid register type
    UNREACHABLE();
}

void Amd64Encoder::EncodeAdd(Reg dst, Reg src0, Shift src1)
{
    if (dst.IsFloat()) {
        SetFalseResult();
        return;
    }

    ASSERT(dst.GetSize() >= src0.GetSize());

    auto shiftReg = MakeShift(src1);

    if (src0.GetSize() < WORD_SIZE) {
        EncodeAdd(dst, src0, shiftReg);
        return;
    }

    if (src0.GetSize() == DOUBLE_WORD_SIZE && shiftReg.GetSize() < DOUBLE_WORD_SIZE) {
        GetMasm()->movsxd(ArchReg(shiftReg, DOUBLE_WORD_SIZE), ArchReg(shiftReg));
    }

    GetMasm()->lea(ArchReg(dst), asmjit::x86::ptr(ArchReg(src0), ArchReg(shiftReg, src0.GetSize())));
}

void Amd64Encoder::EncodeAdd(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        auto size = std::max<uint8_t>(WORD_SIZE, dst.GetSize());
        GetMasm()->lea(ArchReg(dst, size), asmjit::x86::ptr(ArchReg(src0, size), ArchReg(src1, size)));
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->addss(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->addss(ArchVReg(dst), ArchVReg(src0));
        } else {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->addss(ArchVReg(dst), ArchVReg(src1));
        }
    } else {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->addsd(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->addsd(ArchVReg(dst), ArchVReg(src0));
        } else {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->addsd(ArchVReg(dst), ArchVReg(src1));
        }
    }
}

void Amd64Encoder::EncodeSub(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->sub(ArchReg(dst), ArchReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->sub(ArchReg(dst), ArchReg(src0));
            GetMasm()->neg(ArchReg(dst));
        } else {
            EncodeMov(dst, src0);
            GetMasm()->sub(ArchReg(dst), ArchReg(src1));
        }
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->subss(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() != src1.GetId()) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->subss(ArchVReg(dst), ArchVReg(src1));
        } else {
            ScopedTmpReg tmpReg(this, dst.GetType());
            GetMasm()->movss(ArchVReg(tmpReg), ArchVReg(src0));
            GetMasm()->subss(ArchVReg(tmpReg), ArchVReg(src1));
            GetMasm()->movss(ArchVReg(dst), ArchVReg(tmpReg));
        }
    } else {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->subsd(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() != src1.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->subsd(ArchVReg(dst), ArchVReg(src1));
        } else {
            ScopedTmpReg tmpReg(this, dst.GetType());
            GetMasm()->movsd(ArchVReg(tmpReg), ArchVReg(src0));
            GetMasm()->subsd(ArchVReg(tmpReg), ArchVReg(src1));
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(tmpReg));
        }
    }
}

void Amd64Encoder::EncodeMul(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        auto size = std::max<uint8_t>(WORD_SIZE, dst.GetSize());

        if (dst.GetId() == src0.GetId()) {
            GetMasm()->imul(ArchReg(dst, size), ArchReg(src1, size));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->imul(ArchReg(dst, size), ArchReg(src0, size));
        } else {
            GetMasm()->mov(ArchReg(dst, size), ArchReg(src0, size));
            GetMasm()->imul(ArchReg(dst, size), ArchReg(src1, size));
        }
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->mulss(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->mulss(ArchVReg(dst), ArchVReg(src0));
        } else {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->mulss(ArchVReg(dst), ArchVReg(src1));
        }
    } else {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->mulsd(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->mulsd(ArchVReg(dst), ArchVReg(src0));
        } else {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->mulsd(ArchVReg(dst), ArchVReg(src1));
        }
    }
}

void Amd64Encoder::EncodeAddOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(!dst.IsFloat() && !src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::VS || cc == Condition::VC);
    auto size = dst.GetSize();
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->add(ArchReg(dst, size), ArchReg(src1, size));
    } else if (dst.GetId() == src1.GetId()) {
        GetMasm()->add(ArchReg(dst, size), ArchReg(src0, size));
    } else {
        GetMasm()->mov(ArchReg(dst, size), ArchReg(src0, size));
        GetMasm()->add(ArchReg(dst, size), ArchReg(src1, size));
    }
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->j(ArchCc(cc, false), *label);
}

void Amd64Encoder::EncodeSubOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(!dst.IsFloat() && !src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::VS || cc == Condition::VC);
    auto size = dst.GetSize();
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->sub(ArchReg(dst, size), ArchReg(src1, size));
    } else if (dst.GetId() == src1.GetId()) {
        ScopedTmpReg tmpReg(this, dst.GetType());
        GetMasm()->mov(ArchReg(tmpReg, size), ArchReg(src1, size));
        GetMasm()->mov(ArchReg(dst, size), ArchReg(src0, size));
        GetMasm()->sub(ArchReg(dst, size), ArchReg(tmpReg, size));
    } else {
        GetMasm()->mov(ArchReg(dst, size), ArchReg(src0, size));
        GetMasm()->sub(ArchReg(dst, size), ArchReg(src1, size));
    }
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->j(ArchCc(cc, false), *label);
}

void Amd64Encoder::EncodeNegOverflowAndZero(compiler::LabelHolder::LabelId id, Reg dst, Reg src)
{
    ASSERT(!dst.IsFloat() && !src.IsFloat());
    auto size = dst.GetSize();
    // NOLINTNEXTLINE(readability-magic-numbers)
    EncodeJumpTest(id, src, Imm(0x7fffffff), Condition::TST_EQ);
    EncodeMov(dst, src);
    GetMasm()->neg(ArchReg(dst, size));
}

void Amd64Encoder::EncodeDivFloat(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsFloat());
    if (dst.GetType() == FLOAT32_TYPE) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->divss(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() != src1.GetId()) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->divss(ArchVReg(dst), ArchVReg(src1));
        } else {
            ScopedTmpRegF32 tmp(this);
            GetMasm()->movss(ArchVReg(tmp), ArchVReg(src0));
            GetMasm()->divss(ArchVReg(tmp), ArchVReg(src1));
            GetMasm()->movss(ArchVReg(dst), ArchVReg(tmp));
        }
    } else {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->divsd(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() != src1.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->divsd(ArchVReg(dst), ArchVReg(src1));
        } else {
            ScopedTmpRegF64 tmp(this);
            GetMasm()->movsd(ArchVReg(tmp), ArchVReg(src0));
            GetMasm()->divsd(ArchVReg(tmp), ArchVReg(src1));
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(tmp));
        }
    }
}

static void EncodeDivSpillDst(asmjit::x86::Assembler *masm, Reg dst)
{
    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rdx.id())) {
        masm->push(asmjit::x86::rdx);
    }
    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        masm->push(asmjit::x86::rax);
    }
}

static void EncodeDivFillDst(asmjit::x86::Assembler *masm, Reg dst)
{
    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        masm->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::x86::rax);
        masm->pop(asmjit::x86::rax);
    }

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rdx.id())) {
        masm->pop(asmjit::x86::rdx);
    }
}

void Amd64Encoder::EncodeDiv(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        EncodeDivFloat(dst, src0, src1);
        return;
    }

    auto negPath = GetMasm()->newLabel();
    auto crossroad = GetMasm()->newLabel();

    if (dstSigned) {
        GetMasm()->cmp(ArchReg(src1), asmjit::imm(-1));
        GetMasm()->je(negPath);
    }

    EncodeDivSpillDst(GetMasm(), dst);

    ScopedTmpReg tmpReg(this, dst.GetType());
    Reg op1 {src1};
    if (src1.GetId() == ConvertRegNumber(asmjit::x86::rax.id()) ||
        src1.GetId() == ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->mov(ArchReg(tmpReg), ArchReg(src1));
        op1 = Reg(tmpReg);
    }

    if (src0.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->mov(asmjit::x86::rax, ArchReg(src0, DOUBLE_WORD_SIZE));
    }
    if (dstSigned) {
        if (dst.GetSize() <= WORD_SIZE) {
            GetMasm()->cdq();
        } else {
            GetMasm()->cqo();
        }
        GetMasm()->idiv(ArchReg(op1));
    } else {
        GetMasm()->xor_(asmjit::x86::rdx, asmjit::x86::rdx);
        GetMasm()->div(ArchReg(op1));
    }

    EncodeDivFillDst(GetMasm(), dst);

    GetMasm()->jmp(crossroad);

    GetMasm()->bind(negPath);
    if (dst.GetId() != src0.GetId()) {
        GetMasm()->mov(ArchReg(dst), ArchReg(src0));
    }
    GetMasm()->neg(ArchReg(dst));

    GetMasm()->bind(crossroad);
}

void Amd64Encoder::EncodeSignedDiv(Reg dst, Reg src0, Imm imm)
{
    int64_t divisor = imm.GetAsInt();

    Reg ax(ConvertRegNumber(asmjit::x86::rax.id()), dst.GetType());
    Reg dx(ConvertRegNumber(asmjit::x86::rdx.id()), dst.GetType());

    if (dst != ax) {
        GetMasm()->push(ArchReg(ax, DOUBLE_WORD_SIZE));
    }
    if (dst != dx) {
        GetMasm()->push(ArchReg(dx, DOUBLE_WORD_SIZE));
    }

    FastConstSignedDivisor fastDivisor(divisor, dst.GetSize());
    int64_t magic = fastDivisor.GetMagic();

    ScopedTmpReg tmp(this, dst.GetType());
    EncodeMov(tmp, src0);
    EncodeMov(ax, src0);
    EncodeMov(dx, Imm(magic));
    GetMasm()->imul(ArchReg(dx));

    if (divisor > 0 && magic < 0) {
        EncodeAdd(dx, dx, tmp);
    } else if (divisor < 0 && magic > 0) {
        EncodeSub(dx, dx, tmp);
    }

    int64_t shift = fastDivisor.GetShift();
    EncodeAShr(dst, dx, Imm(shift));

    // result = (result < 0 ? result + 1 : result)
    EncodeShr(tmp, dst, Imm(dst.GetSize() - 1U));
    EncodeAdd(dst, dst, tmp);

    if (dst != dx) {
        GetMasm()->pop(ArchReg(dx, DOUBLE_WORD_SIZE));
    }
    if (dst != ax) {
        GetMasm()->pop(ArchReg(ax, DOUBLE_WORD_SIZE));
    }
}

void Amd64Encoder::EncodeUnsignedDiv(Reg dst, Reg src0, Imm imm)
{
    auto divisor = bit_cast<uint64_t>(imm.GetAsInt());

    Reg ax(ConvertRegNumber(asmjit::x86::rax.id()), dst.GetType());
    Reg dx(ConvertRegNumber(asmjit::x86::rdx.id()), dst.GetType());

    if (dst != ax) {
        GetMasm()->push(ArchReg(ax, DOUBLE_WORD_SIZE));
    }
    if (dst != dx) {
        GetMasm()->push(ArchReg(dx, DOUBLE_WORD_SIZE));
    }

    FastConstUnsignedDivisor fastDivisor(divisor, dst.GetSize());
    uint64_t magic = fastDivisor.GetMagic();

    ScopedTmpReg tmp(this, dst.GetType());
    if (fastDivisor.GetAdd()) {
        EncodeMov(tmp, src0);
    }
    EncodeMov(ax, src0);
    EncodeMov(dx, Imm(magic));
    GetMasm()->mul(ArchReg(dx));

    uint64_t shift = fastDivisor.GetShift();
    if (!fastDivisor.GetAdd()) {
        EncodeShr(dst, dx, Imm(shift));
    } else {
        ASSERT(shift >= 1U);
        EncodeSub(tmp, tmp, dx);
        EncodeShr(tmp, tmp, Imm(1U));
        EncodeAdd(tmp, tmp, dx);
        EncodeShr(dst, tmp, Imm(shift - 1U));
    }

    if (dst != dx) {
        GetMasm()->pop(ArchReg(dx, DOUBLE_WORD_SIZE));
    }
    if (dst != ax) {
        GetMasm()->pop(ArchReg(ax, DOUBLE_WORD_SIZE));
    }
}

void Amd64Encoder::EncodeDiv(Reg dst, Reg src0, Imm imm, bool isSigned)
{
    ASSERT(dst.IsScalar() && dst.GetSize() >= WORD_SIZE);
    ASSERT(CanOptimizeImmDivMod(bit_cast<uint64_t>(imm.GetAsInt()), isSigned));
    if (isSigned) {
        EncodeSignedDiv(dst, src0, imm);
    } else {
        EncodeUnsignedDiv(dst, src0, imm);
    }
}

void Amd64Encoder::EncodeMod(Reg dst, Reg src0, Imm imm, bool isSigned)
{
    ASSERT(dst.IsScalar() && dst.GetSize() >= WORD_SIZE);
    ASSERT(CanOptimizeImmDivMod(bit_cast<uint64_t>(imm.GetAsInt()), isSigned));

    // dst = src0 - imm * (src0 / imm)
    ScopedTmpReg tmp(this, dst.GetType());
    EncodeDiv(tmp, src0, imm, isSigned);
    if (dst.GetSize() == WORD_SIZE) {
        GetMasm()->imul(ArchReg(tmp), ArchReg(tmp), asmjit::imm(imm.GetAsInt()));
    } else {
        ScopedTmpRegU64 immReg(this);
        EncodeMov(immReg, imm);
        EncodeMul(tmp, tmp, immReg);
    }
    EncodeSub(dst, src0, tmp);
}

void Amd64Encoder::EncodeModFloat(Reg dst, Reg src0, Reg src1)
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

void Amd64Encoder::EncodeMod(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        EncodeModFloat(dst, src0, src1);
        return;
    }

    auto zeroPath = GetMasm()->newLabel();
    auto crossroad = GetMasm()->newLabel();

    if (dstSigned) {
        GetMasm()->cmp(ArchReg(src1), asmjit::imm(-1));
        GetMasm()->je(zeroPath);
    }

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->push(asmjit::x86::rax);
    }
    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->push(asmjit::x86::rdx);
    }

    ScopedTmpReg tmpReg(this, dst.GetType());
    Reg op1 {src1};
    if (src1.GetId() == ConvertRegNumber(asmjit::x86::rax.id()) ||
        src1.GetId() == ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->mov(ArchReg(tmpReg), ArchReg(src1));
        op1 = Reg(tmpReg);
    }

    if (src0.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->mov(asmjit::x86::rax, ArchReg(src0, DOUBLE_WORD_SIZE));
    }

    if (dstSigned) {
        if (dst.GetSize() <= WORD_SIZE) {
            GetMasm()->cdq();
        } else {
            GetMasm()->cqo();
        }
        GetMasm()->idiv(ArchReg(op1));
    } else {
        GetMasm()->xor_(asmjit::x86::rdx, asmjit::x86::rdx);
        GetMasm()->div(ArchReg(op1));
    }

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::x86::rdx);
        GetMasm()->pop(asmjit::x86::rdx);
    }

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->pop(asmjit::x86::rax);
    }
    GetMasm()->jmp(crossroad);

    GetMasm()->bind(zeroPath);
    GetMasm()->xor_(ArchReg(dst, WORD_SIZE), ArchReg(dst, WORD_SIZE));

    GetMasm()->bind(crossroad);
}

void Amd64Encoder::EncodeMin(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        ScopedTmpReg tmpReg(this, dst.GetType());
        GetMasm()->mov(ArchReg(tmpReg), ArchReg(src1));
        GetMasm()->cmp(ArchReg(src0), ArchReg(src1));

        auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
        if (dstSigned) {
            GetMasm()->cmovle(ArchReg(tmpReg, size), ArchReg(src0, size));
        } else {
            GetMasm()->cmovb(ArchReg(tmpReg, size), ArchReg(src0, size));
        }
        EncodeMov(dst, tmpReg);
        return;
    }

    EncodeMinMaxFp<false>(dst, src0, src1);
}

void Amd64Encoder::EncodeMax(Reg dst, bool dstSigned, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        ScopedTmpReg tmpReg(this, dst.GetType());
        GetMasm()->mov(ArchReg(tmpReg), ArchReg(src1));
        GetMasm()->cmp(ArchReg(src0), ArchReg(src1));

        auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
        if (dstSigned) {
            GetMasm()->cmovge(ArchReg(tmpReg, size), ArchReg(src0, size));
        } else {
            GetMasm()->cmova(ArchReg(tmpReg, size), ArchReg(src0, size));
        }
        EncodeMov(dst, tmpReg);
        return;
    }

    EncodeMinMaxFp<true>(dst, src0, src1);
}

template <bool IS_MAX>
void Amd64Encoder::EncodeMinMaxFp(Reg dst, Reg src0, Reg src1)
{
    auto end = GetMasm()->newLabel();
    auto notEqual = GetMasm()->newLabel();
    auto gotNan = GetMasm()->newLabel();
    auto &srcA = dst.GetId() != src1.GetId() ? src0 : src1;
    auto &srcB = srcA.GetId() == src0.GetId() ? src1 : src0;
    if (dst.GetType() == FLOAT32_TYPE) {
        GetMasm()->movaps(ArchVReg(dst), ArchVReg(srcA));
        GetMasm()->ucomiss(ArchVReg(srcB), ArchVReg(srcA));
        GetMasm()->jne(notEqual);
        GetMasm()->jp(gotNan);
        // calculate result for positive/negative zero operands
        if (IS_MAX) {
            GetMasm()->andps(ArchVReg(dst), ArchVReg(srcB));
        } else {
            GetMasm()->orps(ArchVReg(dst), ArchVReg(srcB));
        }
        GetMasm()->jmp(end);
        GetMasm()->bind(gotNan);
        // if any operand is NaN result is NaN
        GetMasm()->por(ArchVReg(dst), ArchVReg(srcB));
        GetMasm()->jmp(end);
        GetMasm()->bind(notEqual);
        if (IS_MAX) {
            GetMasm()->maxss(ArchVReg(dst), ArchVReg(srcB));
        } else {
            GetMasm()->minss(ArchVReg(dst), ArchVReg(srcB));
        }
        GetMasm()->bind(end);
    } else {
        GetMasm()->movapd(ArchVReg(dst), ArchVReg(srcA));
        GetMasm()->ucomisd(ArchVReg(srcB), ArchVReg(srcA));
        GetMasm()->jne(notEqual);
        GetMasm()->jp(gotNan);
        // calculate result for positive/negative zero operands
        if (IS_MAX) {
            GetMasm()->andpd(ArchVReg(dst), ArchVReg(srcB));
        } else {
            GetMasm()->orpd(ArchVReg(dst), ArchVReg(srcB));
        }
        GetMasm()->jmp(end);
        GetMasm()->bind(gotNan);
        // if any operand is NaN result is NaN
        GetMasm()->por(ArchVReg(dst), ArchVReg(srcB));
        GetMasm()->jmp(end);
        GetMasm()->bind(notEqual);
        if (IS_MAX) {
            GetMasm()->maxsd(ArchVReg(dst), ArchVReg(srcB));
        } else {
            GetMasm()->minsd(ArchVReg(dst), ArchVReg(srcB));
        }
        GetMasm()->bind(end);
    }
}

void Amd64Encoder::EncodeShl(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    ScopedTmpReg tmpReg(this, dst.GetType());
    Reg rcx(ConvertRegNumber(asmjit::x86::rcx.id()), dst.GetType());
    GetMasm()->mov(ArchReg(tmpReg), ArchReg(src0));
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->push(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(rcx), ArchReg(src1));
    GetMasm()->shl(ArchReg(tmpReg), asmjit::x86::cl);
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->pop(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(dst), ArchReg(tmpReg));
}

void Amd64Encoder::EncodeShr(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    ScopedTmpReg tmpReg(this, dst.GetType());
    Reg rcx(ConvertRegNumber(asmjit::x86::rcx.id()), dst.GetType());
    GetMasm()->mov(ArchReg(tmpReg), ArchReg(src0));
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->push(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(rcx), ArchReg(src1));
    GetMasm()->shr(ArchReg(tmpReg), asmjit::x86::cl);
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->pop(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(dst), ArchReg(tmpReg));
}

void Amd64Encoder::EncodeAShr(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    ScopedTmpReg tmpReg(this, dst.GetType());
    Reg rcx(ConvertRegNumber(asmjit::x86::rcx.id()), dst.GetType());
    GetMasm()->mov(ArchReg(tmpReg), ArchReg(src0));
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->push(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(rcx), ArchReg(src1));
    GetMasm()->sar(ArchReg(tmpReg), asmjit::x86::cl);
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->pop(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(dst), ArchReg(tmpReg));
}

void Amd64Encoder::EncodeAnd(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->and_(ArchReg(dst), ArchReg(src1));
    } else if (dst.GetId() == src1.GetId()) {
        GetMasm()->and_(ArchReg(dst), ArchReg(src0));
    } else {
        EncodeMov(dst, src0);
        GetMasm()->and_(ArchReg(dst), ArchReg(src1));
    }
}

void Amd64Encoder::EncodeOr(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->or_(ArchReg(dst), ArchReg(src1));
    } else if (dst.GetId() == src1.GetId()) {
        GetMasm()->or_(ArchReg(dst), ArchReg(src0));
    } else {
        EncodeMov(dst, src0);
        GetMasm()->or_(ArchReg(dst), ArchReg(src1));
    }
}

void Amd64Encoder::EncodeXor(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->xor_(ArchReg(dst), ArchReg(src1));
    } else if (dst.GetId() == src1.GetId()) {
        GetMasm()->xor_(ArchReg(dst), ArchReg(src0));
    } else {
        EncodeMov(dst, src0);
        GetMasm()->xor_(ArchReg(dst), ArchReg(src1));
    }
}

void Amd64Encoder::EncodeAdd(Reg dst, Reg src, Imm imm)
{
    if (dst.IsFloat()) {
        SetFalseResult();
        return;
    }

    auto immVal = imm.GetAsInt();
    auto size = std::max<uint8_t>(WORD_SIZE, dst.GetSize());
    if (ImmFitsSize(immVal, size)) {
        GetMasm()->lea(ArchReg(dst, size), asmjit::x86::ptr(ArchReg(src, size), immVal));
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(immVal));
            GetMasm()->add(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmpReg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(immVal));
            GetMasm()->add(ArchReg(dst), ArchReg(tmpReg));
        }
    }
}

void Amd64Encoder::EncodeSub(Reg dst, Reg src, Imm imm)
{
    if (dst.IsFloat()) {
        SetFalseResult();
        return;
    }

    auto immVal = -imm.GetAsInt();
    auto size = std::max<uint8_t>(WORD_SIZE, dst.GetSize());
    if (ImmFitsSize(immVal, size)) {
        GetMasm()->lea(ArchReg(dst, size), asmjit::x86::ptr(ArchReg(src, size), immVal));
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(immVal));
            GetMasm()->add(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmpReg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(immVal));
            GetMasm()->add(ArchReg(dst), ArchReg(tmpReg));
        }
    }
}

void Amd64Encoder::EncodeShl(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    EncodeMov(dst, src);
    GetMasm()->shl(ArchReg(dst), ArchImm(imm));
}

void Amd64Encoder::EncodeShr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());

    EncodeMov(dst, src);
    GetMasm()->shr(ArchReg(dst), ArchImm(imm));
}

void Amd64Encoder::EncodeAShr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    EncodeMov(dst, src);
    GetMasm()->sar(ArchReg(dst), ArchImm(imm));
}

void Amd64Encoder::EncodeAnd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    auto immVal = ImmToUnsignedInt(imm);

    switch (src.GetSize()) {
        case BYTE_SIZE:
            immVal |= ~uint64_t(0xFF);  // NOLINT
            break;
        case HALF_SIZE:
            immVal |= ~uint64_t(0xFFFF);  // NOLINT
            break;
        case WORD_SIZE:
            immVal |= ~uint64_t(0xFFFFFFFF);  // NOLINT
            break;
        default:
            break;
    }

    if (dst.GetSize() != DOUBLE_WORD_SIZE) {
        // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
        immVal &= (uint64_t(1) << dst.GetSize()) - 1;
    }

    if (ImmFitsSize(immVal, dst.GetSize())) {
        EncodeMov(dst, src);
        GetMasm()->and_(ArchReg(dst), immVal);
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(immVal));
            GetMasm()->and_(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmpReg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(immVal));
            GetMasm()->and_(ArchReg(dst), ArchReg(tmpReg));
        }
    }
}

void Amd64Encoder::EncodeOr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    auto immVal = ImmToUnsignedInt(imm);
    if (ImmFitsSize(immVal, dst.GetSize())) {
        EncodeMov(dst, src);
        GetMasm()->or_(ArchReg(dst), immVal);
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(immVal));
            GetMasm()->or_(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmpReg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(immVal));
            GetMasm()->or_(ArchReg(dst), ArchReg(tmpReg));
        }
    }
}

void Amd64Encoder::EncodeXor(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    auto immVal = ImmToUnsignedInt(imm);
    if (ImmFitsSize(immVal, dst.GetSize())) {
        EncodeMov(dst, src);
        GetMasm()->xor_(ArchReg(dst), immVal);
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(immVal));
            GetMasm()->xor_(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmpReg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(immVal));
            GetMasm()->xor_(ArchReg(dst), ArchReg(tmpReg));
        }
    }
}

void Amd64Encoder::EncodeMov(Reg dst, Imm src)
{
    if (dst.IsScalar()) {
        if (dst.GetSize() < WORD_SIZE) {
            GetMasm()->xor_(ArchReg(dst, WORD_SIZE), ArchReg(dst, WORD_SIZE));
        }
        GetMasm()->mov(ArchReg(dst), ArchImm(src));
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegU32 tmpReg(this);
        auto val = bit_cast<uint32_t>(src.GetAsFloat());
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(val));
        GetMasm()->movd(ArchVReg(dst), ArchReg(tmpReg));
    } else {
        ScopedTmpRegU64 tmpReg(this);
        auto val = bit_cast<uint64_t>(src.GetAsDouble());
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(val));
        GetMasm()->movq(ArchVReg(dst), ArchReg(tmpReg));
    }
}

void Amd64Encoder::EncodeLdr(Reg dst, bool dstSigned, MemRef mem)
{
    auto m = ArchMem(mem).Prepare(GetMasm());

    if (dst.GetType() == FLOAT32_TYPE) {
        GetMasm()->movss(ArchVReg(dst), m);
        return;
    }
    if (dst.GetType() == FLOAT64_TYPE) {
        GetMasm()->movsd(ArchVReg(dst), m);
        return;
    }

    m.setSize(dst.GetSize() / BITS_PER_BYTE);

    if (dstSigned && dst.GetSize() < DOUBLE_WORD_SIZE) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->movsxd(ArchReg(dst, DOUBLE_WORD_SIZE), m);
        } else {
            GetMasm()->movsx(ArchReg(dst, DOUBLE_WORD_SIZE), m);
        }
        return;
    }
    if (!dstSigned && dst.GetSize() < WORD_SIZE) {
        GetMasm()->movzx(ArchReg(dst, WORD_SIZE), m);
        return;
    }

    GetMasm()->mov(ArchReg(dst), m);
}

void Amd64Encoder::EncodeLdrAcquire(Reg dst, bool dstSigned, MemRef mem)
{
    EncodeLdr(dst, dstSigned, mem);
    // LoadLoad and LoadStore barrier should be here, but this is no-op in amd64 memory model
}

void Amd64Encoder::EncodeStr(Reg src, MemRef mem)
{
    auto m = ArchMem(mem).Prepare(GetMasm());

    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->movss(m, ArchVReg(src));
        return;
    }
    if (src.GetType() == FLOAT64_TYPE) {
        GetMasm()->movsd(m, ArchVReg(src));
        return;
    }

    m.setSize(src.GetSize() / BITS_PER_BYTE);
    GetMasm()->mov(m, ArchReg(src));
}

void Amd64Encoder::EncodeStrRelease(Reg src, MemRef mem)
{
    // StoreStore barrier should be here, but this is no-op in amd64 memory model
    EncodeStr(src, mem);
    // this is StoreLoad barrier (which is also full memory barrier in amd64 memory model)
    GetMasm()->lock().add(asmjit::x86::dword_ptr(asmjit::x86::rsp), asmjit::imm(0));
}

void Amd64Encoder::EncodeStrz(Reg src, MemRef mem)
{
    if (src.IsScalar()) {
        if (src.GetSize() == DOUBLE_WORD_SIZE) {
            GetMasm()->mov(ArchMem(mem).Prepare(GetMasm()), ArchReg(src));
        } else {
            ScopedTmpRegU64 tmpReg(this);
            GetMasm()->xor_(ArchReg(tmpReg), ArchReg(tmpReg));
            GetMasm()->mov(ArchReg(tmpReg, src.GetSize()), ArchReg(src));
            GetMasm()->mov(ArchMem(mem).Prepare(GetMasm()), ArchReg(tmpReg));
        }
    } else {
        if (src.GetType() == FLOAT64_TYPE) {
            GetMasm()->movsd(ArchMem(mem).Prepare(GetMasm()), ArchVReg(src));
        } else {
            ScopedTmpRegF64 tmpReg(this);

            GetMasm()->xorpd(ArchVReg(tmpReg), ArchVReg(tmpReg));
            GetMasm()->movss(ArchVReg(tmpReg), ArchVReg(src));
            GetMasm()->movsd(ArchMem(mem).Prepare(GetMasm()), ArchVReg(tmpReg));
        }
    }
}

void Amd64Encoder::EncodeSti(int64_t src, uint8_t srcSizeBytes, MemRef mem)
{
    ASSERT(srcSizeBytes <= 8U);
    auto m = ArchMem(mem).Prepare(GetMasm());
    if (srcSizeBytes <= HALF_WORD_SIZE_BYTES) {
        m.setSize(srcSizeBytes);
        GetMasm()->mov(m, asmjit::imm(src));
    } else {
        m.setSize(DOUBLE_WORD_SIZE_BYTES);

        if (ImmFitsSize(src, DOUBLE_WORD_SIZE)) {
            GetMasm()->mov(m, asmjit::imm(src));
        } else {
            ScopedTmpRegU64 tmpReg(this);
            GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(src));
            GetMasm()->mov(m, ArchReg(tmpReg));
        }
    }
}

void Amd64Encoder::EncodeSti(float src, MemRef mem)
{
    EncodeSti(bit_cast<int32_t>(src), sizeof(int32_t), mem);
}

void Amd64Encoder::EncodeSti(double src, MemRef mem)
{
    EncodeSti(bit_cast<int64_t>(src), sizeof(int64_t), mem);
}

void Amd64Encoder::EncodeMemCopy(MemRef memFrom, MemRef memTo, size_t size)
{
    ScopedTmpRegU64 tmpReg(this);
    GetMasm()->mov(ArchReg(tmpReg, size), ArchMem(memFrom).Prepare(GetMasm()));
    GetMasm()->mov(ArchMem(memTo).Prepare(GetMasm()), ArchReg(tmpReg, size));
}

void Amd64Encoder::EncodeMemCopyz(MemRef memFrom, MemRef memTo, size_t size)
{
    ScopedTmpRegU64 tmpReg(this);
    if (size < DOUBLE_WORD_SIZE) {
        GetMasm()->xor_(ArchReg(tmpReg), ArchReg(tmpReg));
    }
    GetMasm()->mov(ArchReg(tmpReg, size), ArchMem(memFrom).Prepare(GetMasm()));
    GetMasm()->mov(ArchMem(memTo).Prepare(GetMasm()), ArchReg(tmpReg));
}

void Amd64Encoder::EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc)
{
    if (src0.IsScalar()) {
        GetMasm()->cmp(ArchReg(src0), ArchReg(src1));
    } else {
        if (src0.GetType() == FLOAT32_TYPE) {
            GetMasm()->ucomiss(ArchVReg(src0), ArchVReg(src1));
        } else {
            GetMasm()->ucomisd(ArchVReg(src0), ArchVReg(src1));
        }
    }
    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(0));

    if (src0.IsScalar()) {
        GetMasm()->set(ArchCc(cc), ArchReg(dst, BYTE_SIZE));
        return;
    }

    auto end = GetMasm()->newLabel();

    if (CcMatchesNan(cc)) {
        GetMasm()->setp(ArchReg(dst, BYTE_SIZE));
    }
    GetMasm()->jp(end);
    GetMasm()->set(ArchCc(cc, true), ArchReg(dst, BYTE_SIZE));

    GetMasm()->bind(end);
}

void Amd64Encoder::EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsScalar());

    GetMasm()->test(ArchReg(src0), ArchReg(src1));

    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(0));
    GetMasm()->set(ArchCcTest(cc), ArchReg(dst, BYTE_SIZE));
}

void Amd64Encoder::EncodeAtomicByteOr(Reg addr, Reg value, [[maybe_unused]] bool fastEncoding)
{
    GetMasm()->lock().or_(asmjit::x86::byte_ptr(ArchReg(addr)), ArchReg(value, ark::compiler::BYTE_SIZE));
}

void Amd64Encoder::EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc)
{
    auto end = GetMasm()->newLabel();

    if (src0.IsFloat()) {
        ASSERT(src1.IsFloat());
        ASSERT(cc == Condition::MI || cc == Condition::LT);

        if (src0.GetType() == FLOAT32_TYPE) {
            GetMasm()->ucomiss(ArchVReg(src0), ArchVReg(src1));
        } else {
            GetMasm()->ucomisd(ArchVReg(src0), ArchVReg(src1));
        }

        GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), cc == Condition::LT ? asmjit::imm(-1) : asmjit::imm(1));
        cc = Condition::LO;

        GetMasm()->jp(end);
    } else {
        ASSERT(src0.IsScalar() && src1.IsScalar());
        ASSERT(cc == Condition::LO || cc == Condition::LT);
        GetMasm()->cmp(ArchReg(src0), ArchReg(src1));
    }
    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(0));
    GetMasm()->setne(ArchReg(dst, BYTE_SIZE));

    GetMasm()->j(asmjit::x86::Condition::negate(ArchCc(cc)), end);
    GetMasm()->neg(ArchReg(dst));

    GetMasm()->bind(end);
}

void Amd64Encoder::EncodeSelect(const ArgsSelect &args)
{
    auto [dst, src0, src1, src2, src3, cc] = args;
    ASSERT(!src0.IsFloat() && !src1.IsFloat());
    if (src2.IsScalar()) {
        GetMasm()->cmp(ArchReg(src2), ArchReg(src3));
    } else if (src2.GetType() == FLOAT32_TYPE) {
        GetMasm()->comiss(ArchVReg(src2), ArchVReg(src3));
    } else {
        GetMasm()->comisd(ArchVReg(src2), ArchVReg(src3));
    }

    auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
    bool dstAliased = dst.GetId() == src0.GetId();
    ScopedTmpReg tmpReg(this, dst.GetType());
    auto dstReg = dstAliased ? ArchReg(tmpReg, size) : ArchReg(dst, size);

    GetMasm()->mov(dstReg, ArchReg(src1, size));

    if (src2.IsScalar()) {
        GetMasm()->cmov(ArchCc(cc), dstReg, ArchReg(src0, size));
    } else if (CcMatchesNan(cc)) {
        GetMasm()->cmovp(dstReg, ArchReg(src0, size));
        GetMasm()->cmov(ArchCc(cc, src2.IsFloat()), dstReg, ArchReg(src0, size));
    } else {
        auto end = GetMasm()->newLabel();

        GetMasm()->jp(end);
        GetMasm()->cmov(ArchCc(cc, src2.IsFloat()), dstReg, ArchReg(src0, size));

        GetMasm()->bind(end);
    }
    if (dstAliased) {
        EncodeMov(dst, tmpReg);
    }
}

void Amd64Encoder::EncodeSelect(const ArgsSelectImm &args)
{
    auto [dst, src0, src1, src2, imm, cc] = args;
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    auto immVal = imm.GetAsInt();
    if (ImmFitsSize(immVal, src2.GetSize())) {
        GetMasm()->cmp(ArchReg(src2), asmjit::imm(immVal));
    } else {
        ScopedTmpReg tmpReg(this, src2.GetType());
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(immVal));
        GetMasm()->cmp(ArchReg(src2), ArchReg(tmpReg));
    }

    ScopedTmpReg tmpReg(this, dst.GetType());
    auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
    bool dstAliased = dst.GetId() == src0.GetId();
    auto dstReg = dstAliased ? ArchReg(tmpReg, size) : ArchReg(dst, size);

    GetMasm()->mov(dstReg, ArchReg(src1, size));
    GetMasm()->cmov(ArchCc(cc), dstReg, ArchReg(src0, size));
    if (dstAliased) {
        EncodeMov(dst, tmpReg);
    }
}

void Amd64Encoder::EncodeSelectTest(const ArgsSelect &args)
{
    auto [dst, src0, src1, src2, src3, cc] = args;
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    GetMasm()->test(ArchReg(src2), ArchReg(src3));

    ScopedTmpReg tmpReg(this, dst.GetType());
    auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
    bool dstAliased = dst.GetId() == src0.GetId();
    auto dstReg = dstAliased ? ArchReg(tmpReg, size) : ArchReg(dst, size);

    GetMasm()->mov(dstReg, ArchReg(src1, size));
    GetMasm()->cmov(ArchCcTest(cc), dstReg, ArchReg(src0, size));
    if (dstAliased) {
        EncodeMov(dst, tmpReg);
    }
}

void Amd64Encoder::EncodeSelectTest(const ArgsSelectImm &args)
{
    auto [dst, src0, src1, src2, imm, cc] = args;
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    auto immVal = imm.GetAsInt();
    if (ImmFitsSize(immVal, src2.GetSize())) {
        GetMasm()->test(ArchReg(src2), asmjit::imm(immVal));
    } else {
        ScopedTmpReg tmpReg(this, src2.GetType());
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(immVal));
        GetMasm()->test(ArchReg(src2), ArchReg(tmpReg));
    }

    ScopedTmpReg tmpReg(this, dst.GetType());
    auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
    bool dstAliased = dst.GetId() == src0.GetId();
    auto dstReg = dstAliased ? ArchReg(tmpReg, size) : ArchReg(dst, size);

    GetMasm()->mov(dstReg, ArchReg(src1, size));
    GetMasm()->cmov(ArchCcTest(cc), dstReg, ArchReg(src0, size));
    if (dstAliased) {
        EncodeMov(dst, tmpReg);
    }
}

void Amd64Encoder::EncodeLdp(Reg dst0, Reg dst1, bool dstSigned, MemRef mem)
{
    ASSERT(dst0.IsFloat() == dst1.IsFloat());
    ASSERT(dst0.GetSize() == dst1.GetSize());

    auto m = ArchMem(mem).Prepare(GetMasm());

    if (dst0.IsFloat()) {
        if (dst0.GetType() == FLOAT32_TYPE) {
            GetMasm()->movss(ArchVReg(dst0), m);

            m.addOffset(WORD_SIZE_BYTES);
            GetMasm()->movss(ArchVReg(dst1), m);
        } else {
            GetMasm()->movsd(ArchVReg(dst0), m);

            m.addOffset(DOUBLE_WORD_SIZE_BYTES);
            GetMasm()->movsd(ArchVReg(dst1), m);
        }
        return;
    }

    if (dstSigned && dst0.GetSize() == WORD_SIZE) {
        m.setSize(WORD_SIZE_BYTES);
        GetMasm()->movsxd(ArchReg(dst0, DOUBLE_WORD_SIZE), m);

        m.addOffset(WORD_SIZE_BYTES);
        GetMasm()->movsxd(ArchReg(dst1, DOUBLE_WORD_SIZE), m);
        return;
    }

    GetMasm()->mov(ArchReg(dst0), m);

    m.addOffset(dst0.GetSize() / BITS_PER_BYTE);
    GetMasm()->mov(ArchReg(dst1), m);
}

void Amd64Encoder::EncodeStp(Reg src0, Reg src1, MemRef mem)
{
    ASSERT(src0.IsFloat() == src1.IsFloat());
    ASSERT(src0.GetSize() == src1.GetSize());

    auto m = ArchMem(mem).Prepare(GetMasm());

    if (src0.IsFloat()) {
        if (src0.GetType() == FLOAT32_TYPE) {
            GetMasm()->movss(m, ArchVReg(src0));

            m.addOffset(WORD_SIZE_BYTES);
            GetMasm()->movss(m, ArchVReg(src1));
        } else {
            GetMasm()->movsd(m, ArchVReg(src0));

            m.addOffset(DOUBLE_WORD_SIZE_BYTES);
            GetMasm()->movsd(m, ArchVReg(src1));
        }
        return;
    }

    GetMasm()->mov(m, ArchReg(src0));

    m.addOffset(src0.GetSize() / BITS_PER_BYTE);
    GetMasm()->mov(m, ArchReg(src1));
}

void Amd64Encoder::EncodeReverseBytes(Reg dst, Reg src)
{
    ASSERT(src.GetSize() > BYTE_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());
    ASSERT(src.IsValid());
    ASSERT(dst.IsValid());

    if (src != dst) {
        GetMasm()->mov(ArchReg(dst), ArchReg(src));
    }

    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->rol(ArchReg(dst), BYTE_SIZE);
        GetMasm()->movsx(ArchReg(dst, WORD_SIZE), ArchReg(dst));
    } else {
        GetMasm()->bswap(ArchReg(dst));
    }
}

void Amd64Encoder::EncodeUnsignedExtendBytesToShorts(Reg dst, Reg src)
{
    GetMasm()->pmovzxbw(ArchVReg(dst), ArchVReg(src));
}

/* Attention: the encoder belows operates on vector registers not GPRs */
void Amd64Encoder::EncodeReverseHalfWords(Reg dst, Reg src)
{
    ASSERT(src.GetSize() == dst.GetSize());
    ASSERT(src.IsValid());
    ASSERT(dst.IsValid());

    constexpr unsigned MASK = 0x1b;  // reverse mask: 00 01 10 11
    GetMasm()->pshuflw(ArchVReg(dst), ArchVReg(src), MASK);
}

bool Amd64Encoder::CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, [[maybe_unused]] bool signedCompare)
{
    return ImmFitsSize(imm, size);
}

void Amd64Encoder::EncodeBitCount(Reg dst0, Reg src0)
{
    ASSERT(src0.GetSize() == WORD_SIZE || src0.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(dst0.GetSize() == WORD_SIZE);
    ASSERT(src0.IsScalar() && dst0.IsScalar());

    GetMasm()->popcnt(ArchReg(dst0, src0.GetSize()), ArchReg(src0));
}

void Amd64Encoder::EncodeCountLeadingZeroBits(Reg dst, Reg src)
{
    auto end = CreateLabel();
    auto zero = CreateLabel();
    EncodeJump(zero, src, Condition::EQ);
    GetMasm()->bsr(ArchReg(dst), ArchReg(src));
    GetMasm()->xor_(ArchReg(dst), asmjit::imm(dst.GetSize() - 1));
    EncodeJump(end);

    BindLabel(zero);
    GetMasm()->mov(ArchReg(dst), asmjit::imm(dst.GetSize()));

    BindLabel(end);
}

void Amd64Encoder::EncodeCountTrailingZeroBits(Reg dst, Reg src)
{
    ScopedTmpReg tmp(this, src.GetType());
    GetMasm()->bsf(ArchReg(tmp), ArchReg(src));
    GetMasm()->mov(ArchReg(dst), asmjit::imm(dst.GetSize()));
    GetMasm()->cmovne(ArchReg(dst), ArchReg(tmp));
}

void Amd64Encoder::EncodeCeil(Reg dst, Reg src)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->roundsd(ArchVReg(dst), ArchVReg(src), asmjit::imm(2_I));
}

void Amd64Encoder::EncodeFloor(Reg dst, Reg src)
{
    GetMasm()->roundsd(ArchVReg(dst), ArchVReg(src), asmjit::imm(1));
}

void Amd64Encoder::EncodeRint(Reg dst, Reg src)
{
    GetMasm()->roundsd(ArchVReg(dst), ArchVReg(src), asmjit::imm(0));
}

void Amd64Encoder::EncodeTrunc(Reg dst, Reg src)
{
    GetMasm()->roundsd(ArchVReg(dst), ArchVReg(src), asmjit::imm(3_I));
}

void Amd64Encoder::EncodeRoundAway(Reg dst, Reg src)
{
    ASSERT(src.GetType() == FLOAT64_TYPE);
    ASSERT(dst.GetType() == FLOAT64_TYPE);

    ScopedTmpReg tv(this, src.GetType());
    ScopedTmpReg tv1(this, src.GetType());
    ScopedTmpRegU64 ti(this);
    auto dest = dst;

    auto shared = src == dst;

    if (shared) {
        dest = tv1.GetReg();
    }
    GetMasm()->movapd(ArchVReg(dest), ArchVReg(src));

    constexpr auto SIGN_BIT_MASK = 0x8000000000000000ULL;
    GetMasm()->mov(ArchReg(ti), asmjit::imm(SIGN_BIT_MASK));
    GetMasm()->movq(ArchVReg(tv), ArchReg(ti));
    GetMasm()->andpd(ArchVReg(dest), ArchVReg(tv));

    constexpr auto DOUBLE_POINT_FIVE = 0x3fdfffffffffffffULL;  // .49999999999999994
    GetMasm()->mov(ArchReg(ti), asmjit::imm(DOUBLE_POINT_FIVE));
    GetMasm()->movq(ArchVReg(tv), ArchReg(ti));
    GetMasm()->orpd(ArchVReg(dest), ArchVReg(tv));

    GetMasm()->addsd(ArchVReg(dest), ArchVReg(src));
    GetMasm()->roundsd(ArchVReg(dest), ArchVReg(dest), asmjit::imm(3_I));
    if (shared) {
        GetMasm()->movapd(ArchVReg(dst), ArchVReg(dest));
    }
}

void Amd64Encoder::EncodeRoundToPInfFloat(Reg dst, Reg src)
{
    ScopedTmpReg t1(this, src.GetType());
    ScopedTmpReg t2(this, src.GetType());
    ScopedTmpReg t3(this, src.GetType());
    ScopedTmpReg t4(this, dst.GetType());

    auto skipIncrId = CreateLabel();
    auto doneId = CreateLabel();

    auto skipIncr = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(skipIncrId);
    auto done = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(doneId);

    GetMasm()->movss(ArchVReg(t2), ArchVReg(src));
    GetMasm()->roundss(ArchVReg(t1), ArchVReg(src), asmjit::imm(1));
    GetMasm()->subss(ArchVReg(t2), ArchVReg(t1));
    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->mov(ArchReg(t4), asmjit::imm(bit_cast<int32_t, float>(0.5F)));
    GetMasm()->movd(ArchVReg(t3), ArchReg(t4));
    GetMasm()->comiss(ArchVReg(t2), ArchVReg(t3));
    GetMasm()->j(asmjit::x86::Condition::Code::kB, *skipIncr);
    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->mov(ArchReg(t4), asmjit::imm(bit_cast<int32_t, float>(1.0F)));
    GetMasm()->movd(ArchVReg(t3), ArchReg(t4));
    GetMasm()->addss(ArchVReg(t1), ArchVReg(t3));
    BindLabel(skipIncrId);

    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->mov(ArchReg(dst), asmjit::imm(0x7FFFFFFF));
    GetMasm()->cvtsi2ss(ArchVReg(t2), ArchReg(dst));
    GetMasm()->comiss(ArchVReg(t1), ArchVReg(t2));
    GetMasm()->j(asmjit::x86::Condition::Code::kAE,
                 *done);                           // clipped to max (already in dst), does not jump on unordered
    GetMasm()->mov(ArchReg(dst), asmjit::imm(0));  // does not change flags
    GetMasm()->j(asmjit::x86::Condition::Code::kParityEven, *done);  // NaN mapped to 0 (just moved in dst)
    GetMasm()->cvttss2si(ArchReg(dst), ArchVReg(t1));
    BindLabel(doneId);
}

void Amd64Encoder::EncodeRoundToPInfDouble(Reg dst, Reg src)
{
    ScopedTmpReg t1(this, src.GetType());
    ScopedTmpReg t2(this, src.GetType());
    ScopedTmpReg t3(this, src.GetType());
    ScopedTmpReg t4(this, dst.GetType());

    auto skipIncrId = CreateLabel();
    auto doneId = CreateLabel();

    auto skipIncr = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(skipIncrId);
    auto done = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(doneId);

    GetMasm()->movsd(ArchVReg(t2), ArchVReg(src));
    GetMasm()->roundsd(ArchVReg(t1), ArchVReg(src), asmjit::imm(1));
    GetMasm()->subsd(ArchVReg(t2), ArchVReg(t1));
    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->mov(ArchReg(t4), asmjit::imm(bit_cast<int64_t, double>(0.5F)));
    GetMasm()->movq(ArchVReg(t3), ArchReg(t4));
    GetMasm()->comisd(ArchVReg(t2), ArchVReg(t3));
    GetMasm()->j(asmjit::x86::Condition::Code::kB, *skipIncr);
    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->mov(ArchReg(t4), asmjit::imm(bit_cast<int64_t, double>(1.0)));
    GetMasm()->movq(ArchVReg(t3), ArchReg(t4));
    GetMasm()->addsd(ArchVReg(t1), ArchVReg(t3));
    BindLabel(skipIncrId);

    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->mov(ArchReg(dst), asmjit::imm(0x7FFFFFFFFFFFFFFFL));
    GetMasm()->cvtsi2sd(ArchVReg(t2), ArchReg(dst));
    GetMasm()->comisd(ArchVReg(t1), ArchVReg(t2));
    GetMasm()->j(asmjit::x86::Condition::Code::kAE,
                 *done);                           // clipped to max (already in dst), does not jump on unordered
    GetMasm()->mov(ArchReg(dst), asmjit::imm(0));  // does not change flags
    GetMasm()->j(asmjit::x86::Condition::Code::kParityEven, *done);  // NaN mapped to 0 (just moved in dst)
    GetMasm()->cvttsd2si(ArchReg(dst), ArchVReg(t1));
    BindLabel(doneId);
}

void Amd64Encoder::EncodeRoundToPInfReturnScalar(Reg dst, Reg src)
{
    if (src.GetType() == FLOAT32_TYPE) {
        EncodeRoundToPInfFloat(dst, src);
    } else if (src.GetType() == FLOAT64_TYPE) {
        EncodeRoundToPInfDouble(dst, src);
    } else {
        UNREACHABLE();
    }
}

void Amd64Encoder::EncodeRoundToPInfReturnFloat(Reg dst, Reg src)
{
    ASSERT(src.GetType() == FLOAT64_TYPE);
    ASSERT(dst.GetType() == FLOAT64_TYPE);

    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr int64_t HALF = 0x3FE0000000000000;  // double precision representation of 0.5
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr int64_t ONE = 0x3FF0000000000000;  // double precision representation of 1.0
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr int64_t NEG_HALF = 0xBFE0000000000000;  // double precision representation of -0.5
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr int64_t ZERO = 0x0000000000000000;  // double precision representation of 0.0
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr int64_t NEG_ZERO = 0x8000000000000000;  // double precision representation of -0.0

    auto skipZero = GetMasm()->newLabel();
    auto negZeroCase = GetMasm()->newLabel();
    auto end = GetMasm()->newLabel();
    {
        ScopedTmpRegF64 constReg(this);
        ScopedTmpRegU64 tmpReg(this);

        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(HALF));
        GetMasm()->movq(ArchVReg(constReg), ArchReg(tmpReg));
        GetMasm()->comisd(ArchVReg(src), ArchVReg(constReg));
        GetMasm()->jae(skipZero);

        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(NEG_HALF));
        GetMasm()->movq(ArchVReg(constReg), ArchReg(tmpReg));
        GetMasm()->comisd(ArchVReg(src), ArchVReg(constReg));
        GetMasm()->jb(skipZero);

        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(ZERO));
        GetMasm()->movq(ArchVReg(constReg), ArchReg(tmpReg));
        GetMasm()->comisd(ArchVReg(src), ArchVReg(constReg));
        GetMasm()->jb(negZeroCase);

        GetMasm()->xorpd(ArchVReg(dst), ArchVReg(dst));
        GetMasm()->jmp(end);
    }
    GetMasm()->bind(negZeroCase);
    {
        ScopedTmpRegU64 tmpReg(this);
        GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(NEG_ZERO));
        GetMasm()->movq(ArchVReg(dst), ArchReg(tmpReg));
        GetMasm()->jmp(end);
    }
    GetMasm()->bind(skipZero);

    ScopedTmpRegF64 ceil(this);
    GetMasm()->roundsd(ArchVReg(ceil), ArchVReg(src), asmjit::imm(0b10));

    // calculate ceil(val) - val
    ScopedTmpRegF64 diff(this);
    GetMasm()->movapd(ArchVReg(diff), ArchVReg(ceil));
    GetMasm()->subsd(ArchVReg(diff), ArchVReg(src));

    // load 0.5 constant and compare
    ScopedTmpRegF64 constReg(this);
    ScopedTmpRegU64 tmpReg(this);
    GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(HALF));
    GetMasm()->movq(ArchVReg(constReg), ArchReg(tmpReg));
    GetMasm()->comisd(ArchVReg(diff), ArchVReg(constReg));

    // if difference > 0.5, subtract 1 from result
    auto done = GetMasm()->newLabel();
    GetMasm()->jbe(done);  // If difference <= 0.5, jump to end

    // Load 1.0 and subtract
    GetMasm()->mov(ArchReg(tmpReg), asmjit::imm(ONE));
    GetMasm()->movq(ArchVReg(constReg), ArchReg(tmpReg));
    GetMasm()->subsd(ArchVReg(ceil), ArchVReg(constReg));

    GetMasm()->bind(done);

    // move result to destination register
    GetMasm()->movapd(ArchVReg(dst), ArchVReg(ceil));
    GetMasm()->bind(end);
}

template <typename T>
void Amd64Encoder::EncodeReverseBitsImpl(Reg dst0, Reg src0)
{
    ASSERT(std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed);
    [[maybe_unused]] constexpr auto IMM_8 = 8;
    ASSERT(sizeof(T) * IMM_8 == dst0.GetSize());
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static constexpr T MASKS[] = {static_cast<T>(UINT64_C(0x5555555555555555)),
                                  static_cast<T>(UINT64_C(0x3333333333333333)),
                                  static_cast<T>(UINT64_C(0x0f0f0f0f0f0f0f0f))};

    ScopedTmpReg tmp(this, dst0.GetType());
    ScopedTmpReg immHolder(this, dst0.GetType());
    auto immHolderReg = ArchReg(immHolder);

    GetMasm()->mov(ArchReg(dst0), ArchReg(src0));
    GetMasm()->mov(ArchReg(tmp), ArchReg(src0));
    constexpr auto MAX_ROUNDS = 3;
    for (uint64_t round = 0; round < MAX_ROUNDS; round++) {
        auto shift = 1U << round;
        auto mask = asmjit::imm(MASKS[round]);
        GetMasm()->shr(ArchReg(dst0), shift);
        if (dst0.GetSize() == DOUBLE_WORD_SIZE) {
            GetMasm()->mov(immHolderReg, mask);
            GetMasm()->and_(ArchReg(tmp), immHolderReg);
            GetMasm()->and_(ArchReg(dst0), immHolderReg);
        } else {
            GetMasm()->and_(ArchReg(tmp), mask);
            GetMasm()->and_(ArchReg(dst0), mask);
        }
        GetMasm()->shl(ArchReg(tmp), shift);
        GetMasm()->or_(ArchReg(dst0), ArchReg(tmp));
        constexpr auto ROUND_2 = 2;
        if (round != ROUND_2) {
            GetMasm()->mov(ArchReg(tmp), ArchReg(dst0));
        }
    }

    GetMasm()->bswap(ArchReg(dst0));
}

void Amd64Encoder::EncodeReverseBits(Reg dst0, Reg src0)
{
    ASSERT(src0.GetSize() == WORD_SIZE || src0.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(src0.GetSize() == dst0.GetSize());

    if (src0.GetSize() == WORD_SIZE) {
        EncodeReverseBitsImpl<uint32_t>(dst0, src0);
        return;
    }

    EncodeReverseBitsImpl<uint64_t>(dst0, src0);
}

bool Amd64Encoder::CanEncodeScale(uint64_t imm, [[maybe_unused]] uint32_t size)
{
    return imm <= 3U;
}

bool Amd64Encoder::CanEncodeImmLogical(uint64_t imm, uint32_t size)
{
#ifndef NDEBUG
    if (size < DOUBLE_WORD_SIZE) {
        // Test if the highest part is consistent:
        ASSERT(((imm >> size) == 0) || (((~imm) >> size) == 0));
    }
#endif  // NDEBUG
    return ImmFitsSize(imm, size);
}

bool Amd64Encoder::CanEncodeBitCount()
{
    return asmjit::CpuInfo::host().hasFeature(asmjit::x86::Features::kPOPCNT);
}

bool Amd64Encoder::CanEncodeCompressedStringCharAt()
{
    return true;
}

bool Amd64Encoder::CanOptimizeImmDivMod(uint64_t imm, bool isSigned) const
{
    return CanOptimizeImmDivModCommon(imm, isSigned);
}

void Amd64Encoder::EncodeIsInf(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());

    GetMasm()->xor_(ArchReg(dst, DOUBLE_WORD_SIZE), ArchReg(dst, DOUBLE_WORD_SIZE));

    if (src.GetSize() == WORD_SIZE) {
        constexpr auto INF_MASK = uint32_t(0x7f800000) << 1U;

        ScopedTmpRegU32 tmpReg(this);
        ScopedTmpRegU32 tmp1Reg(this);
        auto tmp = ArchReg(tmpReg);
        auto tmp1 = ArchReg(tmp1Reg);

        GetMasm()->movd(tmp1, ArchVReg(src));
        GetMasm()->shl(tmp1, 1);
        GetMasm()->mov(tmp, INF_MASK);
        GetMasm()->cmp(tmp, tmp1);
    } else {
        constexpr auto INF_MASK = uint64_t(0x7ff0000000000000) << 1U;

        ScopedTmpRegU64 tmpReg(this);
        ScopedTmpRegU64 tmp1Reg(this);
        auto tmp = ArchReg(tmpReg);
        auto tmp1 = ArchReg(tmp1Reg);

        GetMasm()->movq(tmp1, ArchVReg(src));
        GetMasm()->shl(tmp1, 1);

        GetMasm()->mov(tmp, INF_MASK);
        GetMasm()->cmp(tmp, tmp1);
    }

    GetMasm()->sete(ArchReg(dst, BYTE_SIZE));
}

void Amd64Encoder::EncodeCmpFracWithZero(Reg src)
{
    ASSERT(src.IsFloat());
    ASSERT(src.GetType() == FLOAT32_TYPE || src.GetType() == FLOAT64_TYPE);

    // Rounding control bits: Truncated (aka Round to Zero)
    constexpr uint8_t RND_CTL_TRUNCATED = 0b00000011;

    // Encode (fabs(src - trunc(src)) <= DELTA)
    if (src.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF32 tmp(this);
        ScopedTmpRegF32 delta(this);
        GetMasm()->roundss(ArchVReg(tmp), ArchVReg(src), asmjit::imm(RND_CTL_TRUNCATED));
        EncodeSub(tmp, src, tmp);
        EncodeAbs(tmp, tmp);
        EncodeMov(delta, Imm(static_cast<float>(0.0)));
        GetMasm()->ucomiss(ArchVReg(tmp), ArchVReg(delta));
    } else {
        ScopedTmpRegF64 tmp(this);
        ScopedTmpRegF64 delta(this);
        GetMasm()->roundsd(ArchVReg(tmp), ArchVReg(src), asmjit::imm(RND_CTL_TRUNCATED));
        EncodeSub(tmp, src, tmp);
        EncodeAbs(tmp, tmp);
        EncodeMov(delta, Imm(static_cast<double>(0.0)));
        GetMasm()->ucomisd(ArchVReg(tmp), ArchVReg(delta));
    }
}

void Amd64Encoder::EncodeIsInteger(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(src.GetType() == FLOAT32_TYPE || src.GetType() == FLOAT64_TYPE);

    auto labelExit = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->xor_(ArchReg(dst, WORD_SIZE), ArchReg(dst, WORD_SIZE));
    EncodeCmpFracWithZero(src);
    GetMasm()->jp(*labelExit);  // Inf or NaN
    GetMasm()->set(ArchCc(Condition::LE, true), ArchReg(dst, BYTE_SIZE));
    GetMasm()->bind(*labelExit);
}

void Amd64Encoder::EncodeIsSafeInteger(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(src.GetType() == FLOAT32_TYPE || src.GetType() == FLOAT64_TYPE);

    auto labelExit = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->xor_(ArchReg(dst, WORD_SIZE), ArchReg(dst, WORD_SIZE));

    // Check if IsInteger
    EncodeCmpFracWithZero(src);
    GetMasm()->jp(*labelExit);  // Inf or NaN
    GetMasm()->j(ArchCc(Condition::GT, true), *labelExit);

    // Check if it is safe, i.e. src can be represented in float/double without losing precision
    if (src.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF32 tmp1(this);
        ScopedTmpRegF32 tmp2(this);
        EncodeAbs(tmp1, src);
        EncodeMov(tmp2, Imm(MaxIntAsExactFloat()));
        GetMasm()->ucomiss(ArchVReg(tmp1), ArchVReg(tmp2));
    } else {
        ScopedTmpRegF64 tmp1(this);
        ScopedTmpRegF64 tmp2(this);
        EncodeAbs(tmp1, src);
        EncodeMov(tmp2, Imm(MaxIntAsExactDouble()));
        GetMasm()->ucomisd(ArchVReg(tmp1), ArchVReg(tmp2));
    }
    GetMasm()->set(ArchCc(Condition::LE, true), ArchReg(dst, BYTE_SIZE));
    GetMasm()->bind(*labelExit);
}

/* Since NaNs have to be canonicalized we compare the
 * input with itself, if it is NaN the comparison will
 * set the parity flag (PF) */
void Amd64Encoder::EncodeFpToBits(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());

    if (dst.GetType() == INT32_TYPE) {
        ASSERT(src.GetSize() == WORD_SIZE);

        constexpr auto FLOAT_NAN = uint32_t(0x7fc00000);

        ScopedTmpRegU32 tmp(this);

        GetMasm()->ucomiss(ArchVReg(src), ArchVReg(src));
        GetMasm()->mov(ArchReg(tmp), FLOAT_NAN);
        GetMasm()->movd(ArchReg(dst), ArchVReg(src));
        GetMasm()->cmovpe(ArchReg(dst), ArchReg(tmp));
    } else {
        ASSERT(src.GetSize() == DOUBLE_WORD_SIZE);

        constexpr auto DOUBLE_NAN = uint64_t(0x7ff8000000000000);
        ScopedTmpRegU64 tmp(this);

        GetMasm()->ucomisd(ArchVReg(src), ArchVReg(src));
        GetMasm()->mov(ArchReg(tmp), DOUBLE_NAN);
        GetMasm()->movq(ArchReg(dst), ArchVReg(src));
        GetMasm()->cmovpe(ArchReg(dst), ArchReg(tmp));
    }
}

void Amd64Encoder::EncodeMoveBitsRaw(Reg dst, Reg src)
{
    ASSERT((dst.IsFloat() && src.IsScalar()) || (src.IsFloat() && dst.IsScalar()));
    if (src.IsScalar()) {
        ASSERT((dst.GetSize() == src.GetSize()));
        if (src.GetSize() == WORD_SIZE) {
            GetMasm()->movd(ArchVReg(dst), ArchReg(src));
        } else {
            GetMasm()->movq(ArchVReg(dst), ArchReg(src));
        }
    } else {
        ASSERT((src.GetSize() == dst.GetSize()));
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->movd(ArchReg(dst), ArchVReg(src));
        } else {
            GetMasm()->movq(ArchReg(dst), ArchVReg(src));
        }
    }
}

/* Unsafe intrinsics */
void Amd64Encoder::EncodeCompareAndSwap(Reg dst, Reg obj, const Reg *offset, Reg val, Reg newval)
{
    /*
     * movl    old, %eax
     * lock    cmpxchgl   new, addr
     * sete    %al
     */
    ScopedTmpRegU64 tmp1(this);
    ScopedTmpRegU64 tmp2(this);
    ScopedTmpRegU64 tmp3(this);
    Reg newvalue = newval;
    auto addr = ArchMem(MemRef(tmp2)).Prepare(GetMasm());
    auto addrReg = ArchReg(tmp2);
    Reg rax(ConvertRegNumber(asmjit::x86::rax.id()), INT64_TYPE);

    /* NOTE(ayodkev) this is a workaround for the failure of
     * jsr166.ScheduledExecutorTest, have to figure out if there
     * is less crude way to avoid this */
    if (newval.GetId() == rax.GetId()) {
        SetFalseResult();
        return;
    }

    if (offset != nullptr) {
        GetMasm()->lea(addrReg, asmjit::x86::ptr(ArchReg(obj), ArchReg(*offset)));
    } else {
        GetMasm()->mov(addrReg, ArchReg(obj));
    }

    /* the [er]ax register will be overwritten by cmpxchg instruction
     * save it unless it is set as a destination register */
    if (dst.GetId() != rax.GetId()) {
        GetMasm()->mov(ArchReg(tmp1), asmjit::x86::rax);
    }

    /* if the new value comes in [er]ax register we have to use a
     * different register as [er]ax will contain the current value */
    if (newval.GetId() == rax.GetId()) {
        GetMasm()->mov(ArchReg(tmp3, newval.GetSize()), ArchReg(newval));
        newvalue = tmp3;
    }

    if (val.GetId() != rax.GetId()) {
        GetMasm()->mov(asmjit::x86::rax, ArchReg(val).r64());
    }

    GetMasm()->lock().cmpxchg(addr, ArchReg(newvalue));
    GetMasm()->sete(ArchReg(dst));

    if (dst.GetId() != rax.GetId()) {
        GetMasm()->mov(asmjit::x86::rax, ArchReg(tmp1));
    }
}

void Amd64Encoder::EncodeCompareAndSwap(Reg dst, Reg obj, Reg offset, Reg val, Reg newval)
{
    EncodeCompareAndSwap(dst, obj, &offset, val, newval);
}

void Amd64Encoder::EncodeCompareAndSwap(Reg dst, Reg addr, Reg val, Reg newval)
{
    EncodeCompareAndSwap(dst, addr, nullptr, val, newval);
}

void Amd64Encoder::EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val)
{
    ScopedTmpRegU64 tmp(this);
    auto addrReg = ArchReg(tmp);
    auto addr = ArchMem(MemRef(tmp)).Prepare(GetMasm());
    GetMasm()->lea(addrReg, asmjit::x86::ptr(ArchReg(obj), ArchReg(offset)));
    GetMasm()->mov(ArchReg(dst), ArchReg(val));
    GetMasm()->lock().xchg(addr, ArchReg(dst));
}

void Amd64Encoder::EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, [[maybe_unused]] Reg tmp)
{
    ScopedTmpRegU64 tmp1(this);
    auto addrReg = ArchReg(tmp1);
    auto addr = ArchMem(MemRef(tmp1)).Prepare(GetMasm());
    GetMasm()->lea(addrReg, asmjit::x86::ptr(ArchReg(obj), ArchReg(offset)));
    GetMasm()->mov(ArchReg(dst), ArchReg(val));
    GetMasm()->lock().xadd(addr, ArchReg(dst));
}

void Amd64Encoder::EncodeMemoryBarrier(memory_order::Order order)
{
    if (order == memory_order::FULL) {
        /* does the same as mfence but faster, not applicable for NT-writes, though */
        GetMasm()->lock().add(asmjit::x86::dword_ptr(asmjit::x86::rsp), asmjit::imm(0));
    }
}

void Amd64Encoder::EncodeStackOverflowCheck(ssize_t offset)
{
    MemRef mem(GetTarget().GetStackReg(), offset);
    auto m = ArchMem(mem).Prepare(GetMasm());
    GetMasm()->test(m, ArchReg(GetTarget().GetParamReg(0)));
}

size_t Amd64Encoder::GetCursorOffset() const
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    return GetMasm()->offset();
}

void Amd64Encoder::SetCursorOffset(size_t offset)
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    GetMasm()->setOffset(offset);
}

void Amd64Encoder::EncodeGetCurrentPc(Reg dst)
{
    ASSERT(dst.GetType() == INT64_TYPE);
    EncodeRelativePcMov(dst, 0L, [this](Reg reg, intptr_t offset) {
        GetMasm()->long_().lea(ArchReg(reg), asmjit::x86::ptr(asmjit::x86::rip, offset));
    });
}

Reg Amd64Encoder::AcquireScratchRegister(TypeInfo type)
{
    return (static_cast<Amd64RegisterDescription *>(GetRegfile()))->AcquireScratchRegister(type);
}

void Amd64Encoder::AcquireScratchRegister(Reg reg)
{
    (static_cast<Amd64RegisterDescription *>(GetRegfile()))->AcquireScratchRegister(reg);
}

void Amd64Encoder::ReleaseScratchRegister(Reg reg)
{
    (static_cast<Amd64RegisterDescription *>(GetRegfile()))->ReleaseScratchRegister(reg);
}

bool Amd64Encoder::IsScratchRegisterReleased(Reg reg) const
{
    return (static_cast<Amd64RegisterDescription *>(GetRegfile()))->IsScratchRegisterReleased(reg);
}

RegMask Amd64Encoder::GetScratchRegistersMask() const
{
    return (static_cast<const Amd64RegisterDescription *>(GetRegfile()))->GetScratchRegistersMask();
}

RegMask Amd64Encoder::GetScratchFpRegistersMask() const
{
    return (static_cast<const Amd64RegisterDescription *>(GetRegfile()))->GetScratchFpRegistersMask();
}

RegMask Amd64Encoder::GetAvailableScratchRegisters() const
{
    auto regfile = static_cast<const Amd64RegisterDescription *>(GetRegfile());
    return RegMask(regfile->GetScratchRegisters().GetMask());
}

VRegMask Amd64Encoder::GetAvailableScratchFpRegisters() const
{
    auto regfile = static_cast<const Amd64RegisterDescription *>(GetRegfile());
    return VRegMask(regfile->GetScratchFPRegisters().GetMask());
}

TypeInfo Amd64Encoder::GetRefType()
{
    return INT64_TYPE;
}

void *Amd64Encoder::BufferData() const
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    return GetMasm()->bufferData();
}

size_t Amd64Encoder::BufferSize() const
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    return GetMasm()->offset();
}

void Amd64Encoder::MakeLibCall(Reg dst, Reg src0, Reg src1, void *entryPoint)
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

        if (src0.GetId() != asmjit::x86::xmm0.id() || src1.GetId() != asmjit::x86::xmm1.id()) {
            ScopedTmpRegF32 tmp(this);
            GetMasm()->movss(ArchVReg(tmp), ArchVReg(src1));
            GetMasm()->movss(asmjit::x86::xmm0, ArchVReg(src0));
            GetMasm()->movss(asmjit::x86::xmm1, ArchVReg(tmp));
        }

        MakeCall(entryPoint);

        if (dst.GetId() != asmjit::x86::xmm0.id()) {
            GetMasm()->movss(ArchVReg(dst), asmjit::x86::xmm0);
        }
    } else if (dst.GetType() == FLOAT64_TYPE) {
        if (!src0.IsFloat() || !src1.IsFloat()) {
            SetFalseResult();
            return;
        }

        if (src0.GetId() != asmjit::x86::xmm0.id() || src1.GetId() != asmjit::x86::xmm1.id()) {
            ScopedTmpRegF64 tmp(this);
            GetMasm()->movsd(ArchVReg(tmp), ArchVReg(src1));
            GetMasm()->movsd(asmjit::x86::xmm0, ArchVReg(src0));
            GetMasm()->movsd(asmjit::x86::xmm1, ArchVReg(tmp));
        }

        MakeCall(entryPoint);

        if (dst.GetId() != asmjit::x86::xmm0.id()) {
            GetMasm()->movsd(ArchVReg(dst), asmjit::x86::xmm0);
        }
    } else {
        UNREACHABLE();
    }
}

template <bool IS_STORE>
void Amd64Encoder::LoadStoreRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp)
{
    for (size_t i {0}; i < registers.size(); ++i) {
        if (!registers.test(i)) {
            continue;
        }

        asmjit::x86::Mem mem = asmjit::x86::ptr(asmjit::x86::rsp, (slot + i - startReg) * DOUBLE_WORD_SIZE_BYTES);

        if constexpr (IS_STORE) {  // NOLINT
            if (isFp) {
                GetMasm()->movsd(mem, asmjit::x86::xmm(i));
            } else {
                GetMasm()->mov(mem, asmjit::x86::gpq(ConvertRegNumber(i)));
            }
        } else {  // NOLINT
            if (isFp) {
                GetMasm()->movsd(asmjit::x86::xmm(i), mem);
            } else {
                GetMasm()->mov(asmjit::x86::gpq(ConvertRegNumber(i)), mem);
            }
        }
    }
}

template <bool IS_STORE>
void Amd64Encoder::LoadStoreRegisters(RegMask registers, bool isFp, int32_t slot, Reg base, RegMask mask)
{
    auto baseReg = ArchReg(base);
    bool hasMask = mask.any();
    int32_t index = hasMask ? static_cast<int32_t>(mask.GetMinRegister()) : 0;
    slot -= index;
    for (size_t i = index; i < registers.size(); ++i) {
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

        // `-1` because we've incremented `index` in advance
        asmjit::x86::Mem mem = asmjit::x86::ptr(baseReg, (slot + index - 1) * DOUBLE_WORD_SIZE_BYTES);

        if constexpr (IS_STORE) {  // NOLINT
            if (isFp) {
                GetMasm()->movsd(mem, asmjit::x86::xmm(i));
            } else {
                GetMasm()->mov(mem, asmjit::x86::gpq(ConvertRegNumber(i)));
            }
        } else {  // NOLINT
            if (isFp) {
                GetMasm()->movsd(asmjit::x86::xmm(i), mem);
            } else {
                GetMasm()->mov(asmjit::x86::gpq(ConvertRegNumber(i)), mem);
            }
        }
    }
}

void Amd64Encoder::SaveRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp)
{
    LoadStoreRegisters<true>(registers, slot, startReg, isFp);
}

void Amd64Encoder::LoadRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp)
{
    LoadStoreRegisters<false>(registers, slot, startReg, isFp);
}

void Amd64Encoder::SaveRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask)
{
    LoadStoreRegisters<true>(registers, isFp, slot, base, mask);
}

void Amd64Encoder::LoadRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask)
{
    LoadStoreRegisters<false>(registers, isFp, slot, base, mask);
}

void Amd64Encoder::PushRegisters(RegMask registers, bool isFp)
{
    for (size_t i = 0; i < registers.size(); i++) {
        if (registers[i]) {
            if (isFp) {
                GetMasm()->sub(asmjit::x86::rsp, DOUBLE_WORD_SIZE_BYTES);
                GetMasm()->movsd(asmjit::x86::ptr(asmjit::x86::rsp), ArchVReg(Reg(i, FLOAT64_TYPE)));
            } else {
                GetMasm()->push(asmjit::x86::gpq(ConvertRegNumber(i)));
            }
        }
    }
}

void Amd64Encoder::PopRegisters(RegMask registers, bool isFp)
{
    for (ssize_t i = registers.size() - 1; i >= 0; i--) {
        if (registers[i]) {
            if (isFp) {
                GetMasm()->movsd(ArchVReg(Reg(i, FLOAT64_TYPE)), asmjit::x86::ptr(asmjit::x86::rsp));
                GetMasm()->add(asmjit::x86::rsp, DOUBLE_WORD_SIZE_BYTES);
            } else {
                GetMasm()->pop(asmjit::x86::gpq(ConvertRegNumber(i)));
            }
        }
    }
}

asmjit::x86::Assembler *Amd64Encoder::GetMasm() const
{
    ASSERT(masm_ != nullptr);
    return masm_;
}

size_t Amd64Encoder::GetLabelAddress(LabelHolder::LabelId label)
{
    auto code = GetMasm()->code();
    ASSERT(code->isLabelBound(label));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return code->baseAddress() + code->labelOffset(label);
}

bool Amd64Encoder::LabelHasLinks(LabelHolder::LabelId label)
{
    auto code = GetMasm()->code();
    auto entry = code->labelEntry(label);
    return entry->links() != nullptr;
}

template <typename T, size_t N>
void Amd64Encoder::CopyArrayToXmm(Reg xmm, const std::array<T, N> &arr)
{
    static constexpr auto SIZE {N * sizeof(T)};
    static_assert((SIZE == DOUBLE_WORD_SIZE_BYTES) || (SIZE == 2U * DOUBLE_WORD_SIZE_BYTES));
    ASSERT(xmm.GetType() == FLOAT64_TYPE);

    auto data {reinterpret_cast<const uint64_t *>(arr.data())};

    ScopedTmpRegU64 tmpGpr(this);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    GetMasm()->mov(ArchReg(tmpGpr), asmjit::imm(data[0]));
    GetMasm()->movq(ArchVReg(xmm), ArchReg(tmpGpr));

    if constexpr (SIZE == 2U * DOUBLE_WORD_SIZE_BYTES) {
        ScopedTmpRegF64 tmpXmm(this);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        GetMasm()->mov(ArchReg(tmpGpr), asmjit::imm(data[1]));
        GetMasm()->movq(ArchVReg(tmpXmm), ArchReg(tmpGpr));
        GetMasm()->unpcklpd(ArchVReg(xmm), ArchVReg(tmpXmm));
    }
}

template <typename T>
void Amd64Encoder::CopyImmToXmm(Reg xmm, T imm)
{
    static_assert((sizeof(imm) == WORD_SIZE_BYTES) || (sizeof(imm) == DOUBLE_WORD_SIZE_BYTES));
    ASSERT(xmm.GetSize() == BYTE_SIZE * sizeof(imm));

    if constexpr (sizeof(imm) == WORD_SIZE_BYTES) {  // NOLINT
        ScopedTmpRegU32 tmpGpr(this);
        GetMasm()->mov(ArchReg(tmpGpr), asmjit::imm(bit_cast<uint32_t>(imm)));
        GetMasm()->movd(ArchVReg(xmm), ArchReg(tmpGpr));
    } else {  // NOLINT
        ScopedTmpRegU64 tmpGpr(this);
        GetMasm()->mov(ArchReg(tmpGpr), asmjit::imm(bit_cast<uint64_t>(imm)));
        GetMasm()->movq(ArchVReg(xmm), ArchReg(tmpGpr));
    }
}

size_t Amd64Encoder::DisasmInstr(std::ostream &stream, size_t pc, ssize_t codeOffset) const
{
    if (codeOffset < 0) {
        (const_cast<Amd64Encoder *>(this))->Finalize();
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    Span code(GetMasm()->bufferData(), GetMasm()->offset());

    [[maybe_unused]] size_t dataLeft = code.Size() - pc;
    [[maybe_unused]] constexpr size_t LENGTH = ZYDIS_MAX_INSTRUCTION_LENGTH;  // 15 bytes is max inst length in amd64

    // Initialize decoder context
    ZydisDecoder decoder;
    [[maybe_unused]] bool res =
        ZYAN_SUCCESS(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64));

    // Initialize formatter
    ZydisFormatter formatter;
    res &= ZYAN_SUCCESS(ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_ATT));
    ZydisFormatterSetProperty(&formatter, ZYDIS_FORMATTER_PROP_FORCE_RELATIVE_BRANCHES, 1);
    ASSERT(res);

    ZydisDecodedInstruction instruction;

    res &= ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, &code[pc], std::min(LENGTH, dataLeft), &instruction));

    // Format & print the binary instruction structure to human readable format
    char buffer[256];  // NOLINT (modernize-avoid-c-arrays, readability-identifier-naming, readability-magic-numbers)
    res &= ZYAN_SUCCESS(
        ZydisFormatterFormatInstruction(&formatter, &instruction, buffer, sizeof(buffer), uintptr_t(&code[pc])));

    ASSERT(res);

    // Print disassembly
    if (codeOffset < 0) {
        stream << buffer;
    } else {
        stream << std::setw(0x8) << std::right << std::setfill('0') << std::hex << pc + codeOffset << std::dec
               << std::setfill(' ') << ": " << buffer;
    }

    return pc + instruction.length;
}

void Amd64Encoder::EncodeCompressedStringCharAt(ArgsCompressedStringCharAt &&args)
{
    auto [dst, str, idx, length, tmp, dataOffset, shift] = args;
    ASSERT(dst.GetSize() == HALF_SIZE);

    auto labelNotCompressed = CreateLabel();
    auto labelCharLoaded = CreateLabel();

    EncodeAdd(tmp, str, Imm(dataOffset));

    EncodeJumpTest(labelNotCompressed, length, Imm(1U), Condition::TST_NE);

    GetMasm()->movzx(ArchReg(dst, WORD_SIZE), asmjit::x86::byte_ptr(ArchReg(tmp), ArchReg(idx)));
    EncodeJump(labelCharLoaded);

    BindLabel(labelNotCompressed);
    GetMasm()->movzx(ArchReg(dst, WORD_SIZE), asmjit::x86::word_ptr(ArchReg(tmp), ArchReg(idx), shift));

    BindLabel(labelCharLoaded);
}

}  // namespace ark::compiler::amd64
