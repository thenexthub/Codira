//===-- BPF.h - Top-level interface for BPF representation ------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_BPF_BPF_H
#define LLVM_LIB_TARGET_BPF_BPF_H

#include "MCTargetDesc/BPFMCTargetDesc.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Pass.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {
class BPFRegisterBankInfo;
class BPFSubtarget;
class BPFTargetMachine;
class InstructionSelector;
class PassRegistry;

#define BPF_TRAP "__bpf_trap"

ModulePass *createBPFCheckAndAdjustIR();

FunctionPass *createBPFISelDag(BPFTargetMachine &TM);
FunctionPass *createBPFMISimplifyPatchablePass();
FunctionPass *createBPFMIPeepholePass();
FunctionPass *createBPFMIPreEmitPeepholePass();
FunctionPass *createBPFMIPreEmitCheckingPass();

InstructionSelector *createBPFInstructionSelector(const BPFTargetMachine &,
                                                  const BPFSubtarget &,
                                                  const BPFRegisterBankInfo &);

void initializeBPFAsmPrinterPass(PassRegistry &);
void initializeBPFCheckAndAdjustIRPass(PassRegistry&);
void initializeBPFDAGToDAGISelLegacyPass(PassRegistry &);
void initializeBPFMIPeepholePass(PassRegistry &);
void initializeBPFMIPreEmitCheckingPass(PassRegistry &);
void initializeBPFMIPreEmitPeepholePass(PassRegistry &);
void initializeBPFMISimplifyPatchablePass(PassRegistry &);

class BPFAbstractMemberAccessPass
    : public PassInfoMixin<BPFAbstractMemberAccessPass> {
  BPFTargetMachine *TM;

public:
  BPFAbstractMemberAccessPass(BPFTargetMachine *TM) : TM(TM) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  static bool isRequired() { return true; }
};

class BPFPreserveDITypePass : public PassInfoMixin<BPFPreserveDITypePass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  static bool isRequired() { return true; }
};

class BPFIRPeepholePass : public PassInfoMixin<BPFIRPeepholePass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  static bool isRequired() { return true; }
};

class BPFASpaceCastSimplifyPass
    : public PassInfoMixin<BPFASpaceCastSimplifyPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  static bool isRequired() { return true; }
};

class BPFAdjustOptPass : public PassInfoMixin<BPFAdjustOptPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

class BPFPreserveStaticOffsetPass
    : public PassInfoMixin<BPFPreserveStaticOffsetPass> {
  bool AllowPartial;

public:
  BPFPreserveStaticOffsetPass(bool AllowPartial) : AllowPartial(AllowPartial) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  static bool isRequired() { return true; }

  static std::pair<GetElementPtrInst *, LoadInst *>
  reconstructLoad(CallInst *Call);

  static std::pair<GetElementPtrInst *, StoreInst *>
  reconstructStore(CallInst *Call);
};

} // namespace vm::core

#endif
