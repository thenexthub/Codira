//===- MIRPrintingPass.cpp - Pass that prints out using the MIR format ----===//
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
// This file implements a pass that prints out the LLVM module using the MIR
// serialization format.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MIRPrinter.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/Function.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

PreservedAnalyses PrintMIRPreparePass::run(Module &M, ModuleAnalysisManager &) {
  printMIR(OS, M);
  return PreservedAnalyses::all();
}

PreservedAnalyses PrintMIRPass::run(MachineFunction &MF,
                                    MachineFunctionAnalysisManager &MFAM) {
  auto &FAM = MFAM.getResult<FunctionAnalysisManagerMachineFunctionProxy>(MF)
                  .getManager();

  printMIR(OS, FAM, MF);
  return PreservedAnalyses::all();
}

namespace {

/// This pass prints out the LLVM IR to an output stream using the MIR
/// serialization format.
struct MIRPrintingPass : public MachineFunctionPass {
  static char ID;
  raw_ostream &OS;
  std::string MachineFunctions;

  MIRPrintingPass() : MachineFunctionPass(ID), OS(dbgs()) {}
  MIRPrintingPass(raw_ostream &OS) : MachineFunctionPass(ID), OS(OS) {}

  StringRef getPassName() const override { return "MIR Printing Pass"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    std::string Str;
    raw_string_ostream StrOS(Str);

    MachineModuleInfo *MMI =
        &getAnalysis<MachineModuleInfoWrapperPass>().getMMI();

    printMIR(StrOS, *MMI, MF);
    MachineFunctions.append(Str);
    return false;
  }

  bool doFinalization(Module &M) override {
    printMIR(OS, M);
    OS << MachineFunctions;
    return false;
  }
};

char MIRPrintingPass::ID = 0;

} // end anonymous namespace

char &toolchain::MIRPrintingPassID = MIRPrintingPass::ID;
INITIALIZE_PASS(MIRPrintingPass, "mir-printer", "MIR Printer", false, false)

MachineFunctionPass *toolchain::createPrintMIRPass(raw_ostream &OS) {
  return new MIRPrintingPass(OS);
}
