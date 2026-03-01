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

#ifndef COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH64_TARGET_H
#define COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH64_TARGET_H

#include "operands.h"
#include "encode.h"
#include "callconv.h"
#include "target_info.h"

#ifdef USE_VIXL_ARM64
#include "aarch64/constants-aarch64.h"
#include "aarch64/macro-assembler-aarch64.h"
#include "aarch64/operands-aarch64.h"
#else
#error "Wrong build type, please add VIXL in build"
#endif  // USE_VIXL_ARM64

namespace ark::compiler::aarch64 {
// Ensure that vixl has same callee regs as our arch util
static constexpr auto CALLEE_REG_LIST =
    vixl::aarch64::CPURegList(vixl::aarch64::CPURegister::kRegister, vixl::aarch64::kXRegSize,
                              GetFirstCalleeReg(Arch::AARCH64, false), GetLastCalleeReg(Arch::AARCH64, false));
static constexpr auto CALLEE_VREG_LIST =
    vixl::aarch64::CPURegList(vixl::aarch64::CPURegister::kRegister, vixl::aarch64::kDRegSize,
                              GetFirstCalleeReg(Arch::AARCH64, true), GetLastCalleeReg(Arch::AARCH64, true));
static constexpr auto CALLER_REG_LIST =
    vixl::aarch64::CPURegList(vixl::aarch64::CPURegister::kRegister, vixl::aarch64::kXRegSize,
                              GetCallerRegsMask(Arch::AARCH64, false).GetValue());
static constexpr auto CALLER_VREG_LIST = vixl::aarch64::CPURegList(
    vixl::aarch64::CPURegister::kRegister, vixl::aarch64::kXRegSize, GetCallerRegsMask(Arch::AARCH64, true).GetValue());

static_assert(vixl::aarch64::kCalleeSaved.GetList() == CALLEE_REG_LIST.GetList());
static_assert(vixl::aarch64::kCalleeSavedV.GetList() == CALLEE_VREG_LIST.GetList());
static_assert(vixl::aarch64::kCallerSaved.GetList() == CALLER_REG_LIST.GetList());
static_assert(vixl::aarch64::kCallerSavedV.GetList() == CALLER_VREG_LIST.GetList());

const size_t MAX_SCALAR_PARAM_ID = 7;  // r0-r7
const size_t MAX_VECTOR_PARAM_ID = 7;  // v0-v7

class Aarch64CallingConvention;

inline vixl::aarch64::Register VixlRegCaseScalar(Reg reg)
{
    size_t regSize = reg.GetSize();
    if (regSize < WORD_SIZE) {
        regSize = WORD_SIZE;
    }
    if (regSize > DOUBLE_WORD_SIZE) {
        regSize = DOUBLE_WORD_SIZE;
    }
    auto vixlReg = vixl::aarch64::Register(reg.GetId(), regSize);
    ASSERT(vixlReg.IsValid());
    return vixlReg;
}

inline vixl::aarch64::Register VixlReg(Reg reg)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        return VixlRegCaseScalar(reg);
    }
    if (reg.GetId() == vixl::aarch64::sp.GetCode()) {
        return vixl::aarch64::sp;
    }

    // Invalid register type
    UNREACHABLE();
    return vixl::aarch64::xzr;
}

inline vixl::aarch64::Register VixlRegCaseScalar(Reg reg, const uint8_t size)
{
    auto vixlReg = vixl::aarch64::Register(reg.GetId(), (size < WORD_SIZE ? WORD_SIZE : size));
    ASSERT(vixlReg.IsValid());
    return vixlReg;
}

inline vixl::aarch64::Register VixlReg(Reg reg, const uint8_t size)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        return VixlRegCaseScalar(reg, size);
    }
    if (reg.GetId() == vixl::aarch64::sp.GetCode()) {
        return vixl::aarch64::sp;
    }

    // Invalid register type
    UNREACHABLE();
    return vixl::aarch64::xzr;
}

inline vixl::aarch64::Operand VixlImm(const int64_t imm)
{
    return vixl::aarch64::Operand(imm);
}

inline vixl::aarch64::Operand VixlImm(Imm imm)
{
    return vixl::aarch64::Operand(imm.GetAsInt());
}

class Aarch64RegisterDescription final : public RegistersDescription {
public:
    explicit Aarch64RegisterDescription(ArenaAllocator *allocator);

    NO_MOVE_SEMANTIC(Aarch64RegisterDescription);
    NO_COPY_SEMANTIC(Aarch64RegisterDescription);
    ~Aarch64RegisterDescription() override = default;

    ArenaVector<Reg> GetCalleeSaved() override;
    void SetCalleeSaved(const ArenaVector<Reg> &regs) override;
    // Set used regs - change GetCallee
    void SetUsedRegs(const ArenaVector<Reg> &regs) override;

    RegMask GetCallerSavedRegMask() const override;
    VRegMask GetCallerSavedVRegMask() const override;
    bool IsCalleeRegister(Reg reg) override;
    Reg GetZeroReg() const override;
    bool IsZeroReg(Reg reg) const override;
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
    vixl::aarch64::CPURegList GetCalleeSavedR();
    vixl::aarch64::CPURegList GetCalleeSavedV();
    vixl::aarch64::CPURegList GetCallerSavedR();
    vixl::aarch64::CPURegList GetCallerSavedV();
    uint8_t GetAlignmentVreg(bool isCallee);

private:
    ArenaVector<Reg> usedRegs_;

    vixl::aarch64::CPURegList calleeSaved_ {vixl::aarch64::kCalleeSaved};
    vixl::aarch64::CPURegList callerSaved_ {vixl::aarch64::kCallerSaved};

    vixl::aarch64::CPURegList calleeSavedv_ {vixl::aarch64::kCalleeSavedV};
    vixl::aarch64::CPURegList callerSavedv_ {vixl::aarch64::kCallerSavedV};

    static constexpr const uint8_t UNDEF_VREG = std::numeric_limits<uint8_t>::max();
    // The number of register in Push/Pop list must be even. The regisers are used for alignment vetor register lists
    uint8_t allignmentVregCallee_ {UNDEF_VREG};
    uint8_t allignmentVregCaller_ {UNDEF_VREG};
};  // Aarch64RegisterDescription

class Aarch64Encoder;

class Aarch64LabelHolder final : public LabelHolder {
public:
    using LabelType = vixl::aarch64::Label;
    explicit Aarch64LabelHolder(Encoder *enc) : LabelHolder(enc), labels_(enc->GetAllocator()->Adapter()) {};

    NO_MOVE_SEMANTIC(Aarch64LabelHolder);
    NO_COPY_SEMANTIC(Aarch64LabelHolder);
    ~Aarch64LabelHolder() override = default;

    LabelId CreateLabel() override;
    void CreateLabels(LabelId size) override;
    void BindLabel(LabelId id) override;
    LabelType *GetLabel(LabelId id) const;
    LabelId Size() override;

private:
    ArenaVector<LabelType *> labels_;
    LabelId id_ {0};
    friend Aarch64Encoder;
};  // Aarch64LabelHolder

class Aarch64Encoder final : public Encoder {
public:
    explicit Aarch64Encoder(ArenaAllocator *allocator);
    ~Aarch64Encoder() override;
    NO_COPY_SEMANTIC(Aarch64Encoder);
    NO_MOVE_SEMANTIC(Aarch64Encoder);

    LabelHolder *GetLabels() const override;
    bool IsValid() const override;
    static constexpr auto GetTarget();
    void LoadPcRelative(Reg reg, intptr_t offset, Reg regAddr = INVALID_REGISTER);
    void SetMaxAllocatedBytes(size_t size) override;

#ifndef PANDA_MINIMAL_VIXL
    inline vixl::aarch64::Decoder &GetDecoder() const;
    inline vixl::aarch64::Disassembler &GetDisasm() const;
#endif

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
    void CheckAlignment(MemRef mem, size_t size);

    // Additional special instructions
    void EncodeAdd(Reg dst, Reg src0, Shift src1) override;
    void EncodeSub(Reg dst, Reg src0, Shift src1) override;
    void EncodeAnd(Reg dst, Reg src0, Shift src1) override;
    void EncodeOr(Reg dst, Reg src0, Shift src1) override;
    void EncodeXor(Reg dst, Reg src0, Shift src1) override;
    void EncodeOrNot(Reg dst, Reg src0, Shift src1) override;
    void EncodeAndNot(Reg dst, Reg src0, Shift src1) override;
    void EncodeXorNot(Reg dst, Reg src0, Shift src1) override;
    void EncodeNeg(Reg dst, Shift src) override;

    void EncodeCast(Reg dst, bool dstSigned, Reg src, bool srcSigned) override;
    void EncodeFastPathDynamicCast(Reg dst, Reg src, LabelHolder::LabelId slow) override;
    void EncodeJsDoubleToCharCast(Reg dst, Reg src) override;
    void EncodeJsDoubleToCharCast(Reg dst, Reg src, Reg tmp, uint32_t failureResult) override;
    void EncodeCastToBool(Reg dst, Reg src) override;

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
    void EncodeLdrPreIndex(Reg dst, bool dstSigned, Reg srcBase, Imm offset) override;
    void EncodeLdrPostIndex(Reg dst, bool dstSigned, Reg srcBase, Imm offset) override;
    void EncodeLdrAcquire(Reg dst, bool dstSigned, MemRef mem) override;
    void EncodeLdrAcquireInvalid(Reg dst, bool dstSigned, MemRef mem);
    void EncodeLdrAcquireScalar(Reg dst, bool dstSigned, MemRef mem);

    void EncodeMov(Reg dst, Imm src) override;
    void EncodeStr(Reg src, MemRef mem) override;
    void EncodeStrPreIndex(Reg src, Reg dstBase, Imm offset) override;
    void EncodeStrPostIndex(Reg src, Reg dstBase, Imm offset) override;
    void EncodeStrRelease(Reg src, MemRef mem) override;

    void EncodeLdrExclusive(Reg dst, Reg addr, bool acquire) override;
    void EncodeStrExclusive(Reg dst, Reg src, Reg addr, bool release) override;

    // zerod high part: [reg.size, 64)
    void EncodeStrz(Reg src, MemRef mem) override;
    void EncodeSti(int64_t src, uint8_t srcSizeBytes, MemRef mem) override;
    void EncodeSti(double src, MemRef mem) override;
    void EncodeSti(float src, MemRef mem) override;
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
    void EncodeSelectTransform(const ArgsSelectTransform &args) override;
    void EncodeSelectTransform(const ArgsSelectImmTransform &args) override;
    void EncodeSelectTest(const ArgsSelect &args) override;
    void EncodeSelectTest(const ArgsSelectImm &args) override;
    void EncodeSelectTestTransform(const ArgsSelectTransform &args) override;
    void EncodeSelectTestTransform(const ArgsSelectImmTransform &args) override;

    void EncodeLdp(Reg dst0, Reg dst1, bool dstSigned, MemRef mem) override;
    void EncodeLdpPreIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, Imm offset) override;
    void EncodeLdpPostIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, Imm offset) override;

    void EncodeStp(Reg src0, Reg src1, MemRef mem) override;
    void EncodeStpPreIndex(Reg src0, Reg src1, Reg dstBase, Imm offset) override;
    void EncodeStpPostIndex(Reg src0, Reg src1, Reg dstBase, Imm offset) override;

    void EncodeMAdd(Reg dst, Reg src0, Reg src1, Reg src2) override;
    void EncodeMSub(Reg dst, Reg src0, Reg src1, Reg src2) override;

    void EncodeMNeg(Reg dst, Reg src0, Reg src1) override;
    void EncodeXorNot(Reg dst, Reg src0, Reg src1) override;
    void EncodeAndNot(Reg dst, Reg src0, Reg src1) override;
    void EncodeOrNot(Reg dst, Reg src0, Reg src1) override;

    void EncodeExtractBits(Reg dst, Reg src0, Imm imm1, Imm imm2, bool signExt) override;

    /* builtins-related encoders */
    void EncodeIsInf(Reg dst, Reg src) override;
    void EncodeIsInteger(Reg dst, Reg src) override;
    void EncodeIsSafeInteger(Reg dst, Reg src) override;
    void EncodeBitCount(Reg dst, Reg src) override;
    void EncodeCountLeadingZeroBits(Reg dst, Reg src) override;
    void EncodeCountTrailingZeroBits(Reg dst, Reg src) override;
    void EncodeCeil(Reg dst, Reg src) override;
    void EncodeFloor(Reg dst, Reg src) override;
    void EncodeRint(Reg dst, Reg src) override;
    void EncodeTrunc(Reg dst, Reg src) override;
    void EncodeRoundAway(Reg dst, Reg src) override;
    void EncodeRoundToPInfReturnScalar(Reg dst, Reg src) override;
    void EncodeRoundToPInfReturnFloat(Reg dst, Reg src) override;

    void EncodeReverseBytes(Reg dst, Reg src) override;
    void EncodeReverseBits(Reg dst, Reg src) override;
    void EncodeReverseHalfWords(Reg dst, Reg src) override;
    void EncodeRotate(Reg dst, Reg src1, Reg src2, bool isRor) override;
    void EncodeSignum(Reg dst, Reg src) override;
    void EncodeCompressedStringCharAt(ArgsCompressedStringCharAt &&args) override;
    void EncodeCompressedStringCharAtI(ArgsCompressedStringCharAtI &&args) override;

    void EncodeFpToBits(Reg dst, Reg src) override;
    void EncodeMoveBitsRaw(Reg dst, Reg src) override;
    void EncodeGetTypeSize(Reg size, Reg type) override;

    bool CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signedCompare) override;
    bool CanEncodeImmLogical(uint64_t imm, uint32_t size) override;
    bool CanEncodeScale(uint64_t imm, uint32_t size) override;
    bool CanEncodeFloatSelect() override;
    bool CanEncodeSelectTransformFor(DataType::Type type) override;
    bool CanEncodeBitfieldExtractionFor(DataType::Type type) override;

    void EncodeCompareAndSwap(Reg dst, Reg obj, Reg offset, Reg val, Reg newval) override;
    void EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val) override;
    void EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, Reg tmp) override;
    void EncodeMemoryBarrier(memory_order::Order order) override;

    void EncodeStackOverflowCheck(ssize_t offset) override;
    void EncodeCrc32Update(Reg dst, Reg crcReg, Reg valReg) override;
    void EncodeCompressEightUtf16ToUtf8CharsUsingSimd(Reg srcAddr, Reg dstAddr) override;
    void EncodeCompressSixteenUtf16ToUtf8CharsUsingSimd(Reg srcAddr, Reg dstAddr) override;
    void EncodeMemCharU8X32UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp) override;
    void EncodeMemCharU16X16UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp) override;
    void EncodeMemCharU8X16UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp) override;
    void EncodeMemCharU16X8UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp) override;
    void EncodeMemLastCharU16X8UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp) override;
    void EncodeMemLastCharU8X16UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp) override;
    void EncodeCharsToUpperCaseU8X16UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp) override;
    void EncodeCharsToUpperCaseU8X8UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp) override;
    void EncodeCharsToLowerCaseU8X16UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp) override;
    void EncodeCharsToLowerCaseU8X8UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp) override;

    void EncodeUnsignedExtendBytesToShorts(Reg dst, Reg src) override;

    void EncodeGetCurrentPc(Reg dst) override;

    bool CanEncodeBitCount() override;
    bool CanEncodeCompressedStringCharAt() override;
    bool CanEncodeCompressedStringCharAtI() override;
    bool CanEncodeMAdd() override;
    bool CanEncodeMSub() override;
    bool CanEncodeMNeg() override;
    bool CanEncodeOrNot() override;
    bool CanEncodeAndNot() override;
    bool CanEncodeXorNot() override;
    bool CanEncodeShiftedOperand(ShiftOpcode opcode, ShiftType shiftType) override;
    bool CanOptimizeImmDivMod(uint64_t imm, bool isSigned) const override;

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
    void MakeCall(MemRef entryPoint) override;
    void MakeCall(Reg reg) override;

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

    void MakeLibCall(Reg dst, Reg src0, Reg src1, const void *entryPoint);

    void SaveRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp) override;
    void LoadRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp) override;
    void SaveRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask) override;
    void LoadRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask) override;
    void PushRegisters(RegMask registers, bool isFp) override;
    void PopRegisters(RegMask registers, bool isFp) override;

    vixl::aarch64::MacroAssembler *GetMasm() const;
    size_t GetLabelAddress(LabelHolder::LabelId label) override;
    bool LabelHasLinks(LabelHolder::LabelId label) override;

private:
    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp);

    template <bool IS_STORE>
    void LoadStoreRegistersLoop(RegMask registers, ssize_t slot, size_t startReg, bool isFp,
                                const vixl::aarch64::Register &baseReg);

    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, bool isFp, int32_t slot, Reg base, RegMask mask);

    template <bool IS_STORE>
    void LoadStoreRegistersMainLoop(RegMask registers, bool isFp, int32_t slot, Reg base, RegMask mask);

    void EncodeCastFloat(Reg dst, bool dstSigned, Reg src, bool srcSigned);
    // This function not used, but it is working and can be used.
    // Unlike "EncodeCastFloat", it implements castes float32/64 to int8/16.
    void EncodeCastFloatWithSmallDst(Reg dst, bool dstSigned, Reg src, bool srcSigned);

    void EncodeCastScalar(Reg dst, bool dstSigned, Reg src, bool srcSigned);

    void EncodeCastSigned(Reg dst, Reg src);
    void EncodeCastUnsigned(Reg dst, Reg src);

    void EncodeCastCheckNaN(Reg dst, Reg src, LabelHolder::LabelId exitId);

    void EncodeFMod(Reg dst, Reg src0, Reg src1);
    void HandleChar(int32_t ch, const vixl::aarch64::Register &tmp, vixl::aarch64::Label *labelNotFound,
                    vixl::aarch64::Label *labelUncompressedString);

    void EncodeCmpFracWithZero(Reg src);

private:
    Aarch64LabelHolder *labels_ {nullptr};
    vixl::aarch64::MacroAssembler *masm_ {nullptr};
#ifndef PANDA_MINIMAL_VIXL
    mutable std::optional<vixl::aarch64::Decoder> decoder_;
    mutable std::optional<vixl::aarch64::Disassembler> disasm_;
#endif
    bool lrAcquired_ {false};
};  // Aarch64Encoder

class Aarch64ParameterInfo : public ParameterInfo {
public:
    std::variant<Reg, uint8_t> GetNativeParam(const TypeInfo &type) override;
    Location GetNextLocation(DataType::Type type) override;
};

class Aarch64CallingConvention : public CallingConvention {
public:
    Aarch64CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr, CallConvMode mode);
    NO_MOVE_SEMANTIC(Aarch64CallingConvention);
    NO_COPY_SEMANTIC(Aarch64CallingConvention);
    ~Aarch64CallingConvention() override = default;

    static constexpr auto GetTarget();
    bool IsValid() const override;

    void GeneratePrologue(const FrameInfo &frameInfo) override;
    void GenerateEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override;
    void GenerateNativePrologue(const FrameInfo &frameInfo) override;
    void GenerateNativeEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override;
    void GenerateEpilogueHead(const FrameInfo &frameInfo, std::function<void()> postJob) override;

    void *GetCodeEntry() override;
    uint32_t GetCodeSize() override;

    Reg InitFlagsReg(bool hasFloatRegs);
    size_t SaveFpLr(const FrameInfo &frameInfo, [[maybe_unused]] Encoder *encoder, [[maybe_unused]] Reg fp,
                    [[maybe_unused]] Reg lr);
    void EncodeDynCallMode([[maybe_unused]] const FrameInfo &frameInfo, Encoder *encoder);

    // Pushes regs and returns number of regs(from boths vectos)
    size_t PushRegs(vixl::aarch64::CPURegList regs, vixl::aarch64::CPURegList vregs, bool isCallee);
    // Pops regs and returns number of regs(from boths vectos)
    size_t PopRegs(vixl::aarch64::CPURegList regs, vixl::aarch64::CPURegList vregs, bool isCallee);
    void PrepareToPushPopRegs(vixl::aarch64::CPURegList regs, vixl::aarch64::CPURegList vregs, bool isCallee);

    // Calculating information about parameters and save regs_offset registers for special needs
    ParameterInfo *GetParameterInfo(uint8_t regsOffset) override;

    vixl::aarch64::MacroAssembler *GetMasm();

private:
    void SaveCalleeSavedRegs(const FrameInfo &frameInfo, const CFrameLayout &fl, size_t spToRegsSlots, bool isNative);
    template <bool IS_NATIVE>
    void GenerateEpilogueImpl(const FrameInfo &frameInfo, const std::function<void()> &postJob);
};  // Aarch64CallingConvention
}  // namespace ark::compiler::aarch64
#endif  // COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH64_TARGET_H
