//===-- SparcTargetMachine.cpp - Define TargetMachine for Sparc -----------===//
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

#include "SparcTargetMachine.h"
#include "LeonPasses.h"
#include "Sparc.h"
#include "SparcMachineFunctionInfo.h"
#include "SparcTargetObjectFile.h"
#include "TargetInfo/SparcTargetInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include <optional>
using namespace vm::core;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSparcTarget() {
  // Register the target.
  RegisterTargetMachine<SparcV8TargetMachine> X(getTheSparcTarget());
  RegisterTargetMachine<SparcV9TargetMachine> Y(getTheSparcV9Target());
  RegisterTargetMachine<SparcelTargetMachine> Z(getTheSparcelTarget());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeSparcAsmPrinterPass(PR);
  initializeSparcDAGToDAGISelLegacyPass(PR);
  initializeErrataWorkaroundPass(PR);
}

static cl::opt<bool>
    BranchRelaxation("sparc-enable-branch-relax", cl::Hidden, cl::init(true),
                     cl::desc("Relax out of range conditional branches"));

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

// Code models. Some only make sense for 64-bit code.
//
// SunCC  Reloc   CodeModel  Constraints
// abs32  Static  Small      text+data+bss linked below 2^32 bytes
// abs44  Static  Medium     text+data+bss linked below 2^44 bytes
// abs64  Static  Large      text smaller than 2^31 bytes
// pic13  PIC_    Small      GOT < 2^13 bytes
// pic32  PIC_    Medium     GOT < 2^32 bytes
//
// All code models require that the text segment is smaller than 2GB.
static CodeModel::Model
getEffectiveSparcCodeModel(std::optional<CodeModel::Model> CM, Reloc::Model RM,
                           bool Is64Bit, bool JIT) {
  if (CM) {
    if (*CM == CodeModel::Tiny)
      report_fatal_error("Target does not support the tiny CodeModel", false);
    if (*CM == CodeModel::Kernel)
      report_fatal_error("Target does not support the kernel CodeModel", false);
    return *CM;
  }
  if (Is64Bit) {
    if (JIT)
      return CodeModel::Large;
    return RM == Reloc::PIC_ ? CodeModel::Small : CodeModel::Medium;
  }
  return CodeModel::Small;
}

/// Create an ILP32 architecture model
SparcTargetMachine::SparcTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       std::optional<Reloc::Model> RM,
                                       std::optional<CodeModel::Model> CM,
                                       CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(
          T, TT.computeDataLayout(), TT, CPU, FS, Options,
          getEffectiveRelocModel(RM),
          getEffectiveSparcCodeModel(CM, getEffectiveRelocModel(RM),
                                     TT.isSPARC64(), JIT),
          OL),
      TLOF(std::make_unique<SparcELFTargetObjectFile>()) {
  initAsmInfo();
}

SparcTargetMachine::~SparcTargetMachine() = default;

const SparcSubtarget *
SparcTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute TuneAttr = F.getFnAttribute("tune-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  std::string TuneCPU =
      TuneAttr.isValid() ? TuneAttr.getValueAsString().str() : CPU;
  std::string FS =
      FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;

  // FIXME: This is related to the code below to reset the target options,
  // we need to know whether or not the soft float flag is set on the
  // function, so we can enable it as a subtarget feature.
  bool softFloat = F.getFnAttribute("use-soft-float").getValueAsBool();

  if (softFloat)
    FS += FS.empty() ? "+soft-float" : ",+soft-float";

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<SparcSubtarget>(CPU, TuneCPU, FS, *this);
  }
  return I.get();
}

MachineFunctionInfo *SparcTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return SparcMachineFunctionInfo::create<SparcMachineFunctionInfo>(Allocator,
                                                                    F, STI);
}

namespace {
/// Sparc Code Generator Pass Configuration Options.
class SparcPassConfig : public TargetPassConfig {
public:
  SparcPassConfig(SparcTargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  SparcTargetMachine &getSparcTargetMachine() const {
    return getTM<SparcTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *SparcTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SparcPassConfig(*this, PM);
}

void SparcPassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());

  TargetPassConfig::addIRPasses();
}

bool SparcPassConfig::addInstSelector() {
  addPass(createSparcISelDag(getSparcTargetMachine()));
  return false;
}

void SparcPassConfig::addPreEmitPass(){
  if (BranchRelaxation)
    addPass(&BranchRelaxationPassID);

  addPass(createSparcDelaySlotFillerPass());
  addPass(new InsertNOPLoad());
  addPass(new DetectRoundChange());
  addPass(new FixAllFDIVSQRT());
  addPass(new ErrataWorkaround());
}

void SparcV8TargetMachine::anchor() { }

SparcV8TargetMachine::SparcV8TargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           std::optional<Reloc::Model> RM,
                                           std::optional<CodeModel::Model> CM,
                                           CodeGenOptLevel OL, bool JIT)
    : SparcTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT) {}

void SparcV9TargetMachine::anchor() { }

SparcV9TargetMachine::SparcV9TargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           std::optional<Reloc::Model> RM,
                                           std::optional<CodeModel::Model> CM,
                                           CodeGenOptLevel OL, bool JIT)
    : SparcTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT) {}

void SparcelTargetMachine::anchor() {}

SparcelTargetMachine::SparcelTargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           std::optional<Reloc::Model> RM,
                                           std::optional<CodeModel::Model> CM,
                                           CodeGenOptLevel OL, bool JIT)
    : SparcTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT) {}
