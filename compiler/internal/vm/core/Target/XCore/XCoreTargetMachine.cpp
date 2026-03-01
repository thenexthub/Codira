//===-- XCoreTargetMachine.cpp - Define TargetMachine for XCore -----------===//
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

#include "XCoreTargetMachine.h"
#include "TargetInfo/XCoreTargetInfo.h"
#include "XCore.h"
#include "XCoreMachineFunctionInfo.h"
#include "XCoreTargetObjectFile.h"
#include "XCoreTargetTransformInfo.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/CodeGen.h"
#include "vm/core/Support/Compiler.h"
#include <optional>

using namespace vm::core;

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

static CodeModel::Model
getEffectiveXCoreCodeModel(std::optional<CodeModel::Model> CM) {
  if (CM) {
    if (*CM != CodeModel::Small && *CM != CodeModel::Large)
      report_fatal_error("Target only supports CodeModel Small or Large");
    return *CM;
  }
  return CodeModel::Small;
}

/// Create an ILP32 architecture model
///
XCoreTargetMachine::XCoreTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       std::optional<Reloc::Model> RM,
                                       std::optional<CodeModel::Model> CM,
                                       CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               getEffectiveRelocModel(RM),
                               getEffectiveXCoreCodeModel(CM), OL),
      TLOF(std::make_unique<XCoreTargetObjectFile>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

XCoreTargetMachine::~XCoreTargetMachine() = default;

namespace {

/// XCore Code Generator Pass Configuration Options.
class XCorePassConfig : public TargetPassConfig {
public:
  XCorePassConfig(XCoreTargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  XCoreTargetMachine &getXCoreTargetMachine() const {
    return getTM<XCoreTargetMachine>();
  }

  void addIRPasses() override;
  bool addPreISel() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
};

} // end anonymous namespace

TargetPassConfig *XCoreTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new XCorePassConfig(*this, PM);
}

void XCorePassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());

  TargetPassConfig::addIRPasses();
}

bool XCorePassConfig::addPreISel() {
  addPass(createXCoreLowerThreadLocalPass());
  return false;
}

bool XCorePassConfig::addInstSelector() {
  addPass(createXCoreISelDag(getXCoreTargetMachine(), getOptLevel()));
  return false;
}

void XCorePassConfig::addPreEmitPass() {
  addPass(createXCoreFrameToArgsOffsetEliminationPass());
}

// Force static initialization.
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXCoreTarget() {
  RegisterTargetMachine<XCoreTargetMachine> X(getTheXCoreTarget());
  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeXCoreAsmPrinterPass(PR);
  initializeXCoreDAGToDAGISelLegacyPass(PR);
  initializeXCoreLowerThreadLocalPass(PR);
}

TargetTransformInfo
XCoreTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<XCoreTTIImpl>(this, F));
}

MachineFunctionInfo *XCoreTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return XCoreFunctionInfo::create<XCoreFunctionInfo>(Allocator, F, STI);
}
