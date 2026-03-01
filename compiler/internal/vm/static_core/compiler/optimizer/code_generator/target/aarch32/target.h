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

#ifndef COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH32_TARGET_H
#define COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH32_TARGET_H

#include "operands.h"
#include "encode.h"
#include "callconv.h"
#include "target_info.h"

#ifdef USE_VIXL_ARM32
#include "aarch32/constants-aarch32.h"
#include "aarch32/instructions-aarch32.h"
#include "aarch32/macro-assembler-aarch32.h"
#else
#error "Wrong build type, please add VIXL in build"
#endif  // USE_VIXL_ARM32

namespace ark::compiler::aarch32 {
inline constexpr uint32_t AVAILABLE_DOUBLE_WORD_REGISTERS = 4;
inline constexpr size_t AARCH32_COUNT_REG = 3;
inline constexpr size_t AARCH32_COUNT_VREG = 2;

// Temporary registers used (r12 already used by vixl)
// r11 is used as FP register for frames
const std::array<unsigned, AARCH32_COUNT_REG> AARCH32_TMP_REG = {
    vixl::aarch32::r8.GetCode(), vixl::aarch32::r9.GetCode(), vixl::aarch32::r12.GetCode()};

// Temporary vector registers used
const std::array<unsigned, AARCH32_COUNT_VREG> AARCH32_TMP_VREG = {vixl::aarch32::s30.GetCode(),
                                                                   vixl::aarch32::s31.GetCode()};

static inline constexpr const uint8_t UNDEF_REG = std::numeric_limits<uint8_t>::max();

static inline vixl::aarch32::Register VixlReg(Reg reg)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        auto vixlReg = vixl::aarch32::Register(reg.GetId());
        ASSERT(vixlReg.IsValid());
        return vixlReg;
    }
    // Unsupported register type
    UNREACHABLE();
    return vixl::aarch32::Register();
}

// Upper half-part for register
static inline vixl::aarch32::Register VixlRegU(Reg reg)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        auto vixlReg = vixl::aarch32::Register(reg.GetId() + 1);
        ASSERT(reg.GetId() <= AVAILABLE_DOUBLE_WORD_REGISTERS * 2U);
        ASSERT(vixlReg.IsValid());
        return vixlReg;
    }
    // Unsupported register type
    UNREACHABLE();
    return vixl::aarch32::Register();
}
static inline vixl::aarch32::VRegister VixlVRegCaseWordSize(Reg reg)
{
    // Aarch32 Vreg map double regs for 2 single-word registers
    auto vixlVreg = vixl::aarch32::SRegister(reg.GetId());
    ASSERT(vixlVreg.IsValid());
    return vixlVreg;
}

static inline vixl::aarch32::VRegister VixlVReg(Reg reg)
{
    ASSERT(reg.IsValid());
    ASSERT(reg.IsFloat());
    if (reg.GetSize() == WORD_SIZE) {
        return VixlVRegCaseWordSize(reg);
    }
    ASSERT(reg.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(reg.GetId() % 2U == 0);
    auto vixlVreg = vixl::aarch32::DRegister(reg.GetId() / 2U);
    ASSERT(vixlVreg.IsValid());
    return vixlVreg;
}

static inline vixl::aarch32::Operand VixlImm(Imm imm)
{
    // Unsupported 64-bit values - force cast
    return vixl::aarch32::Operand(static_cast<int32_t>(imm.GetRawValue()));
}

// Upper half for immediate
static inline vixl::aarch32::Operand VixlImmU(Imm imm)
{
    // Unsupported 64-bit values - force cast
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    auto data = static_cast<int32_t>(imm.GetRawValue() >> WORD_SIZE);
    return vixl::aarch32::Operand(data);
}

static inline vixl::aarch32::Operand VixlImm(const int32_t imm)
{
    return vixl::aarch32::Operand(imm);
}

static inline vixl::aarch32::NeonImmediate VixlNeonImm(const float imm)
{
    return vixl::aarch32::NeonImmediate(imm);
}

static inline vixl::aarch32::NeonImmediate VixlNeonImm(const double imm)
{
    return vixl::aarch32::NeonImmediate(imm);
}

class Aarch32RegisterDescription final : public RegistersDescription {
    //                 r4-r10 - "0000011111110000"
    // NOLINTNEXTLINE(readability-identifier-naming)
    const RegMask CALLEE_SAVED = RegMask(GetCalleeRegsMask(Arch::AARCH32, false));
    //                 s16-s31 - "11111111111111110000000000000000"
    // NOLINTNEXTLINE(readability-identifier-naming)
    const VRegMask CALLEE_SAVEDV = VRegMask(GetCalleeRegsMask(Arch::AARCH32, true));
    //                 r0-r3 -  "0000000000001111"
    // NOLINTNEXTLINE(readability-identifier-naming)
    const RegMask CALLER_SAVED = RegMask(GetCallerRegsMask(Arch::AARCH32, false));
    //                 s0-s15 - "00000000000000001111111111111111"
    // NOLINTNEXTLINE(readability-identifier-naming)
    const VRegMask CALLER_SAVEDV = VRegMask(GetCallerRegsMask(Arch::AARCH32, true));

public:
    explicit Aarch32RegisterDescription(ArenaAllocator *allocator);
    NO_MOVE_SEMANTIC(Aarch32RegisterDescription);
    NO_COPY_SEMANTIC(Aarch32RegisterDescription);
    ~Aarch32RegisterDescription() override = default;

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

    bool SupportMapping(uint32_t type) override;

    bool IsValid() const override;

    bool IsRegUsed(ArenaVector<Reg> vecReg, Reg reg) override;

    // NOTE(igorban): implement as virtual
    static bool IsTmp(Reg reg);

public:
    // Special implementation-specific getters
    RegMask GetCalleeSavedR();
    VRegMask GetCalleeSavedV();
    RegMask GetCallerSavedR();
    VRegMask GetCallerSavedV();
    uint8_t GetAligmentReg(bool isCallee);

private:
    // Full list of arm64 General-purpose registers (with vector registers)
    ArenaVector<Reg> aarch32RegList_;
    //
    ArenaVector<Reg> usedRegs_;
    Reg tmpReg1_;
    Reg tmpReg2_;

    RegMask calleeSaved_ {CALLEE_SAVED};
    RegMask callerSaved_ {CALLER_SAVED};

    VRegMask calleeSavedv_ {CALLEE_SAVEDV};
    VRegMask callerSavedv_ {CALLER_SAVEDV};

    uint8_t allignmentRegCallee_ {UNDEF_REG};
    uint8_t allignmentRegCaller_ {UNDEF_REG};
};  // Aarch32RegisterDescription

class Aarch32Encoder;

class Aarch32LabelHolder final : public LabelHolder {
public:
    using LabelType = vixl::aarch32::Label;
    explicit Aarch32LabelHolder(Encoder *enc) : LabelHolder(enc), labels_(enc->GetAllocator()->Adapter()) {};

    LabelId CreateLabel() override;
    void CreateLabels(LabelId size) override;
    void BindLabel(LabelId id) override;
    LabelType *GetLabel(LabelId id);
    LabelId Size() override;
    NO_MOVE_SEMANTIC(Aarch32LabelHolder);
    NO_COPY_SEMANTIC(Aarch32LabelHolder);
    ~Aarch32LabelHolder() override = default;

private:
    ArenaVector<LabelType *> labels_;
    LabelId id_ {0};
    friend Aarch32Encoder;
};  // Aarch32LabelHolder

class Aarch32ParameterInfo final : public ParameterInfo {
public:
    std::variant<Reg, uint8_t> GetNativeParam(const TypeInfo &type) override;
    Location GetNextLocation(DataType::Type type) override;
};

class Aarch32CallingConvention : public CallingConvention {
public:
    Aarch32CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr, CallConvMode mode);

    static constexpr auto GetTarget();

    bool IsValid() const override;

    void GeneratePrologue(const FrameInfo &frameInfo) override;
    void GenerateEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override;
    void GenerateNativePrologue(const FrameInfo &frameInfo) override;
    void GenerateNativeEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override;
    void GenerateEpilogueHead(const FrameInfo &frameInfo, std::function<void()> postJob) override;

    void *GetCodeEntry() override;
    uint32_t GetCodeSize() override;

    vixl::aarch32::MacroAssembler *GetMasm();

    // Calculating information about parameters and save regs_offset registers for special needs
    ParameterInfo *GetParameterInfo(uint8_t regsOffset) override;

    NO_MOVE_SEMANTIC(Aarch32CallingConvention);
    NO_COPY_SEMANTIC(Aarch32CallingConvention);
    ~Aarch32CallingConvention() override = default;

private:
    uint8_t PushPopVRegs(VRegMask vregs, bool isPush);
    uint8_t PushRegs(RegMask regs, VRegMask vregs, bool isCallee);
    uint8_t PopRegs(RegMask regs, VRegMask vregs, bool isCallee);
};  // Aarch32CallingConvention

class Aarch32Encoder final : public Encoder {
public:
    explicit Aarch32Encoder(ArenaAllocator *allocator);

    LabelHolder *GetLabels() const override;
    ~Aarch32Encoder() override;

    NO_COPY_SEMANTIC(Aarch32Encoder);
    NO_MOVE_SEMANTIC(Aarch32Encoder);

    bool IsValid() const override;
    static constexpr auto GetTarget();
    void SetMaxAllocatedBytes(size_t size) override;

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
    void EncodeCastToBool(Reg dst, Reg src) override;
    void EncodeCast(Reg dst, bool dstSigned, Reg src, bool srcSigned) override;
    void EncodeMin(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeDiv(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeMod(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeMax(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeExtractBits(Reg dst, Reg src, Imm imm1, Imm imm2, bool signExt) override;

    void EncodeLdr(Reg dst, bool dstSigned, MemRef mem) override;
    void EncodeLdr(Reg dst, bool dstSigned, const vixl::aarch32::MemOperand &vixlMem);
    void EncodeLdrAcquire(Reg dst, bool dstSigned, MemRef mem) override;

    void EncodeMemoryBarrier(memory_order::Order order) override;

    void EncodeMov(Reg dst, Imm src) override;
    void EncodeStr(Reg src, const vixl::aarch32::MemOperand &vixlMem);
    void EncodeStr(Reg src, MemRef mem) override;
    void EncodeStrRelease(Reg src, MemRef mem) override;
    void EncodeStp(Reg src0, Reg src1, MemRef mem) override;

    /* builtins-related encoders */
    void EncodeIsInf(Reg dst, Reg src) override;
    void EncodeIsInteger(Reg dst, Reg src) override;
    void EncodeIsSafeInteger(Reg dst, Reg src) override;
    void EncodeBitCount(Reg dst, Reg src) override;
    void EncodeCountLeadingZeroBits(Reg dst, Reg src) override;
    void EncodeCeil([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeFloor([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRint([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeTrunc([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRoundAway([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRoundToPInfReturnScalar([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRoundToPInfReturnFloat([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeReverseBytes(Reg dst, Reg src) override;
    void EncodeReverseBits(Reg dst, Reg src) override;
    void EncodeFpToBits(Reg dst, Reg src) override;
    void EncodeMoveBitsRaw(Reg dst, Reg src) override;

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
    void EncodeSelectTest(const ArgsSelect &args) override;
    void EncodeSelectTest(const ArgsSelectImm &args) override;

    bool CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signedCompare) override;
    bool CanEncodeImmLogical(uint64_t imm, uint32_t size) override;
    bool CanEncodeBitfieldExtractionFor(DataType::Type type) override;

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
    void SetRegister(RegMask *mask, VRegMask *vmask, Reg reg, bool val) const override;

    TypeInfo GetRefType() override;

    size_t DisasmInstr(std::ostream &stream, size_t pc, ssize_t codeOffset) const override;

    void *BufferData() const override;
    size_t BufferSize() const override;

    bool InitMasm() override;
    void Finalize() override;

    void MakeCall(compiler::RelocationInfo *relocation) override;
    void MakeCall(const void *entryPoint) override;
    void MakeCall(MemRef entryPoint) override;
    void MakeCall(Reg reg) override;

    void MakeCallAot(intptr_t offset) override;
    void MakeCallByOffset(intptr_t offset) override;
    void MakeLoadAotTable(intptr_t offset, Reg reg) override;
    void MakeLoadAotTableAddr(intptr_t offset, Reg addr, Reg val) override;

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
    void EncodeStackOverflowCheck(ssize_t offset) override;

    void SaveRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp) override;
    void LoadRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp) override;
    void SaveRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask) override;
    void LoadRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask) override;
    void PushRegisters(RegMask registers, bool isFp) override;
    void PopRegisters(RegMask registers, bool isFp) override;

    static vixl::aarch32::MemOperand ConvertMem(MemRef mem);

    static bool IsNeedToPrepareMemLdS(MemRef mem, const TypeInfo &memType, bool isSigned);
    vixl::aarch32::MemOperand PrepareMemLdS(MemRef mem, const TypeInfo &memType, vixl::aarch32::Register tmp,
                                            bool isSigned, bool copySp = false);

    void MakeLibCall(Reg dst, Reg src0, Reg src1, void *entryPoint, bool secondValue = false);

    void MakeLibCall(Reg dst, Reg src, void *entryPoint);

    vixl::aarch32::MacroAssembler *GetMasm() const;
    size_t GetLabelAddress(LabelHolder::LabelId label) override;
    bool LabelHasLinks(LabelHolder::LabelId label) override;

private:
    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp);
    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, ssize_t slot, Reg base, RegMask mask, bool isFp);
    template <bool IS_STORE>
    void LoadStoreRegistersMainLoop(RegMask registers, ssize_t slot, Reg base, RegMask mask, bool isFp);
    template <bool IS_STORE>
    void ConstructLdrStr(vixl::aarch32::MemOperand mem, size_t i, bool isFp);
    void ConstructAddForBigOffset(vixl::aarch32::Register tmp, vixl::aarch32::Register *baseReg, ssize_t *slot,
                                  ssize_t maxOffset, bool isFp);

private:
    vixl::aarch32::MemOperand PrepareMemLdSForFloat(MemRef mem, vixl::aarch32::Register tmp);
    void EncodeCastFloatToFloat(Reg dst, Reg src);
    void EncodeCastFloatToInt64(Reg dst, Reg src);
    void EncodeCastDoubleToInt64(Reg dst, Reg src);
    void EncodeCastScalarToFloat(Reg dst, Reg src, bool srcSigned);
    void EncodeCastFloatToScalar(Reg dst, bool dstSigned, Reg src);
    void EncodeCastFloatToScalarWithSmallDst(Reg dst, bool dstSigned, Reg src);
    void EncodeCastToDoubleWord(Reg dst, bool dstSigned, Reg src);

    void EncoderCastExtendFromInt32(Reg dst, bool dstSigned);
    void EncodeCastScalar(Reg dst, bool dstSigned, Reg src, bool srcSigned);
    void EncodeCastScalarFromSignedScalar(Reg dst, Reg src);
    void EncodeCastScalarFromUnsignedScalar(Reg dst, Reg src);
    template <bool IS_MAX>
    void EncodeMinMaxFp(Reg dst, Reg src0, Reg src1);
    void EncodeVorr(Reg dst, Reg src0, Reg src1);
    void EncodeVand(Reg dst, Reg src0, Reg src1);

    void MakeLibCallWithFloatResult(Reg dst, Reg src0, Reg src1, void *entryPoint, bool secondValue);
    void MakeLibCallWithDoubleResult(Reg dst, Reg src0, Reg src1, void *entryPoint, bool secondValue);
    void MakeLibCallWithInt64Result(Reg dst, Reg src0, Reg src1, void *entryPoint, bool secondValue);
    void MakeLibCallFromFloatToScalar(Reg dst, Reg src, void *entryPoint);
    void MakeLibCallFromScalarToFloat(Reg dst, Reg src, void *entryPoint);
    void CompareHelper(Reg src0, Reg src1, Condition *cc);
    void TestHelper(Reg src0, Reg src1, Condition cc);
    bool CompareImmHelper(Reg src, int64_t imm, Condition *cc);
    void TestImmHelper(Reg src, Imm imm, Condition cc);
    bool CompareNegImmHelper(Reg src, int64_t value, const Condition *cc);
    bool ComparePosImmHelper(Reg src, int64_t value, Condition *cc);
    Condition TrySwapCc(Condition cc, bool *swap);
    void CompareZeroHelper(Reg src, Condition *cc);
    void EncodeCmpFracWithZero(Reg src);
    static inline constexpr int32_t MEM_BIG_OFFSET = 4095;
    static inline constexpr int32_t MEM_SMALL_OFFSET = 255;
    static inline constexpr int32_t VMEM_OFFSET = 1020;
    Aarch32LabelHolder *labels_ {nullptr};
    vixl::aarch32::MacroAssembler *masm_ {nullptr};
    bool lrAcquired_ {false};
};  // Aarch32Encoder

}  // namespace ark::compiler::aarch32

#endif  // COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH32_TARGET_H
