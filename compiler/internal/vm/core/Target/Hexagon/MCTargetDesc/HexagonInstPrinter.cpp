//===- HexagonInstPrinter.cpp - Convert Hexagon MCInst to assembly syntax -===//
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
// This class prints an Hexagon MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "HexagonInstPrinter.h"
#include "MCTargetDesc/HexagonBaseInfo.h"
#include "MCTargetDesc/HexagonMCInstrInfo.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "asm-printer"

#define GET_INSTRUCTION_NAME
#include "HexagonGenAsmWriter.inc"

void HexagonInstPrinter::printRegName(raw_ostream &O, MCRegister Reg) {
  O << getRegisterName(Reg);
}

void HexagonInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                   StringRef Annot, const MCSubtargetInfo &STI,
                                   raw_ostream &OS) {
  if (HexagonMCInstrInfo::isDuplex(MII, *MI)) {
    printInstruction(MI->getOperand(1).getInst(), Address, OS);
    OS << '\v';
    HasExtender = false;
    printInstruction(MI->getOperand(0).getInst(), Address, OS);
  } else {
    printInstruction(MI, Address, OS);
  }
  HasExtender = HexagonMCInstrInfo::isImmext(*MI);
  if ((MI->getOpcode() & HexagonII::INST_PARSE_MASK) ==
      HexagonII::INST_PARSE_PACKET_END)
    HasExtender = false;
}

void HexagonInstPrinter::printOperand(MCInst const *MI, unsigned OpNo,
                                      raw_ostream &O) const {
  if (HexagonMCInstrInfo::getExtendableOp(MII, *MI) == OpNo &&
      (HasExtender || HexagonMCInstrInfo::isConstExtended(MII, *MI)))
    O << "#";
  MCOperand const &MO = MI->getOperand(OpNo);
  if (MO.isReg()) {
    O << getRegisterName(MO.getReg());
  } else if (MO.isExpr()) {
    int64_t Value;
    if (MO.getExpr()->evaluateAsAbsolute(Value))
      O << formatImm(Value);
    else
      MAI.printExpr(O, *MO.getExpr());
  } else {
    llvm_unreachable("Unknown operand");
  }
}

void HexagonInstPrinter::printBrtarget(MCInst const *MI, unsigned OpNo,
                                       raw_ostream &O) const {
  MCOperand const &MO = MI->getOperand(OpNo);
  assert (MO.isExpr());
  MCExpr const &Expr = *MO.getExpr();
  int64_t Value;
  if (Expr.evaluateAsAbsolute(Value))
    O << format("0x%" PRIx64, Value);
  else {
    if (HasExtender || HexagonMCInstrInfo::isConstExtended(MII, *MI))
      if (HexagonMCInstrInfo::getExtendableOp(MII, *MI) == OpNo)
        O << "##";
    MAI.printExpr(O, Expr);
  }
}
