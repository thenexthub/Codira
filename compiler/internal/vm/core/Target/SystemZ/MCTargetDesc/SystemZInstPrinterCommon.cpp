//=- SystemZInstPrinterCommon.cpp - Common SystemZ MCInst to assembly funcs -=//
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

#include "SystemZInstPrinterCommon.h"
#include "MCTargetDesc/SystemZMCAsmInfo.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCRegister.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/MathExtras.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>

using namespace vm::core;

#define DEBUG_TYPE "asm-printer"

void SystemZInstPrinterCommon::printAddress(const MCAsmInfo *MAI,
                                            MCRegister Base,
                                            const MCOperand &DispMO,
                                            MCRegister Index, raw_ostream &O) {
  printOperand(DispMO, MAI, O);
  if (Base || Index) {
    O << '(';
    if (Index) {
      printRegName(O, Index);
      O << ',';
    }
    if (Base)
      printRegName(O, Base);
    else
      O << '0';
    O << ')';
  }
}

void SystemZInstPrinterCommon::printOperand(const MCOperand &MO,
                                            const MCAsmInfo *MAI,
                                            raw_ostream &O) {
  if (MO.isReg()) {
    if (!MO.getReg())
      O << '0';
    else
      printRegName(O, MO.getReg());
  } else if (MO.isImm())
    markup(O, Markup::Immediate) << MO.getImm();
  else if (MO.isExpr())
    MAI->printExpr(O, *MO.getExpr());
  else
    llvm_unreachable("Invalid operand");
}

void SystemZInstPrinterCommon::printRegName(raw_ostream &O, MCRegister Reg) {
  printFormattedRegName(&MAI, Reg, O);
}

template <unsigned N>
void SystemZInstPrinterCommon::printUImmOperand(const MCInst *MI, int OpNum,
                                                raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNum);
  if (MO.isExpr()) {
    MAI.printExpr(O, *MO.getExpr());
    return;
  }
  uint64_t Value = static_cast<uint64_t>(MO.getImm());
  assert(isUInt<N>(Value) && "Invalid uimm argument");
  markup(O, Markup::Immediate) << Value;
}

template <unsigned N>
void SystemZInstPrinterCommon::printSImmOperand(const MCInst *MI, int OpNum,
                                                raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNum);
  if (MO.isExpr()) {
    MAI.printExpr(O, *MO.getExpr());
    return;
  }
  int64_t Value = MI->getOperand(OpNum).getImm();
  assert(isInt<N>(Value) && "Invalid simm argument");
  markup(O, Markup::Immediate) << Value;
}

void SystemZInstPrinterCommon::printU1ImmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  printUImmOperand<1>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printU2ImmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  printUImmOperand<2>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printU3ImmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  printUImmOperand<3>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printU4ImmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  printUImmOperand<4>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printS8ImmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  printSImmOperand<8>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printU8ImmOperand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  printUImmOperand<8>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printU12ImmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  printUImmOperand<12>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printS16ImmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  printSImmOperand<16>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printU16ImmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  printUImmOperand<16>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printS32ImmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  printSImmOperand<32>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printU32ImmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  printUImmOperand<32>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printU48ImmOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  printUImmOperand<48>(MI, OpNum, O);
}

void SystemZInstPrinterCommon::printPCRelOperand(const MCInst *MI,
                                                 uint64_t Address, int OpNum,
                                                 raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNum);

  // If the label has already been resolved to an immediate offset (say, when
  // we're running the disassembler), just print the immediate.
  if (MO.isImm()) {
    int64_t Offset = MO.getImm();
    if (PrintBranchImmAsAddress)
      markup(O, Markup::Target) << formatHex(Address + Offset);
    else
      markup(O, Markup::Immediate) << formatImm(Offset);
    return;
  }

  // If the branch target is simply an address then print it in hex.
  const MCConstantExpr *BranchTarget = dyn_cast<MCConstantExpr>(MO.getExpr());
  int64_t TargetAddress;
  if (BranchTarget && BranchTarget->evaluateAsAbsolute(TargetAddress)) {
    markup(O, Markup::Target) << formatHex((uint64_t)TargetAddress);
  } else {
    // Otherwise, just print the expression.
    MAI.printExpr(O, *MO.getExpr());
  }
}

void SystemZInstPrinterCommon::printPCRelTLSOperand(const MCInst *MI,
                                                    uint64_t Address, int OpNum,
                                                    raw_ostream &O) {
  // Output the PC-relative operand.
  printPCRelOperand(MI, Address, OpNum, O);

  // Output the TLS marker if present.
  if ((unsigned)OpNum + 1 < MI->getNumOperands()) {
    const MCOperand &MO = MI->getOperand(OpNum + 1);
    const MCSymbolRefExpr &RefExp = cast<MCSymbolRefExpr>(*MO.getExpr());
    switch (RefExp.getSpecifier()) {
    case SystemZ::S_TLSGD:
      O << ":tls_gdcall:";
      break;
    case SystemZ::S_TLSLDM:
      O << ":tls_ldcall:";
      break;
    default:
      llvm_unreachable("Unexpected symbol kind");
    }
    O << RefExp.getSymbol().getName();
  }
}

void SystemZInstPrinterCommon::printOperand(const MCInst *MI, int OpNum,
                                            raw_ostream &O) {
  printOperand(MI->getOperand(OpNum), &MAI, O);
}

void SystemZInstPrinterCommon::printBDAddrOperand(const MCInst *MI, int OpNum,
                                                  raw_ostream &O) {
  printAddress(&MAI, MI->getOperand(OpNum).getReg(), MI->getOperand(OpNum + 1),
               0, O);
}

void SystemZInstPrinterCommon::printBDXAddrOperand(const MCInst *MI, int OpNum,
                                                   raw_ostream &O) {
  printAddress(&MAI, MI->getOperand(OpNum).getReg(), MI->getOperand(OpNum + 1),
               MI->getOperand(OpNum + 2).getReg(), O);
}

void SystemZInstPrinterCommon::printBDLAddrOperand(const MCInst *MI, int OpNum,
                                                   raw_ostream &O) {
  MCRegister Base = MI->getOperand(OpNum).getReg();
  const MCOperand &DispMO = MI->getOperand(OpNum + 1);
  uint64_t Length = MI->getOperand(OpNum + 2).getImm();
  printOperand(DispMO, &MAI, O);
  O << '(' << Length;
  if (Base) {
    O << ",";
    printRegName(O, Base);
  }
  O << ')';
}

void SystemZInstPrinterCommon::printBDRAddrOperand(const MCInst *MI, int OpNum,
                                                   raw_ostream &O) {
  MCRegister Base = MI->getOperand(OpNum).getReg();
  const MCOperand &DispMO = MI->getOperand(OpNum + 1);
  MCRegister Length = MI->getOperand(OpNum + 2).getReg();
  printOperand(DispMO, &MAI, O);
  O << "(";
  printRegName(O, Length);
  if (Base) {
    O << ",";
    printRegName(O, Base);
  }
  O << ')';
}

void SystemZInstPrinterCommon::printBDVAddrOperand(const MCInst *MI, int OpNum,
                                                   raw_ostream &O) {
  printAddress(&MAI, MI->getOperand(OpNum).getReg(), MI->getOperand(OpNum + 1),
               MI->getOperand(OpNum + 2).getReg(), O);
}

void SystemZInstPrinterCommon::printLXAAddrOperand(const MCInst *MI, int OpNum,
                                             raw_ostream &O) {
  printAddress(&MAI, MI->getOperand(OpNum).getReg(), MI->getOperand(OpNum + 1),
               MI->getOperand(OpNum + 2).getReg(), O);
}

void SystemZInstPrinterCommon::printCond4Operand(const MCInst *MI, int OpNum,
                                                 raw_ostream &O) {
  static const char *const CondNames[] = {"o",  "h",  "nle", "l",   "nhe",
                                          "lh", "ne", "e",   "nlh", "he",
                                          "nl", "le", "nh",  "no"};
  uint64_t Imm = MI->getOperand(OpNum).getImm();
  assert(Imm > 0 && Imm < 15 && "Invalid condition");
  O << CondNames[Imm - 1];
}
