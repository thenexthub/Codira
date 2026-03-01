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

#include "encode.h"

namespace ark::compiler {

ArenaAllocator *Encoder::GetAllocator() const
{
    return allocator_;
}

bool Encoder::IsLabelValid(LabelHolder::LabelId label)
{
    return label != LabelHolder::INVALID_LABEL;
}

Target Encoder::GetTarget() const
{
    return target_;
}

Arch Encoder::GetArch() const
{
    return GetTarget().GetArch();
}

bool Encoder::IsJsNumberCast() const
{
    return jsNumberCast_;
}

void Encoder::SetIsJsNumberCast(bool v)
{
    jsNumberCast_ = v;
}
size_t Encoder::DisasmInstr([[maybe_unused]] std::ostream &stream, [[maybe_unused]] size_t pc,
                            [[maybe_unused]] ssize_t codeOffset) const
{
    return 0;
}

void *Encoder::BufferData() const
{
    return nullptr;
}

size_t Encoder::BufferSize() const
{
    return 0;
}

bool Encoder::InitMasm()
{
    return true;
}
void Encoder::EncodeNop()
{
    SetFalseResult();
}

void Encoder::EncodeAddOverflow([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] Reg dst,
                                [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1, [[maybe_unused]] Condition cc)
{
    SetFalseResult();
}

void Encoder::EncodeSubOverflow([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] Reg dst,
                                [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1, [[maybe_unused]] Condition cc)
{
    SetFalseResult();
}

void Encoder::EncodeMulOverflow([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] Reg dst,
                                [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1, [[maybe_unused]] Condition cc)
{
    SetFalseResult();
}

void Encoder::EncodeNegOverflowAndZero([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] Reg dst,
                                       [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeFastPathDynamicCast([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src,
                                        [[maybe_unused]] LabelHolder::LabelId slow)
{
    SetFalseResult();
}

void Encoder::EncodeJsDoubleToCharCast([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeJsDoubleToCharCast([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src, [[maybe_unused]] Reg tmp,
                                       [[maybe_unused]] uint32_t failureResult)
{
    SetFalseResult();
}

void Encoder::EncodeCast([[maybe_unused]] Reg dst, [[maybe_unused]] bool dstSigned, [[maybe_unused]] Reg src,
                         [[maybe_unused]] bool srcSigned)
{
    SetFalseResult();
}
void Encoder::EncodeCastToBool([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeMin([[maybe_unused]] Reg dst, [[maybe_unused]] bool dstSigned, [[maybe_unused]] Reg src0,
                        [[maybe_unused]] Reg src1)
{
    SetFalseResult();
}

void Encoder::EncodeDiv([[maybe_unused]] Reg dst, [[maybe_unused]] bool dstSigned, [[maybe_unused]] Reg src0,
                        [[maybe_unused]] Reg src1)
{
    SetFalseResult();
}

void Encoder::EncodeMod([[maybe_unused]] Reg dst, [[maybe_unused]] bool dstSigned, [[maybe_unused]] Reg src0,
                        [[maybe_unused]] Reg src1)
{
    SetFalseResult();
}

void Encoder::EncodeDiv([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Imm imm,
                        [[maybe_unused]] bool isSigned)
{
    SetFalseResult();
}

void Encoder::EncodeMod([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Imm imm,
                        [[maybe_unused]] bool isSigned)
{
    SetFalseResult();
}

void Encoder::EncodeMax([[maybe_unused]] Reg dst, [[maybe_unused]] bool dstSigned, [[maybe_unused]] Reg src0,
                        [[maybe_unused]] Reg src1)
{
    SetFalseResult();
}
void Encoder::EncodeMov([[maybe_unused]] Reg dst, [[maybe_unused]] Imm src)
{
    SetFalseResult();
}

void Encoder::EncodeLdr([[maybe_unused]] Reg dst, [[maybe_unused]] bool dstSigned, [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}

void Encoder::EncodeLdrPreIndex(Reg dst, bool dstSigned, Reg srcBase, Imm offset)
{
    EncodeAdd(srcBase, srcBase, offset);
    EncodeLdr(dst, dstSigned, MemRef {srcBase});
}

void Encoder::EncodeLdrPostIndex(Reg dst, bool dstSigned, Reg srcBase, Imm offset)
{
    EncodeLdr(dst, dstSigned, MemRef {srcBase});
    EncodeAdd(srcBase, srcBase, offset);
}

void Encoder::EncodeLdrAcquire([[maybe_unused]] Reg dst, [[maybe_unused]] bool dstSigned, [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}
void Encoder::EncodeStr([[maybe_unused]] Reg src, [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}

void Encoder::EncodeStrPreIndex(Reg src, Reg dstBase, Imm offset)
{
    EncodeAdd(dstBase, dstBase, offset);
    EncodeStr(src, MemRef {dstBase});
}

void Encoder::EncodeStrPostIndex(Reg src, Reg dstBase, Imm offset)
{
    EncodeStr(src, MemRef {dstBase});
    EncodeAdd(dstBase, dstBase, offset);
}

void Encoder::EncodeStrRelease([[maybe_unused]] Reg src, [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}
void Encoder::EncodeLdrExclusive([[maybe_unused]] Reg dst, [[maybe_unused]] Reg addr, [[maybe_unused]] bool acquire)
{
    SetFalseResult();
}

void Encoder::EncodeStrExclusive([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src, [[maybe_unused]] Reg addr,
                                 [[maybe_unused]] bool release)
{
    SetFalseResult();
}

// zerod high part: [reg.size, 64)
void Encoder::EncodeStrz([[maybe_unused]] Reg src, [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}
void Encoder::Push([[maybe_unused]] Reg src, [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}

void Encoder::EncodeSti([[maybe_unused]] int64_t src, [[maybe_unused]] uint8_t srcSizeBytes,
                        [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}
void Encoder::EncodeSti([[maybe_unused]] double src, [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}
void Encoder::EncodeSti([[maybe_unused]] float src, [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}

void Encoder::EncodeMemCopy([[maybe_unused]] MemRef memFrom, [[maybe_unused]] MemRef memTo,
                            [[maybe_unused]] size_t size)
{
    SetFalseResult();
}

void Encoder::EncodeMemCopyz([[maybe_unused]] MemRef memFrom, [[maybe_unused]] MemRef memTo,
                             [[maybe_unused]] size_t size)
{
    SetFalseResult();
}

void Encoder::EncodeCmp([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                        [[maybe_unused]] Condition cc)
{
    SetFalseResult();
}

void Encoder::EncodeCompare([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                            [[maybe_unused]] Condition cc)
{
    SetFalseResult();
}

void Encoder::EncodeCompareTest([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                                [[maybe_unused]] Condition cc)
{
    SetFalseResult();
}

void Encoder::EncodeAtomicByteOr([[maybe_unused]] Reg addr, [[maybe_unused]] Reg value,
                                 [[maybe_unused]] bool fastEncoding)
{
    SetFalseResult();
}

void Encoder::EncodeCompressedStringCharAt([[maybe_unused]] ArgsCompressedStringCharAt &&args)
{
    SetFalseResult();
}

void Encoder::EncodeCompressedStringCharAtI([[maybe_unused]] ArgsCompressedStringCharAtI &&args)
{
    SetFalseResult();
}

void Encoder::EncodeSelect([[maybe_unused]] const ArgsSelect &args)
{
    SetFalseResult();
}

void Encoder::EncodeSelectTest([[maybe_unused]] const ArgsSelect &args)
{
    SetFalseResult();
}

void Encoder::EncodeSelectTransform([[maybe_unused]] const ArgsSelectTransform &args)
{
    SetFalseResult();
}

void Encoder::EncodeSelectTestTransform([[maybe_unused]] const ArgsSelectTransform &args)
{
    SetFalseResult();
}

void Encoder::EncodeIsInf([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0)
{
    SetFalseResult();
}

void Encoder::EncodeIsInteger([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0)
{
    SetFalseResult();
}

void Encoder::EncodeIsSafeInteger([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0)
{
    SetFalseResult();
}

void Encoder::EncodeReverseBytes([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeReverseHalfWords([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeReverseBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeBitCount([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeRotate([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src1, [[maybe_unused]] Reg src2,
                           [[maybe_unused]] bool isRor)
{
    SetFalseResult();
}

void Encoder::EncodeSignum([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeCountLeadingZeroBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeCountTrailingZeroBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeCeil([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeFloor([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeRint([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeTrunc([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeRoundAway([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeRoundToPInfReturnFloat([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeRoundToPInfReturnScalar([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeSelect([[maybe_unused]] const ArgsSelectImm &args)
{
    SetFalseResult();
}

void Encoder::EncodeSelectTest([[maybe_unused]] const ArgsSelectImm &args)
{
    SetFalseResult();
}

void Encoder::EncodeSelectTransform([[maybe_unused]] const ArgsSelectImmTransform &args)
{
    SetFalseResult();
}

void Encoder::EncodeSelectTestTransform([[maybe_unused]] const ArgsSelectImmTransform &args)
{
    SetFalseResult();
}

void Encoder::EncodeGetTypeSize([[maybe_unused]] Reg size, [[maybe_unused]] Reg type)
{
    SetFalseResult();
}

void Encoder::EncodeLdp([[maybe_unused]] Reg dst0, [[maybe_unused]] Reg dst1, [[maybe_unused]] bool dstSigned,
                        [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}

void Encoder::EncodeLdpPreIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, Imm offset)
{
    EncodeAdd(srcBase, srcBase, offset);
    EncodeLdp(dst0, dst1, dstSigned, MemRef {srcBase});
}

void Encoder::EncodeLdpPostIndex(Reg dst0, Reg dst1, bool dstSigned, Reg srcBase, Imm offset)
{
    EncodeLdp(dst0, dst1, dstSigned, MemRef {srcBase});
    EncodeAdd(srcBase, srcBase, offset);
}

void Encoder::EncodeStp([[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1, [[maybe_unused]] MemRef mem)
{
    SetFalseResult();
}

void Encoder::EncodeStpPreIndex(Reg src0, Reg src1, Reg dstBase, Imm offset)
{
    EncodeAdd(dstBase, dstBase, offset);
    EncodeStp(src0, src1, MemRef {dstBase});
}

void Encoder::EncodeStpPostIndex(Reg src0, Reg src1, Reg dstBase, Imm offset)
{
    EncodeStp(src0, src1, MemRef {dstBase});
    EncodeAdd(dstBase, dstBase, offset);
}

void Encoder::EncodeMAdd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                         [[maybe_unused]] Reg src2)
{
    SetFalseResult();
}

void Encoder::EncodeMSub([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                         [[maybe_unused]] Reg src2)
{
    SetFalseResult();
}

void Encoder::EncodeMNeg([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1)
{
    SetFalseResult();
}

void Encoder::EncodeOrNot([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1)
{
    SetFalseResult();
}

void Encoder::EncodeAndNot([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1)
{
    SetFalseResult();
}

void Encoder::EncodeXorNot([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1)
{
    SetFalseResult();
}

void Encoder::EncodeNeg([[maybe_unused]] Reg dst, [[maybe_unused]] Shift src)
{
    SetFalseResult();
}

void Encoder::EncodeFpToBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeMoveBitsRaw([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::EncodeExtractBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src, [[maybe_unused]] Imm imm1,
                                [[maybe_unused]] Imm imm2, [[maybe_unused]] bool signExt)
{
    success_ = false;
}

void Encoder::EncodeCrc32Update([[maybe_unused]] Reg dst, [[maybe_unused]] Reg crcReg, [[maybe_unused]] Reg valReg)
{
    SetFalseResult();
}

void Encoder::EncodeStackOverflowCheck([[maybe_unused]] ssize_t offset)
{
    SetFalseResult();
}

bool Encoder::IsValid() const
{
    return false;
}

bool Encoder::GetResult() const
{
    return success_;
}

void Encoder::SetFalseResult()
{
    success_ = false;
}

bool Encoder::CanEncodeImmAddSubCmp([[maybe_unused]] int64_t imm, [[maybe_unused]] uint32_t size,
                                    [[maybe_unused]] bool signedCompare)
{
    return false;
}

bool Encoder::CanEncodeImmMulDivMod([[maybe_unused]] uint64_t imm, [[maybe_unused]] uint32_t size)
{
    return false;
}

bool Encoder::CanOptimizeImmDivMod([[maybe_unused]] uint64_t imm, [[maybe_unused]] bool isSigned) const
{
    return false;
}

bool Encoder::CanEncodeImmLogical([[maybe_unused]] uint64_t imm, [[maybe_unused]] uint32_t size)
{
    return false;
}

bool Encoder::CanEncodeScale([[maybe_unused]] uint64_t imm, [[maybe_unused]] uint32_t size)
{
    return false;
}

bool Encoder::CanEncodeShift([[maybe_unused]] uint32_t size)
{
    return true;
}
bool Encoder::CanEncodeBitCount()
{
    return false;
}

bool Encoder::CanEncodeMAdd()
{
    return false;
}

bool Encoder::CanEncodeMSub()
{
    return false;
}

bool Encoder::CanEncodeMNeg()
{
    return false;
}

bool Encoder::CanEncodeOrNot()
{
    return false;
}

bool Encoder::CanEncodeAndNot()
{
    return false;
}

bool Encoder::CanEncodeXorNot()
{
    return false;
}

bool Encoder::CanEncodeShiftedOperand([[maybe_unused]] ShiftOpcode opcode, [[maybe_unused]] ShiftType shiftType)
{
    return false;
}

bool Encoder::CanEncodeCompressedStringCharAt()
{
    return false;
}

bool Encoder::CanEncodeCompressedStringCharAtI()
{
    return false;
}

bool Encoder::CanEncodeFloatSelect()
{
    return false;
}

bool Encoder::CanEncodeBitfieldExtractionFor([[maybe_unused]] DataType::Type type)
{
    return false;
}

bool Encoder::CanEncodeSelectTransformFor([[maybe_unused]] DataType::Type type)
{
    return false;
}

void Encoder::EncodeCompareAndSwap([[maybe_unused]] Reg dst, [[maybe_unused]] Reg obj, [[maybe_unused]] Reg offset,
                                   [[maybe_unused]] Reg val, [[maybe_unused]] Reg newval)
{
    SetFalseResult();
}

void Encoder::EncodeCompareAndSwap([[maybe_unused]] Reg dst, [[maybe_unused]] Reg addr, [[maybe_unused]] Reg val,
                                   [[maybe_unused]] Reg newval)
{
    SetFalseResult();
}

void Encoder::EncodeUnsafeGetAndSet([[maybe_unused]] Reg dst, [[maybe_unused]] Reg obj, [[maybe_unused]] Reg offset,
                                    [[maybe_unused]] Reg val)
{
    SetFalseResult();
}

void Encoder::EncodeUnsafeGetAndAdd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg obj, [[maybe_unused]] Reg offset,
                                    [[maybe_unused]] Reg val, [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeMemoryBarrier([[maybe_unused]] memory_order::Order order)
{
    SetFalseResult();
}

size_t Encoder::GetCursorOffset() const
{
    return 0;
}

void Encoder::EncodeCompressEightUtf16ToUtf8CharsUsingSimd([[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg dstAddr)
{
    SetFalseResult();
}

void Encoder::EncodeCompressSixteenUtf16ToUtf8CharsUsingSimd([[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg dstAddr)
{
    SetFalseResult();
}

void Encoder::EncodeMemCharU8X32UsingSimd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg ch,
                                          [[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeMemCharU16X16UsingSimd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg ch,
                                           [[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeMemCharU8X16UsingSimd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg ch,
                                          [[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeMemCharU16X8UsingSimd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg ch,
                                          [[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeMemLastCharU16X8UsingSimd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg ch,
                                              [[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeMemLastCharU8X16UsingSimd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg ch,
                                              [[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeCharsToUpperCaseU8X16UsingSimd([[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg dstAddr,
                                                   [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeCharsToUpperCaseU8X8UsingSimd([[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg dstAddr,
                                                  [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeCharsToLowerCaseU8X16UsingSimd([[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg dstAddr,
                                                   [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeCharsToLowerCaseU8X8UsingSimd([[maybe_unused]] Reg srcAddr, [[maybe_unused]] Reg dstAddr,
                                                  [[maybe_unused]] Reg tmp)
{
    SetFalseResult();
}

void Encoder::EncodeUnsignedExtendBytesToShorts([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Encoder::SaveRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] ssize_t slot,
                            [[maybe_unused]] size_t startReg, [[maybe_unused]] bool isFp)
{
    SetFalseResult();
}

void Encoder::LoadRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] ssize_t slot,
                            [[maybe_unused]] size_t startReg, [[maybe_unused]] bool isFp)
{
    SetFalseResult();
}

void Encoder::SaveRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] bool isFp,
                            [[maybe_unused]] ssize_t slot, [[maybe_unused]] Reg base, [[maybe_unused]] RegMask mask)
{
    SetFalseResult();
}

void Encoder::LoadRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] bool isFp,
                            [[maybe_unused]] ssize_t slot, [[maybe_unused]] Reg base, [[maybe_unused]] RegMask mask)
{
    SetFalseResult();
}

void Encoder::PushRegisters(RegMask regs, VRegMask fpRegs, bool isAligned)
{
    ASSERT(GetArch() != Arch::AARCH64 || isAligned);
    PushRegisters(regs, false);
    PushRegisters(fpRegs, true);

    bool isEven {(regs.Count() + fpRegs.Count()) % 2U == 0U};
    if (GetArch() != Arch::AARCH64 && isEven != isAligned) {
        EncodeSub(GetTarget().GetStackReg(), GetTarget().GetStackReg(), Imm(GetTarget().WordSize()));
    }
}

void Encoder::PopRegisters(RegMask regs, VRegMask fpRegs, bool isAligned)
{
    bool isEven {(regs.Count() + fpRegs.Count()) % 2U == 0U};
    if (GetArch() != Arch::AARCH64 && isEven != isAligned) {
        EncodeAdd(GetTarget().GetStackReg(), GetTarget().GetStackReg(), Imm(GetTarget().WordSize()));
    }

    PopRegisters(fpRegs, true);
    PopRegisters(regs, false);
}

void Encoder::PushRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] bool isFp)
{
    SetFalseResult();
}

void Encoder::PopRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] bool isFp)
{
    SetFalseResult();
}

RegistersDescription *Encoder::GetRegfile() const
{
    ASSERT(regfile_ != nullptr);
    return regfile_;
}

void Encoder::SetRegfile(RegistersDescription *regfile)
{
    regfile_ = regfile;
}

compiler::Reg Encoder::AcquireScratchRegister([[maybe_unused]] compiler::TypeInfo type)
{
    return compiler::Reg();
}
void Encoder::AcquireScratchRegister([[maybe_unused]] compiler::Reg reg)
{
    SetFalseResult();
}

void Encoder::ReleaseScratchRegister([[maybe_unused]] compiler::Reg reg)
{
    SetFalseResult();
}

void Encoder::MakeCall([[maybe_unused]] compiler::RelocationInfo *relocation)
{
    SetFalseResult();
}

void Encoder::MakeCall([[maybe_unused]] compiler::LabelHolder::LabelId id)
{
    SetFalseResult();
}

void Encoder::MakeCall([[maybe_unused]] const void *entryPoint)
{
    SetFalseResult();
}

void Encoder::MakeCall([[maybe_unused]] Reg reg)
{
    SetFalseResult();
}

void Encoder::MakeCall([[maybe_unused]] compiler::MemRef entryPoint)
{
    SetFalseResult();
}

bool Encoder::CanMakeCallByOffset([[maybe_unused]] intptr_t offset)
{
    return false;
}

void Encoder::MakeCallAot([[maybe_unused]] intptr_t offset)
{
    SetFalseResult();
}

void Encoder::MakeCallByOffset([[maybe_unused]] intptr_t offset)
{
    SetFalseResult();
}

void Encoder::MakeLoadAotTable([[maybe_unused]] intptr_t offset, [[maybe_unused]] compiler::Reg reg)
{
    SetFalseResult();
}

void Encoder::MakeLoadAotTableAddr([[maybe_unused]] intptr_t offset, [[maybe_unused]] compiler::Reg addr,
                                   [[maybe_unused]] compiler::Reg val)
{
    SetFalseResult();
}

// Encode unconditional branch
void Encoder::EncodeJump([[maybe_unused]] compiler::LabelHolder::LabelId id)
{
    SetFalseResult();
}

void Encoder::EncodeJump([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg reg,
                         [[maybe_unused]] compiler::Condition cond)
{
    SetFalseResult();
}

void Encoder::EncodeJump([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg reg,
                         [[maybe_unused]] compiler::Imm imm, [[maybe_unused]] compiler::Condition c)
{
    SetFalseResult();
}

void Encoder::EncodeJump([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg r,
                         [[maybe_unused]] compiler::Reg reg, [[maybe_unused]] compiler::Condition c)
{
    SetFalseResult();
}

void Encoder::EncodeJumpTest([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg reg,
                             [[maybe_unused]] compiler::Imm imm, [[maybe_unused]] compiler::Condition c)
{
    SetFalseResult();
}

void Encoder::EncodeJumpTest([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg r,
                             [[maybe_unused]] compiler::Reg reg, [[maybe_unused]] compiler::Condition c)
{
    SetFalseResult();
}

// Encode jump by register value
void Encoder::EncodeJump([[maybe_unused]] compiler::Reg reg)
{
    SetFalseResult();
}

void Encoder::EncodeJump([[maybe_unused]] RelocationInfo *relocation)
{
    SetFalseResult();
}

void Encoder::EncodeBitTestAndBranch([[maybe_unused]] compiler::LabelHolder::LabelId id,
                                     [[maybe_unused]] compiler::Reg reg, [[maybe_unused]] uint32_t bitPos,
                                     [[maybe_unused]] bool bitValue)
{
    SetFalseResult();
}

void Encoder::EncodeAbort()
{
    SetFalseResult();
}

void Encoder::EncodeReturn()
{
    SetFalseResult();
}

void Encoder::EncodeGetCurrentPc([[maybe_unused]] Reg dst)
{
    SetFalseResult();
}

void Encoder::SetFrameLayout(CFrameLayout fl)
{
    frameLayout_ = fl;
}

const CFrameLayout &Encoder::GetFrameLayout() const
{
    return frameLayout_;
}

RegMask Encoder::GetLiveTmpRegMask()
{
    return liveTmpRegs_;
}

VRegMask Encoder::GetLiveTmpFpRegMask()
{
    return liveTmpFpRegs_;
}

void Encoder::AddRegInLiveMask(Reg reg)
{
    if (!reg.IsValid()) {
        return;
    }
    if (reg.IsScalar()) {
        liveTmpRegs_.set(reg.GetId(), true);
    } else {
        ASSERT(reg.IsFloat());
        liveTmpFpRegs_.set(reg.GetId(), true);
    }
}

void Encoder::RemoveRegFromLiveMask(Reg reg)
{
    if (!reg.IsValid()) {
        return;
    }
    if (reg.IsScalar()) {
        liveTmpRegs_.set(reg.GetId(), false);
    } else {
        ASSERT(reg.IsFloat());
        liveTmpFpRegs_.set(reg.GetId(), false);
    }
}

void Encoder::SetCodeOffset(size_t offset)
{
    codeOffset_ = offset;
}

size_t Encoder::GetCodeOffset() const
{
    return codeOffset_;
}

void Encoder::EnableLrAsTempReg(bool value)
{
    enableLrAsTempReg_ = value;
}

bool Encoder::IsLrAsTempRegEnabled() const
{
    return enableLrAsTempReg_;
}

bool Encoder::IsLrAsTempRegEnabledAndReleased() const
{
    return IsLrAsTempRegEnabled() && IsScratchRegisterReleased(GetTarget().GetLinkReg());
}

void Encoder::SetFrameSize(size_t size)
{
    frameSize_ = size;
}

size_t Encoder::GetFrameSize() const
{
    return frameSize_;
}

bool Encoder::IsScratchRegisterReleased([[maybe_unused]] compiler::Reg reg) const
{
    return false;
}

size_t Encoder::GetScratchRegistersCount() const
{
    return GetScratchRegistersMask().Count();
}

size_t Encoder::GetScratchRegistersWithLrCount() const
{
    return GetScratchRegistersCount() + static_cast<size_t>(IsLrAsTempRegEnabledAndReleased());
}

RegMask Encoder::GetScratchRegistersMask() const
{
    return 0;
}

size_t Encoder::GetScratchFPRegistersCount() const
{
    return GetScratchFpRegistersMask().Count();
}

RegMask Encoder::GetScratchFpRegistersMask() const
{
    return 0;
}

// Get Scratch registers, that currently are not allocated
RegMask Encoder::GetAvailableScratchRegisters() const
{
    return 0;
}

// Get Floating Point Scratch registers, that currently are not allocated
VRegMask Encoder::GetAvailableScratchFpRegisters() const
{
    return 0;
}

size_t Encoder::MaxArchInstPerEncoded()
{
    static constexpr size_t MAX_ARCH_INST_PER_ENCODE = 32;
    return MAX_ARCH_INST_PER_ENCODE;
}

void Encoder::SetRegister(RegMask *mask, VRegMask *vmask, Reg reg)
{
    SetRegister(mask, vmask, reg, true);
}

void Encoder::SetRegister(RegMask *mask, VRegMask *vmask, Reg reg, bool val) const
{
    if (!reg.IsValid()) {
        return;
    }
    if (reg.IsScalar()) {
        ASSERT(mask != nullptr);
        mask->set(reg.GetId(), val);
    } else {
        ASSERT(vmask != nullptr);
        ASSERT(reg.IsFloat());
        if (vmask != nullptr) {
            vmask->set(reg.GetId(), val);
        }
    }
}

compiler::TypeInfo Encoder::GetRefType()
{
    return compiler::TypeInfo();
}

/// Label-holder interfaces
LabelHolder::LabelId Encoder::CreateLabel()
{
    auto labels = GetLabels();
    ASSERT(labels != nullptr);
    return labels->CreateLabel();
}

void Encoder::BindLabel(LabelHolder::LabelId id)
{
    auto labels = GetLabels();
    ASSERT(labels != nullptr);
    ASSERT(labels->Size() > id);
    labels->BindLabel(id);
}

LabelHolder *Encoder::GetLabels() const
{
    return nullptr;
}
}  // namespace ark::compiler
