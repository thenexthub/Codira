//===- DirectXTargetMachine.cpp - DirectX Target Implementation -*- C++ -*-===//
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
/// This file contains DirectX target initializer.
///
//===----------------------------------------------------------------------===//

#include "DirectXTargetMachine.h"
#include "DXILCBufferAccess.h"
#include "DXILDataScalarization.h"
#include "DXILFinalizeLinkage.h"
#include "DXILFlattenArrays.h"
#include "DXILForwardHandleAccesses.h"
#include "DXILIntrinsicExpansion.h"
#include "DXILLegalizePass.h"
#include "DXILMemIntrinsics.h"
#include "DXILOpLowering.h"
#include "DXILPostOptimizationValidation.h"
#include "DXILPrettyPrinter.h"
#include "DXILResourceAccess.h"
#include "DXILResourceImplicitBinding.h"
#include "DXILRootSignature.h"
#include "DXILShaderFlags.h"
#include "DXILTranslateMetadata.h"
#include "DXILWriter/DXILWriterPass.h"
#include "DirectX.h"
#include "DirectXSubtarget.h"
#include "DirectXTargetTransformInfo.h"
#include "TargetInfo/DirectXTargetInfo.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/IRPrintingPasses.h"
#include "vm/core/IR/LegacyPassManager.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/MC/MCSectionDXContainer.h"
#include "vm/core/MC/SectionKind.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Passes/PassBuilder.h"
#include "vm/core/Support/CodeGen.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"
#include "vm/core/Transforms/IPO/GlobalDCE.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Scalar/Scalarizer.h"
#include <optional>

using namespace vm::core;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeDirectXTarget() {
  RegisterTargetMachine<DirectXTargetMachine> X(getTheDirectXTarget());
  auto *PR = PassRegistry::getPassRegistry();
  initializeDXILIntrinsicExpansionLegacyPass(*PR);
  initializeDXILMemIntrinsicsLegacyPass(*PR);
  initializeDXILDataScalarizationLegacyPass(*PR);
  initializeDXILFlattenArraysLegacyPass(*PR);
  initializeScalarizerLegacyPassPass(*PR);
  initializeDXILLegalizeLegacyPass(*PR);
  initializeDXILPrepareModulePass(*PR);
  initializeEmbedDXILPassPass(*PR);
  initializeWriteDXILPassPass(*PR);
  initializeDXContainerGlobalsPass(*PR);
  initializeGlobalDCELegacyPassPass(*PR);
  initializeDXILOpLoweringLegacyPass(*PR);
  initializeDXILResourceAccessLegacyPass(*PR);
  initializeDXILResourceImplicitBindingLegacyPass(*PR);
  initializeDXILTranslateMetadataLegacyPass(*PR);
  initializeDXILPostOptimizationValidationLegacyPass(*PR);
  initializeShaderFlagsAnalysisWrapperPass(*PR);
  initializeRootSignatureAnalysisWrapperPass(*PR);
  initializeDXILFinalizeLinkageLegacyPass(*PR);
  initializeDXILPrettyPrinterLegacyPass(*PR);
  initializeDXILForwardHandleAccessesLegacyPass(*PR);
  initializeDSELegacyPassPass(*PR);
  initializeDXILCBufferAccessLegacyPass(*PR);
}

class DXILTargetObjectFile : public TargetLoweringObjectFile {
public:
  DXILTargetObjectFile() = default;

  MCSection *getExplicitSectionGlobal(const GlobalObject *GO, SectionKind Kind,
                                      const TargetMachine &TM) const override {
    return getContext().getDXContainerSection(GO->getSection(), Kind);
  }

protected:
  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &TM) const override {
    llvm_unreachable("Not supported!");
  }
};

class DirectXPassConfig : public TargetPassConfig {
public:
  DirectXPassConfig(DirectXTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  DirectXTargetMachine &getDirectXTargetMachine() const {
    return getTM<DirectXTargetMachine>();
  }

  FunctionPass *createTargetRegisterAllocator(bool) override { return nullptr; }
  void addCodeGenPrepare() override {
    addPass(createDXILFinalizeLinkageLegacyPass());
    addPass(createGlobalDCEPass());
    addPass(createDXILMemIntrinsicsLegacyPass());
    addPass(createDXILCBufferAccessLegacyPass());
    addPass(createDXILResourceAccessLegacyPass());
    addPass(createDXILIntrinsicExpansionLegacyPass());
    addPass(createDXILDataScalarizationLegacyPass());
    ScalarizerPassOptions DxilScalarOptions;
    DxilScalarOptions.ScalarizeLoadStore = true;
    addPass(createScalarizerPass(DxilScalarOptions));
    addPass(createDXILFlattenArraysLegacyPass());
    addPass(createDXILForwardHandleAccessesLegacyPass());
    addPass(createDeadStoreEliminationPass());
    addPass(createDXILLegalizeLegacyPass());
    addPass(createDXILResourceImplicitBindingLegacyPass());
    addPass(createDXILTranslateMetadataLegacyPass());
    addPass(createDXILPostOptimizationValidationLegacyPass());
    addPass(createDXILOpLoweringLegacyPass());
    addPass(createDXILPrepareModulePass());
  }
};

DirectXTargetMachine::DirectXTargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           std::optional<Reloc::Model> RM,
                                           std::optional<CodeModel::Model> CM,
                                           CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               Reloc::Static, CodeModel::Small, OL),
      TLOF(std::make_unique<DXILTargetObjectFile>()),
      Subtarget(std::make_unique<DirectXSubtarget>(TT, CPU, FS, *this)) {
  initAsmInfo();
}

DirectXTargetMachine::~DirectXTargetMachine() {}

void DirectXTargetMachine::registerPassBuilderCallbacks(PassBuilder &PB) {
#define GET_PASS_REGISTRY "DirectXPassRegistry.def"
#include "vm/core/Passes/TargetPassRegistry.inc"
}

bool DirectXTargetMachine::addPassesToEmitFile(
    PassManagerBase &PM, raw_pwrite_stream &Out, raw_pwrite_stream *DwoOut,
    CodeGenFileType FileType, bool DisableVerify,
    MachineModuleInfoWrapperPass *MMIWP) {
  TargetPassConfig *PassConfig = createPassConfig(PM);
  PassConfig->addCodeGenPrepare();

  switch (FileType) {
  case CodeGenFileType::AssemblyFile:
    PM.add(createDXILPrettyPrinterLegacyPass(Out));
    PM.add(createPrintModulePass(Out, "", true));
    break;
  case CodeGenFileType::ObjectFile:
    if (TargetPassConfig::willCompleteCodeGenPipeline()) {
      PM.add(createDXILEmbedderPass());
      // We embed the other DXContainer globals after embedding DXIL so that the
      // globals don't pollute the DXIL.
      PM.add(createDXContainerGlobalsPass());

      if (!MMIWP)
        MMIWP = new MachineModuleInfoWrapperPass(this);
      PM.add(MMIWP);
      if (addAsmPrinter(PM, Out, DwoOut, FileType,
                        MMIWP->getMMI().getContext()))
        return true;
    } else
      PM.add(createDXILWriterPass(Out));
    break;
  case CodeGenFileType::Null:
    break;
  }
  return false;
}

bool DirectXTargetMachine::addPassesToEmitMC(PassManagerBase &PM,
                                             MCContext *&Ctx,
                                             raw_pwrite_stream &Out,
                                             bool DisableVerify) {
  return true;
}

TargetPassConfig *DirectXTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new DirectXPassConfig(*this, PM);
}

const DirectXSubtarget *
DirectXTargetMachine::getSubtargetImpl(const Function &) const {
  return Subtarget.get();
}

TargetTransformInfo
DirectXTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<DirectXTTIImpl>(this, F));
}

DirectXTargetLowering::DirectXTargetLowering(const DirectXTargetMachine &TM,
                                             const DirectXSubtarget &STI)
    : TargetLowering(TM, STI) {
  addRegisterClass(MVT::i32, &dxil::DXILClassRegClass);
  computeRegisterProperties(STI.getRegisterInfo());
}
