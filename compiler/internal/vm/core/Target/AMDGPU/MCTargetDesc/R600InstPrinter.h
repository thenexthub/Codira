//===-- R600InstPrinter.h - AMDGPU MC Inst -> ASM interface -----*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_R600INSTPRINTER_H
#define LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_R600INSTPRINTER_H

#include "vm/core/MC/MCInstPrinter.h"

namespace vm::core {

class R600InstPrinter : public MCInstPrinter {
public:
  R600InstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                  const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &O) override;
  std::pair<const char *, uint64_t>
  getMnemonic(const MCInst &MI) const override;
  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &O);
  static const char *getRegisterName(MCRegister Reg);

  void printAbs(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printBankSwizzle(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printClamp(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printCT(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printKCache(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printLast(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printLiteral(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printMemOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printNeg(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printOMOD(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printRel(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printRSel(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printUpdateExecMask(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printUpdatePred(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printWrite(const MCInst *MI, unsigned OpNo, raw_ostream &O);
};

} // End namespace vm::core

#endif
