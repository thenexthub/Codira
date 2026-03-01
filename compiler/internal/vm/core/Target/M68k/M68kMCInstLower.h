//===-- M68kMCInstLower.h - Lower MachineInstr to MCInst --------*- C++ -*-===//
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
///
/// \file
/// This file contains code to lower M68k MachineInstrs to their
/// corresponding MCInst records.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M68K_M68KMCINSTLOWER_H
#define LLVM_LIB_TARGET_M68K_M68KMCINSTLOWER_H

#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {
class MCContext;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineFunction;
class M68kAsmPrinter;

/// This class is used to lower an MachineInstr into an MCInst.
class M68kMCInstLower {
  typedef MachineOperand::MachineOperandType MachineOperandType;
  MCContext &Ctx;
  MachineFunction &MF;
  const TargetMachine &TM;
  const MCAsmInfo &MAI;
  M68kAsmPrinter &AsmPrinter;

public:
  M68kMCInstLower(MachineFunction &MF, M68kAsmPrinter &AP);

  /// Lower an MO_GlobalAddress or MO_ExternalSymbol operand to an MCSymbol.
  MCSymbol *GetSymbolFromOperand(const MachineOperand &MO) const;

  MCOperand LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;

  std::optional<MCOperand> LowerOperand(const MachineInstr *MI,
                                        const MachineOperand &MO) const;

  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_M68K_M68KMCINSTLOWER_H
