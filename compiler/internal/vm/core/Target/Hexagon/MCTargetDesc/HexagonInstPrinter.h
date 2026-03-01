//===-- HexagonInstPrinter.h - Convert Hexagon MCInst to assembly syntax --===//
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
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_INSTPRINTER_HEXAGONINSTPRINTER_H
#define LLVM_LIB_TARGET_HEXAGON_INSTPRINTER_HEXAGONINSTPRINTER_H

#include "vm/core/MC/MCInstPrinter.h"

namespace vm::core {
/// Prints bundles as a newline separated list of individual instructions
/// Duplexes are separated by a vertical tab \v character
/// A trailing line includes bundle properties such as endloop0/1
///
/// r0 = add(r1, r2)
/// r0 = #0 \v jump 0x0
/// :endloop0 :endloop1
class HexagonInstPrinter : public MCInstPrinter {
public:
  explicit HexagonInstPrinter(MCAsmInfo const &MAI, MCInstrInfo const &MII,
                              MCRegisterInfo const &MRI)
    : MCInstPrinter(MAI, MII, MRI), MII(MII) {}

  void printInst(MCInst const *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &O) override;
  void printRegName(raw_ostream &O, MCRegister Reg) override;

  static char const *getRegisterName(MCRegister Reg);

  std::pair<const char *, uint64_t>
  getMnemonic(const MCInst &MI) const override;
  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &O);
  void printOperand(MCInst const *MI, unsigned OpNo, raw_ostream &O) const;
  void printBrtarget(MCInst const *MI, unsigned OpNo, raw_ostream &O) const;

  MCAsmInfo const &getMAI() const { return MAI; }
  MCInstrInfo const &getMII() const { return MII; }

private:
  MCInstrInfo const &MII;
  bool HasExtender = false;
};

} // end namespace vm::core

#endif
