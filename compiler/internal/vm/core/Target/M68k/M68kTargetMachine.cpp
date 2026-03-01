//===-- M68kTargetMachine.cpp - M68k Target Machine -------------*- C++ -*-===//
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
/// This file contains implementation for M68k target machine.
///
//===----------------------------------------------------------------------===//

#include "M68kTargetMachine.h"
#include "M68k.h"
#include "M68kMachineFunction.h"
#include "M68kSubtarget.h"
#include "M68kTargetObjectFile.h"
#include "TargetInfo/M68kTargetInfo.h"
#include "vm/core/CodeGen/GlobalISel/IRTranslator.h"
#include "vm/core/CodeGen/GlobalISel/InstructionSelect.h"
#include "vm/core/CodeGen/GlobalISel/Legalizer.h"
#include "vm/core/CodeGen/GlobalISel/RegBankSelect.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/PassRegistry.h"
#include <memory>
#include <optional>

using namespace vm::core;

#define DEBUG_TYPE "m68k"

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeM68kTarget() {
  RegisterTargetMachine<M68kTargetMachine> X(getTheM68kTarget());
  auto *PR = PassRegistry::getPassRegistry();
  initializeGlobalISel(*PR);
  initializeM68kAsmPrinterPass(*PR);
  initializeM68kDAGToDAGISelLegacyPass(*PR);
  initializeM68kExpandPseudoPass(*PR);
  initializeM68kGlobalBaseRegPass(*PR);
  initializeM68kCollapseMOVEMPass(*PR);
}

namespace {

Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  // If not defined we default to static
  return RM.value_or(Reloc::Static);
}

CodeModel::Model getEffectiveCodeModel(std::optional<CodeModel::Model> CM,
                                       bool JIT) {
  if (!CM) {
    return CodeModel::Small;
  } else if (CM == CodeModel::Kernel) {
    llvm_unreachable("Kernel code model is not implemented yet");
  }
  return CM.value();
}
} // end anonymous namespace

M68kTargetMachine::M68kTargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               getEffectiveRelocModel(RM),
                               ::getEffectiveCodeModel(CM, JIT), OL),
      TLOF(std::make_unique<M68kELFTargetObjectFile>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

M68kTargetMachine::~M68kTargetMachine() {}

const M68kSubtarget *
M68kTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  auto CPU = CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  auto FS = FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<M68kSubtarget>(TargetTriple, CPU, FS, *this);
  }
  return I.get();
}

MachineFunctionInfo *M68kTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return M68kMachineFunctionInfo::create<M68kMachineFunctionInfo>(Allocator, F,
                                                                  STI);
}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//

namespace {
class M68kPassConfig : public TargetPassConfig {
public:
  M68kPassConfig(M68kTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  M68kTargetMachine &getM68kTargetMachine() const {
    return getTM<M68kTargetMachine>();
  }

  const M68kSubtarget &getM68kSubtarget() const {
    return *getM68kTargetMachine().getSubtargetImpl();
  }
  void addIRPasses() override;
  bool addIRTranslator() override;
  bool addLegalizeMachineIR() override;
  bool addRegBankSelect() override;
  bool addGlobalInstructionSelect() override;
  bool addInstSelector() override;
  void addPreSched2() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *M68kTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new M68kPassConfig(*this, PM);
}

void M68kPassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());
  TargetPassConfig::addIRPasses();
}

bool M68kPassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createM68kISelDag(getM68kTargetMachine()));
  addPass(createM68kGlobalBaseRegPass());
  return false;
}

bool M68kPassConfig::addIRTranslator() {
  addPass(new IRTranslator());
  return false;
}

bool M68kPassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

bool M68kPassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}

bool M68kPassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect());
  return false;
}

void M68kPassConfig::addPreSched2() { addPass(createM68kExpandPseudoPass()); }

void M68kPassConfig::addPreEmitPass() {
  addPass(createM68kCollapseMOVEMPass());
}
