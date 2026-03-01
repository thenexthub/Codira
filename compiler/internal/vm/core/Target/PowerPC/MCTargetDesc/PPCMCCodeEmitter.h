//===-- PPCMCCodeEmitter.h - Convert PPC code to machine code -------------===//
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
// This file implements the PPCMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PPC_MCCODEEMITTER_PPCCODEEMITTER_H
#define LLVM_LIB_TARGET_PPC_MCCODEEMITTER_PPCCODEEMITTER_H

#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCInst.h"

namespace vm::core {

class PPCMCCodeEmitter : public MCCodeEmitter {
  const MCInstrInfo &MCII;
  const MCContext &CTX;
  bool IsLittleEndian;

public:
  PPCMCCodeEmitter(const MCInstrInfo &mcii, MCContext &ctx)
      : MCII(mcii), CTX(ctx),
        IsLittleEndian(ctx.getAsmInfo()->isLittleEndian()) {}
  PPCMCCodeEmitter(const PPCMCCodeEmitter &) = delete;
  void operator=(const PPCMCCodeEmitter &) = delete;
  ~PPCMCCodeEmitter() override = default;

  unsigned getDirectBrEncoding(const MCInst &MI, unsigned OpNo,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const;
  unsigned getCondBrEncoding(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getAbsDirectBrEncoding(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups,
                                  const MCSubtargetInfo &STI) const;
  unsigned getAbsCondBrEncoding(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups,
                                const MCSubtargetInfo &STI) const;
  template <MCFixupKind Fixup>
  uint64_t getImmEncoding(const MCInst &MI, unsigned OpNo,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;
  unsigned getDispRIEncoding(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getDispRIXEncoding(const MCInst &MI, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const;
  unsigned getDispRIX16Encoding(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups,
                                const MCSubtargetInfo &STI) const;
  unsigned getDispRIHashEncoding(const MCInst &MI, unsigned OpNo,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;
  uint64_t getDispRI34PCRelEncoding(const MCInst &MI, unsigned OpNo,
                                    SmallVectorImpl<MCFixup> &Fixups,
                                    const MCSubtargetInfo &STI) const;
  uint64_t getDispRI34Encoding(const MCInst &MI, unsigned OpNo,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const;
  unsigned getDispSPE8Encoding(const MCInst &MI, unsigned OpNo,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const;
  unsigned getDispSPE4Encoding(const MCInst &MI, unsigned OpNo,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const;
  unsigned getDispSPE2Encoding(const MCInst &MI, unsigned OpNo,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const;
  unsigned getTLSRegEncoding(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getTLSCallEncoding(const MCInst &MI, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const;
  unsigned get_crbitm_encoding(const MCInst &MI, unsigned OpNo,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const;
  unsigned getVSRpEvenEncoding(const MCInst &MI, unsigned OpNo,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  uint64_t getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // Get the number of bytes used to encode the given MCInst.
  unsigned getInstSizeInBytes(const MCInst &MI) const;

  // Is this instruction a prefixed instruction.
  bool isPrefixedInstruction(const MCInst &MI) const;

  /// Check if Opcode corresponds to a call instruction that should be marked
  /// with the NOTOC relocation.
  bool isNoTOCCallInstr(const MCInst &MI) const;
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_PPC_MCCODEEMITTER_PPCCODEEMITTER_H
