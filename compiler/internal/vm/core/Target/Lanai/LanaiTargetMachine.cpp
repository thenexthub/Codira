//===-- LanaiTargetMachine.cpp - Define TargetMachine for Lanai ---------===//
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
// Implements the info about Lanai target spec.
//
//===----------------------------------------------------------------------===//

#include "LanaiTargetMachine.h"

#include "Lanai.h"
#include "LanaiMachineFunctionInfo.h"
#include "LanaiTargetObjectFile.h"
#include "LanaiTargetTransformInfo.h"
#include "TargetInfo/LanaiTargetInfo.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Target/TargetOptions.h"
#include <optional>

using namespace vm::core;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLanaiTarget() {
  // Register the target.
  RegisterTargetMachine<LanaiTargetMachine> registered_target(
      getTheLanaiTarget());
  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeLanaiAsmPrinterPass(PR);
  initializeLanaiDAGToDAGISelLegacyPass(PR);
  initializeLanaiMemAluCombinerPass(PR);
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::PIC_);
}

LanaiTargetMachine::LanaiTargetMachine(
    const Target &T, const Triple &TT, StringRef Cpu, StringRef FeatureString,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CodeModel, CodeGenOptLevel OptLevel,
    bool JIT)
    : CodeGenTargetMachineImpl(
          T, TT.computeDataLayout(), TT, Cpu, FeatureString, Options,
          getEffectiveRelocModel(RM),
          getEffectiveCodeModel(CodeModel, CodeModel::Medium), OptLevel),
      Subtarget(TT, Cpu, FeatureString, *this, Options, getCodeModel(),
                OptLevel),
      TLOF(new LanaiTargetObjectFile()) {
  initAsmInfo();
}

TargetTransformInfo
LanaiTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<LanaiTTIImpl>(this, F));
}

MachineFunctionInfo *LanaiTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return LanaiMachineFunctionInfo::create<LanaiMachineFunctionInfo>(Allocator,
                                                                    F, STI);
}

namespace {
// Lanai Code Generator Pass Configuration Options.
class LanaiPassConfig : public TargetPassConfig {
public:
  LanaiPassConfig(LanaiTargetMachine &TM, PassManagerBase *PassManager)
      : TargetPassConfig(TM, *PassManager) {}

  LanaiTargetMachine &getLanaiTargetMachine() const {
    return getTM<LanaiTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreSched2() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *
LanaiTargetMachine::createPassConfig(PassManagerBase &PassManager) {
  return new LanaiPassConfig(*this, &PassManager);
}

void LanaiPassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());

  TargetPassConfig::addIRPasses();
}

// Install an instruction selector pass.
bool LanaiPassConfig::addInstSelector() {
  addPass(createLanaiISelDag(getLanaiTargetMachine()));
  return false;
}

// Implemented by targets that want to run passes immediately before
// machine code is emitted.
void LanaiPassConfig::addPreEmitPass() {
  addPass(createLanaiDelaySlotFillerPass(getLanaiTargetMachine()));
}

// Run passes after prolog-epilog insertion and before the second instruction
// scheduling pass.
void LanaiPassConfig::addPreSched2() {
  addPass(createLanaiMemAluCombinerPass());
}
