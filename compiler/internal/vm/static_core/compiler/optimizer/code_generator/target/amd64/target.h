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

#ifndef COMPILER_OPTIMIZER_CODEGEN_TARGET_AMD64_TARGET_H
#define COMPILER_OPTIMIZER_CODEGEN_TARGET_AMD64_TARGET_H

#include "compiler/optimizer/code_generator/callconv.h"

#include "asmjit/x86.h"
#include "target_info.h"

namespace ark::compiler::amd64 {
const size_t MAX_SCALAR_PARAM_ID = 5;  // %rdi, %rsi, %rdx, %rcx, %r8, %r9
const size_t MAX_VECTOR_PARAM_ID = 7;  // %xmm0-%xmm7

class AsmJitErrorHandler : public asmjit::ErrorHandler {
public:
    explicit AsmJitErrorHandler(Encoder *encoder);
    NO_MOVE_SEMANTIC(AsmJitErrorHandler);
    NO_COPY_SEMANTIC(AsmJitErrorHandler);
    ~AsmJitErrorHandler() override = default;

    void handleError([[maybe_unused]] asmjit::Error err, [[maybe_unused]] const char *message,
                     [[maybe_unused]] asmjit::BaseEmitter *origin) override;

private:
    Encoder *encoder_ {nullptr};
};

class RegList {
public:
    explicit RegList(size_t mask) : mask_(mask)
    {
        for (size_t i = 0; i < sizeof(size_t) * BITS_PER_BYTE; ++i) {
            if (Has(i)) {
                ++count_;
            }
        }
    }

    DEFAULT_MOVE_SEMANTIC(RegList);
    DEFAULT_COPY_SEMANTIC(RegList);
    ~RegList() = default;

    explicit operator size_t() const
    {
        return mask_;
    }

    bool IsEmpty() const
    {
        return count_ == size_t(0);
    }

    size_t GetCount() const
    {
        return count_;
    }

    bool Has(size_t i) const
    {
        return (mask_ & (size_t(1) << i)) != 0;
    }

    void Add(size_t i)
    {
        if (Has(i)) {
            return;
        }
        mask_ |= size_t(1) << i;
        ++count_;
    }

    void Remove(size_t i)
    {
        if (!Has(i)) {
            return;
        }
        mask_ &= ~(size_t(1) << i);
        --count_;
    }

    size_t Pop()
    {
        ASSERT(!IsEmpty());
        size_t i = __builtin_ctzll(mask_);
        Remove(i);
        return i;
    }

    size_t GetMask() const
    {
        return mask_;
    }

private:
    size_t mask_ {0};
    size_t count_ {0};
};

class ArchMem {
public:
    explicit ArchMem(MemRef mem);
    DEFAULT_MOVE_SEMANTIC(ArchMem);
    DEFAULT_COPY_SEMANTIC(ArchMem);
    ~ArchMem() = default;

    asmjit::x86::Mem Prepare(asmjit::x86::Assembler *masm);

private:
    int64_t bigShift_ {0};
    asmjit::x86::Mem mem_;
    bool needExtendIndex_ {false};
    bool isPrepared_ {false};
};

/*
 * Scalar registers mapping:
 * +-----------+---------------------+
 * | AMD64 Reg |      Panda Reg      |
 * +-----------+---------------------+
 * | rax       | r0                  |
 * | rcx       | r1                  |
 * | rdx       | r2                  |
 * | rbx       | r3 (renamed to r11) |
 * | rsp       | r4 (renamed to r10) |
 * | rbp       | r5 (renamed to r9)  |
 * | rsi       | r6                  |
 * | rdi       | r7                  |
 * | r8        | r8                  |
 * | r9        | r9 (renamed to r5)  |
 * | r10       | r10 (renamed to r4) |
 * | r11       | r11 (renamed to r3) |
 * | r12       | r12                 |
 * | r13       | r13                 |
 * | r14       | r14                 |
 * | r15       | r15                 |
 * | <no reg>  | r16-r31             |
 * +-----------+---------------------+
 *
 * Vector registers mapping:
 * xmm[i] <-> vreg[i], 0 <= i <= 15
 */
class Amd64RegisterDescription final : public RegistersDescription {
public:
    explicit Amd64RegisterDescription(ArenaAllocator *allocator);
    NO_MOVE_SEMANTIC(Amd64RegisterDescription);
    NO_COPY_SEMANTIC(Amd64RegisterDescription);
    ~Amd64RegisterDescription() override = default;

    ArenaVector<Reg> GetCalleeSaved() override;
    void SetCalleeSaved(const ArenaVector<Reg> &regs) override;
    // Set used regs - change GetCallee
    void SetUsedRegs(const ArenaVector<Reg> &regs) override;

    RegMask GetCallerSavedRegMask() const override;
    VRegMask GetCallerSavedVRegMask() const override;
    bool IsCalleeRegister(Reg reg) override;
    Reg GetZeroReg() const override;
    bool IsZeroReg([[maybe_unused]] Reg reg) const override;
    Reg::RegIDType GetTempReg() override;
    Reg::RegIDType GetTempVReg() override;
    RegMask GetDefaultRegMask() const override;
    VRegMask GetVRegMask() override;

    // Check register mapping
    bool SupportMapping(uint32_t type) override;
    bool IsValid() const override;
    bool IsRegUsed(ArenaVector<Reg> vecReg, Reg reg) override;

public:
    // Special implementation-specific getters
    size_t GetCalleeSavedR();
    size_t GetCalleeSavedV();
    size_t GetCallerSavedR();
    size_t GetCallerSavedV();

    Reg AcquireScratchRegister(TypeInfo type);
    void AcquireScratchRegister(Reg reg);
    void ReleaseScratchRegister(Reg reg);
    bool IsScratchRegisterReleased(Reg reg) const;
    RegList GetScratchRegisters() const;
    RegList GetScratchFPRegisters() const;
    size_t GetScratchRegistersCount() const;
    size_t GetScratchFPRegistersCount() const;
    RegMask GetScratchRegistersMask() const;
    RegMask GetScratchFpRegistersMask() const;

private:
    ArenaVector<Reg> usedRegs_;

    RegList calleeSaved_ {GetCalleeRegsMask(Arch::X86_64, false).GetValue()};
    RegList callerSaved_ {GetCallerRegsMask(Arch::X86_64, false).GetValue()};

    RegList calleeSavedv_ {GetCalleeRegsMask(Arch::X86_64, true).GetValue()};
    RegList callerSavedv_ {GetCallerRegsMask(Arch::X86_64, true).GetValue()};

    RegList scratch_ {compiler::arch_info::x86_64::TEMP_REGS.to_ulong()};
    RegList scratchv_ {compiler::arch_info::x86_64::TEMP_FP_REGS.to_ulong()};
};  // Amd64RegisterDescription

class Amd64Encoder;

class Amd64LabelHolder final : public LabelHolder {
public:
    using LabelType = asmjit::Label;

    explicit Amd64LabelHolder(Encoder *enc) : LabelHolder(enc), labels_(enc->GetAllocator()->Adapter()) {};
    NO_MOVE_SEMANTIC(Amd64LabelHolder);
    NO_COPY_SEMANTIC(Amd64LabelHolder);
    ~Amd64LabelHolder() override = default;

    LabelId CreateLabel() override;
    void CreateLabels(LabelId max) override;
    void BindLabel(LabelId id) override;
    LabelType *GetLabel(LabelId id);
    LabelId Size() override;

private:
    ArenaVector<LabelType *> labels_;
    LabelId id_ {0};
    friend Amd64Encoder;
};  // Amd64LabelHolder

class Amd64Encoder final : public Encoder {
public:
    using Encoder::Encoder;
    explicit Amd64Encoder(ArenaAllocator *allocator);
    ~Amd64Encoder() override;
    NO_COPY_SEMANTIC(Amd64Encoder);
    NO_MOVE_SEMANTIC(Amd64Encoder);

    LabelHolder *GetLabels() const override;
    bool IsValid() const override;
    static constexpr auto GetTarget();

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNARY_OPERATION(opc) void Encode##opc(Reg dst, Reg src0) override; /* CC-OFF(G.PRE.09) code generation */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_OPERATION(opc)                               \
    void Encode##opc(Reg dst, Reg src0, Reg src1) override; \
    void Encode##opc(Reg dst, Reg src0, Imm src1) override; /* CC-OFF(G.PRE.09) code generation */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, MACRO) MACRO(OPCODE)

    ENCODE_MATH_LIST(INST_DEF)

#undef UNARY_OPERATION
#undef BINARY_OPERATION
#undef INST_DEF

    void EncodeNop() override;

    // Additional special instructions
    void EncodeAdd(Reg dst, Reg src0, Shift src1) override;

    void EncodeCastToBool(Reg dst, Reg src) override;
    void EncodeCast(Reg dst, bool dstSigned, Reg src, bool srcSigned) override;
    void EncodeFastPathDynamicCast(Reg dst, Reg src, LabelHolder::LabelId slow) override;
    void EncodeJsDoubleToCharCast(Reg dst, Reg src, Reg tmp, uint32_t failureResult) override;
    void EncodeMin(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeDiv(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeMod(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeDiv(Reg dst, Reg src0, Imm imm, bool isSigned) override;
    void EncodeSignedDiv(Reg dst, Reg src0, Imm imm);
    void EncodeUnsignedDiv(Reg dst, Reg src0, Imm imm);
    void EncodeMod(Reg dst, Reg src0, Imm imm, bool isSigned) override;
    void EncodeMax(Reg dst, bool dstSigned, Reg src0, Reg src1) override;

    void EncodeAddOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeSubOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeNegOverflowAndZero(compiler::LabelHolder::LabelId id, Reg dst, Reg src) override;

    void EncodeLdr(Reg dst, bool dstSigned, MemRef mem) override;
    void EncodeLdrAcquire(Reg dst, bool dstSigned, MemRef mem) override;

    void EncodeMov(Reg dst, Imm src) override;
    void EncodeStr(Reg src, MemRef mem) override;
    void EncodeStrRelease(Reg src, MemRef mem) override;
    // zerod high part: [reg.size, 64)
    void EncodeStrz(Reg src, MemRef mem) override;
    void EncodeSti(int64_t src, uint8_t srcSizeBytes, MemRef mem) override;
    void EncodeSti(float src, MemRef mem) override;
    void EncodeSti(double src, MemRef mem) override;
    // size must be 8, 16,32 or 64
    void EncodeMemCopy(MemRef memFrom, MemRef memTo, size_t size) override;
    // size must be 8, 16,32 or 64
    // zerod high part: [reg.size, 64)
    void EncodeMemCopyz(MemRef memFrom, MemRef memTo, size_t size) override;

    void EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeAtomicByteOr(Reg addr, Reg value, bool fastEncoding) override;

    void EncodeSelect(const ArgsSelect &args) override;
    void EncodeSelect(const ArgsSelectImm &args) override;
    void EncodeSelectTest(const ArgsSelect &args) override;
    void EncodeSelectTest(const ArgsSelectImm &args) override;

    void EncodeLdp(Reg dst0, Reg dst1, bool dstSigned, MemRef mem) override;
    void EncodeStp(Reg src0, Reg src1, MemRef mem) override;

    /* builtins-related encoders */
    void EncodeIsInf(Reg dst, Reg src) override;
    void EncodeIsInteger(Reg dst, Reg src) override;
    void EncodeIsSafeInteger(Reg dst, Reg src) override;
    void EncodeBitCount(Reg dst, Reg src) override;
    void EncodeCountLeadingZeroBits(Reg dst, Reg src) override;
    void EncodeCountTrailingZeroBits(Reg dst, Reg src) override;
    void EncodeCeil([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeFloor([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRint([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeTrunc([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRoundAway([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRoundToPInfReturnFloat([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRoundToPInfReturnScalar([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeReverseBytes(Reg dst, Reg src) override;
    void EncodeReverseBits(Reg dst, Reg src) override;
    void EncodeReverseHalfWords(Reg dst, Reg src) override;
    void EncodeFpToBits(Reg dst, Reg src) override;
    void EncodeMoveBitsRaw(Reg dst, Reg src) override;
    void EncodeUnsignedExtendBytesToShorts(Reg dst, Reg src) override;

    bool CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signedCompare) override;
    bool CanEncodeImmLogical(uint64_t imm, uint32_t size) override;
    bool CanEncodeScale(uint64_t imm, uint32_t size) override;
    bool CanEncodeBitCount() override;
    bool CanEncodeCompressedStringCharAt() override;
    bool CanOptimizeImmDivMod(uint64_t imm, bool isSigned) const override;

    void EncodeCompressedStringCharAt(ArgsCompressedStringCharAt &&args) override;
    void EncodeCompareAndSwap(Reg dst, Reg obj, Reg offset, Reg val, Reg newval) override;
    void EncodeCompareAndSwap(Reg dst, Reg addr, Reg val, Reg newval) override;
    void EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val) override;
    void EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, Reg tmp) override;
    void EncodeMemoryBarrier(memory_order::Order order) override;
    void EncodeStackOverflowCheck(ssize_t offset) override;

    void EncodeGetCurrentPc(Reg dst) override;

    size_t GetCursorOffset() const override;
    void SetCursorOffset(size_t offset) override;

    Reg AcquireScratchRegister(TypeInfo type) override;
    void AcquireScratchRegister(Reg reg) override;
    void ReleaseScratchRegister(Reg reg) override;
    bool IsScratchRegisterReleased(Reg reg) const override;
    RegMask GetScratchRegistersMask() const override;
    RegMask GetScratchFpRegistersMask() const override;
    RegMask GetAvailableScratchRegisters() const override;
    VRegMask GetAvailableScratchFpRegisters() const override;
    TypeInfo GetRefType() override;

    size_t DisasmInstr(std::ostream &stream, size_t pc, ssize_t codeOffset) const override;

    void *BufferData() const override;
    size_t BufferSize() const override;

    bool InitMasm() override;
    void Finalize() override;

    void MakeCall(compiler::RelocationInfo *relocation) override;
    void MakeCall(LabelHolder::LabelId id) override;
    void MakeCall(const void *entryPoint) override;
    void MakeCall(Reg reg) override;
    void MakeCall(MemRef entryPoint) override;
    void MakeCallAot(intptr_t offset) override;
    void MakeCallByOffset(intptr_t offset) override;
    void MakeLoadAotTable(intptr_t offset, Reg reg) override;
    void MakeLoadAotTableAddr(intptr_t offset, Reg addr, Reg val) override;
    bool CanMakeCallByOffset(intptr_t offset) override;

    // Encode unconditional branch
    void EncodeJump(LabelHolder::LabelId id) override;

    // Encode jump with compare to zero
    void EncodeJump(LabelHolder::LabelId id, Reg src, Condition cc) override;

    // Compare reg and immediate and branch
    void EncodeJump(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc) override;

    // Compare two regs and branch
    void EncodeJump(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc) override;

    // Compare reg and immediate and branch
    void EncodeJumpTest(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc) override;

    // Compare two regs and branch
    void EncodeJumpTest(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc) override;

    // Encode jump by register value
    void EncodeJump(Reg dst) override;

    void EncodeJump(RelocationInfo *relocation) override;
    void EncodeBitTestAndBranch(LabelHolder::LabelId id, compiler::Reg reg, uint32_t bitPos, bool bitValue) override;
    void EncodeAbort() override;
    void EncodeReturn() override;

    void MakeLibCall(Reg dst, Reg src0, Reg src1, void *entryPoint);

    void SaveRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp) override;
    void LoadRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp) override;
    void SaveRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask) override;
    void LoadRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask) override;
    void PushRegisters(RegMask registers, bool isFp) override;
    void PopRegisters(RegMask registers, bool isFp) override;

    template <typename Func>
    void EncodeRelativePcMov(Reg reg, intptr_t offset, Func encodeInstruction);

    asmjit::x86::Assembler *GetMasm() const;
    size_t GetLabelAddress(LabelHolder::LabelId label) override;
    bool LabelHasLinks(LabelHolder::LabelId label) override;

private:
    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp);

    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, bool isFp, int32_t slot, Reg base, RegMask mask);

    inline Reg MakeShift(Shift shift);

    template <typename T>
    void EncodeReverseBitsImpl(Reg dst0, Reg src0);

    void EncodeRoundToPInfFloat(Reg dst, Reg src);
    void EncodeRoundToPInfDouble(Reg dst, Reg src);

    void EncodeCastFloatToScalar(Reg dst, bool dstSigned, Reg src);
    inline void EncodeCastFloatSignCheckRange(Reg dst, Reg src, const asmjit::Label &end);
    inline void EncodeCastFloatUnsignCheckRange(Reg dst, Reg src, const asmjit::Label &end);
    void EncodeCastFloatCheckNan(Reg dst, Reg src, const asmjit::Label &end);
    void EncodeCastFloatCheckRange(Reg dst, Reg src, const asmjit::Label &end, int64_t minValue, uint64_t maxValue);
    void EncodeCastFloat32ToUint64(Reg dst, Reg src);
    void EncodeCastFloat64ToUint64(Reg dst, Reg src);

    void EncodeCastScalarToFloat(Reg dst, Reg src, bool srcSigned);
    void EncodeCastScalarToFloatUnsignDouble(Reg dst, Reg src);
    void EncodeCastScalar(Reg dst, bool dstSigned, Reg src, bool srcSigned);

    void EncodeDivFloat(Reg dst, Reg src0, Reg src1);
    void EncodeModFloat(Reg dst, Reg src0, Reg src1);
    template <bool IS_MAX>
    void EncodeMinMaxFp(Reg dst, Reg src0, Reg src1);

    template <typename T, size_t N>
    void CopyArrayToXmm(Reg xmm, const std::array<T, N> &arr);

    template <typename T>
    void CopyImmToXmm(Reg xmm, T imm);

    void EncodeCompareAndSwap(Reg dst, Reg obj, const Reg *offset, Reg val, Reg newval);
    void EncodeCmpFracWithZero(Reg src);

private:
    Amd64LabelHolder *labels_ {nullptr};
    asmjit::ErrorHandler *errorHandler_ {nullptr};
    asmjit::CodeHolder *codeHolder_ {nullptr};
    asmjit::x86::Assembler *masm_ {nullptr};
};  // Amd64Encoder

class Amd64ParameterInfo : public ParameterInfo {
public:
    std::variant<Reg, uint8_t> GetNativeParam(const TypeInfo &type) override;
    Location GetNextLocation(DataType::Type type) override;
};

class Amd64CallingConvention : public CallingConvention {
public:
    Amd64CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr, CallConvMode mode);
    NO_MOVE_SEMANTIC(Amd64CallingConvention);
    NO_COPY_SEMANTIC(Amd64CallingConvention);
    ~Amd64CallingConvention() override = default;

    static constexpr auto GetTarget();
    bool IsValid() const override;

    void GeneratePrologue(const FrameInfo &frameInfo) override;
    void GenerateEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override;
    void GenerateNativePrologue(const FrameInfo &frameInfo) override;
    void GenerateNativeEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override;
    void GenerateEpilogueHead(const FrameInfo &frameInfo, std::function<void()> postJob) override;

    void *GetCodeEntry() override;
    uint32_t GetCodeSize() override;

    // Pushes regs and returns number of regs(from boths vectors)
    size_t PushRegs(RegList regs, RegList vregs);
    // Pops regs and returns number of regs(from boths vectors)
    size_t PopRegs(RegList regs, RegList vregs);

    // Calculating information about parameters and save regs_offset registers for special needs
    ParameterInfo *GetParameterInfo(uint8_t regsOffset) override;
    asmjit::x86::Assembler *GetMasm();

private:
    void GenerateEpilogueImpl(const FrameInfo &frameInfo, const std::function<void()> &postJob);
};  // Amd64CallingConvention
}  // namespace ark::compiler::amd64

#endif  // COMPILER_OPTIMIZER_CODEGEN_TARGET_AMD64_TARGET_H
