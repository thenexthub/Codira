//===-- toolchain/CodeGen/MachineModuleInfo.cpp ----------------------*- C++ -*-===//
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

#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Module.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"
#include "vm/core/Target/TargetMachine.h"
#include <cassert>

using namespace vm::core;
using namespace vm::core::dwarf;

// Out of line virtual method.
MachineModuleInfoImpl::~MachineModuleInfoImpl() = default;

void MachineModuleInfo::initialize() {
  ObjFileMMI = nullptr;
  NextFnNum = 0;
}

void MachineModuleInfo::finalize() {
  Context.reset();
  // We don't clear the ExternalContext.

  delete ObjFileMMI;
  ObjFileMMI = nullptr;
}

MachineModuleInfo::MachineModuleInfo(MachineModuleInfo &&MMI)
    : TM(std::move(MMI.TM)),
      Context(TM.getTargetTriple(), TM.getMCAsmInfo(), TM.getMCRegisterInfo(),
              TM.getMCSubtargetInfo(), nullptr, &TM.Options.MCOptions, false),
      MachineFunctions(std::move(MMI.MachineFunctions)) {
  Context.setObjectFileInfo(TM.getObjFileLowering());
  ObjFileMMI = MMI.ObjFileMMI;
  ExternalContext = MMI.ExternalContext;
  TheModule = MMI.TheModule;
}

MachineModuleInfo::MachineModuleInfo(const TargetMachine *TM)
    : TM(*TM), Context(TM->getTargetTriple(), TM->getMCAsmInfo(),
                       TM->getMCRegisterInfo(), TM->getMCSubtargetInfo(),
                       nullptr, &TM->Options.MCOptions, false) {
  Context.setObjectFileInfo(TM->getObjFileLowering());
  initialize();
}

MachineModuleInfo::MachineModuleInfo(const TargetMachine *TM,
                                     MCContext *ExtContext)
    : TM(*TM), Context(TM->getTargetTriple(), TM->getMCAsmInfo(),
                       TM->getMCRegisterInfo(), TM->getMCSubtargetInfo(),
                       nullptr, &TM->Options.MCOptions, false),
      ExternalContext(ExtContext) {
  Context.setObjectFileInfo(TM->getObjFileLowering());
  initialize();
}

MachineModuleInfo::~MachineModuleInfo() { finalize(); }

MachineFunction *
MachineModuleInfo::getMachineFunction(const Function &F) const {
  auto I = MachineFunctions.find(&F);
  return I != MachineFunctions.end() ? I->second.get() : nullptr;
}

MachineFunction &MachineModuleInfo::getOrCreateMachineFunction(Function &F) {
  // Shortcut for the common case where a sequence of MachineFunctionPasses
  // all query for the same Function.
  if (LastRequest == &F)
    return *LastResult;

  auto I = MachineFunctions.insert(
      std::make_pair(&F, std::unique_ptr<MachineFunction>()));
  MachineFunction *MF;
  if (I.second) {
    // No pre-existing machine function, create a new one.
    const TargetSubtargetInfo &STI = *TM.getSubtargetImpl(F);
    MF = new MachineFunction(F, TM, STI, getContext(), NextFnNum++);
    MF->initTargetMachineFunctionInfo(STI);

    // MRI callback for target specific initializations.
    TM.registerMachineRegisterInfoCallback(*MF);

    // Update the set entry.
    I.first->second.reset(MF);
  } else {
    MF = I.first->second.get();
  }

  LastRequest = &F;
  LastResult = MF;
  return *MF;
}

void MachineModuleInfo::deleteMachineFunctionFor(Function &F) {
  MachineFunctions.erase(&F);
  LastRequest = nullptr;
  LastResult = nullptr;
}

void MachineModuleInfo::insertFunction(const Function &F,
                                       std::unique_ptr<MachineFunction> &&MF) {
  auto I = MachineFunctions.insert(std::make_pair(&F, std::move(MF)));
  assert(I.second && "machine function already mapped");
  (void)I;
}

namespace {

/// This pass frees the MachineFunction object associated with a Function.
class FreeMachineFunction : public FunctionPass {
public:
  static char ID;

  FreeMachineFunction() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineModuleInfoWrapperPass>();
    AU.addPreserved<MachineModuleInfoWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    MachineModuleInfo &MMI =
        getAnalysis<MachineModuleInfoWrapperPass>().getMMI();
    MMI.deleteMachineFunctionFor(F);
    return true;
  }

  StringRef getPassName() const override {
    return "Free MachineFunction";
  }
};

} // end anonymous namespace

char FreeMachineFunction::ID;

FunctionPass *toolchain::createFreeMachineFunctionPass() {
  return new FreeMachineFunction();
}

MachineModuleInfoWrapperPass::MachineModuleInfoWrapperPass(
    const TargetMachine *TM)
    : ImmutablePass(ID), MMI(TM) {
  initializeMachineModuleInfoWrapperPassPass(*PassRegistry::getPassRegistry());
}

MachineModuleInfoWrapperPass::MachineModuleInfoWrapperPass(
    const TargetMachine *TM, MCContext *ExtContext)
    : ImmutablePass(ID), MMI(TM, ExtContext) {
  initializeMachineModuleInfoWrapperPassPass(*PassRegistry::getPassRegistry());
}

// Handle the Pass registration stuff necessary to use DataLayout's.
INITIALIZE_PASS(MachineModuleInfoWrapperPass, "machinemoduleinfo",
                "Machine Module Information", false, false)
char MachineModuleInfoWrapperPass::ID = 0;

static uint64_t getLocCookie(const SMDiagnostic &SMD, const SourceMgr &SrcMgr,
                             std::vector<const MDNode *> &LocInfos) {
  // Look up a LocInfo for the buffer this diagnostic is coming from.
  unsigned BufNum = SrcMgr.FindBufferContainingLoc(SMD.getLoc());
  const MDNode *LocInfo = nullptr;
  if (BufNum > 0 && BufNum <= LocInfos.size())
    LocInfo = LocInfos[BufNum - 1];

  // If the inline asm had metadata associated with it, pull out a location
  // cookie corresponding to which line the error occurred on.
  uint64_t LocCookie = 0;
  if (LocInfo) {
    unsigned ErrorLine = SMD.getLineNo() - 1;
    if (ErrorLine >= LocInfo->getNumOperands())
      ErrorLine = 0;

    if (LocInfo->getNumOperands() != 0)
      if (const ConstantInt *CI =
              mdconst::dyn_extract<ConstantInt>(LocInfo->getOperand(ErrorLine)))
        LocCookie = CI->getZExtValue();
  }

  return LocCookie;
}

bool MachineModuleInfoWrapperPass::doInitialization(Module &M) {
  MMI.initialize();
  MMI.TheModule = &M;
  LLVMContext &Ctx = M.getContext();
  MMI.getContext().setDiagnosticHandler(
      [&Ctx, &M](const SMDiagnostic &SMD, bool IsInlineAsm,
                 const SourceMgr &SrcMgr,
                 std::vector<const MDNode *> &LocInfos) {
        uint64_t LocCookie = 0;
        if (IsInlineAsm)
          LocCookie = getLocCookie(SMD, SrcMgr, LocInfos);
        Ctx.diagnose(
            DiagnosticInfoSrcMgr(SMD, M.getName(), IsInlineAsm, LocCookie));
      });
  return false;
}

bool MachineModuleInfoWrapperPass::doFinalization(Module &M) {
  MMI.finalize();
  return false;
}

AnalysisKey MachineModuleAnalysis::Key;

MachineModuleAnalysis::Result
MachineModuleAnalysis::run(Module &M, ModuleAnalysisManager &) {
  MMI.TheModule = &M;
  LLVMContext &Ctx = M.getContext();
  MMI.getContext().setDiagnosticHandler(
      [&Ctx, &M](const SMDiagnostic &SMD, bool IsInlineAsm,
                 const SourceMgr &SrcMgr,
                 std::vector<const MDNode *> &LocInfos) {
        unsigned LocCookie = 0;
        if (IsInlineAsm)
          LocCookie = getLocCookie(SMD, SrcMgr, LocInfos);
        Ctx.diagnose(
            DiagnosticInfoSrcMgr(SMD, M.getName(), IsInlineAsm, LocCookie));
      });
  return Result(MMI);
}
