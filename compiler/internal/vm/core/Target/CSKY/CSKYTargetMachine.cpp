//===--- CSKYTargetMachine.cpp - Define TargetMachine for CSKY ------------===//
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
// Implements the info about CSKY target spec.
//
//===----------------------------------------------------------------------===//

#include "CSKYTargetMachine.h"
#include "CSKY.h"
#include "CSKYMachineFunctionInfo.h"
#include "CSKYSubtarget.h"
#include "CSKYTargetObjectFile.h"
#include "TargetInfo/CSKYTargetInfo.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include <optional>

using namespace vm::core;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCSKYTarget() {
  RegisterTargetMachine<CSKYTargetMachine> X(getTheCSKYTarget());

  PassRegistry *Registry = PassRegistry::getPassRegistry();
  initializeCSKYConstantIslandsPass(*Registry);
  initializeCSKYDAGToDAGISelLegacyPass(*Registry);
}

CSKYTargetMachine::CSKYTargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               RM.value_or(Reloc::Static),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<CSKYELFTargetObjectFile>()) {
  initAsmInfo();
}

const CSKYSubtarget *
CSKYTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute TuneAttr = F.getFnAttribute("tune-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  std::string TuneCPU =
      TuneAttr.isValid() ? TuneAttr.getValueAsString().str() : CPU;
  std::string FS =
      FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;

  std::string Key = CPU + TuneCPU + FS;
  auto &I = SubtargetMap[Key];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<CSKYSubtarget>(TargetTriple, CPU, TuneCPU, FS, *this);
    if (I->useHardFloat() && !I->hasAnyFloatExt())
      errs() << "Hard-float can't be used with current CPU,"
                " set to Soft-float\n";
  }
  return I.get();
}

MachineFunctionInfo *CSKYTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return CSKYMachineFunctionInfo::create<CSKYMachineFunctionInfo>(Allocator, F,
                                                                  STI);
}

namespace {
class CSKYPassConfig : public TargetPassConfig {
public:
  CSKYPassConfig(CSKYTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  CSKYTargetMachine &getCSKYTargetMachine() const {
    return getTM<CSKYTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
};

} // namespace

TargetPassConfig *CSKYTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new CSKYPassConfig(*this, PM);
}

void CSKYPassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());
  TargetPassConfig::addIRPasses();
}

bool CSKYPassConfig::addInstSelector() {
  addPass(createCSKYISelDag(getCSKYTargetMachine(), getOptLevel()));

  return false;
}

void CSKYPassConfig::addPreEmitPass() {
  addPass(createCSKYConstantIslandPass());
}
