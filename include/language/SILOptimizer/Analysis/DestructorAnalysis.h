//===--- DestructorAnalysis.h -----------------------------------*- C++ -*-===//
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
#ifndef SWIFT_SILOPTIMIZER_ANALYSIS_DESTRUCTORANALYSIS_H
#define SWIFT_SILOPTIMIZER_ANALYSIS_DESTRUCTORANALYSIS_H

#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "llvm/ADT/DenseMap.h"

namespace language {

/// This analysis determines memory effects during destruction.
class DestructorAnalysis : public SILAnalysis {
  SILModule *Mod;
  llvm::DenseMap<CanType, bool> Cached;
public:
  DestructorAnalysis(SILModule *M)
      : SILAnalysis(SILAnalysisKind::Destructor), Mod(M) {}

  static bool classof(const SILAnalysis *S) {
    return S->getKind() == SILAnalysisKind::Destructor;
  }

  /// Returns true if destruction of T may store to memory.
  bool mayStoreToMemoryOnDestruction(SILType T);

  /// No invalidation is needed.
  virtual void invalidate() override {
    // Nothing can invalidate, because types are static and cannot be changed
    // during the SIL pass pipeline.
  }

  /// No invalidation is needed.
  virtual void invalidate(SILFunction *F, InvalidationKind K)  override {
    // Nothing can invalidate, because types are static and cannot be changed
    // during the SIL pass pipeline.
  }

  /// Notify the analysis about a newly created function.
  virtual void notifyAddedOrModifiedFunction(SILFunction *F) override {}

  /// Notify the analysis about a function which will be deleted from the
  /// module.
  virtual void notifyWillDeleteFunction(SILFunction *F) override {}

  /// Notify the analysis about changed witness or vtables.
  virtual void invalidateFunctionTables() override { }

protected:
  bool cacheResult(CanType Type, bool Result);
  bool isSafeType(CanType Ty);
  bool implementsDestructorSafeContainerProtocol(NominalTypeDecl *NomDecl);
  bool areTypeParametersSafe(CanType Ty);
  ASTContext &getASTContext();
};
}
#endif
