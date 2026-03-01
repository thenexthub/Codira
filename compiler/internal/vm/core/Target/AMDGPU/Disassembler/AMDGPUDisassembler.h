//===- AMDGPUDisassembler.hpp - Disassembler for AMDGPU ISA -----*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
/// \file
///
/// This file contains declaration for AMDGPU ISA disassembler
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_DISASSEMBLER_AMDGPUDISASSEMBLER_H
#define LLVM_LIB_TARGET_AMDGPU_DISASSEMBLER_AMDGPUDISASSEMBLER_H

#include "SIDefines.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/MC/MCDisassembler/MCDisassembler.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/Support/DataExtractor.h"
#include <memory>

namespace vm::core {

class MCAsmInfo;
class MCInst;
class MCOperand;
class MCSubtargetInfo;
class Twine;

//===----------------------------------------------------------------------===//
// AMDGPUDisassembler
//===----------------------------------------------------------------------===//

class AMDGPUDisassembler : public MCDisassembler {
private:
  std::unique_ptr<MCInstrInfo const> const MCII;
  const MCRegisterInfo &MRI;
  const MCAsmInfo &MAI;
  const unsigned HwModeRegClass;
  const unsigned TargetMaxInstBytes;
  mutable ArrayRef<uint8_t> Bytes;
  mutable uint64_t Literal;
  mutable bool HasLiteral;
  mutable std::optional<bool> EnableWavefrontSize32;
  unsigned CodeObjectVersion;
  const MCExpr *UCVersionW64Expr;
  const MCExpr *UCVersionW32Expr;
  const MCExpr *UCVersionMDPExpr;

  const MCExpr *createConstantSymbolExpr(StringRef Id, int64_t Val);

  void decodeImmOperands(MCInst &MI, const MCInstrInfo &MCII) const;

public:
  AMDGPUDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx,
                     MCInstrInfo const *MCII);
  ~AMDGPUDisassembler() override = default;

  void setABIVersion(unsigned Version) override;

  DecodeStatus getInstruction(MCInst &MI, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CS) const override;

  const char* getRegClassName(unsigned RegClassID) const;

  MCOperand createRegOperand(MCRegister Reg) const;
  MCOperand createRegOperand(unsigned RegClassID, unsigned Val) const;
  MCOperand createSRegOperand(unsigned SRegClassID, unsigned Val) const;
  MCOperand createVGPR16Operand(unsigned RegIdx, bool IsHi) const;

  MCOperand errOperand(unsigned V, const Twine& ErrMsg) const;

  template <typename InsnType>
  DecodeStatus tryDecodeInst(const uint8_t *Table, MCInst &MI, InsnType Inst,
                             uint64_t Address, raw_ostream &Comments) const;
  template <typename InsnType>
  DecodeStatus tryDecodeInst(const uint8_t *Table1, const uint8_t *Table2,
                             MCInst &MI, InsnType Inst, uint64_t Address,
                             raw_ostream &Comments) const;

  Expected<bool> onSymbolStart(SymbolInfoTy &Symbol, uint64_t &Size,
                               ArrayRef<uint8_t> Bytes,
                               uint64_t Address) const override;

  Expected<bool> decodeKernelDescriptor(StringRef KdName,
                                        ArrayRef<uint8_t> Bytes,
                                        uint64_t KdAddress) const;

  Expected<bool>
  decodeKernelDescriptorDirective(DataExtractor::Cursor &Cursor,
                                  ArrayRef<uint8_t> Bytes,
                                  raw_string_ostream &KdStream) const;

  /// Decode as directives that handle COMPUTE_PGM_RSRC1.
  /// \param FourByteBuffer - Bytes holding contents of COMPUTE_PGM_RSRC1.
  /// \param KdStream       - Stream to write the disassembled directives to.
  // NOLINTNEXTLINE(readability-identifier-naming)
  Expected<bool> decodeCOMPUTE_PGM_RSRC1(uint32_t FourByteBuffer,
                                         raw_string_ostream &KdStream) const;

  /// Decode as directives that handle COMPUTE_PGM_RSRC2.
  /// \param FourByteBuffer - Bytes holding contents of COMPUTE_PGM_RSRC2.
  /// \param KdStream       - Stream to write the disassembled directives to.
  // NOLINTNEXTLINE(readability-identifier-naming)
  Expected<bool> decodeCOMPUTE_PGM_RSRC2(uint32_t FourByteBuffer,
                                         raw_string_ostream &KdStream) const;

  /// Decode as directives that handle COMPUTE_PGM_RSRC3.
  /// \param FourByteBuffer - Bytes holding contents of COMPUTE_PGM_RSRC3.
  /// \param KdStream       - Stream to write the disassembled directives to.
  // NOLINTNEXTLINE(readability-identifier-naming)
  Expected<bool> decodeCOMPUTE_PGM_RSRC3(uint32_t FourByteBuffer,
                                         raw_string_ostream &KdStream) const;

  void convertEXPInst(MCInst &MI) const;
  void convertVINTERPInst(MCInst &MI) const;
  void convertFMAanyK(MCInst &MI) const;
  void convertSDWAInst(MCInst &MI) const;
  void convertMAIInst(MCInst &MI) const;
  void convertWMMAInst(MCInst &MI) const;
  void convertDPP8Inst(MCInst &MI) const;
  void convertMIMGInst(MCInst &MI) const;
  void convertVOP3DPPInst(MCInst &MI) const;
  void convertVOP3PDPPInst(MCInst &MI) const;
  void convertVOPCDPPInst(MCInst &MI) const;
  void convertVOPC64DPPInst(MCInst &MI) const;
  void convertMacDPPInst(MCInst &MI) const;
  void convertTrue16OpSel(MCInst &MI) const;

  unsigned getVgprClassId(unsigned Width) const;
  unsigned getAgprClassId(unsigned Width) const;
  unsigned getSgprClassId(unsigned Width) const;
  unsigned getTtmpClassId(unsigned Width) const;

  static MCOperand decodeIntImmed(unsigned Imm);

  MCOperand decodeMandatoryLiteralConstant(unsigned Imm) const;
  MCOperand decodeMandatoryLiteral64Constant(uint64_t Imm) const;
  MCOperand decodeLiteralConstant(const MCInstrDesc &Desc,
                                  const MCOperandInfo &OpDesc) const;
  MCOperand decodeLiteral64Constant() const;

  MCOperand decodeSrcOp(const MCInst &Inst, unsigned Width, unsigned Val) const;

  MCOperand decodeNonVGPRSrcOp(const MCInst &Inst, unsigned Width,
                               unsigned Val) const;

  MCOperand decodeVOPDDstYOp(MCInst &Inst, unsigned Val) const;
  MCOperand decodeSpecialReg32(unsigned Val) const;
  MCOperand decodeSpecialReg64(unsigned Val) const;
  MCOperand decodeSpecialReg96Plus(unsigned Val) const;

  MCOperand decodeSDWASrc(unsigned Width, unsigned Val) const;
  MCOperand decodeSDWASrc16(unsigned Val) const;
  MCOperand decodeSDWASrc32(unsigned Val) const;
  MCOperand decodeSDWAVopcDst(unsigned Val) const;

  MCOperand decodeBoolReg(const MCInst &Inst, unsigned Val) const;
  MCOperand decodeSplitBarrier(const MCInst &Inst, unsigned Val) const;
  MCOperand decodeDpp8FI(unsigned Val) const;

  MCOperand decodeVersionImm(unsigned Imm) const;

  int getTTmpIdx(unsigned Val) const;

  const MCInstrInfo *getMCII() const { return MCII.get(); }

  bool isVI() const;
  bool isGFX9() const;
  bool isGFX90A() const;
  bool isGFX9Plus() const;
  bool isGFX10() const;
  bool isGFX10Plus() const;
  bool isGFX11() const;
  bool isGFX11Plus() const;
  bool isGFX12() const;
  bool isGFX12Plus() const;
  bool isGFX1250() const;

  bool hasArchitectedFlatScratch() const;
  bool hasKernargPreload() const;

  bool isMacDPP(MCInst &MI) const;

  /// Check if the instruction is a buffer operation (MUBUF, MTBUF, or S_BUFFER)
  bool isBufferInstruction(const MCInst &MI) const;
};

//===----------------------------------------------------------------------===//
// AMDGPUSymbolizer
//===----------------------------------------------------------------------===//

class AMDGPUSymbolizer : public MCSymbolizer {
private:
  void *DisInfo;
  std::vector<uint64_t> ReferencedAddresses;

public:
  AMDGPUSymbolizer(MCContext &Ctx, std::unique_ptr<MCRelocationInfo> &&RelInfo,
                   void *disInfo)
                   : MCSymbolizer(Ctx, std::move(RelInfo)), DisInfo(disInfo) {}

  bool tryAddingSymbolicOperand(MCInst &Inst, raw_ostream &cStream,
                                int64_t Value, uint64_t Address, bool IsBranch,
                                uint64_t Offset, uint64_t OpSize,
                                uint64_t InstSize) override;

  void tryAddingPcLoadReferenceComment(raw_ostream &cStream,
                                       int64_t Value,
                                       uint64_t Address) override;

  ArrayRef<uint64_t> getReferencedAddresses() const override {
    return ReferencedAddresses;
  }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_DISASSEMBLER_AMDGPUDISASSEMBLER_H
