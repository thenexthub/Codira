//===-- VEInstPrinter.cpp - Convert VE MCInst to assembly syntax -----------==//
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
// This class prints an VE MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "VEInstPrinter.h"
#include "VE.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "ve-asmprinter"

#define GET_INSTRUCTION_NAME
#define PRINT_ALIAS_INSTR
#include "VEGenAsmWriter.inc"

void VEInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) {
  // Generic registers have identical register name among register classes.
  unsigned AltIdx = VE::AsmName;
  // Misc registers have each own name, so no use alt-names.
  if (MRI.getRegClass(VE::MISCRegClassID).contains(Reg))
    AltIdx = VE::NoRegAltName;
  OS << '%' << getRegisterName(Reg, AltIdx);
}

void VEInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                              StringRef Annot, const MCSubtargetInfo &STI,
                              raw_ostream &OS) {
  if (!printAliasInstr(MI, Address, STI, OS))
    printInstruction(MI, Address, STI, OS);
  printAnnotation(OS, Annot);
}

void VEInstPrinter::printOperand(const MCInst *MI, int OpNum,
                                 const MCSubtargetInfo &STI, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNum);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    // Expects signed 32bit literals.
    int32_t TruncatedImm = static_cast<int32_t>(MO.getImm());
    O << TruncatedImm;
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MAI.printExpr(O, *MO.getExpr());
}

void VEInstPrinter::printMemASXOperand(const MCInst *MI, int OpNum,
                                       const MCSubtargetInfo &STI,
                                       raw_ostream &O) {
  if (MI->getOperand(OpNum + 2).isImm() &&
      MI->getOperand(OpNum + 2).getImm() == 0) {
    // don't print "+0"
  } else {
    printOperand(MI, OpNum + 2, STI, O);
  }
  if (MI->getOperand(OpNum + 1).isImm() &&
      MI->getOperand(OpNum + 1).getImm() == 0 &&
      MI->getOperand(OpNum).isImm() && MI->getOperand(OpNum).getImm() == 0) {
    if (MI->getOperand(OpNum + 2).isImm() &&
        MI->getOperand(OpNum + 2).getImm() == 0) {
      O << "0";
    } else {
      // don't print "+0,+0"
    }
  } else {
    O << "(";
    if (MI->getOperand(OpNum + 1).isImm() &&
        MI->getOperand(OpNum + 1).getImm() == 0) {
      // don't print "+0"
    } else {
      printOperand(MI, OpNum + 1, STI, O);
    }
    if (MI->getOperand(OpNum).isImm() && MI->getOperand(OpNum).getImm() == 0) {
      // don't print "+0"
    } else {
      O << ", ";
      printOperand(MI, OpNum, STI, O);
    }
    O << ")";
  }
}

void VEInstPrinter::printMemASOperandASX(const MCInst *MI, int OpNum,
                                         const MCSubtargetInfo &STI,
                                         raw_ostream &O) {
  if (MI->getOperand(OpNum + 1).isImm() &&
      MI->getOperand(OpNum + 1).getImm() == 0) {
    // don't print "+0"
  } else {
    printOperand(MI, OpNum + 1, STI, O);
  }
  if (MI->getOperand(OpNum).isImm() && MI->getOperand(OpNum).getImm() == 0) {
    if (MI->getOperand(OpNum + 1).isImm() &&
        MI->getOperand(OpNum + 1).getImm() == 0) {
      O << "0";
    } else {
      // don't print "(0)"
    }
  } else {
    O << "(, ";
    printOperand(MI, OpNum, STI, O);
    O << ")";
  }
}

void VEInstPrinter::printMemASOperandRRM(const MCInst *MI, int OpNum,
                                         const MCSubtargetInfo &STI,
                                         raw_ostream &O) {
  if (MI->getOperand(OpNum + 1).isImm() &&
      MI->getOperand(OpNum + 1).getImm() == 0) {
    // don't print "+0"
  } else {
    printOperand(MI, OpNum + 1, STI, O);
  }
  if (MI->getOperand(OpNum).isImm() && MI->getOperand(OpNum).getImm() == 0) {
    if (MI->getOperand(OpNum + 1).isImm() &&
        MI->getOperand(OpNum + 1).getImm() == 0) {
      O << "0";
    } else {
      // don't print "(0)"
    }
  } else {
    O << "(";
    printOperand(MI, OpNum, STI, O);
    O << ")";
  }
}

void VEInstPrinter::printMemASOperandHM(const MCInst *MI, int OpNum,
                                        const MCSubtargetInfo &STI,
                                        raw_ostream &O) {
  if (MI->getOperand(OpNum + 1).isImm() &&
      MI->getOperand(OpNum + 1).getImm() == 0) {
    // don't print "+0"
  } else {
    printOperand(MI, OpNum + 1, STI, O);
  }
  O << "(";
  if (MI->getOperand(OpNum).isReg())
    printOperand(MI, OpNum, STI, O);
  O << ")";
}

void VEInstPrinter::printMImmOperand(const MCInst *MI, int OpNum,
                                     const MCSubtargetInfo &STI,
                                     raw_ostream &O) {
  int MImm = (int)MI->getOperand(OpNum).getImm() & 0x7f;
  if (MImm > 63)
    O << "(" << MImm - 64 << ")0";
  else
    O << "(" << MImm << ")1";
}

void VEInstPrinter::printCCOperand(const MCInst *MI, int OpNum,
                                   const MCSubtargetInfo &STI, raw_ostream &O) {
  int CC = (int)MI->getOperand(OpNum).getImm();
  O << VECondCodeToString((VECC::CondCode)CC);
}

void VEInstPrinter::printRDOperand(const MCInst *MI, int OpNum,
                                   const MCSubtargetInfo &STI, raw_ostream &O) {
  int RD = (int)MI->getOperand(OpNum).getImm();
  O << VERDToString((VERD::RoundingMode)RD);
}
