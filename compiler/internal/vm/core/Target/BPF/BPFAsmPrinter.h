//===-- BPFFrameLowering.h - Define frame lowering for BPF -----*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_BPF_BPFASMPRINTER_H
#define LLVM_LIB_TARGET_BPF_BPFASMPRINTER_H

#include "BPFTargetMachine.h"
#include "BTFDebug.h"
#include "vm/core/CodeGen/AsmPrinter.h"

namespace vm::core {

class BPFAsmPrinter : public AsmPrinter {
public:
  explicit BPFAsmPrinter(TargetMachine &TM,
                         std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID), BTF(nullptr), TM(TM) {}

  StringRef getPassName() const override { return "BPF Assembly Printer"; }
  bool doInitialization(Module &M) override;
  bool doFinalization(Module &M) override;
  void printOperand(const MachineInstr *MI, int OpNum, raw_ostream &O);
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &O) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNum,
                             const char *ExtraCode, raw_ostream &O) override;

  void emitInstruction(const MachineInstr *MI) override;
  MCSymbol *getJTPublicSymbol(unsigned JTI);
  void emitJumpTableInfo() override;

  static char ID;

private:
  BTFDebug *BTF;
  TargetMachine &TM;
  bool SawTrapCall = false;

  const BPFTargetMachine &getBTM() const;
};

} // namespace vm::core

#endif /* LLVM_LIB_TARGET_BPF_BPFASMPRINTER_H */
