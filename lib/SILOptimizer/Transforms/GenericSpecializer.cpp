//===--- GenericSpecializer.cpp - Specialization of generic functions -----===//
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
//
// Specialize calls to generic functions by substituting static type
// information.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-generic-specializer"

#include "language/AST/AvailabilityInference.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/OptimizationRemark.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/BasicBlockOptUtils.h"
#include "language/SILOptimizer/Utils/ConstantFolding.h"
#include "language/SILOptimizer/Utils/Devirtualize.h"
#include "language/SILOptimizer/Utils/Generics.h"
#include "language/SILOptimizer/Utils/InstructionDeleter.h"
#include "language/SILOptimizer/Utils/SILInliner.h"
#include "language/SILOptimizer/Utils/SILOptFunctionBuilder.h"
#include "language/SILOptimizer/Utils/StackNesting.h"
#include "toolchain/ADT/SmallVector.h"

using namespace language;

namespace {
static void transferSpecializeAttributeTargets(SILModule &M,
                                               SILOptFunctionBuilder &builder,
                                               Decl *d) {
  auto *vd = cast<AbstractFunctionDecl>(d);
  for (auto *A : vd->getAttrs().getAttributes<AbstractSpecializeAttr>()) {
    auto *SA = cast<AbstractSpecializeAttr>(A);
    // Filter _spi.
    auto spiGroups = SA->getSPIGroups();
    auto hasSPIGroup = !spiGroups.empty();
    if (hasSPIGroup) {
      if (vd->getModuleContext() != M.getCodiraModule() &&
          !M.getCodiraModule()->isImportedAsSPI(SA, vd)) {
        continue;
      }
    }
    if (auto *targetFunctionDecl = SA->getTargetFunctionDecl(vd)) {
      auto kind = SA->getSpecializationKind() ==
                          SpecializeAttr::SpecializationKind::Full
                      ? SILSpecializeAttr::SpecializationKind::Full
                      : SILSpecializeAttr::SpecializationKind::Partial;
      Identifier spiGroupIdent;
      if (hasSPIGroup) {
        spiGroupIdent = spiGroups[0];
      }
      auto availability = AvailabilityInference::annotatedAvailableRangeForAttr(
          vd, SA, M.getCodiraModule()->getASTContext());

      auto *attr = SILSpecializeAttr::create(
          M, SA->getSpecializedSignature(vd), SA->getTypeErasedParams(),
          SA->isExported(), kind, nullptr,
          spiGroupIdent, vd->getModuleContext(), availability);

      auto target = SILDeclRef(targetFunctionDecl);
      std::string targetName = target.mangle();
      if (SILFunction *targetSILFunction = M.lookUpFunction(targetName)) {
        targetSILFunction->addSpecializeAttr(attr);
      } else {
        M.addPendingSpecializeAttr(targetName, attr);
      }
    }
  }
}
} // end anonymous namespace

bool language::specializeAppliesInFunction(SILFunction &F,
                                        SILTransform *transform,
                                        bool isMandatory) {
  SILOptFunctionBuilder FunctionBuilder(*transform);
  DeadInstructionSet DeadApplies;
  toolchain::SmallSetVector<SILInstruction *, 8> Applies;
  OptRemark::Emitter ORE(DEBUG_TYPE, F);

  bool Changed = false;
  for (auto &BB : F) {
    // Collect the applies for this block in reverse order so that we
    // can pop them off the end of our vector and process them in
    // forward order.
    for (auto &I : toolchain::reverse(BB)) {

      // Skip non-apply instructions, apply instructions with no
      // substitutions, apply instructions where we do not statically
      // know the called function, and apply instructions where we do
      // not have the body of the called function.
      ApplySite Apply = ApplySite::isa(&I);
      if (!Apply || !Apply.hasSubstitutions())
        continue;

      auto *Callee = Apply.getReferencedFunctionOrNull();
      if (!Callee)
        continue;

      FunctionBuilder.getModule().performOnceForPrespecializedImportedExtensions(
        [&FunctionBuilder](AbstractFunctionDecl *pre) {
        transferSpecializeAttributeTargets(FunctionBuilder.getModule(), FunctionBuilder,
                                           pre);
        });

      if (!Callee->isDefinition() && !Callee->hasPrespecialization()) {
        ORE.emit([&]() {
          using namespace OptRemark;
          return RemarkMissed("NoDef", I)
                 << "Unable to specialize generic function "
                 << NV("Callee", Callee) << " since definition is not visible";
        });
        continue;
      }

      Applies.insert(Apply.getInstruction());
    }

    // Attempt to specialize each apply we collected, deleting any
    // that we do specialize (along with other instructions we clone
    // in the process of doing so). We pop from the end of the list to
    // avoid tricky iterator invalidation issues.
    while (!Applies.empty()) {
      auto *I = Applies.pop_back_val();
      auto Apply = ApplySite::isa(I);
      assert(Apply && "Expected an apply!");
      SILFunction *Callee = Apply.getReferencedFunctionOrNull();
      assert(Callee && "Expected to have a known callee");

      if (!Apply.canOptimize())
        continue;

      if (!isMandatory && !Callee->shouldOptimize())
        continue;

      // We have a call that can potentially be specialized, so
      // attempt to do so.
      toolchain::SmallVector<SILFunction *, 2> NewFunctions;
      trySpecializeApplyOfGeneric(FunctionBuilder, Apply, DeadApplies,
                                  NewFunctions, ORE, isMandatory);

      // Remove all the now-dead applies. We must do this immediately
      // rather than defer it in order to avoid problems with cloning
      // dead instructions when doing recursive specialization.
      while (!DeadApplies.empty()) {
        auto *AI = DeadApplies.pop_back_val();

        // Remove any applies we are deleting so that we don't attempt
        // to specialize them.
        Applies.remove(AI);

        recursivelyDeleteTriviallyDeadInstructions(AI, true);
        Changed = true;
      }

      if (auto *sft = dyn_cast<SILFunctionTransform>(transform)) {
        // If calling the specialization utility resulted in new functions
        // (as opposed to returning a previous specialization), we need to notify
        // the pass manager so that the new functions get optimized.
        for (SILFunction *NewF : reverse(NewFunctions)) {
          sft->addFunctionToPassManagerWorklist(NewF, Callee);
        }
      }
    }
  }

  return Changed;
}

namespace {

/// The generic specializer, used in the optimization pipeline.
class GenericSpecializer : public SILFunctionTransform {

  /// The entry point to the transformation.
  void run() override {
    SILFunction &F = *getFunction();

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "***** GenericSpecializer on function:"
                            << F.getName() << " *****\n");

    if (specializeAppliesInFunction(F, this, /*isMandatory*/ false)) {
      invalidateAnalysis(SILAnalysis::InvalidationKind::FunctionBody);
    }
  }
};

} // end anonymous namespace

SILTransform *language::createGenericSpecializer() {
  return new GenericSpecializer();
}
