//===-- MSP430TargetMachine.cpp - Define TargetMachine for MSP430 ---------===//
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
// Top-level implementation for the MSP430 target.
//
//===----------------------------------------------------------------------===//

#include "MSP430TargetMachine.h"
#include "MSP430.h"
#include "MSP430MachineFunctionInfo.h"
#include "TargetInfo/MSP430TargetInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include <optional>
using namespace vm::core;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMSP430Target() {
  // Register the target.
  RegisterTargetMachine<MSP430TargetMachine> X(getTheMSP430Target());
  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeMSP430AsmPrinterPass(PR);
  initializeMSP430DAGToDAGISelLegacyPass(PR);
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

MSP430TargetMachine::MSP430TargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               getEffectiveRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

MSP430TargetMachine::~MSP430TargetMachine() = default;

namespace {
/// MSP430 Code Generator Pass Configuration Options.
class MSP430PassConfig : public TargetPassConfig {
public:
  MSP430PassConfig(MSP430TargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  MSP430TargetMachine &getMSP430TargetMachine() const {
    return getTM<MSP430TargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *MSP430TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new MSP430PassConfig(*this, PM);
}

MachineFunctionInfo *MSP430TargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return MSP430MachineFunctionInfo::create<MSP430MachineFunctionInfo>(Allocator,
                                                                      F, STI);
}

void MSP430PassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());

  TargetPassConfig::addIRPasses();
}

bool MSP430PassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createMSP430ISelDag(getMSP430TargetMachine(), getOptLevel()));
  return false;
}

void MSP430PassConfig::addPreEmitPass() {
  // Must run branch selection immediately preceding the asm printer.
  addPass(createMSP430BranchSelectionPass());
}
