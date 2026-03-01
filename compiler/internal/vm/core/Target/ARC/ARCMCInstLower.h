//===- ARCMCInstLower.h - Lower MachineInstr to MCInst ----------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_ARC_ARCMCINSTLOWER_H
#define LLVM_LIB_TARGET_ARC_ARCMCINSTLOWER_H

#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {

class MCContext;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineFunction;
class Mangler;
class AsmPrinter;

/// This class is used to lower an MachineInstr into an MCInst.
class LLVM_LIBRARY_VISIBILITY ARCMCInstLower {
  using MachineOperandType = MachineOperand::MachineOperandType;
  MCContext *Ctx;
  AsmPrinter &Printer;

public:
  ARCMCInstLower(MCContext *C, AsmPrinter &asmprinter);
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
  MCOperand LowerOperand(const MachineOperand &MO, unsigned offset = 0) const;

private:
  MCOperand LowerSymbolOperand(const MachineOperand &MO,
                               MachineOperandType MOTy, unsigned Offset) const;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARC_ARCMCINSTLOWER_H
