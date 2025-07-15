//===--- PassManagerVerifierAnalysis.h ------------------------------------===//
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

#ifndef LANGUAGE_SILOPTIMIZER_ANALYSIS_PASSMANAGERVERIFIERANALYSIS_H
#define LANGUAGE_SILOPTIMIZER_ANALYSIS_PASSMANAGERVERIFIERANALYSIS_H

#include "language/SIL/SILFunction.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "toolchain/ADT/StringSet.h"

namespace language {

/// An analysis that validates that the pass manager properly sends add/delete
/// messages as functions are added/deleted from the module.
///
/// All methods are no-ops when asserts are disabled.
class PassManagerVerifierAnalysis : public SILAnalysis {
  /// The module that we are processing.
  TOOLCHAIN_ATTRIBUTE_UNUSED
  SILModule &mod;

  /// The set of "live" functions that we are tracking. We store the names of
  /// the functions so that if a function is deleted we do not need to touch its
  /// memory to get its name.
  ///
  /// All functions in mod must be in liveFunctions and vis-a-versa.
  toolchain::StringSet<> liveFunctionNames;

public:
  PassManagerVerifierAnalysis(SILModule *mod);

  static bool classof(const SILAnalysis *analysis) {
    return analysis->getKind() == SILAnalysisKind::PassManagerVerifier;
  }

  /// Validate that the analysis is able to look up all functions and that those
  /// functions are live.
  void invalidate() override final;

  /// Validate that the analysis is able to look up the given function.
  void invalidate(SILFunction *f, InvalidationKind k) override final;

  /// If a function has not yet been seen start tracking it.
  void notifyAddedOrModifiedFunction(SILFunction *f) override final;

  /// Stop tracking a function.
  void notifyWillDeleteFunction(SILFunction *f) override final;

  /// Make sure that when we invalidate a function table, make sure we can find
  /// all functions for all witness tables.
  void invalidateFunctionTables() override final;

  /// Run the entire verification.
  void verifyFull() const override final;
};

} // namespace language

#endif
