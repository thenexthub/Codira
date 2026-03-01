//===- LoongArchAsmPrinter.h - LoongArch LLVM Assembly Printer -*- C++ -*--===//
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
// LoongArch Assembly printer class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LOONGARCH_LOONGARCHASMPRINTER_H
#define LLVM_LIB_TARGET_LOONGARCH_LOONGARCHASMPRINTER_H

#include "LoongArchSubtarget.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/StackMaps.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {

class LLVM_LIBRARY_VISIBILITY LoongArchAsmPrinter : public AsmPrinter {
public:
  static char ID;

private:
  const MCSubtargetInfo *STI;

public:
  explicit LoongArchAsmPrinter(TargetMachine &TM,
                               std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID), STI(TM.getMCSubtargetInfo()) {}

  StringRef getPassName() const override {
    return "LoongArch Assembly Printer";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void emitInstruction(const MachineInstr *MI) override;

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &OS) override;

  void LowerSTATEPOINT(const MachineInstr &MI);
  void LowerPATCHABLE_FUNCTION_ENTER(const MachineInstr &MI);
  void LowerPATCHABLE_FUNCTION_EXIT(const MachineInstr &MI);
  void LowerPATCHABLE_TAIL_CALL(const MachineInstr &MI);
  void emitSled(const MachineInstr &MI, SledKind Kind);

  // tblgen'erated function.
  bool lowerPseudoInstExpansion(const MachineInstr *MI, MCInst &Inst);

  // Wrapper needed for tblgenned pseudo lowering.
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const {
    return lowerLoongArchMachineOperandToMCOperand(MO, MCOp, *this);
  }
  void emitJumpTableInfo() override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LOONGARCH_LOONGARCHASMPRINTER_H
