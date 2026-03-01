//==-LTOInternalize.cpp - LLVM Link Time Optimizer Internalization Utility -==//
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
// This file defines a helper to run the internalization part of LTO.
//
//===----------------------------------------------------------------------===//

#include "vm/core/LTO/legacy/UpdateCompilerUsed.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/CodeGen/TargetLowering.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

using namespace vm::core;

namespace {

// Helper class that collects AsmUsed and user supplied libcalls.
class PreserveLibCallsAndAsmUsed {
public:
  PreserveLibCallsAndAsmUsed(const StringSet<> &AsmUndefinedRefs,
                             const TargetMachine &TM,
                             std::vector<GlobalValue *> &LLVMUsed)
      : AsmUndefinedRefs(AsmUndefinedRefs), TM(TM), LLVMUsed(LLVMUsed) {}

  void findInModule(Module &TheModule) {
    initializeLibCalls(TheModule);
    for (Function &F : TheModule)
      findLibCallsAndAsm(F);
    for (GlobalVariable &GV : TheModule.globals())
      findLibCallsAndAsm(GV);
    for (GlobalAlias &GA : TheModule.aliases())
      findLibCallsAndAsm(GA);
  }

private:
  // Inputs
  const StringSet<> &AsmUndefinedRefs;
  const TargetMachine &TM;

  // Temps
  toolchain::Mangler Mangler;
  StringSet<> Libcalls;

  // Output
  std::vector<GlobalValue *> &LLVMUsed;

  // Collect names of runtime library functions. User-defined functions with the
  // same names are added to toolchain.compiler.used to prevent them from being
  // deleted by optimizations.
  void initializeLibCalls(const Module &TheModule) {
    TargetLibraryInfoImpl TLII(TM.getTargetTriple(), TM.Options.VecLib);
    TargetLibraryInfo TLI(TLII);

    // TargetLibraryInfo has info on C runtime library calls on the current
    // target.
    for (unsigned I = LibFunc::Begin_LibFunc, E = LibFunc::End_LibFunc; I != E;
         ++I) {
      LibFunc F = static_cast<LibFunc>(I);
      if (TLI.has(F))
        Libcalls.insert(TLI.getName(F));
    }

    SmallPtrSet<const TargetLowering *, 1> TLSet;

    for (const Function &F : TheModule) {
      const TargetLowering *Lowering =
          TM.getSubtargetImpl(F)->getTargetLowering();

      if (Lowering && TLSet.insert(Lowering).second)
        // TargetLowering has info on library calls that CodeGen expects to be
        // available, both from the C runtime and compiler-rt.
        for (unsigned I = 0, E = static_cast<unsigned>(RTLIB::UNKNOWN_LIBCALL);
             I != E; ++I)
          if (const char *Name =
                  Lowering->getLibcallName(static_cast<RTLIB::Libcall>(I)))
            Libcalls.insert(Name);
    }
  }

  void findLibCallsAndAsm(GlobalValue &GV) {
    // There are no restrictions to apply to declarations.
    if (GV.isDeclaration())
      return;

    // There is nothing more restrictive than private linkage.
    if (GV.hasPrivateLinkage())
      return;

    // Conservatively append user-supplied runtime library functions (supplied
    // either directly, or via a function alias) to toolchain.compiler.used.  These
    // could be internalized and deleted by optimizations like -globalopt,
    // causing problems when later optimizations add new library calls (e.g.,
    // toolchain.memset => memset and printf => puts).
    // Leave it to the linker to remove any dead code (e.g. with -dead_strip).
    GlobalValue *FuncAliasee = nullptr;
    if (isa<GlobalAlias>(GV)) {
      auto *A = cast<GlobalAlias>(&GV);
      FuncAliasee = dyn_cast<Function>(A->getAliasee());
    }
    if ((isa<Function>(GV) || FuncAliasee) && Libcalls.count(GV.getName())) {
      LLVMUsed.push_back(&GV);
      return;
    }

    SmallString<64> Buffer;
    TM.getNameWithPrefix(Buffer, &GV, Mangler);
    if (AsmUndefinedRefs.count(Buffer))
      LLVMUsed.push_back(&GV);
  }
};

} // namespace anonymous

void toolchain::updateCompilerUsed(Module &TheModule, const TargetMachine &TM,
                              const StringSet<> &AsmUndefinedRefs) {
  std::vector<GlobalValue *> UsedValues;
  PreserveLibCallsAndAsmUsed(AsmUndefinedRefs, TM, UsedValues)
      .findInModule(TheModule);

  if (UsedValues.empty())
    return;

  appendToCompilerUsed(TheModule, UsedValues);
}
