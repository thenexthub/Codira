//===- SPIRVTargetMachine.cpp - Define TargetMachine for SPIR-V -*- C++ -*-===//
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
// Implements the info about SPIR-V target spec.
//
//===----------------------------------------------------------------------===//

#include "SPIRVTargetMachine.h"
#include "SPIRV.h"
#include "SPIRVCBufferAccess.h"
#include "SPIRVGlobalRegistry.h"
#include "SPIRVLegalizeZeroSizeArrays.h"
#include "SPIRVLegalizerInfo.h"
#include "SPIRVPushConstantAccess.h"
#include "SPIRVStructurizerWrapper.h"
#include "SPIRVTargetObjectFile.h"
#include "SPIRVTargetTransformInfo.h"
#include "TargetInfo/SPIRVTargetInfo.h"
#include "vm/core/CodeGen/GlobalISel/IRTranslator.h"
#include "vm/core/CodeGen/GlobalISel/InstructionSelect.h"
#include "vm/core/CodeGen/GlobalISel/Legalizer.h"
#include "vm/core/CodeGen/GlobalISel/RegBankSelect.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Pass.h"
#include "vm/core/Passes/PassBuilder.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Target/TargetOptions.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Utils.h"
#include <optional>

using namespace vm::core;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSPIRVTarget() {
  // Register the target.
  RegisterTargetMachine<SPIRVTargetMachine> X(getTheSPIRV32Target());
  RegisterTargetMachine<SPIRVTargetMachine> Y(getTheSPIRV64Target());
  RegisterTargetMachine<SPIRVTargetMachine> Z(getTheSPIRVLogicalTarget());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeGlobalISel(PR);
  initializeSPIRVModuleAnalysisPass(PR);
  initializeSPIRVAsmPrinterPass(PR);
  initializeSPIRVConvergenceRegionAnalysisWrapperPassPass(PR);
  initializeSPIRVStructurizerPass(PR);
  initializeSPIRVCBufferAccessLegacyPass(PR);
  initializeSPIRVPushConstantAccessLegacyPass(PR);
  initializeSPIRVPreLegalizerCombinerPass(PR);
  initializeSPIRVLegalizePointerCastPass(PR);
  initializeSPIRVLegalizeZeroSizeArraysLegacyPass(PR);
  initializeSPIRVRegularizerPass(PR);
  initializeSPIRVPreLegalizerPass(PR);
  initializeSPIRVPostLegalizerPass(PR);
  initializeSPIRVMergeRegionExitTargetsPass(PR);
  initializeSPIRVEmitIntrinsicsPass(PR);
  initializeSPIRVEmitNonSemanticDIPass(PR);
  initializeSPIRVPrepareFunctionsPass(PR);
  initializeSPIRVPrepareGlobalsPass(PR);
  initializeSPIRVStripConvergentIntrinsicsPass(PR);
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  if (!RM)
    return Reloc::PIC_;
  return *RM;
}

// Pin SPIRVTargetObjectFile's vtables to this file.
SPIRVTargetObjectFile::~SPIRVTargetObjectFile() = default;

SPIRVTargetMachine::SPIRVTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       std::optional<Reloc::Model> RM,
                                       std::optional<CodeModel::Model> CM,
                                       CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               getEffectiveRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<SPIRVTargetObjectFile>()),
      Subtarget(TT, CPU.str(), FS.str(), *this) {
  initAsmInfo();
  setGlobalISel(true);
  setFastISel(false);
  setO0WantsFastISel(false);
  setRequiresStructuredCFG(false);
}

void SPIRVTargetMachine::registerPassBuilderCallbacks(PassBuilder &PB) {
#define GET_PASS_REGISTRY "SPIRVPassRegistry.def"
#include "vm/core/Passes/TargetPassRegistry.inc"
}

namespace {
// SPIR-V Code Generator Pass Configuration Options.
class SPIRVPassConfig : public TargetPassConfig {
public:
  SPIRVPassConfig(SPIRVTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM), TM(TM) {}

  SPIRVTargetMachine &getSPIRVTargetMachine() const {
    return getTM<SPIRVTargetMachine>();
  }
  void addMachineSSAOptimization() override;
  void addIRPasses() override;
  void addISelPrepare() override;

  bool addIRTranslator() override;
  void addPreLegalizeMachineIR() override;
  bool addLegalizeMachineIR() override;
  bool addRegBankSelect() override;
  bool addGlobalInstructionSelect() override;

  FunctionPass *createTargetRegisterAllocator(bool) override;
  void addFastRegAlloc() override {}
  void addOptimizedRegAlloc() override {}

  void addPostRegAlloc() override;
  void addPreEmitPass() override;

private:
  const SPIRVTargetMachine &TM;
};
} // namespace

// We do not use physical registers, and maintain virtual registers throughout
// the entire pipeline, so return nullptr to disable register allocation.
FunctionPass *SPIRVPassConfig::createTargetRegisterAllocator(bool) {
  return nullptr;
}

// A place to disable passes that may break CFG.
void SPIRVPassConfig::addMachineSSAOptimization() {
  TargetPassConfig::addMachineSSAOptimization();
}

// Disable passes that break from assuming no virtual registers exist.
void SPIRVPassConfig::addPostRegAlloc() {
  // Do not work with vregs instead of physical regs.
  disablePass(&MachineCopyPropagationID);
  disablePass(&PostRAMachineSinkingID);
  disablePass(&PostRASchedulerID);
  disablePass(&FuncletLayoutID);
  disablePass(&StackMapLivenessID);
  disablePass(&PatchableFunctionID);
  disablePass(&ShrinkWrapID);
  disablePass(&LiveDebugValuesID);
  disablePass(&MachineLateInstrsCleanupID);
  disablePass(&RemoveLoadsIntoFakeUsesID);

  // Do not work with OpPhi.
  disablePass(&BranchFolderPassID);
  disablePass(&MachineBlockPlacementID);

  TargetPassConfig::addPostRegAlloc();
}

TargetTransformInfo
SPIRVTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<SPIRVTTIImpl>(this, F));
}

TargetPassConfig *SPIRVTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SPIRVPassConfig(*this, PM);
}

void SPIRVPassConfig::addIRPasses() {
  TargetPassConfig::addIRPasses();

  addPass(createSPIRVRegularizerPass());
  addPass(createSPIRVPrepareFunctionsPass(TM));
  addPass(createSPIRVPrepareGlobalsPass());
}

void SPIRVPassConfig::addISelPrepare() {
  if (TM.getSubtargetImpl()->isShader()) {
    // Vulkan does not allow address space casts. This pass is run to remove
    // address space casts that can be removed.
    // If an address space cast is not removed while targeting Vulkan, lowering
    // will fail during MIR lowering.
    addPass(createInferAddressSpacesPass());

    // 1.  Simplify loop for subsequent transformations. After this steps, loops
    // have the following properties:
    //  - loops have a single entry edge (pre-header to loop header).
    //  - all loop exits are dominated by the loop pre-header.
    //  - loops have a single back-edge.
    addPass(createLoopSimplifyPass());

    // 2. Removes registers whose lifetime spans across basic blocks. Also
    // removes phi nodes. This will greatly simplify the next steps.
    addPass(createRegToMemWrapperPass());

    // 3. Merge the convergence region exit nodes into one. After this step,
    // regions are single-entry, single-exit. This will help determine the
    // correct merge block.
    addPass(createSPIRVMergeRegionExitTargetsPass());

    // 4. Structurize.
    addPass(createSPIRVStructurizerPass());

    // 5. Reduce the amount of variables required by pushing some operations
    // back to virtual registers.
    addPass(createPromoteMemoryToRegisterPass());
  }
  SPIRVTargetMachine &TM = getTM<SPIRVTargetMachine>();
  addPass(createSPIRVStripConvergenceIntrinsicsPass());
  addPass(createSPIRVLegalizeImplicitBindingPass());
  addPass(createSPIRVLegalizeZeroSizeArraysPass(TM));
  addPass(createSPIRVCBufferAccessLegacyPass());
  addPass(createSPIRVPushConstantAccessLegacyPass(&TM));
  addPass(createSPIRVEmitIntrinsicsPass(&TM));
  if (TM.getSubtargetImpl()->isLogicalSPIRV())
    addPass(createSPIRVLegalizePointerCastPass(&TM));
  TargetPassConfig::addISelPrepare();
}

bool SPIRVPassConfig::addIRTranslator() {
  addPass(new IRTranslator(getOptLevel()));
  return false;
}

void SPIRVPassConfig::addPreLegalizeMachineIR() {
  addPass(createSPIRVPreLegalizerCombiner());
  addPass(createSPIRVPreLegalizerPass());
}

// Use the default legalizer.
bool SPIRVPassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  addPass(createSPIRVPostLegalizerPass());
  return false;
}

// Do not add the RegBankSelect pass, as we only ever need virtual registers.
bool SPIRVPassConfig::addRegBankSelect() {
  disablePass(&RegBankSelect::ID);
  return false;
}

static cl::opt<bool> SPVEnableNonSemanticDI(
    "spv-emit-nonsemantic-debug-info",
    cl::desc("Emit SPIR-V NonSemantic.Shader.DebugInfo.100 instructions"),
    cl::Optional, cl::init(false));

void SPIRVPassConfig::addPreEmitPass() {
  if (SPVEnableNonSemanticDI ||
      getSPIRVTargetMachine().getTargetTriple().getVendor() == Triple::AMD) {
    addPass(createSPIRVEmitNonSemanticDIPass(&getTM<SPIRVTargetMachine>()));
  }
}

namespace {
// A custom subclass of InstructionSelect, which is mostly the same except from
// not requiring RegBankSelect to occur previously.
class SPIRVInstructionSelect : public InstructionSelect {
  // We don't use register banks, so unset the requirement for them
  MachineFunctionProperties getRequiredProperties() const override {
    return InstructionSelect::getRequiredProperties().resetRegBankSelected();
  }
};
} // namespace

// Add the custom SPIRVInstructionSelect from above.
bool SPIRVPassConfig::addGlobalInstructionSelect() {
  addPass(new SPIRVInstructionSelect());
  return false;
}
