//===-- BPFTargetMachine.cpp - Define TargetMachine for BPF ---------------===//
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
// Implements the info about BPF target spec.
//
//===----------------------------------------------------------------------===//

#include "BPFTargetMachine.h"
#include "BPF.h"
#include "BPFTargetLoweringObjectFile.h"
#include "BPFTargetTransformInfo.h"
#include "MCTargetDesc/BPFMCAsmInfo.h"
#include "TargetInfo/BPFTargetInfo.h"
#include "vm/core/CodeGen/GlobalISel/IRTranslator.h"
#include "vm/core/CodeGen/GlobalISel/InstructionSelect.h"
#include "vm/core/CodeGen/GlobalISel/Legalizer.h"
#include "vm/core/CodeGen/GlobalISel/RegBankSelect.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Passes/PassBuilder.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Target/TargetOptions.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Scalar/SimplifyCFG.h"
#include "vm/core/Transforms/Utils/SimplifyCFGOptions.h"
#include <optional>
using namespace vm::core;

static cl::
opt<bool> DisableMIPeephole("disable-bpf-peephole", cl::Hidden,
                            cl::desc("Disable machine peepholes for BPF"));

static cl::opt<bool>
    DisableCheckUnreachable("bpf-disable-trap-unreachable", cl::Hidden,
                            cl::desc("Disable Trap Unreachable for BPF"));

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeBPFTarget() {
  // Register the target.
  RegisterTargetMachine<BPFTargetMachine> X(getTheBPFleTarget());
  RegisterTargetMachine<BPFTargetMachine> Y(getTheBPFbeTarget());
  RegisterTargetMachine<BPFTargetMachine> Z(getTheBPFTarget());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeGlobalISel(PR);
  initializeBPFAsmPrinterPass(PR);
  initializeBPFCheckAndAdjustIRPass(PR);
  initializeBPFMIPeepholePass(PR);
  initializeBPFMIPreEmitPeepholePass(PR);
  initializeBPFDAGToDAGISelLegacyPass(PR);
  initializeBPFMISimplifyPatchablePass(PR);
  initializeBPFMIPreEmitCheckingPass(PR);
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::PIC_);
}

BPFTargetMachine::BPFTargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   std::optional<Reloc::Model> RM,
                                   std::optional<CodeModel::Model> CM,
                                   CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               getEffectiveRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<BPFTargetLoweringObjectFileELF>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  if (!DisableCheckUnreachable) {
    this->Options.TrapUnreachable = true;
    this->Options.NoTrapAfterNoreturn = true;
  }

  initAsmInfo();

  BPFMCAsmInfo *MAI =
      static_cast<BPFMCAsmInfo *>(const_cast<MCAsmInfo *>(AsmInfo.get()));
  MAI->setDwarfUsesRelocationsAcrossSections(!Subtarget.getUseDwarfRIS());
}

namespace {
// BPF Code Generator Pass Configuration Options.
class BPFPassConfig : public TargetPassConfig {
public:
  BPFPassConfig(BPFTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  BPFTargetMachine &getBPFTargetMachine() const {
    return getTM<BPFTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addMachineSSAOptimization() override;
  void addPreEmitPass() override;

  bool addIRTranslator() override;
  bool addLegalizeMachineIR() override;
  bool addRegBankSelect() override;
  bool addGlobalInstructionSelect() override;
};
}

TargetPassConfig *BPFTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new BPFPassConfig(*this, PM);
}

static Expected<bool> parseBPFPreserveStaticOffsetOptions(StringRef Params) {
  return PassBuilder::parseSinglePassOption(Params, "allow-partial",
                                            "BPFPreserveStaticOffsetPass");
}

void BPFTargetMachine::registerPassBuilderCallbacks(PassBuilder &PB) {
#define GET_PASS_REGISTRY "BPFPassRegistry.def"
#include "vm/core/Passes/TargetPassRegistry.inc"

  PB.registerPipelineStartEPCallback(
      [=](ModulePassManager &MPM, OptimizationLevel) {
        FunctionPassManager FPM;
        FPM.addPass(BPFPreserveStaticOffsetPass(true));
        FPM.addPass(BPFAbstractMemberAccessPass(this));
        FPM.addPass(BPFPreserveDITypePass());
        FPM.addPass(BPFIRPeepholePass());
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
      });
  PB.registerPeepholeEPCallback([=](FunctionPassManager &FPM,
                                    OptimizationLevel Level) {
    FPM.addPass(SimplifyCFGPass(SimplifyCFGOptions().hoistCommonInsts(true)));
    FPM.addPass(BPFASpaceCastSimplifyPass());
  });
  PB.registerScalarOptimizerLateEPCallback(
      [=](FunctionPassManager &FPM, OptimizationLevel Level) {
        // Run this after loop unrolling but before
        // SimplifyCFGPass(... .sinkCommonInsts(true))
        FPM.addPass(BPFPreserveStaticOffsetPass(false));
      });
  PB.registerPipelineEarlySimplificationEPCallback(
      [=](ModulePassManager &MPM, OptimizationLevel, ThinOrFullLTOPhase) {
        MPM.addPass(BPFAdjustOptPass());
      });
}

void BPFPassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());
  addPass(createBPFCheckAndAdjustIR());

  TargetPassConfig::addIRPasses();
}

TargetTransformInfo
BPFTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<BPFTTIImpl>(this, F));
}

// Install an instruction selector pass using
// the ISelDag to gen BPF code.
bool BPFPassConfig::addInstSelector() {
  addPass(createBPFISelDag(getBPFTargetMachine()));

  return false;
}

void BPFPassConfig::addMachineSSAOptimization() {
  addPass(createBPFMISimplifyPatchablePass());

  // The default implementation must be called first as we want eBPF
  // Peephole ran at last.
  TargetPassConfig::addMachineSSAOptimization();

  const BPFSubtarget *Subtarget = getBPFTargetMachine().getSubtargetImpl();
  if (!DisableMIPeephole) {
    if (Subtarget->getHasAlu32())
      addPass(createBPFMIPeepholePass());
  }
}

void BPFPassConfig::addPreEmitPass() {
  addPass(createBPFMIPreEmitCheckingPass());
  if (getOptLevel() != CodeGenOptLevel::None)
    if (!DisableMIPeephole)
      addPass(createBPFMIPreEmitPeepholePass());
}

bool BPFPassConfig::addIRTranslator() {
  addPass(new IRTranslator());
  return false;
}

bool BPFPassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

bool BPFPassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}

bool BPFPassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect(getOptLevel()));
  return false;
}
