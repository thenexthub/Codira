//===-- VETargetMachine.cpp - Define TargetMachine for VE -----------------===//
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

#include "VETargetMachine.h"
#include "TargetInfo/VETargetInfo.h"
#include "VE.h"
#include "VEMachineFunctionInfo.h"
#include "VETargetTransformInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/LegacyPassManager.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include <optional>

using namespace vm::core;

#define DEBUG_TYPE "ve"

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeVETarget() {
  // Register the target.
  RegisterTargetMachine<VETargetMachine> X(getTheVETarget());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeVEAsmPrinterPass(PR);
  initializeVEDAGToDAGISelLegacyPass(PR);
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

namespace {
class VEELFTargetObjectFile : public TargetLoweringObjectFileELF {
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override {
    TargetLoweringObjectFileELF::Initialize(Ctx, TM);
    InitializeELF(TM.Options.UseInitArray);
  }
};
} // namespace

static std::unique_ptr<TargetLoweringObjectFile> createTLOF() {
  return std::make_unique<VEELFTargetObjectFile>();
}

/// Create an Aurora VE architecture model
VETargetMachine::VETargetMachine(const Target &T, const Triple &TT,
                                 StringRef CPU, StringRef FS,
                                 const TargetOptions &Options,
                                 std::optional<Reloc::Model> RM,
                                 std::optional<CodeModel::Model> CM,
                                 CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               getEffectiveRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(createTLOF()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

VETargetMachine::~VETargetMachine() = default;

TargetTransformInfo
VETargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<VETTIImpl>(this, F));
}

MachineFunctionInfo *VETargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return VEMachineFunctionInfo::create<VEMachineFunctionInfo>(Allocator, F,
                                                              STI);
}

namespace {
/// VE Code Generator Pass Configuration Options.
class VEPassConfig : public TargetPassConfig {
public:
  VEPassConfig(VETargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  VETargetMachine &getVETargetMachine() const {
    return getTM<VETargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *VETargetMachine::createPassConfig(PassManagerBase &PM) {
  return new VEPassConfig(*this, PM);
}

void VEPassConfig::addIRPasses() {
  // VE requires atomic expand pass.
  addPass(createAtomicExpandLegacyPass());
  TargetPassConfig::addIRPasses();
}

bool VEPassConfig::addInstSelector() {
  addPass(createVEISelDag(getVETargetMachine()));
  return false;
}

void VEPassConfig::addPreEmitPass() {
  // LVLGen should be called after scheduling and register allocation
  addPass(createLVLGenPass());
}
