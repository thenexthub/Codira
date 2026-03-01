//===- lib/MC/MCInst.cpp - MCInst implementation --------------------------===//
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

#include "vm/core/MC/MCInst.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInstPrinter.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

void MCOperand::print(raw_ostream &OS, const MCContext *Ctx) const {
  OS << "<MCOperand ";
  if (!isValid())
    OS << "INVALID";
  else if (isReg()) {
    OS << "Reg:";
    if (Ctx && Ctx->getRegisterInfo())
      OS << Ctx->getRegisterInfo()->getName(getReg());
    else
      OS << getReg().id();
  } else if (isImm())
    OS << "Imm:" << getImm();
  else if (isSFPImm())
    OS << "SFPImm:" << bit_cast<float>(getSFPImm());
  else if (isDFPImm())
    OS << "DFPImm:" << bit_cast<double>(getDFPImm());
  else if (isExpr()) {
    OS << "Expr:";
    if (Ctx)
      Ctx->getAsmInfo()->printExpr(OS, *getExpr());
    else
      getExpr()->print(OS, nullptr);
  } else if (isInst()) {
    OS << "Inst:(";
    if (const auto *Inst = getInst())
      Inst->print(OS, Ctx);
    else
      OS << "NULL";
    OS << ")";
  } else
    OS << "UNDEFINED";
  OS << ">";
}

bool MCOperand::evaluateAsConstantImm(int64_t &Imm) const {
  if (isImm()) {
    Imm = getImm();
    return true;
  }
  return false;
}

bool MCOperand::isBareSymbolRef() const {
  assert(isExpr() &&
         "isBareSymbolRef expects only expressions");
  const MCExpr *Expr = getExpr();
  MCExpr::ExprKind Kind = getExpr()->getKind();
  return Kind == MCExpr::SymbolRef &&
         cast<MCSymbolRefExpr>(Expr)->getSpecifier() == 0;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void MCOperand::dump() const {
  print(dbgs());
  dbgs() << "\n";
}
#endif

void MCInst::print(raw_ostream &OS, const MCContext *Ctx) const {
  OS << "<MCInst " << getOpcode();
  for (unsigned i = 0, e = getNumOperands(); i != e; ++i) {
    OS << " ";
    getOperand(i).print(OS, Ctx);
  }
  OS << ">";
}

void MCInst::dump_pretty(raw_ostream &OS, const MCInstPrinter *Printer,
                         StringRef Separator, const MCContext *Ctx) const {
  StringRef InstName = Printer ? Printer->getOpcodeName(getOpcode()) : "";
  dump_pretty(OS, InstName, Separator, Ctx);
}

void MCInst::dump_pretty(raw_ostream &OS, StringRef Name, StringRef Separator,
                         const MCContext *Ctx) const {
  OS << "<MCInst #" << getOpcode();

  // Show the instruction opcode name if we have it.
  if (!Name.empty())
    OS << ' ' << Name;

  for (unsigned i = 0, e = getNumOperands(); i != e; ++i) {
    OS << Separator;
    getOperand(i).print(OS, Ctx);
  }
  OS << ">";
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void MCInst::dump() const {
  print(dbgs());
  dbgs() << "\n";
}
#endif
