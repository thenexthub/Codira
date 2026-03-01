// WebAssemblyTargetMachine.h - Define TargetMachine for WebAssembly -*- C++ -*-
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
/// This file declares the WebAssembly-specific subclass of
/// TargetMachine.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYTARGETMACHINE_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYTARGETMACHINE_H

#include "WebAssemblySubtarget.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include <optional>

namespace vm::core {

namespace WebAssembly {
// Exception handling / setjmp-longjmp handling command-line options
extern cl::opt<bool> WasmEnableEmEH;   // asm.js-style EH
extern cl::opt<bool> WasmEnableEmSjLj; // asm.js-style SjLJ
extern cl::opt<bool> WasmEnableEH;     // EH using Wasm EH instructions
extern cl::opt<bool> WasmEnableSjLj;   // SjLj using Wasm EH instructions
extern cl::opt<bool> WasmUseLegacyEH;  // Legacy Wasm EH
} // namespace WebAssembly

class WebAssemblyTargetMachine final : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  mutable StringMap<std::unique_ptr<WebAssemblySubtarget>> SubtargetMap;
  bool UsesMultivalueABI = false;

public:
  WebAssemblyTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                           StringRef FS, const TargetOptions &Options,
                           std::optional<Reloc::Model> RM,
                           std::optional<CodeModel::Model> CM,
                           CodeGenOptLevel OL, bool JIT);

  ~WebAssemblyTargetMachine() override;

  const WebAssemblySubtarget *getSubtargetImpl() const;
  const WebAssemblySubtarget *getSubtargetImpl(std::string CPU,
                                               std::string FS) const;
  const WebAssemblySubtarget *
  getSubtargetImpl(const Function &F) const override;

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  bool usesPhysRegsForValues() const override { return false; }

  yaml::MachineFunctionInfo *createDefaultFuncInfoYAML() const override;
  yaml::MachineFunctionInfo *
  convertFuncInfoToYAML(const MachineFunction &MF) const override;
  bool parseMachineFunctionInfo(const yaml::MachineFunctionInfo &,
                                PerFunctionMIParsingState &PFS,
                                SMDiagnostic &Error,
                                SMRange &SourceRange) const override;

  bool usesMultivalueABI() const { return UsesMultivalueABI; }
};

} // end namespace vm::core

#endif
