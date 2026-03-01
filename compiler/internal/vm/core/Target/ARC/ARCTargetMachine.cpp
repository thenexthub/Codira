//===- ARCTargetMachine.cpp - Define TargetMachine for ARC ------*- C++ -*-===//
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
//
//===----------------------------------------------------------------------===//

#include "ARCTargetMachine.h"
#include "ARC.h"
#include "ARCMachineFunctionInfo.h"
#include "ARCTargetTransformInfo.h"
#include "TargetInfo/ARCTargetInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/MC/TargetRegistry.h"
#include <optional>

using namespace vm::core;

static Reloc::Model getRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

/// ARCTargetMachine ctor - Create an ILP32 architecture model
ARCTargetMachine::ARCTargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   std::optional<Reloc::Model> RM,
                                   std::optional<CodeModel::Model> CM,
                                   CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               getRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

ARCTargetMachine::~ARCTargetMachine() = default;

namespace {

/// ARC Code Generator Pass Configuration Options.
class ARCPassConfig : public TargetPassConfig {
public:
  ARCPassConfig(ARCTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  ARCTargetMachine &getARCTargetMachine() const {
    return getTM<ARCTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
  void addPreRegAlloc() override;
};

} // end anonymous namespace

TargetPassConfig *ARCTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new ARCPassConfig(*this, PM);
}

void ARCPassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());

  TargetPassConfig::addIRPasses();
}

bool ARCPassConfig::addInstSelector() {
  addPass(createARCISelDag(getARCTargetMachine(), getOptLevel()));
  return false;
}

void ARCPassConfig::addPreEmitPass() { addPass(createARCBranchFinalizePass()); }

void ARCPassConfig::addPreRegAlloc() {
    addPass(createARCExpandPseudosPass());
    addPass(createARCOptAddrMode());
}

MachineFunctionInfo *ARCTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
    return ARCFunctionInfo::create<ARCFunctionInfo>(Allocator, F, STI);
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeARCTarget() {
  RegisterTargetMachine<ARCTargetMachine> X(getTheARCTarget());
  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeARCAsmPrinterPass(PR);
  initializeARCDAGToDAGISelLegacyPass(PR);
}

TargetTransformInfo
ARCTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<ARCTTIImpl>(this, F));
}
