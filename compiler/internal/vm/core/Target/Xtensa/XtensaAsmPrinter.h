//===- XtensaAsmPrinter.h - Xtensa LLVM Assembly Printer --------*- C++-*--===//
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
// Xtensa Assembly printer class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XTENSA_XTENSAASMPRINTER_H
#define LLVM_LIB_TARGET_XTENSA_XTENSAASMPRINTER_H

#include "XtensaTargetMachine.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineConstantPool.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {
class MCStreamer;
class MachineBasicBlock;
class MachineInstr;
class Module;
class raw_ostream;

class LLVM_LIBRARY_VISIBILITY XtensaAsmPrinter : public AsmPrinter {
  const MCSubtargetInfo *STI;

public:
  static char ID;

  explicit XtensaAsmPrinter(TargetMachine &TM,
                            std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID), STI(TM.getMCSubtargetInfo()) {}

  StringRef getPassName() const override { return "Xtensa Assembly Printer"; }
  void emitInstruction(const MachineInstr *MI) override;

  void emitConstantPool() override;

  void emitMachineConstantPoolEntry(const MachineConstantPoolEntry &CPE, int i);

  void emitMachineConstantPoolValue(MachineConstantPoolValue *MCPV) override;

  void printOperand(const MachineInstr *MI, int opNum, raw_ostream &O);

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &O) override;

  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &OS) override;

  MCSymbol *GetConstantPoolIndexSymbol(const MachineOperand &MO) const;

  MCSymbol *GetJumpTableSymbol(const MachineOperand &MO) const;

  MCOperand LowerSymbolOperand(const MachineOperand &MO,
                               MachineOperand::MachineOperandType MOTy,
                               unsigned Offset) const;

  // Lower MachineInstr MI to MCInst OutMI.
  void lowerToMCInst(const MachineInstr *MI, MCInst &OutMI) const;

  // Return an MCOperand for MO.  Return an empty operand if MO is implicit.
  MCOperand lowerOperand(const MachineOperand &MO, unsigned Offset = 0) const;
};
} // end namespace vm::core

#endif /* LLVM_LIB_TARGET_XTENSA_XTENSAASMPRINTER_H */
