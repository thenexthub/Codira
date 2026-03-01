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

#ifndef COMPILER_OPTIMIZER_CODEGEN_ENCODE_H
#define COMPILER_OPTIMIZER_CODEGEN_ENCODE_H

/*
    Hi-level interface for encoding
    Wrapper for specialize concrete used encoding

    Responsible for
        Primitive (not-branch) instruction encoding
        Memory-instructions encoding
        Immediate and Memory operands
*/

#include <variant>

#include "operands.h"
#include "optimizer/select_transform_type.h"
#include "registers_description.h"
#include "libarkbase/utils/cframe_layout.h"
#include "target_info.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENCODE_MATH_UNARY_OP_LIST(DEF) \
    DEF(Mov, UNARY_OPERATION)          \
    DEF(Neg, UNARY_OPERATION)          \
    DEF(Abs, UNARY_OPERATION)          \
    DEF(Not, UNARY_OPERATION)          \
    DEF(Sqrt, UNARY_OPERATION)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENCODE_MATH_BINARY_OP_LIST(DEF) \
    DEF(Add, BINARY_OPERATION)          \
    DEF(Sub, BINARY_OPERATION)          \
    DEF(Mul, BINARY_OPERATION)          \
    DEF(Shl, BINARY_OPERATION)          \
    DEF(Shr, BINARY_OPERATION)          \
    DEF(AShr, BINARY_OPERATION)         \
    DEF(And, BINARY_OPERATION)          \
    DEF(Or, BINARY_OPERATION)           \
    DEF(Xor, BINARY_OPERATION)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENCODE_MATH_LIST(DEF)      \
    ENCODE_MATH_UNARY_OP_LIST(DEF) \
    ENCODE_MATH_BINARY_OP_LIST(DEF)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENCODE_INST_WITH_SHIFTED_OPERAND(DEF)      \
    DEF(And, BINARY_SHIFTED_REGISTER_OPERATION)    \
    DEF(Or, BINARY_SHIFTED_REGISTER_OPERATION)     \
    DEF(Xor, BINARY_SHIFTED_REGISTER_OPERATION)    \
    DEF(OrNot, BINARY_SHIFTED_REGISTER_OPERATION)  \
    DEF(AndNot, BINARY_SHIFTED_REGISTER_OPERATION) \
    DEF(XorNot, BINARY_SHIFTED_REGISTER_OPERATION) \
    DEF(Add, BINARY_SHIFTED_REGISTER_OPERATION)    \
    DEF(Sub, BINARY_SHIFTED_REGISTER_OPERATION)

namespace ark::compiler {
class Encoder;
class CompilerOptions;
class RelocationInfo;

namespace memory_order {
enum Order { ACQUIRE, RELEASE, FULL };
}  // namespace memory_order

class LabelHolder {
public:
    using LabelId = uintptr_t;
    static constexpr LabelId INVALID_LABEL = static_cast<uintptr_t>(-1);

    explicit LabelHolder(Encoder *enc) : enc_ {enc} {};
    virtual ~LabelHolder() = default;

    // NOTE (igorban) : hide all this methods in CallConv
    virtual void CreateLabels(LabelId size) = 0;
    virtual LabelId CreateLabel() = 0;
    virtual LabelId Size() = 0;

    Encoder *GetEncoder() const
    {
        return enc_;
    }

    NO_COPY_SEMANTIC(LabelHolder);
    NO_MOVE_SEMANTIC(LabelHolder);

protected:
    virtual void BindLabel(LabelId) = 0;

private:
    Encoder *enc_ {nullptr};
    friend Encoder;
};

class Encoder {
public:
    // Main constructor
    explicit Encoder(ArenaAllocator *aa, Arch arch) : Encoder(aa, arch, false) {}
    Encoder(ArenaAllocator *aa, Arch arch, bool jsNumberCast)
        : allocator_(aa), frameLayout_(arch, 0), target_(arch), jsNumberCast_(jsNumberCast)
    {
    }
    virtual ~Encoder() = default;

    ArenaAllocator *GetAllocator() const;
    bool IsLabelValid(LabelHolder::LabelId label);

    Target GetTarget() const;
    Arch GetArch() const;
    bool IsJsNumberCast() const;
    void SetIsJsNumberCast(bool v);

    /// Print instruction and return next pc
    virtual size_t DisasmInstr(std::ostream &stream, size_t pc, ssize_t codeOffset) const;
    virtual void *BufferData() const;
    virtual size_t BufferSize() const;

    virtual bool InitMasm();

    virtual void SetMaxAllocatedBytes([[maybe_unused]] size_t size) {};

// Define default math operations
// Encode (dst, src)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNARY_OPERATION(opc)           \
    virtual void Encode##opc(Reg, Reg) \
    {                                  \
        SetFalseResult();              \
    }

// Encode (dst, src0, src1)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_OPERATION(opc)               \
    virtual void Encode##opc(Reg, Reg, Reg) \
    {                                       \
        SetFalseResult();                   \
    }                                       \
    virtual void Encode##opc(Reg, Reg, Imm) \
    {                                       \
        SetFalseResult();                   \
    }

// Encode (dst, src0, src1)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_SHIFTED_REGISTER_OPERATION(opc) \
    virtual void Encode##opc(Reg, Reg, Shift)  \
    {                                          \
        SetFalseResult();                      \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, MACRO) MACRO(OPCODE)

    ENCODE_MATH_LIST(INST_DEF)
    ENCODE_INST_WITH_SHIFTED_OPERAND(INST_DEF)

#undef UNARY_OPERATION
#undef BINARY_OPERATION
#undef BINARY_SHIFTED_REGISTER_OPERATION
#undef INST_DEF

    virtual void EncodeNop();

    virtual void EncodeAddOverflow(LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc);
    virtual void EncodeSubOverflow(LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc);
    virtual void EncodeMulOverflow(LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc);
    virtual void EncodeNegOverflowAndZero(LabelHolder::LabelId id, Reg dst, Reg src);
    virtual void EncodeFastPathDynamicCast(Reg dst, Reg src, LabelHolder::LabelId slow);
    virtual void EncodeJsDoubleToCharCast(Reg dst, Reg src);
    virtual void EncodeJsDoubleToCharCast(Reg dst, Reg src, Reg tmp, uint32_t failureResult);
    virtual void EncodeCast(Reg dst, bool dstSigned, Reg src, bool srcSigned);
    virtual void EncodeCastToBool(Reg dst, Reg src);
    virtual void EncodeMin(Reg dst, bool dstSigned, Reg src0, Reg src1);
    virtual void EncodeDiv(Reg dst, bool dstSigned, Reg src0, Reg src1);
    virtual void EncodeMod(Reg dst, bool dstSigned, Reg src0, Reg src1);
    virtual void EncodeDiv(Reg dst, Reg src0, Imm imm, bool isSigned);
    virtual void EncodeMod(Reg dst, Reg src0, Imm imm, bool isSigned);
    virtual void EncodeMax(Reg dst, bool dstSigned, Reg src0, Reg src1);
    virtual void EncodeMov(Reg dst, Imm src);
    virtual void EncodeLdr(Reg dst, bool dstSigned, MemRef mem);
    virtual void EncodeLdrPreIndex(Reg dst, bool dstSigned, Reg srcBase, Imm offset);
    virtual void EncodeLdrPostIndex(Reg dst, bool dstSigned, Reg srcBase, Imm offset);
    virtual void EncodeLdrAcquire(Reg dst, bool dstSigned, MemRef mem);
    virtual void EncodeStr(Reg src, MemRef mem);
    virtual void EncodeStrPreIndex(Reg src, Reg dstBase, Imm offset);
    virtual void EncodeStrPostIndex(Reg src, Reg dstBase, Imm offset);
    virtual void EncodeStrRelease(Reg src, MemRef mem);
    virtual void EncodeLdrExclusive(Reg dst, Reg addr, bool acquire);
    virtual void EncodeStrExclusive(Reg dst, Reg src, Reg addr, bool release);
    // zerod high part: [reg.size, 64)
    virtual void EncodeStrz(Reg src, MemRef mem);
    virtual void Push(Reg src, MemRef mem);
    virtual void EncodeSti(int64_t src, uint8_t srcSizeBytes, MemRef mem);
    virtual void EncodeSti(double src, MemRef mem);
    virtual void EncodeSti(float src, MemRef mem);
    // size must be 8, 16,32 or 64
    virtual void EncodeMemCopy(MemRef memFrom, MemRef memTo, size_t size);
    // size must be 8, 16,32 or 64
    // zerod high part: [size, 64)
    virtual void EncodeMemCopyz(MemRef memFrom, MemRef memTo, size_t size);
    virtual void EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc);
    // Additional check for isnan-comparison
    virtual void EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc);
    virtual void EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc);
    virtual void EncodeAtomicByteOr(Reg addr, Reg value, bool fastEncoding);
    struct ArgsCompressedStringCharAt {
        Reg dst;
        Reg str;
        Reg idx;
        Reg length;
        Reg tmp;
        size_t dataOffset;
        uint8_t shift;
    };
    virtual void EncodeCompressedStringCharAt(ArgsCompressedStringCharAt &&args);

    struct ArgsCompressedStringCharAtI {
        Reg dst;
        Reg str;
        Reg length;
        size_t dataOffset;
        uint32_t index;
        uint8_t shift;
    };
    virtual void EncodeCompressedStringCharAtI(ArgsCompressedStringCharAtI &&args);
    struct ArgsSelect {
        Reg dst;
        Reg src0;
        Reg src1;
        Reg src2;
        Reg src3;
        Condition cc;
    };
    virtual void EncodeSelect(const ArgsSelect &args);
    virtual void EncodeSelectTest(const ArgsSelect &args);

    struct ArgsSelectImm {
        Reg dst;
        Reg src0;
        Reg src1;
        Reg src2;
        Imm imm;
        Condition cc;
    };
    virtual void EncodeSelect(const ArgsSelectImm &args);
    virtual void EncodeSelectTest(const ArgsSelectImm &args);

    struct ArgsSelectTransform {
        Reg dst;
        Reg src0;
        Reg src1;
        Reg src2;
        Reg src3;
        Condition cc;
        SelectTransformType transform;
    };
    virtual void EncodeSelectTransform(const ArgsSelectTransform &args);
    virtual void EncodeSelectTestTransform(const ArgsSelectTransform &args);

    struct ArgsSelectImmTransform {
        Reg dst;
        Reg src0;
        Reg src1;
        Reg src2;
        Imm imm;
        Condition cc;
        SelectTransformType transform;
    };
    virtual void EncodeSelectTransform(const ArgsSelectImmTransform &args);
    virtual void EncodeSelectTestTransform(const ArgsSelectImmTransform &args);

    virtual void EncodeIsInf(Reg dst, Reg src0);
    virtual void EncodeIsInteger(Reg dst, Reg src0);
    virtual void EncodeIsSafeInteger(Reg dst, Reg src0);
    virtual void EncodeReverseBytes(Reg dst, Reg src);
    virtual void EncodeReverseBits(Reg dst, Reg src);
    virtual void EncodeReverseHalfWords(Reg dst, Reg src);
    virtual void EncodeBitCount(Reg dst, Reg src);
    virtual void EncodeRotate(Reg dst, Reg src1, Reg src2, bool isRor);
    virtual void EncodeSignum(Reg dst, Reg src);
    virtual void EncodeCountLeadingZeroBits(Reg dst, Reg src);
    virtual void EncodeCountTrailingZeroBits(Reg dst, Reg src);
    virtual void EncodeCeil(Reg dst, Reg src);
    virtual void EncodeFloor(Reg dst, Reg src);
    virtual void EncodeRint(Reg dst, Reg src);
    virtual void EncodeTrunc(Reg dst, Reg src);
    virtual void EncodeRoundAway(Reg dst, Reg src);
    virtual void EncodeRoundToPInfReturnFloat(Reg dst, Reg src);
    virtual void EncodeRoundToPInfReturnScalar(Reg dst, Reg src);

    virtual void EncodeGetTypeSize(Reg size, Reg type);
    virtual void EncodeLdp(Reg dst0, Reg dst1, bool dstSigned, MemRef mem);
    virtual void EncodeLdpPreIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, Imm offset);
    virtual void EncodeLdpPostIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, Imm offset);
    virtual void EncodeStp(Reg src0, Reg src1, MemRef mem);
    virtual void EncodeStpPreIndex(Reg src0, Reg src1, Reg dstBase, Imm offset);
    virtual void EncodeStpPostIndex(Reg src0, Reg src1, Reg dstBase, Imm offset);
    virtual void EncodeMAdd(Reg dst, Reg src0, Reg src1, Reg src2);
    virtual void EncodeMSub(Reg dst, Reg src0, Reg src1, Reg src2);
    virtual void EncodeMNeg(Reg dst, Reg src0, Reg src1);
    virtual void EncodeOrNot(Reg dst, Reg src0, Reg src1);
    virtual void EncodeAndNot(Reg dst, Reg src0, Reg src1);
    virtual void EncodeXorNot(Reg dst, Reg src0, Reg src1);
    virtual void EncodeNeg(Reg dst, Shift src);
    virtual void EncodeFpToBits(Reg dst, Reg src);
    virtual void EncodeMoveBitsRaw(Reg dst, Reg src);
    virtual void EncodeExtractBits(Reg dst, Reg src, Imm imm1, Imm imm2, bool signExt);
    virtual void EncodeCrc32Update(Reg dst, Reg crcReg, Reg valReg);
    virtual void EncodeGetCurrentPc(Reg dst);
    /**
     * Encode dummy load from the address [sp + offset].
     * @param offset offset from the stack pointer register
     */
    virtual void EncodeStackOverflowCheck(ssize_t offset);
    virtual bool IsValid() const;
    virtual bool GetResult() const;
    void SetFalseResult();
    // Encoder builder - implement in target.cpp
    static Encoder *Create(ArenaAllocator *arenaAllocator, Arch arch, bool printAsm, bool jsNumberCast = false);
    // For now it is one function for Add/Sub and Cmp, it suits all considered targets (x86, amd64, arm32, arm64).
    // We probably should revisit this if we add new targets, like Thumb-2 or others.
    virtual bool CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signedCompare);
    virtual bool CanEncodeImmMulDivMod(uint64_t imm, uint32_t size);
    virtual bool CanEncodeImmLogical(uint64_t imm, uint32_t size);
    virtual bool CanOptimizeImmDivMod(uint64_t imm, bool isSigned) const;
    virtual bool CanEncodeScale(uint64_t imm, uint32_t size);
    virtual bool CanEncodeShift(uint32_t size);
    virtual bool CanEncodeBitCount();
    virtual bool CanEncodeMAdd();
    virtual bool CanEncodeMSub();
    virtual bool CanEncodeMNeg();
    virtual bool CanEncodeOrNot();
    virtual bool CanEncodeAndNot();
    virtual bool CanEncodeXorNot();
    // Check if encoder is capable of encoding operations where an operand is a register with
    // a value shifted by shift operation with specified type by some immediate value.
    virtual bool CanEncodeShiftedOperand(ShiftOpcode opcode, ShiftType shiftType);
    virtual bool CanEncodeCompressedStringCharAt();
    virtual bool CanEncodeCompressedStringCharAtI();
    virtual bool CanEncodeFloatSelect();
    virtual bool CanEncodeSelectTransformFor(DataType::Type type);
    virtual bool CanEncodeBitfieldExtractionFor(DataType::Type type);
    virtual void EncodeCompareAndSwap(Reg dst, Reg obj, Reg offset, Reg val, Reg newval);
    virtual void EncodeCompareAndSwap(Reg dst, Reg addr, Reg val, Reg newval);
    virtual void EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val);
    virtual void EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, Reg tmp);
    virtual void EncodeMemoryBarrier(memory_order::Order order);
    virtual size_t GetCursorOffset() const;
    virtual void EncodeCompressEightUtf16ToUtf8CharsUsingSimd(Reg srcAddr, Reg dstAddr);
    virtual void EncodeCompressSixteenUtf16ToUtf8CharsUsingSimd(Reg srcAddr, Reg dstAddr);
    virtual void EncodeMemCharU8X32UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp);
    virtual void EncodeMemCharU16X16UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp);
    virtual void EncodeMemCharU8X16UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp);
    virtual void EncodeMemCharU16X8UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp);
    virtual void EncodeMemLastCharU16X8UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp);
    virtual void EncodeMemLastCharU8X16UsingSimd(Reg dst, Reg ch, Reg srcAddr, Reg tmp);
    virtual void EncodeCharsToUpperCaseU8X16UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp);
    virtual void EncodeCharsToUpperCaseU8X8UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp);
    virtual void EncodeCharsToLowerCaseU8X16UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp);
    virtual void EncodeCharsToLowerCaseU8X8UsingSimd(Reg srcAddr, Reg dstAddr, Reg tmp);
    virtual void EncodeUnsignedExtendBytesToShorts(Reg dst, Reg src);
    virtual void SetCursorOffset([[maybe_unused]] size_t offset) {}
    virtual void SaveRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp);
    virtual void LoadRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp);
    /**
     * Save/load registers to/from the memory.
     *
     * If `mask` is empty (all bits are zero), then registers will be saved densely, otherwise place for each register
     * will be determined according to this mask.
     * Example: registers' bits = [1, 3, 10], mask's bits = [0, 1, 2, 3, 8, 9, 10, 11]
     * We can see that mask has the gap in 4-7 bits. So, registers will be saved in the following slots:
     *      slots: 0   1   2   3   4   5   6   7
     *      regs :     1       3          10
     * If the mask would be zero, then the following layout will be used:
     *      slots: 0   1   2
     *      regs : 1   3  10
     *
     * @param registers mask of registers to be saved
     * @param is_fp if true, registers are floating point registers
     * @param slot offset from the `base` register to the destination address (in words)
     * @param base base register
     * @param mask determine memory layout for the registers
     */
    virtual void SaveRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask);
    virtual void LoadRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask);
    void PushRegisters(RegMask regs, VRegMask fpRegs, bool isAligned = true);
    void PopRegisters(RegMask regs, VRegMask fpRegs, bool isAligned = true);
    virtual void PushRegisters(RegMask registers, bool isFp);
    virtual void PopRegisters(RegMask registers, bool isFp);
    RegistersDescription *GetRegfile() const;
    void SetRegfile(RegistersDescription *regfile);
    virtual Reg AcquireScratchRegister(TypeInfo type);
    virtual void AcquireScratchRegister(Reg reg);
    virtual void ReleaseScratchRegister(Reg reg);
    virtual bool IsScratchRegisterReleased(Reg reg) const;
    size_t GetScratchRegistersCount() const;
    size_t GetScratchRegistersWithLrCount() const;
    virtual RegMask GetScratchRegistersMask() const;
    size_t GetScratchFPRegistersCount() const;
    virtual RegMask GetScratchFpRegistersMask() const;
    // Get Scratch registers, that currently are not allocated
    virtual RegMask GetAvailableScratchRegisters() const;
    // Get Floating Point Scratch registers, that currently are not allocated
    virtual VRegMask GetAvailableScratchFpRegisters() const;
    virtual size_t MaxArchInstPerEncoded();
    virtual void SetRegister(RegMask *mask, VRegMask *vmask, Reg reg);
    virtual void SetRegister(RegMask *mask, VRegMask *vmask, Reg reg, bool val) const;
    virtual TypeInfo GetRefType();
    virtual void Finalize() = 0;

public:
    /// Label-holder interfaces
    LabelHolder::LabelId CreateLabel();
    void BindLabel(LabelHolder::LabelId id);
    virtual LabelHolder *GetLabels() const;
    virtual size_t GetLabelAddress(LabelHolder::LabelId label) = 0;
    virtual bool LabelHasLinks(LabelHolder::LabelId label) = 0;

public:
    virtual void MakeCall(RelocationInfo *relocation);
    virtual void MakeCall(LabelHolder::LabelId id);
    virtual void MakeCall(const void *entryPoint);
    virtual void MakeCall(Reg reg);
    virtual void MakeCall(MemRef entryPoint);
    virtual void MakeCallAot(intptr_t offset);
    virtual bool CanMakeCallByOffset(intptr_t offset);
    virtual void MakeCallByOffset(intptr_t offset);
    virtual void MakeLoadAotTable(intptr_t offset, Reg reg);
    virtual void MakeLoadAotTableAddr(intptr_t offset, Reg addr, Reg val);

    // Encode unconditional branch
    virtual void EncodeJump(LabelHolder::LabelId id);
    // Encode jump with compare to zero
    virtual void EncodeJump(LabelHolder::LabelId id, Reg reg, Condition cond);
    // Compare reg and immediate and branch
    virtual void EncodeJump(LabelHolder::LabelId id, Reg reg, Imm imm, Condition c);
    // Compare two regs and branch
    virtual void EncodeJump(LabelHolder::LabelId id, Reg r, Reg reg, Condition c);
    // Compare reg and immediate and branch
    virtual void EncodeJumpTest(LabelHolder::LabelId id, Reg reg, Imm imm, Condition c);
    // Compare two regs and branch
    virtual void EncodeJumpTest(LabelHolder::LabelId id, Reg r, Reg reg, Condition c);
    // Encode jump by register value
    virtual void EncodeJump(Reg reg);
    virtual void EncodeJump(RelocationInfo *relocation);

    virtual void EncodeBitTestAndBranch(LabelHolder::LabelId id, Reg reg, uint32_t bitPos, bool bitValue);

    virtual void EncodeAbort();
    virtual void EncodeReturn();

    void SetFrameLayout(CFrameLayout fl);

    const CFrameLayout &GetFrameLayout() const;

    RegMask GetLiveTmpRegMask();

    VRegMask GetLiveTmpFpRegMask();

    void AddRegInLiveMask(Reg reg);

    void RemoveRegFromLiveMask(Reg reg);
    void SetCodeOffset(size_t offset);

    size_t GetCodeOffset() const;

    void EnableLrAsTempReg(bool value);

    bool IsLrAsTempRegEnabled() const;

    bool IsLrAsTempRegEnabledAndReleased() const;
    NO_COPY_SEMANTIC(Encoder);
    NO_MOVE_SEMANTIC(Encoder);

protected:
    void SetFrameSize(size_t size);

    size_t GetFrameSize() const;

    static constexpr size_t INVALID_OFFSET = std::numeric_limits<size_t>::max();

    // Max integer that can be represented in float/double without losing precision
    constexpr float MaxIntAsExactFloat() const
    {
        return static_cast<float>((1U << static_cast<unsigned>(std::numeric_limits<float>::digits)) - 1);
    }

    constexpr double MaxIntAsExactDouble() const
    {
        return static_cast<double>((1ULL << static_cast<unsigned>(std::numeric_limits<double>::digits)) - 1);
    }

    static constexpr bool CanOptimizeImmDivModCommon(uint64_t imm, bool isSigned)
    {
        if (!isSigned) {
            return imm > 0U;
        }
        auto signedImm = bit_cast<int64_t>(imm);
        return signedImm <= -2L || signedImm >= 2L;
    }

private:
    ArenaAllocator *allocator_;
    RegistersDescription *regfile_ {nullptr};
    size_t frameSize_ {0};

    CFrameLayout frameLayout_;

    RegMask liveTmpRegs_;
    VRegMask liveTmpFpRegs_;

    // In case of AOT compilation, this variable specifies offset from the start of the AOT file.
    // It is needed for accessing to the entrypoints table and AOT table, that lie right before code.
    size_t codeOffset_ {INVALID_OFFSET};

    Target target_ {Arch::NONE};

    bool success_ {true};
    bool jsNumberCast_ {false};
    // If true, then ScopedTmpReg can use LR as a temp register.
    bool enableLrAsTempReg_ {false};
};  // Encoder
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_ENCODE_H
