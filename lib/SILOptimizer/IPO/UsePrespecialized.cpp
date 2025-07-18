//===--- UsePrespecialized.cpp - use pre-specialized functions ------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "use-prespecialized"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/Generics.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "language/SILOptimizer/Utils/SpecializationMangler.h"
#include "toolchain/Support/Debug.h"

using namespace language;

namespace {



static void collectApplyInst(SILFunction &F,
                             toolchain::SmallVectorImpl<ApplySite> &NewApplies) {
  // Scan all of the instructions in this function in search of ApplyInsts.
  for (auto &BB : F)
    for (auto &I : BB)
      if (ApplySite AI = ApplySite::isa(&I))
        NewApplies.push_back(AI);
}

/// A simple pass which replaces each apply of a generic function by an apply
/// of the corresponding pre-specialized function, if such a pre-specialization
/// exists.
class UsePrespecialized: public SILModuleTransform {
  ~UsePrespecialized() override { }

  void run() override {
    auto &M = *getModule();
    for (auto &F : M) {
      if (replaceByPrespecialized(F)) {
        invalidateAnalysis(&F, SILAnalysis::InvalidationKind::FunctionBody);
      }
    }
  }

  bool replaceByPrespecialized(SILFunction &F);
};

} // end anonymous namespace

// Analyze the function and replace each apply of
// a generic function by an apply of the corresponding
// pre-specialized function, if such a pre-specialization exists.
bool UsePrespecialized::replaceByPrespecialized(SILFunction &F) {
  bool Changed = false;
  auto &M = F.getModule();

  toolchain::SmallVector<ApplySite, 16> NewApplies;
  collectApplyInst(F, NewApplies);

  for (auto &AI : NewApplies) {
    auto *ReferencedF = AI.getReferencedFunctionOrNull();
    if (!ReferencedF)
      continue;

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Trying to use specialized function for:\n";
               AI.getInstruction()->dumpInContext());

    // Check if it is a call of a generic function.
    // If this is the case, check if there is a specialization
    // available for it already and use this specialization
    // instead of the generic version.
    if (!AI.hasSubstitutions())
      continue;

    SubstitutionMap Subs = AI.getSubstitutionMap();

    // Bail if the replacement types depend on the callee's generic
    // environment.
    //
    // TODO: Remove this limitation once public partial specializations
    // are supported and can be provided by other modules.
    if (Subs.getRecursiveProperties().hasArchetype())
      continue;

    ReabstractionInfo ReInfo(M.getCodiraModule(), M.isWholeModule(), AI,
                             ReferencedF, Subs, IsNotSerialized,
                             /*ConvertIndirectToDirect=*/ true,
                             /*dropUnusedArguments=*/ false);

    if (!ReInfo.canBeSpecialized())
      continue;

    auto SpecType = ReInfo.getSpecializedType();
    // Bail if any generic types parameters of the concrete type
    // are unbound.
    if (SpecType->hasArchetype())
      continue;

    // Create a name of the specialization. All external pre-specializations
    // are serialized without bodies. Thus use IsNotSerialized here.
    Mangle::GenericSpecializationMangler NewGenericMangler(M.getASTContext(), ReferencedF,
                                                           IsNotSerialized);
    std::string ClonedName = NewGenericMangler.mangleReabstracted(Subs,
       ReInfo.needAlternativeMangling());
      
    SILFunction *NewF = nullptr;
    // If we already have this specialization, reuse it.
    auto PrevF = M.lookUpFunction(ClonedName);
    if (PrevF) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found a specialization: " << ClonedName
                              << "\n");
      NewF = PrevF;
    }

    if (!PrevF || !NewF) {
      // Check for the existence of this function in another module without
      // loading the function body.
      PrevF = lookupPrespecializedSymbol(M, ClonedName);
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Checked if there is a specialization in a "
                                 "different module: "
                              << PrevF << "\n");
      if (!PrevF)
        continue;
      assert(PrevF->isExternalDeclaration() &&
             "Prespecialized function should be an external declaration");
      NewF = PrevF;
    }

    if (!NewF)
      continue;

    // An existing specialization was found.
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found a specialization of "
                            << ReferencedF->getName()
                            << " : " << NewF->getName() << "\n");

    auto NewAI = replaceWithSpecializedFunction(AI, NewF, ReInfo);
    switch (AI.getKind()) {
    case ApplySiteKind::ApplyInst:
      cast<ApplyInst>(AI)->replaceAllUsesWith(cast<ApplyInst>(NewAI));
      break;
    case ApplySiteKind::PartialApplyInst:
      cast<PartialApplyInst>(AI)->replaceAllUsesWith(
          cast<PartialApplyInst>(NewAI));
      break;
    case ApplySiteKind::TryApplyInst:
    case ApplySiteKind::BeginApplyInst:
      break;
    }
    recursivelyDeleteTriviallyDeadInstructions(AI.getInstruction(), true);
    Changed = true;
  }

  return Changed;
}


SILTransform *language::createUsePrespecialized() {
  return new UsePrespecialized();
}
