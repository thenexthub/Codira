//===-- XCoreInstPrinter.cpp - Convert XCore MCInst to assembly syntax ----===//
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
// This class prints an XCore MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "XCoreInstPrinter.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCRegister.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>

using namespace vm::core;

#define DEBUG_TYPE "asm-printer"

#include "XCoreGenAsmWriter.inc"

void XCoreInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) {
  OS << StringRef(getRegisterName(Reg)).lower();
}

void XCoreInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                 StringRef Annot, const MCSubtargetInfo &STI,
                                 raw_ostream &O) {
  printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void XCoreInstPrinter::
printInlineJT(const MCInst *MI, int opNum, raw_ostream &O) {
  report_fatal_error("can't handle InlineJT");
}

void XCoreInstPrinter::
printInlineJT32(const MCInst *MI, int opNum, raw_ostream &O) {
  report_fatal_error("can't handle InlineJT32");
}

static void printExpr(const MCExpr *Expr, const MCAsmInfo *MAI,
                      raw_ostream &OS) {
  int Offset = 0;
  const MCSymbolRefExpr *SRE;

  if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(Expr)) {
    SRE = dyn_cast<MCSymbolRefExpr>(BE->getLHS());
    const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(BE->getRHS());
    assert(SRE && CE && "Binary expression must be sym+const.");
    Offset = CE->getValue();
  } else {
    SRE = dyn_cast<MCSymbolRefExpr>(Expr);
    assert(SRE && "Unexpected MCExpr type.");
  }
  assert(SRE->getSpecifier() == 0);

  SRE->getSymbol().print(OS, MAI);

  if (Offset) {
    if (Offset > 0)
      OS << '+';
    OS << Offset;
  }
}

void XCoreInstPrinter::
printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isImm()) {
    O << Op.getImm();
    return;
  }

  assert(Op.isExpr() && "unknown operand kind in printOperand");
  printExpr(Op.getExpr(), &MAI, O);
}
