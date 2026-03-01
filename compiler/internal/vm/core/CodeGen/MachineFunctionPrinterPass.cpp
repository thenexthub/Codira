//===-- MachineFunctionPrinterPass.cpp ------------------------------------===//
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
// MachineFunctionPrinterPass implementation.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/SlotIndexes.h"
#include "vm/core/IR/PrintPasses.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

namespace {
/// MachineFunctionPrinterPass - This is a pass to dump the IR of a
/// MachineFunction.
///
struct MachineFunctionPrinterPass : public MachineFunctionPass {
  static char ID;

  raw_ostream &OS;
  const std::string Banner;

  MachineFunctionPrinterPass() : MachineFunctionPass(ID), OS(dbgs()) { }
  MachineFunctionPrinterPass(raw_ostream &os, const std::string &banner)
      : MachineFunctionPass(ID), OS(os), Banner(banner) {}

  StringRef getPassName() const override { return "MachineFunction Printer"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addUsedIfAvailable<SlotIndexesWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    if (!isFunctionInPrintList(MF.getName()))
      return false;
    OS << "# " << Banner << ":\n";
    auto *SIWrapper = getAnalysisIfAvailable<SlotIndexesWrapperPass>();
    MF.print(OS, SIWrapper ? &SIWrapper->getSI() : nullptr);
    return false;
  }
};

char MachineFunctionPrinterPass::ID = 0;
}

char &toolchain::MachineFunctionPrinterPassID = MachineFunctionPrinterPass::ID;
INITIALIZE_PASS(MachineFunctionPrinterPass, "machineinstr-printer",
                "Machine Function Printer", false, false)

/// Returns a newly-created MachineFunction Printer pass. The
/// default banner is empty.
///
MachineFunctionPass *
toolchain::createMachineFunctionPrinterPass(raw_ostream &OS,
                                       const std::string &Banner) {
  return new MachineFunctionPrinterPass(OS, Banner);
}
