//===--- IRPrintingPasses.cpp - Module and Function printing passes -------===//
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
// PrintModulePass and PrintFunctionPass implementations for the legacy pass
// manager.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/IRPrintingPasses.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PrintPasses.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

namespace {

class PrintModulePassWrapper : public ModulePass {
  raw_ostream &OS;
  std::string Banner;
  bool ShouldPreserveUseListOrder;

public:
  static char ID;
  PrintModulePassWrapper() : ModulePass(ID), OS(dbgs()) {}
  PrintModulePassWrapper(raw_ostream &OS, const std::string &Banner,
                         bool ShouldPreserveUseListOrder)
      : ModulePass(ID), OS(OS), Banner(Banner),
        ShouldPreserveUseListOrder(ShouldPreserveUseListOrder) {}

  bool runOnModule(Module &M) override {
    // Remove intrinsic declarations when printing in the new format.
    // TODO: consider removing this as debug-intrinsics are gone.
    M.removeDebugIntrinsicDeclarations();

    if (toolchain::isFunctionInPrintList("*")) {
      if (!Banner.empty())
        OS << Banner << "\n";
      M.print(OS, nullptr, ShouldPreserveUseListOrder);
    } else {
      bool BannerPrinted = false;
      for (const auto &F : M.functions()) {
        if (toolchain::isFunctionInPrintList(F.getName())) {
          if (!BannerPrinted && !Banner.empty()) {
            OS << Banner << "\n";
            BannerPrinted = true;
          }
          F.print(OS);
        }
      }
    }

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  StringRef getPassName() const override { return "Print Module IR"; }
};

class PrintFunctionPassWrapper : public FunctionPass {
  raw_ostream &OS;
  std::string Banner;

public:
  static char ID;
  PrintFunctionPassWrapper() : FunctionPass(ID), OS(dbgs()) {}
  PrintFunctionPassWrapper(raw_ostream &OS, const std::string &Banner)
      : FunctionPass(ID), OS(OS), Banner(Banner) {}

  // This pass just prints a banner followed by the function as it's processed.
  bool runOnFunction(Function &F) override {
    if (isFunctionInPrintList(F.getName())) {
      if (forcePrintModuleIR())
        OS << Banner << " (function: " << F.getName() << ")\n"
           << *F.getParent();
      else
        OS << Banner << '\n' << static_cast<Value &>(F);
    }

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  StringRef getPassName() const override { return "Print Function IR"; }
};

} // namespace

char PrintModulePassWrapper::ID = 0;
INITIALIZE_PASS(PrintModulePassWrapper, "print-module",
                "Print module to stderr", false, true)
char PrintFunctionPassWrapper::ID = 0;
INITIALIZE_PASS(PrintFunctionPassWrapper, "print-function",
                "Print function to stderr", false, true)

ModulePass *toolchain::createPrintModulePass(toolchain::raw_ostream &OS,
                                        const std::string &Banner,
                                        bool ShouldPreserveUseListOrder) {
  return new PrintModulePassWrapper(OS, Banner, ShouldPreserveUseListOrder);
}

FunctionPass *toolchain::createPrintFunctionPass(toolchain::raw_ostream &OS,
                                            const std::string &Banner) {
  return new PrintFunctionPassWrapper(OS, Banner);
}

bool toolchain::isIRPrintingPass(Pass *P) {
  const char *PID = (const char *)P->getPassID();

  return (PID == &PrintModulePassWrapper::ID) ||
         (PID == &PrintFunctionPassWrapper::ID);
}
