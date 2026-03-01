//===-- WebAssemblyAddMissingPrototypes.cpp - Fix prototypeless functions -===//
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
/// Add prototypes to prototypes-less functions.
///
/// WebAssembly has strict function prototype checking so we need functions
/// declarations to match the call sites.  Clang treats prototype-less functions
/// as varargs (foo(...)) which happens to work on existing platforms but
/// doesn't under WebAssembly.  This pass will find all the call sites of each
/// prototype-less function, ensure they agree, and then set the signature
/// on the function declaration accordingly.
///
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Operator.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Transforms/Utils/Local.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-add-missing-prototypes"

namespace {
class WebAssemblyAddMissingPrototypes final : public ModulePass {
  StringRef getPassName() const override {
    return "Add prototypes to prototypes-less functions";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    ModulePass::getAnalysisUsage(AU);
  }

  bool runOnModule(Module &M) override;

public:
  static char ID;
  WebAssemblyAddMissingPrototypes() : ModulePass(ID) {}
};
} // End anonymous namespace

char WebAssemblyAddMissingPrototypes::ID = 0;
INITIALIZE_PASS(WebAssemblyAddMissingPrototypes, DEBUG_TYPE,
                "Add prototypes to prototypes-less functions", false, false)

ModulePass *toolchain::createWebAssemblyAddMissingPrototypes() {
  return new WebAssemblyAddMissingPrototypes();
}

bool WebAssemblyAddMissingPrototypes::runOnModule(Module &M) {
  LLVM_DEBUG(dbgs() << "********** Add Missing Prototypes **********\n");

  std::vector<std::pair<Function *, Function *>> Replacements;

  // Find all the prototype-less function declarations
  for (Function &F : M) {
    if (!F.isDeclaration() || !F.hasFnAttribute("no-prototype"))
      continue;

    LLVM_DEBUG(dbgs() << "Found no-prototype function: " << F.getName()
                      << "\n");

    // When clang emits prototype-less C functions it uses (...), i.e. varargs
    // function that take no arguments (have no sentinel).  When we see a
    // no-prototype attribute we expect the function have these properties.
    if (!F.isVarArg())
      report_fatal_error(
          "Functions with 'no-prototype' attribute must take varargs: " +
          F.getName());
    unsigned NumParams = F.getFunctionType()->getNumParams();
    if (NumParams != 0) {
      if (!(NumParams == 1 && F.arg_begin()->hasStructRetAttr()))
        report_fatal_error("Functions with 'no-prototype' attribute should "
                           "not have params: " +
                           F.getName());
    }

    // Find calls of this function, looking through bitcasts.
    SmallVector<CallBase *> Calls;
    SmallVector<Value *> Worklist;
    Worklist.push_back(&F);
    while (!Worklist.empty()) {
      Value *V = Worklist.pop_back_val();
      for (User *U : V->users()) {
        if (auto *BC = dyn_cast<BitCastOperator>(U))
          Worklist.push_back(BC);
        else if (auto *CB = dyn_cast<CallBase>(U))
          if (CB->getCalledOperand() == V)
            Calls.push_back(CB);
      }
    }

    // Create a function prototype based on the first call site that we find.
    FunctionType *NewType = nullptr;
    for (CallBase *CB : Calls) {
      LLVM_DEBUG(dbgs() << "prototype-less call of " << F.getName() << ":\n");
      LLVM_DEBUG(dbgs() << *CB << "\n");
      FunctionType *DestType = CB->getFunctionType();
      if (!NewType) {
        // Create a new function with the correct type
        NewType = DestType;
        LLVM_DEBUG(dbgs() << "found function type: " << *NewType << "\n");
      } else if (NewType != DestType) {
        errs() << "warning: prototype-less function used with "
                  "conflicting signatures: "
               << F.getName() << "\n";
        LLVM_DEBUG(dbgs() << "  " << *DestType << "\n");
        LLVM_DEBUG(dbgs() << "  " << *NewType << "\n");
      }
    }

    if (!NewType) {
      LLVM_DEBUG(
          dbgs() << "could not derive a function prototype from usage: " +
                        F.getName() + "\n");
      // We could not derive a type for this function.  In this case strip
      // the isVarArg and make it a simple zero-arg function.  This has more
      // chance of being correct.  The current signature of (...) is illegal in
      // C since it doesn't have any arguments before the "...", we this at
      // least makes it possible for this symbol to be resolved by the linker.
      NewType = FunctionType::get(F.getFunctionType()->getReturnType(), false);
    }

    Function *NewF =
        Function::Create(NewType, F.getLinkage(), F.getName() + ".fixed_sig");
    NewF->setAttributes(F.getAttributes());
    NewF->removeFnAttr("no-prototype");
    Replacements.emplace_back(&F, NewF);
  }

  for (auto &Pair : Replacements) {
    Function *OldF = Pair.first;
    Function *NewF = Pair.second;
    std::string Name = std::string(OldF->getName());
    M.getFunctionList().push_back(NewF);
    OldF->replaceAllUsesWith(
        ConstantExpr::getPointerBitCastOrAddrSpaceCast(NewF, OldF->getType()));
    OldF->eraseFromParent();
    NewF->setName(Name);
  }

  return !Replacements.empty();
}
