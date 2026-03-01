//===- MachineCFGPrinter.cpp - DOT Printer for Machine Functions ----------===//
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
//===----------------------------------------------------------------------===//
//
// This file defines the `-dot-machine-cfg` analysis pass, which emits
// Machine Function in DOT format in file titled `<prefix>.<function-name>.dot.
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineCFGPrinter.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"
#include "vm/core/Support/GraphWriter.h"

using namespace vm::core;

#define DEBUG_TYPE "dot-machine-cfg"

static cl::opt<std::string>
    MCFGFuncName("mcfg-func-name", cl::Hidden,
                 cl::desc("The name of a function (or its substring)"
                          " whose CFG is viewed/printed."));

static cl::opt<std::string> MCFGDotFilenamePrefix(
    "mcfg-dot-filename-prefix", cl::Hidden,
    cl::desc("The prefix used for the Machine CFG dot file names."));

static cl::opt<bool>
    CFGOnly("dot-mcfg-only", cl::init(false), cl::Hidden,
            cl::desc("Print only the CFG without blocks body"));

static void writeMCFGToDotFile(MachineFunction &MF) {
  std::string Filename =
      (MCFGDotFilenamePrefix + "." + MF.getName() + ".dot").str();
  errs() << "Writing '" << Filename << "'...";

  std::error_code EC;
  raw_fd_ostream File(Filename, EC, sys::fs::OF_Text);

  DOTMachineFuncInfo MCFGInfo(&MF);

  if (!EC)
    WriteGraph(File, &MCFGInfo, CFGOnly);
  else
    errs() << "  error opening file for writing!";
  errs() << '\n';
}

namespace {

class MachineCFGPrinter : public MachineFunctionPass {
public:
  static char ID;

  MachineCFGPrinter();

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

} // namespace

char MachineCFGPrinter::ID = 0;

char &toolchain::MachineCFGPrinterID = MachineCFGPrinter::ID;

INITIALIZE_PASS(MachineCFGPrinter, DEBUG_TYPE, "Machine CFG Printer Pass",
                false, true)

/// Default construct and initialize the pass.
MachineCFGPrinter::MachineCFGPrinter() : MachineFunctionPass(ID) {
  initializeMachineCFGPrinterPass(*PassRegistry::getPassRegistry());
}

bool MachineCFGPrinter::runOnMachineFunction(MachineFunction &MF) {
  if (!MCFGFuncName.empty() && !MF.getName().contains(MCFGFuncName))
    return false;
  errs() << "Writing Machine CFG for function ";
  errs().write_escaped(MF.getName()) << '\n';

  writeMCFGToDotFile(MF);
  return false;
}
