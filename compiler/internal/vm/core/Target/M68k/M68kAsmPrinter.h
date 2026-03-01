//===-- M68kAsmPrinter.h - M68k LLVM Assembly Printer -----------*- C++ -*-===//
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
/// This file contains M68k assembler printer declarations.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M68K_M68KASMPRINTER_H
#define LLVM_LIB_TARGET_M68K_M68KASMPRINTER_H

#include "M68kMCInstLower.h"
#include "M68kTargetMachine.h"
#include "MCTargetDesc/M68kMemOperandPrinter.h"

#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Target/TargetMachine.h"
#include <memory>
#include <utility>

namespace vm::core {
class MCStreamer;
class MachineInstr;
class MachineBasicBlock;
class Module;
class raw_ostream;

class M68kSubtarget;
class M68kMachineFunctionInfo;

class LLVM_LIBRARY_VISIBILITY M68kAsmPrinter
    : public AsmPrinter,
      public M68kMemOperandPrinter<M68kAsmPrinter, MachineInstr> {

  friend class M68kMemOperandPrinter;

  void EmitInstrWithMacroNoAT(const MachineInstr *MI);

  void printOperand(const MachineInstr *MI, int OpNum, raw_ostream &OS);

  void printDisp(const MachineInstr *MI, unsigned OpNum, raw_ostream &OS);
  void printAbsMem(const MachineInstr *MI, unsigned OpNum, raw_ostream &OS);

public:
  static char ID;

  const M68kSubtarget *Subtarget;
  const M68kMachineFunctionInfo *MMFI;
  std::unique_ptr<M68kMCInstLower> MCInstLowering;

  explicit M68kAsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID) {
    Subtarget = static_cast<M68kTargetMachine &>(TM).getSubtargetImpl();
  }

  StringRef getPassName() const override { return "M68k Assembly Printer"; }

  virtual bool runOnMachineFunction(MachineFunction &MF) override;

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &OS) override;

  void emitInstruction(const MachineInstr *MI) override;
  void emitFunctionBodyStart() override;
  void emitFunctionBodyEnd() override;
  void emitStartOfAsmFile(Module &M) override;
  void emitEndOfAsmFile(Module &M) override;
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_M68K_M68KASMPRINTER_H
