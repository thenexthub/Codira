//===--- SemanticARCOpts.cpp ----------------------------------------------===//
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

#define DEBUG_TYPE "sil-semantic-arc-opts"

#include "SemanticARCOpts.h"
#include "SemanticARCOptVisitor.h"
#include "Transforms.h"

#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/Analysis/DeadEndBlocksAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/OwnershipOptUtils.h"

#include "toolchain/Support/CommandLine.h"

using namespace language;
using namespace language::semanticarc;

static toolchain::cl::list<ARCTransformKind> TransformsToPerform(
    toolchain::cl::values(
        clEnumValN(ARCTransformKind::AllPeepholes,
                   "sil-semantic-arc-peepholes-all",
                   "Perform All ARC canonicalizations and peepholes"),
        clEnumValN(ARCTransformKind::LoadCopyToLoadBorrowPeephole,
                   "sil-semantic-arc-peepholes-loadcopy-to-loadborrow",
                   "Perform the load [copy] to load_borrow peephole"),
        clEnumValN(ARCTransformKind::RedundantBorrowScopeElimPeephole,
                   "sil-semantic-arc-peepholes-redundant-borrowscope-elim",
                   "Perform the redundant borrow scope elimination peephole"),
        clEnumValN(ARCTransformKind::RedundantCopyValueElimPeephole,
                   "sil-semantic-arc-peepholes-redundant-copyvalue-elim",
                   "Perform the redundant copy_value peephole"),
        clEnumValN(ARCTransformKind::LifetimeJoiningPeephole,
                   "sil-semantic-arc-peepholes-lifetime-joining",
                   "Perform the join lifetimes peephole"),
        clEnumValN(ARCTransformKind::OwnershipConversionElimPeephole,
                   "sil-semantic-arc-peepholes-ownership-conversion-elim",
                   "Eliminate unchecked_ownership_conversion insts that are "
                   "not needed"),
        clEnumValN(ARCTransformKind::OwnedToGuaranteedPhi,
                   "sil-semantic-arc-owned-to-guaranteed-phi",
                   "Perform Owned To Guaranteed Phi. NOTE: Seeded by peephole "
                   "optimizer for compile time saving purposes, so run this "
                   "after running peepholes)"),
        clEnumValN(ARCTransformKind::RedundantMoveValueElim,
                   "sil-semantic-arc-redundant-move-value-elim",
                   "Eliminate move_value which don't change owned lifetime "
                   "characteristics.  (Escaping, Lexical).")),
    toolchain::cl::desc(
        "For testing purposes only run the specified list of semantic arc "
        "optimization. If the list is empty, we run all transforms"));

//===----------------------------------------------------------------------===//
//                            Top Level Entrypoint
//===----------------------------------------------------------------------===//

namespace {

// Even though this is a mandatory pass, it is rerun after deserialization in
// case DiagnosticConstantPropagation exposed anything new in this assert
// configuration.
struct SemanticARCOpts : SILFunctionTransform {
  bool mandatoryOptsOnly;

  SemanticARCOpts(bool mandatoryOptsOnly)
      : mandatoryOptsOnly(mandatoryOptsOnly) {}

  void performCommandlineSpecifiedTransforms(SemanticARCOptVisitor &visitor) {
    for (auto transform : TransformsToPerform) {
      visitor.ctx.transformKind = transform;
      LANGUAGE_DEFER {
        visitor.ctx.transformKind = ARCTransformKind::Invalid;
        visitor.reset();
      };
      switch (transform) {
      case ARCTransformKind::LifetimeJoiningPeephole:
      case ARCTransformKind::RedundantCopyValueElimPeephole:
      case ARCTransformKind::RedundantBorrowScopeElimPeephole:
      case ARCTransformKind::LoadCopyToLoadBorrowPeephole:
      case ARCTransformKind::AllPeepholes:
      case ARCTransformKind::OwnershipConversionElimPeephole:
      case ARCTransformKind::RedundantMoveValueElim:
        // We never assume we are at fixed point when running these transforms.
        if (performPeepholesWithoutFixedPoint(visitor)) {
          invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
        }
        continue;
      case ARCTransformKind::OwnedToGuaranteedPhi:
        if (tryConvertOwnedPhisToGuaranteedPhis(visitor.ctx)) {
          invalidateAnalysis(
              SILAnalysis::InvalidationKind::BranchesAndInstructions);
        }
        continue;
      case ARCTransformKind::All:
      case ARCTransformKind::Invalid:
        toolchain_unreachable("unsupported option");
      }
    }
  }

  bool performPeepholesWithoutFixedPoint(SemanticARCOptVisitor &visitor) {
    // Add all the results of all instructions that we want to visit to the
    // worklist.
    for (auto &block : *getFunction()) {
      for (auto &inst : block) {
        if (SemanticARCOptVisitor::shouldVisitInst(&inst)) {
          for (SILValue v : inst.getResults()) {
            visitor.worklist.insert(v);
          }
        }
      }
    }
    // Then process the worklist, performing peepholes.
    return visitor.optimizeWithoutFixedPoint();
  }

  bool performPeepholes(SemanticARCOptVisitor &visitor) {
    // Add all the results of all instructions that we want to visit to the
    // worklist.
    for (auto &block : *getFunction()) {
      for (auto &inst : block) {
        if (SemanticARCOptVisitor::shouldVisitInst(&inst)) {
          for (SILValue v : inst.getResults()) {
            visitor.worklist.insert(v);
          }
        }
      }
    }
    // Then process the worklist, performing peepholes.
    return visitor.optimize();
  }

  void run() override {
    SILFunction &f = *getFunction();

    // Return early if we are not performing OSSA optimizations or we are not in
    // ownership.
    if (!f.getModule().getOptions().EnableOSSAOptimizations ||
        !f.hasOwnership())
      return;

    // Make sure we are running with ownership verification enabled.
    assert(f.getModule().getOptions().VerifySILOwnership &&
           "Can not perform semantic arc optimization unless ownership "
           "verification is enabled");

    auto *deBlocksAnalysis = getAnalysis<DeadEndBlocksAnalysis>();
    SemanticARCOptVisitor visitor(f, getPassManager(), *deBlocksAnalysis->get(&f),
                                  mandatoryOptsOnly);

    // If we are being asked for testing purposes to run a series of transforms
    // expressed on the command line, run that and return.
    if (!TransformsToPerform.empty()) {
      return performCommandlineSpecifiedTransforms(visitor);
    }

    // Otherwise, perform our standard optimizations.
    bool didEliminateARCInsts = performPeepholes(visitor);

    // Now that we have seeded the map of phis to incoming values that could be
    // converted to guaranteed, ignoring the phi, try convert those phis to be
    // guaranteed.
    if (tryConvertOwnedPhisToGuaranteedPhis(visitor.ctx)) {
      updateAllGuaranteedPhis(getPassManager(), &f);

      // We return here early to save a little compile time so we do not
      // invalidate analyses redundantly.
      return invalidateAnalysis(
          SILAnalysis::InvalidationKind::BranchesAndInstructions);
    }

    // Otherwise, we only deleted instructions and did not touch phis.
    if (didEliminateARCInsts) {
      updateAllGuaranteedPhis(getPassManager(), &f);
      invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
    }
  }
};

} // end anonymous namespace

SILTransform *language::createSemanticARCOpts() {
  return new SemanticARCOpts(false /*mandatory*/);
}

SILTransform *language::createMandatoryARCOpts() {
  return new SemanticARCOpts(true /*mandatory*/);
}
