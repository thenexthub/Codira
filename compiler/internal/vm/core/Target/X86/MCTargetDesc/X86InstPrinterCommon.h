//===-- X86InstPrinterCommon.cpp - X86 assembly instruction printing ------===//
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
// This file includes code common for rendering MCInst instances as AT&T-style
// and Intel-style assembly.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_MCTARGETDESC_X86INSTPRINTERCOMMON_H
#define LLVM_LIB_TARGET_X86_MCTARGETDESC_X86INSTPRINTERCOMMON_H

#include "vm/core/MC/MCInstPrinter.h"

namespace vm::core {
class MCExpr;

class X86InstPrinterCommon : public MCInstPrinter {
public:
  using MCInstPrinter::MCInstPrinter;

  virtual void printExprOperand(raw_ostream &OS, const MCExpr &E);
  virtual void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O) = 0;
  void printCondCode(const MCInst *MI, unsigned Op, raw_ostream &OS);
  void printCondFlags(const MCInst *MI, unsigned Op, raw_ostream &OS);
  void printSSEAVXCC(const MCInst *MI, unsigned Op, raw_ostream &OS);
  void printVPCOMMnemonic(const MCInst *MI, raw_ostream &OS);
  void printVPCMPMnemonic(const MCInst *MI, raw_ostream &OS);
  void printCMPMnemonic(const MCInst *MI, bool IsVCmp, raw_ostream &OS);
  void printRoundingControl(const MCInst *MI, unsigned Op, raw_ostream &O);
  void printPCRelImm(const MCInst *MI, uint64_t Address, unsigned OpNo,
                     raw_ostream &O);

protected:
  void printInstFlags(const MCInst *MI, raw_ostream &O,
                      const MCSubtargetInfo &STI);
  void printOptionalSegReg(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printVKPair(const MCInst *MI, unsigned OpNo, raw_ostream &OS);
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_X86_MCTARGETDESC_X86INSTPRINTERCOMMON_H
