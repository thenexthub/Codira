//===- InjectTLIMAppings.cpp - TLI to VFABI attribute injection  ----------===//
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
// Populates the VFABI attribute with the scalar-to-vector mappings
// from the TargetLibraryInfo.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/InjectTLIMappings.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/DemandedBits.h"
#include "vm/core/Analysis/GlobalsModRef.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/Analysis/VectorUtils.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/VFABIDemangler.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

using namespace vm::core;

#define DEBUG_TYPE "inject-tli-mappings"

STATISTIC(NumCallInjected,
          "Number of calls in which the mappings have been injected.");

STATISTIC(NumVFDeclAdded,
          "Number of function declarations that have been added.");
STATISTIC(NumCompUsedAdded,
          "Number of `@toolchain.compiler.used` operands that have been added.");

/// A helper function that adds the vector variant declaration for vectorizing
/// the CallInst \p CI with a vectorization factor of \p VF lanes. For each
/// mapping, TLI provides a VABI prefix, which contains all information required
/// to create vector function declaration.
static void addVariantDeclaration(CallInst &CI, const ElementCount &VF,
                                  const VecDesc *VD) {
  Module *M = CI.getModule();
  FunctionType *ScalarFTy = CI.getFunctionType();

  assert(!ScalarFTy->isVarArg() && "VarArg functions are not supported.");

  const std::optional<VFInfo> Info = VFABI::tryDemangleForVFABI(
      VD->getVectorFunctionABIVariantString(), ScalarFTy);

  assert(Info && "Failed to demangle vector variant");
  assert(Info->Shape.VF == VF && "Mangled name does not match VF");

  const StringRef VFName = VD->getVectorFnName();
  FunctionType *VectorFTy = VFABI::createFunctionType(*Info, ScalarFTy);
  Function *VecFunc =
      Function::Create(VectorFTy, Function::ExternalLinkage, VFName, M);
  VecFunc->copyAttributesFrom(CI.getCalledFunction());
  if (auto CC = VD->getCallingConv())
    VecFunc->setCallingConv(*CC);
  ++NumVFDeclAdded;
  LLVM_DEBUG(dbgs() << DEBUG_TYPE << ": Added to the module: `" << VFName
                    << "` of type " << *VectorFTy << "\n");

  // Make function declaration (without a body) "sticky" in the IR by
  // listing it in the @toolchain.compiler.used intrinsic.
  assert(!VecFunc->size() && "VFABI attribute requires `@toolchain.compiler.used` "
                             "only on declarations.");
  appendToCompilerUsed(*M, {VecFunc});
  LLVM_DEBUG(dbgs() << DEBUG_TYPE << ": Adding `" << VFName
                    << "` to `@toolchain.compiler.used`.\n");
  ++NumCompUsedAdded;
}

static void addMappingsFromTLI(const TargetLibraryInfo &TLI, CallInst &CI) {
  // This is needed to make sure we don't query the TLI for calls to
  // bitcast of function pointers, like `%call = call i32 (i32*, ...)
  // bitcast (i32 (...)* @goo to i32 (i32*, ...)*)(i32* nonnull %i)`,
  // as such calls make the `isFunctionVectorizable` raise an
  // exception.
  if (CI.isNoBuiltin() || !CI.getCalledFunction())
    return;

  StringRef ScalarName = CI.getCalledFunction()->getName();

  // Nothing to be done if the TLI thinks the function is not
  // vectorizable.
  if (!TLI.isFunctionVectorizable(ScalarName))
    return;
  SmallVector<std::string, 8> Mappings;
  VFABI::getVectorVariantNames(CI, Mappings);
  Module *M = CI.getModule();
  const SetVector<StringRef> OriginalSetOfMappings(toolchain::from_range, Mappings);

  auto AddVariantDecl = [&](const ElementCount &VF, bool Predicate) {
    const VecDesc *VD = TLI.getVectorMappingInfo(ScalarName, VF, Predicate);
    if (VD && !VD->getVectorFnName().empty()) {
      std::string MangledName = VD->getVectorFunctionABIVariantString();
      if (!OriginalSetOfMappings.count(MangledName)) {
        Mappings.push_back(MangledName);
        ++NumCallInjected;
      }
      Function *VariantF = M->getFunction(VD->getVectorFnName());
      if (!VariantF)
        addVariantDeclaration(CI, VF, VD);
    }
  };

  //  All VFs in the TLI are powers of 2.
  ElementCount WidestFixedVF, WidestScalableVF;
  TLI.getWidestVF(ScalarName, WidestFixedVF, WidestScalableVF);

  for (bool Predicated : {false, true}) {
    for (ElementCount VF = ElementCount::getFixed(2);
         ElementCount::isKnownLE(VF, WidestFixedVF); VF *= 2)
      AddVariantDecl(VF, Predicated);

    for (ElementCount VF = ElementCount::getScalable(2);
         ElementCount::isKnownLE(VF, WidestScalableVF); VF *= 2)
      AddVariantDecl(VF, Predicated);
  }

  VFABI::setVectorVariantNames(&CI, Mappings);
}

static bool runImpl(const TargetLibraryInfo &TLI, Function &F) {
  for (auto &I : instructions(F))
    if (auto CI = dyn_cast<CallInst>(&I))
      addMappingsFromTLI(TLI, *CI);
  // Even if the pass adds IR attributes, the analyses are preserved.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// New pass manager implementation.
////////////////////////////////////////////////////////////////////////////////
PreservedAnalyses InjectTLIMappings::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  const TargetLibraryInfo &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  runImpl(TLI, F);
  // Even if the pass adds IR attributes, the analyses are preserved.
  return PreservedAnalyses::all();
}
