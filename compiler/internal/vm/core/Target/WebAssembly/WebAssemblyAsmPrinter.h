// WebAssemblyAsmPrinter.h - WebAssembly implementation of AsmPrinter-*- C++ -*-
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

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYASMPRINTER_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYASMPRINTER_H

#include "WebAssemblyMachineFunctionInfo.h"
#include "WebAssemblySubtarget.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {
class WebAssemblyTargetStreamer;

class LLVM_LIBRARY_VISIBILITY WebAssemblyAsmPrinter final : public AsmPrinter {
public:
  static char ID;

private:
  const WebAssemblySubtarget *Subtarget;
  const MachineRegisterInfo *MRI;
  WebAssemblyFunctionInfo *MFI;
  bool signaturesEmitted = false;

public:
  explicit WebAssemblyAsmPrinter(TargetMachine &TM,
                                 std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID), Subtarget(nullptr),
        MRI(nullptr), MFI(nullptr) {}

  StringRef getPassName() const override {
    return "WebAssembly Assembly Printer";
  }

  const WebAssemblySubtarget &getSubtarget() const { return *Subtarget; }

  //===------------------------------------------------------------------===//
  // MachineFunctionPass Implementation.
  //===------------------------------------------------------------------===//

  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<WebAssemblySubtarget>();
    MRI = &MF.getRegInfo();
    MFI = MF.getInfo<WebAssemblyFunctionInfo>();
    return AsmPrinter::runOnMachineFunction(MF);
  }

  //===------------------------------------------------------------------===//
  // AsmPrinter Implementation.
  //===------------------------------------------------------------------===//

  void emitEndOfAsmFile(Module &M) override;
  void EmitProducerInfo(Module &M);
  void EmitTargetFeatures(Module &M);
  void EmitFunctionAttributes(Module &M);
  void emitSymbolType(const MCSymbolWasm *Sym);
  void emitGlobalVariable(const GlobalVariable *GV) override;
  void emitJumpTableInfo() override;
  void emitConstantPool() override;
  void emitFunctionBodyStart() override;
  void emitInstruction(const MachineInstr *MI) override;
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &OS) override;

  MVT getRegType(unsigned RegNo) const;
  std::string regToString(const MachineOperand &MO);
  WebAssemblyTargetStreamer *getTargetStreamer();
  MCSymbolWasm *getMCSymbolForFunction(const Function *F,
                                       wasm::WasmSignature *Sig,
                                       bool &InvokeDetected);
  MCSymbol *getOrCreateWasmSymbol(StringRef Name);
  void emitDecls(const Module &M);
};

} // end namespace vm::core

#endif
