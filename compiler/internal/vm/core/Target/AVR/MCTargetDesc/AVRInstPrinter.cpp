//===-- AVRInstPrinter.cpp - Convert AVR MCInst to assembly syntax --------===//
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
// This class prints an AVR MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "AVRInstPrinter.h"

#include "MCTargetDesc/AVRMCTargetDesc.h"

#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/Support/ErrorHandling.h"

#include <cstring>

#define DEBUG_TYPE "asm-printer"

namespace vm::core {

// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "AVRGenAsmWriter.inc"

void AVRInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                               StringRef Annot, const MCSubtargetInfo &STI,
                               raw_ostream &O) {
  unsigned Opcode = MI->getOpcode();

  // First handle load and store instructions with postinc or predec
  // of the form "ld reg, X+".
  // TODO: We should be able to rewrite this using TableGen data.
  switch (Opcode) {
  case AVR::LDRdPtr:
  case AVR::LDRdPtrPi:
  case AVR::LDRdPtrPd:
    O << "\tld\t";
    printOperand(MI, 0, O);
    O << ", ";

    if (Opcode == AVR::LDRdPtrPd)
      O << '-';

    printOperand(MI, 1, O);

    if (Opcode == AVR::LDRdPtrPi)
      O << '+';
    break;
  case AVR::STPtrRr:
    O << "\tst\t";
    printOperand(MI, 0, O);
    O << ", ";
    printOperand(MI, 1, O);
    break;
  case AVR::STPtrPiRr:
  case AVR::STPtrPdRr:
    O << "\tst\t";

    if (Opcode == AVR::STPtrPdRr)
      O << '-';

    printOperand(MI, 1, O);

    if (Opcode == AVR::STPtrPiRr)
      O << '+';

    O << ", ";
    printOperand(MI, 2, O);
    break;
  default:
    if (!printAliasInstr(MI, Address, O))
      printInstruction(MI, Address, O);

    printAnnotation(O, Annot);
    break;
  }
}

const char *AVRInstPrinter::getPrettyRegisterName(MCRegister Reg,
                                                  MCRegisterInfo const &MRI) {
  // GCC prints register pairs by just printing the lower register
  // If the register contains a subregister, print it instead
  if (MRI.getNumSubRegIndices() > 0) {
    MCRegister RegLo = MRI.getSubReg(Reg, AVR::sub_lo);
    Reg = (RegLo != AVR::NoRegister) ? RegLo : Reg;
  }

  return getRegisterName(Reg);
}

void AVRInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &O) {
  const MCOperandInfo &MOI = this->MII.get(MI->getOpcode()).operands()[OpNo];
  const MCOperand &Op = MI->getOperand(OpNo);

  if (Op.isReg()) {
    bool isPtrReg = (MOI.RegClass == AVR::PTRREGSRegClassID) ||
                    (MOI.RegClass == AVR::PTRDISPREGSRegClassID) ||
                    (MOI.RegClass == AVR::ZREGRegClassID);

    if (isPtrReg) {
      O << getRegisterName(Op.getReg(), AVR::ptr);
    } else {
      O << getPrettyRegisterName(Op.getReg(), MRI);
    }
  } else if (Op.isImm()) {
    O << formatImm(Op.getImm());
  } else {
    assert(Op.isExpr() && "Unknown operand kind in printOperand");
    MAI.printExpr(O, *Op.getExpr());
  }
}

/// This is used to print an immediate value that ends up
/// being encoded as a pc-relative value.
void AVRInstPrinter::printPCRelImm(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  if (OpNo >= MI->size()) {
    // Not all operands are correctly disassembled at the moment. This means
    // that some machine instructions won't have all the necessary operands
    // set.
    // To avoid asserting, print <unknown> instead until the necessary support
    // has been implemented.
    O << "<unknown>";
    return;
  }

  const MCOperand &Op = MI->getOperand(OpNo);

  if (Op.isImm()) {
    int64_t Imm = Op.getImm();
    O << '.';

    // Print a position sign if needed.
    // Negative values have their sign printed automatically.
    if (Imm >= 0)
      O << '+';

    O << Imm;
  } else {
    assert(Op.isExpr() && "Unknown pcrel immediate operand");
    MAI.printExpr(O, *Op.getExpr());
  }
}

void AVRInstPrinter::printMemri(const MCInst *MI, unsigned OpNo,
                                raw_ostream &O) {
  assert(MI->getOperand(OpNo).isReg() &&
         "Expected a register for the first operand");

  const MCOperand &OffsetOp = MI->getOperand(OpNo + 1);

  // Print the register.
  printOperand(MI, OpNo, O);

  // Print the {+,-}offset.
  if (OffsetOp.isImm()) {
    int64_t Offset = OffsetOp.getImm();

    if (Offset >= 0)
      O << '+';

    O << Offset;
  } else if (OffsetOp.isExpr()) {
    MAI.printExpr(O, *OffsetOp.getExpr());
  } else {
    llvm_unreachable("unknown type for offset");
  }
}

} // end of namespace vm::core
