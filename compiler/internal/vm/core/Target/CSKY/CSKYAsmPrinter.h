//===-- CSKYAsmPrinter.h - CSKY implementation of AsmPrinter ----*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_CSKY_CSKYASMPRINTER_H
#define LLVM_LIB_TARGET_CSKY_CSKYASMPRINTER_H

#include "CSKYMCInstLower.h"
#include "CSKYSubtarget.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/MC/MCDirectives.h"

namespace vm::core {
class LLVM_LIBRARY_VISIBILITY CSKYAsmPrinter : public AsmPrinter {
  CSKYMCInstLower MCInstLowering;

  const MCSubtargetInfo *Subtarget;
  const TargetInstrInfo *TII;

  bool InConstantPool = false;

  /// Keep a pointer to constantpool entries of the current
  /// MachineFunction.
  MachineConstantPool *MCP;

  void expandTLSLA(const MachineInstr *MI);
  void emitCustomConstantPool(const MachineInstr *MI);
  void emitAttributes();

public:
  explicit CSKYAsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer);

  StringRef getPassName() const override { return "CSKY Assembly Printer"; }

  void EmitToStreamer(MCStreamer &S, const MCInst &Inst);

  /// tblgen'erated driver function for lowering simple MI->MC
  /// pseudo instructions.
  bool lowerPseudoInstExpansion(const MachineInstr *MI, MCInst &Inst);

  void emitMachineConstantPoolValue(MachineConstantPoolValue *MCPV) override;

  void emitFunctionBodyEnd() override;

  void emitStartOfAsmFile(Module &M) override;

  void emitEndOfAsmFile(Module &M) override;

  void emitInstruction(const MachineInstr *MI) override;

  bool runOnMachineFunction(MachineFunction &MF) override;

  // we emit constant pools customly!
  void emitConstantPool() override {}

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;

  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &OS) override;
};
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_CSKY_CSKYASMPRINTER_H
