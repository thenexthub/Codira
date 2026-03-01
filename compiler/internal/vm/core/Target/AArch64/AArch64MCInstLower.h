//===-- AArch64MCInstLower.h - Lower MachineInstr to MCInst ---------------===//
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

#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64MCINSTLOWER_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64MCINSTLOWER_H

#include "vm/core/IR/GlobalValue.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/TargetParser/Triple.h"

namespace vm::core {
class AsmPrinter;
class MCContext;
class MCInst;
class MCOperand;
class MCSymbol;
class MachineInstr;
class MachineOperand;

/// AArch64MCInstLower - This class is used to lower an MachineInstr
/// into an MCInst.
class LLVM_LIBRARY_VISIBILITY AArch64MCInstLower {
  MCContext &Ctx;
  AsmPrinter &Printer;
  Triple TargetTriple;

public:
  AArch64MCInstLower(MCContext &ctx, AsmPrinter &printer);

  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const;
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;

  MCOperand lowerSymbolOperandMachO(const MachineOperand &MO,
                                    MCSymbol *Sym) const;
  MCOperand lowerSymbolOperandELF(const MachineOperand &MO,
                                  MCSymbol *Sym) const;
  MCOperand lowerSymbolOperandCOFF(const MachineOperand &MO,
                                   MCSymbol *Sym) const;
  MCOperand LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;

  MCSymbol *GetGlobalValueSymbol(const GlobalValue *GV,
                                 unsigned TargetFlags) const;
  MCSymbol *GetGlobalAddressSymbol(const MachineOperand &MO) const;
  MCSymbol *GetExternalSymbolSymbol(const MachineOperand &MO) const;
};
}

#endif
