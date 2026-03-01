//===-- AMDGPUAlwaysInlinePass.cpp - Promote Allocas ----------------------===//
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
/// \file
/// This pass marks all internal functions as always_inline and creates
/// duplicates of all other functions and marks the duplicates as always_inline.
//
//===----------------------------------------------------------------------===//

#include "AMDGPU.h"
#include "AMDGPUTargetMachine.h"
#include "Utils/AMDGPUBaseInfo.h"
#include "vm/core/CodeGen/CommandFlags.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;

namespace {

static cl::opt<bool> StressCalls(
  "amdgpu-stress-function-calls",
  cl::Hidden,
  cl::desc("Force all functions to be noinline"),
  cl::init(false));

class AMDGPUAlwaysInline : public ModulePass {
  bool GlobalOpt;

public:
  static char ID;

  AMDGPUAlwaysInline(bool GlobalOpt = false) :
    ModulePass(ID), GlobalOpt(GlobalOpt) { }
  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

} // End anonymous namespace

INITIALIZE_PASS(AMDGPUAlwaysInline, "amdgpu-always-inline",
                "AMDGPU Inline All Functions", false, false)

char AMDGPUAlwaysInline::ID = 0;

static void
recursivelyVisitUsers(GlobalValue &GV,
                      SmallPtrSetImpl<Function *> &FuncsToAlwaysInline) {
  SmallVector<User *, 16> Stack(GV.users());

  SmallPtrSet<const Value *, 8> Visited;

  while (!Stack.empty()) {
    User *U = Stack.pop_back_val();
    if (!Visited.insert(U).second)
      continue;

    if (Instruction *I = dyn_cast<Instruction>(U)) {
      Function *F = I->getFunction();
      if (!AMDGPU::isEntryFunctionCC(F->getCallingConv())) {
        // FIXME: This is a horrible hack. We should always respect noinline,
        // and just let us hit the error when we can't handle this.
        //
        // Unfortunately, clang adds noinline to all functions at -O0. We have
        // to override this here until that's fixed.
        F->removeFnAttr(Attribute::NoInline);

        FuncsToAlwaysInline.insert(F);
        Stack.push_back(F);
      }

      // No need to look at further users, but we do need to inline any callers.
      continue;
    }

    append_range(Stack, U->users());
  }
}

static bool alwaysInlineImpl(Module &M, bool GlobalOpt) {
  std::vector<GlobalAlias*> AliasesToRemove;

  bool Changed = false;
  SmallPtrSet<Function *, 8> FuncsToAlwaysInline;
  SmallPtrSet<Function *, 8> FuncsToNoInline;
  Triple TT(M.getTargetTriple());

  for (GlobalAlias &A : M.aliases()) {
    if (Function* F = dyn_cast<Function>(A.getAliasee())) {
      if (TT.isAMDGCN() && A.getLinkage() != GlobalValue::InternalLinkage)
        continue;
      Changed = true;
      A.replaceAllUsesWith(F);
      AliasesToRemove.push_back(&A);
    }

    // FIXME: If the aliasee isn't a function, it's some kind of constant expr
    // cast that won't be inlined through.
  }

  if (GlobalOpt) {
    for (GlobalAlias* A : AliasesToRemove) {
      A->eraseFromParent();
    }
  }

  // Always force inlining of any function that uses an LDS global address. This
  // is something of a workaround because we don't have a way of supporting LDS
  // objects defined in functions. LDS is always allocated by a kernel, and it
  // is difficult to manage LDS usage if a function may be used by multiple
  // kernels.
  //
  // OpenCL doesn't allow declaring LDS in non-kernels, so in practice this
  // should only appear when IPO passes manages to move LDs defined in a kernel
  // into a single user function.

  for (GlobalVariable &GV : M.globals()) {
    // TODO: Region address
    unsigned AS = GV.getAddressSpace();
    if ((AS == AMDGPUAS::REGION_ADDRESS) ||
        (AS == AMDGPUAS::LOCAL_ADDRESS &&
         (!AMDGPUTargetMachine::EnableLowerModuleLDS)))
      recursivelyVisitUsers(GV, FuncsToAlwaysInline);
  }

  if (!AMDGPUTargetMachine::EnableFunctionCalls || StressCalls) {
    auto IncompatAttr
      = StressCalls ? Attribute::AlwaysInline : Attribute::NoInline;

    for (Function &F : M) {
      if (!F.isDeclaration() && !F.use_empty() &&
          !F.hasFnAttribute(IncompatAttr)) {
        if (StressCalls) {
          if (!FuncsToAlwaysInline.count(&F))
            FuncsToNoInline.insert(&F);
        } else
          FuncsToAlwaysInline.insert(&F);
      }
    }
  }

  for (Function *F : FuncsToAlwaysInline)
    F->addFnAttr(Attribute::AlwaysInline);

  for (Function *F : FuncsToNoInline)
    F->addFnAttr(Attribute::NoInline);

  return Changed || !FuncsToAlwaysInline.empty() || !FuncsToNoInline.empty();
}

bool AMDGPUAlwaysInline::runOnModule(Module &M) {
  return alwaysInlineImpl(M, GlobalOpt);
}

ModulePass *toolchain::createAMDGPUAlwaysInlinePass(bool GlobalOpt) {
  return new AMDGPUAlwaysInline(GlobalOpt);
}

PreservedAnalyses AMDGPUAlwaysInlinePass::run(Module &M,
                                              ModuleAnalysisManager &AM) {
  const bool Changed = alwaysInlineImpl(M, GlobalOpt);
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
