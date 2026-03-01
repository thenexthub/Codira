//===-- PPCGenScalarMASSEntries.cpp ---------------------------------------===//
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
// This transformation converts standard math functions into their
// corresponding MASS (scalar) entries for PowerPC targets.
// Following are examples of such conversion:
//     tanh ---> __xl_tanh_finite
// Such lowering is legal under the fast-math option.
//
//===----------------------------------------------------------------------===//

#include "PPC.h"
#include "PPCSubtarget.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Module.h"

#define DEBUG_TYPE "ppc-gen-scalar-mass"

using namespace vm::core;

namespace {

class PPCGenScalarMASSEntries : public ModulePass {
public:
  static char ID;

  PPCGenScalarMASSEntries() : ModulePass(ID) {
    ScalarMASSFuncs = {
#define TLI_DEFINE_SCALAR_MASS_FUNCS
#include "vm/core/Analysis/ScalarFuncs.def"
    };
  }

  bool runOnModule(Module &M) override;

  StringRef getPassName() const override {
    return "PPC Generate Scalar MASS Entries";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }

private:
  std::map<StringRef, StringRef> ScalarMASSFuncs;
  bool isCandidateSafeToLower(const CallInst &CI) const;
  bool isFiniteCallSafe(const CallInst &CI) const;
  bool createScalarMASSCall(StringRef MASSEntry, CallInst &CI,
                            Function &Func) const;
};

} // namespace

// Returns true if 'afn' flag exists on the call instruction with the math
// function
bool PPCGenScalarMASSEntries::isCandidateSafeToLower(const CallInst &CI) const {
  // skip functions with no scalar or vector FP type (like cosisin)
  if (!isa<FPMathOperator>(CI))
    return false;

  return CI.hasApproxFunc();
}

// Returns true if 'nnan', 'ninf' and 'nsz' flags exist on the call instruction
// with the math function
bool PPCGenScalarMASSEntries::isFiniteCallSafe(const CallInst &CI) const {
  // skip functions with no scalar or vector FP type (like cosisin)
  if (!isa<FPMathOperator>(CI))
    return false;

  // FIXME: no-errno and trapping-math need to be set for MASS converstion
  // but they don't have IR representation.
  return CI.hasNoNaNs() && CI.hasNoInfs() && CI.hasNoSignedZeros();
}

/// Lowers scalar math functions to scalar MASS functions.
///     e.g.: tanh         --> __xl_tanh_finite or __xl_tanh
/// Both function prototype and its callsite is updated during lowering.
bool PPCGenScalarMASSEntries::createScalarMASSCall(StringRef MASSEntry,
                                                   CallInst &CI,
                                                   Function &Func) const {
  if (CI.use_empty())
    return false;

  Module *M = Func.getParent();
  assert(M && "Expecting a valid Module");

  std::string MASSEntryStr = MASSEntry.str();
  if (isFiniteCallSafe(CI))
    MASSEntryStr += "_finite";

  FunctionCallee FCache = M->getOrInsertFunction(
      MASSEntryStr, Func.getFunctionType(), Func.getAttributes());

  CI.setCalledFunction(FCache);

  return true;
}

bool PPCGenScalarMASSEntries::runOnModule(Module &M) {
  bool Changed = false;

  auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
  if (!TPC || skipModule(M))
    return false;

  for (Function &Func : M) {
    if (!Func.isDeclaration())
      continue;

    auto Iter = ScalarMASSFuncs.find(Func.getName());
    if (Iter == ScalarMASSFuncs.end())
      continue;

    // The call to createScalarMASSCall() invalidates the iterator over users
    // upon replacing the users. Precomputing the current list of users allows
    // us to replace all the call sites.
    SmallVector<User *, 4> TheUsers(Func.users());

    for (auto *User : TheUsers)
      if (auto *CI = dyn_cast_or_null<CallInst>(User)) {
        if (isCandidateSafeToLower(*CI))
          Changed |= createScalarMASSCall(Iter->second, *CI, Func);
      }
  }

  return Changed;
}

char PPCGenScalarMASSEntries::ID = 0;

char &toolchain::PPCGenScalarMASSEntriesID = PPCGenScalarMASSEntries::ID;

INITIALIZE_PASS(PPCGenScalarMASSEntries, DEBUG_TYPE,
                "Generate Scalar MASS entries", false, false)

ModulePass *toolchain::createPPCGenScalarMASSEntriesPass() {
  return new PPCGenScalarMASSEntries();
}
