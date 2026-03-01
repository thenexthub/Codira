//===-- AVRTargetMachine.cpp - Define TargetMachine for AVR ---------------===//
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
// This file defines the AVR specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#include "AVRTargetMachine.h"

#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

#include "AVR.h"
#include "AVRMachineFunctionInfo.h"
#include "AVRTargetObjectFile.h"
#include "AVRTargetTransformInfo.h"
#include "MCTargetDesc/AVRMCTargetDesc.h"
#include "TargetInfo/AVRTargetInfo.h"

#include <optional>

namespace vm::core {

/// Processes a CPU name.
static StringRef getCPU(StringRef CPU) {
  if (CPU.empty() || CPU == "generic") {
    return "avr2";
  }

  return CPU;
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

AVRTargetMachine::AVRTargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   std::optional<Reloc::Model> RM,
                                   std::optional<CodeModel::Model> CM,
                                   CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, getCPU(CPU), FS,
                               Options, getEffectiveRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      SubTarget(TT, std::string(getCPU(CPU)), std::string(FS), *this) {
  this->TLOF = std::make_unique<AVRTargetObjectFile>();
  initAsmInfo();
}

namespace {
/// AVR Code Generator Pass Configuration Options.
class AVRPassConfig : public TargetPassConfig {
public:
  AVRPassConfig(AVRTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {
    EnableLoopTermFold = true;
  }

  AVRTargetMachine &getAVRTargetMachine() const {
    return getTM<AVRTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreSched2() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *AVRTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new AVRPassConfig(*this, PM);
}

void AVRPassConfig::addIRPasses() {
  // Expand instructions like
  //   %result = shl i32 %n, %amount
  // to a loop so that library calls are avoided.
  addPass(createAVRShiftExpandPass());

  TargetPassConfig::addIRPasses();
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeAVRTarget() {
  // Register the target.
  RegisterTargetMachine<AVRTargetMachine> X(getTheAVRTarget());

  auto &PR = *PassRegistry::getPassRegistry();
  initializeAVRAsmPrinterPass(PR);
  initializeAVRExpandPseudoPass(PR);
  initializeAVRShiftExpandPass(PR);
  initializeAVRDAGToDAGISelLegacyPass(PR);
}

const AVRSubtarget *AVRTargetMachine::getSubtargetImpl() const {
  return &SubTarget;
}

const AVRSubtarget *AVRTargetMachine::getSubtargetImpl(const Function &) const {
  return &SubTarget;
}

TargetTransformInfo
AVRTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<AVRTTIImpl>(this, F));
}

MachineFunctionInfo *AVRTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return AVRMachineFunctionInfo::create<AVRMachineFunctionInfo>(Allocator, F,
                                                                STI);
}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//

bool AVRPassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createAVRISelDag(getAVRTargetMachine(), getOptLevel()));
  // Create the frame analyzer pass used by the PEI pass.
  addPass(createAVRFrameAnalyzerPass());

  return false;
}

void AVRPassConfig::addPreSched2() { addPass(createAVRExpandPseudoPass()); }

void AVRPassConfig::addPreEmitPass() {
  // Must run branch selection immediately preceding the asm printer.
  addPass(&BranchRelaxationPassID);
}

} // end of namespace vm::core
