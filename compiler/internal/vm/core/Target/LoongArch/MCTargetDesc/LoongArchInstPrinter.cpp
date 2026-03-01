//===- LoongArchInstPrinter.cpp - Convert LoongArch MCInst to asm syntax --===//
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
// This class prints an LoongArch MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "LoongArchInstPrinter.h"
#include "LoongArchMCTargetDesc.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/Support/CommandLine.h"
using namespace vm::core;

#define DEBUG_TYPE "loongarch-asm-printer"

// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "LoongArchGenAsmWriter.inc"

static cl::opt<bool>
    NoAliases("loongarch-no-aliases",
              cl::desc("Disable the emission of assembler pseudo instructions"),
              cl::init(false), cl::Hidden);

static cl::opt<bool>
    NumericReg("loongarch-numeric-reg",
               cl::desc("Print numeric register names rather than the ABI "
                        "names (such as $r0 instead of $zero)"),
               cl::init(false), cl::Hidden);

// The command-line flag above is used by toolchain-mc and llc. It can be used by
// `toolchain-objdump`, but we override the value here to handle options passed to
// `toolchain-objdump` with `-M` (which matches GNU objdump). There did not seem to
// be an easier way to allow these options in all these tools, without doing it
// this way.
bool LoongArchInstPrinter::applyTargetSpecificCLOption(StringRef Opt) {
  if (Opt == "no-aliases") {
    PrintAliases = false;
    return true;
  }

  if (Opt == "numeric") {
    NumericReg = true;
    return true;
  }

  return false;
}

void LoongArchInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                     StringRef Annot,
                                     const MCSubtargetInfo &STI,
                                     raw_ostream &O) {
  if (!PrintAliases || NoAliases || !printAliasInstr(MI, Address, STI, O))
    printInstruction(MI, Address, STI, O);
  printAnnotation(O, Annot);
}

void LoongArchInstPrinter::printRegName(raw_ostream &O, MCRegister Reg) {
  O << '$' << getRegisterName(Reg);
}

void LoongArchInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                        const MCSubtargetInfo &STI,
                                        raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    O << MO.getImm();
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MAI.printExpr(O, *MO.getExpr());
}

void LoongArchInstPrinter::printAtomicMemOp(const MCInst *MI, unsigned OpNo,
                                            const MCSubtargetInfo &STI,
                                            raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);
  assert(MO.isReg() && "printAtomicMemOp can only print register operands");
  printRegName(O, MO.getReg());
}

const char *LoongArchInstPrinter::getRegisterName(MCRegister Reg) {
  // Default print reg alias name
  return getRegisterName(Reg, NumericReg ? LoongArch::NoRegAltName
                                         : LoongArch::RegAliasName);
}
