//===-- MachineFunctionPass.cpp -------------------------------------------===//
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
// This file contains the definitions of the MachineFunctionPass members.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/Analysis/BasicAliasAnalysis.h"
#include "vm/core/Analysis/DominanceFrontier.h"
#include "vm/core/Analysis/GlobalsModRef.h"
#include "vm/core/Analysis/IVUsers.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/Analysis/MemoryDependenceAnalysis.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/Analysis/ScalarEvolution.h"
#include "vm/core/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "vm/core/CodeGen/DroppedVariableStatsMIR.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PrintPasses.h"

using namespace vm::core;
using namespace ore;

static cl::opt<bool> DroppedVarStatsMIR(
    "dropped-variable-stats-mir", cl::Hidden,
    cl::desc("Dump dropped debug variables stats for MIR passes"),
    cl::init(false));

Pass *MachineFunctionPass::createPrinterPass(raw_ostream &O,
                                             const std::string &Banner) const {
  return createMachineFunctionPrinterPass(O, Banner);
}

bool MachineFunctionPass::runOnFunction(Function &F) {
  // Do not codegen any 'available_externally' functions at all, they have
  // definitions outside the translation unit.
  if (F.hasAvailableExternallyLinkage())
    return false;

  MachineModuleInfo &MMI = getAnalysis<MachineModuleInfoWrapperPass>().getMMI();
  MachineFunction &MF = MMI.getOrCreateMachineFunction(F);

  MachineFunctionProperties &MFProps = MF.getProperties();

#ifndef NDEBUG
  if (!MFProps.verifyRequiredProperties(RequiredProperties)) {
    errs() << "MachineFunctionProperties required by " << getPassName()
           << " pass are not met by function " << F.getName() << ".\n"
           << "Required properties: ";
    RequiredProperties.print(errs());
    errs() << "\nCurrent properties: ";
    MFProps.print(errs());
    errs() << "\n";
    llvm_unreachable("MachineFunctionProperties check failed");
  }
#endif
  // Collect the MI count of the function before the pass.
  unsigned CountBefore, CountAfter;

  // Check if the user asked for size remarks.
  bool ShouldEmitSizeRemarks =
      F.getParent()->shouldEmitInstrCountChangedRemark();

  // If we want size remarks, collect the number of MachineInstrs in our
  // MachineFunction before the pass runs.
  if (ShouldEmitSizeRemarks)
    CountBefore = MF.getInstructionCount();

  // For --print-changed, if the function name is a candidate, save the
  // serialized MF to be compared later.
  SmallString<0> BeforeStr, AfterStr;
  StringRef PassID;
  if (PrintChanged != ChangePrinter::None) {
    if (const PassInfo *PI = Pass::lookupPassInfo(getPassID()))
      PassID = PI->getPassArgument();
  }
  const bool IsInterestingPass = isPassInPrintList(PassID);
  const bool ShouldPrintChanged = PrintChanged != ChangePrinter::None &&
                                  IsInterestingPass &&
                                  isFunctionInPrintList(MF.getName());
  if (ShouldPrintChanged) {
    raw_svector_ostream OS(BeforeStr);
    MF.print(OS);
  }

  MFProps.reset(ClearedProperties);

  bool RV;
  if (DroppedVarStatsMIR) {
    DroppedVariableStatsMIR DroppedVarStatsMF;
    auto PassName = getPassName();
    DroppedVarStatsMF.runBeforePass(PassName, &MF);
    RV = runOnMachineFunction(MF);
    DroppedVarStatsMF.runAfterPass(PassName, &MF);
  } else {
    RV = runOnMachineFunction(MF);
  }

  if (ShouldEmitSizeRemarks) {
    // We wanted size remarks. Check if there was a change to the number of
    // MachineInstrs in the module. Emit a remark if there was a change.
    CountAfter = MF.getInstructionCount();
    if (CountBefore != CountAfter) {
      MachineOptimizationRemarkEmitter MORE(MF, nullptr);
      MORE.emit([&]() {
        int64_t Delta = static_cast<int64_t>(CountAfter) -
                        static_cast<int64_t>(CountBefore);
        MachineOptimizationRemarkAnalysis R("size-info", "FunctionMISizeChange",
                                            MF.getFunction().getSubprogram(),
                                            &MF.front());
        R << NV("Pass", getPassName())
          << ": Function: " << NV("Function", F.getName()) << ": "
          << "MI Instruction count changed from "
          << NV("MIInstrsBefore", CountBefore) << " to "
          << NV("MIInstrsAfter", CountAfter)
          << "; Delta: " << NV("Delta", Delta);
        return R;
      });
    }
  }

  MFProps.set(SetProperties);

  // For --print-changed, print if the serialized MF has changed. Modes other
  // than quiet/verbose are unimplemented and treated the same as 'quiet'.
  if (ShouldPrintChanged || !IsInterestingPass) {
    if (ShouldPrintChanged) {
      raw_svector_ostream OS(AfterStr);
      MF.print(OS);
    }
    if (IsInterestingPass && BeforeStr != AfterStr) {
      errs() << ("*** IR Dump After " + getPassName() + " (" + PassID +
                 ") on " + MF.getName() + " ***\n");
      switch (PrintChanged) {
      case ChangePrinter::None:
        llvm_unreachable("");
      case ChangePrinter::Quiet:
      case ChangePrinter::Verbose:
      case ChangePrinter::DotCfgQuiet:   // unimplemented
      case ChangePrinter::DotCfgVerbose: // unimplemented
        errs() << AfterStr;
        break;
      case ChangePrinter::DiffQuiet:
      case ChangePrinter::DiffVerbose:
      case ChangePrinter::ColourDiffQuiet:
      case ChangePrinter::ColourDiffVerbose: {
        bool Color = toolchain::is_contained(
            {ChangePrinter::ColourDiffQuiet, ChangePrinter::ColourDiffVerbose},
            PrintChanged.getValue());
        StringRef Removed = Color ? "\033[31m-%l\033[0m\n" : "-%l\n";
        StringRef Added = Color ? "\033[32m+%l\033[0m\n" : "+%l\n";
        StringRef NoChange = " %l\n";
        errs() << doSystemDiff(BeforeStr, AfterStr, Removed, Added, NoChange);
        break;
      }
      }
    } else if (toolchain::is_contained({ChangePrinter::Verbose,
                                   ChangePrinter::DiffVerbose,
                                   ChangePrinter::ColourDiffVerbose},
                                  PrintChanged.getValue())) {
      const char *Reason =
          IsInterestingPass ? " omitted because no change" : " filtered out";
      errs() << "*** IR Dump After " << getPassName();
      if (!PassID.empty())
        errs() << " (" << PassID << ")";
      errs() << " on " << MF.getName() + Reason + " ***\n";
    }
  }
  return RV;
}

void MachineFunctionPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<MachineModuleInfoWrapperPass>();
  AU.addPreserved<MachineModuleInfoWrapperPass>();

  // MachineFunctionPass preserves all LLVM IR passes, but there's no
  // high-level way to express this. Instead, just list a bunch of
  // passes explicitly. This does not include setPreservesCFG,
  // because CodeGen overloads that to mean preserving the MachineBasicBlock
  // CFG in addition to the LLVM IR CFG.
  AU.addPreserved<BasicAAWrapperPass>();
  AU.addPreserved<DominanceFrontierWrapperPass>();
  AU.addPreserved<DominatorTreeWrapperPass>();
  AU.addPreserved<AAResultsWrapperPass>();
  AU.addPreserved<GlobalsAAWrapperPass>();
  AU.addPreserved<IVUsersWrapperPass>();
  AU.addPreserved<LoopInfoWrapperPass>();
  AU.addPreserved<MemoryDependenceWrapperPass>();
  AU.addPreserved<ScalarEvolutionWrapperPass>();
  AU.addPreserved<SCEVAAWrapperPass>();

  FunctionPass::getAnalysisUsage(AU);
}
