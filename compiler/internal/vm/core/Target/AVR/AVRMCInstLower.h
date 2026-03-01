//===-- AVRMCInstLower.h - Lower MachineInstr to MCInst ---------*- C++ -*-===//
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

#ifndef LLVM_AVR_MCINST_LOWER_H
#define LLVM_AVR_MCINST_LOWER_H

#include "AVRSubtarget.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {

class AsmPrinter;
class MachineInstr;
class MachineOperand;
class MCContext;
class MCInst;
class MCOperand;
class MCSymbol;

/// Lowers `MachineInstr` objects into `MCInst` objects.
class AVRMCInstLower {
public:
  AVRMCInstLower(MCContext &Ctx, AsmPrinter &Printer)
      : Ctx(Ctx), Printer(Printer) {}

  /// Lowers a `MachineInstr` into a `MCInst`.
  void lowerInstruction(const MachineInstr &MI, MCInst &OutMI) const;
  MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym,
                               const AVRSubtarget &Subtarget) const;

private:
  MCContext &Ctx;
  AsmPrinter &Printer;
};

} // end namespace vm::core

#endif // LLVM_AVR_MCINST_LOWER_H
