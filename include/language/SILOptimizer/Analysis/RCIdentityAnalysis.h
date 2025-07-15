//===--- RCIdentityAnalysis.h -----------------------------------*- C++ -*-===//
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
//  This is an analysis that determines the ref count identity (i.e. gc root) of
//  a pointer. Any values with the same ref count identity are able to be
//  retained and released interchangeably.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_ANALYSIS_RCIDENTITYANALYSIS_H
#define LANGUAGE_SILOPTIMIZER_ANALYSIS_RCIDENTITYANALYSIS_H

#include "language/SIL/SILValue.h"
#include "language/SIL/SILArgument.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/PassManager/PassManager.h"

namespace language {

/// Limit the size of the rc identity cache. We keep a cache per function.
constexpr unsigned MaxRCIdentityCacheSize = 64;

class DominanceAnalysis;

/// This class is a simple wrapper around an identity cache.
class RCIdentityFunctionInfo {
  toolchain::DenseSet<SILArgument *> VisitedArgs;
  // RC identity cache.
  toolchain::DenseMap<SILValue, SILValue> RCCache;
  DominanceAnalysis *DA;

  /// This number is arbitrary and conservative. At some point if compile time
  /// is not an issue, this value should be made more aggressive (i.e. greater).
  enum { MaxRecursionDepth = 16 };

public:
  RCIdentityFunctionInfo(DominanceAnalysis *D) : VisitedArgs(),
  DA(D) {}

  SILValue getRCIdentityRoot(SILValue V);

  /// Return all recursive users of V, looking through users which propagate
  /// RCIdentity.
  ///
  /// *NOTE* This ignores obvious ARC escapes where the a potential
  /// user of the RC is not managed by ARC. For instance
  /// unchecked_trivial_bit_cast.
  void getRCUses(SILValue V, SmallVectorImpl<Operand *> &Uses);

  /// A helper method that calls getRCUses and then maps each operand to the
  /// operands user and then uniques the list.
  ///
  /// *NOTE* The routine asserts that the passed in Users array is empty for
  /// simplicity. If needed this can be changed, but it is not necessary given
  /// current uses.
  void getRCUsers(SILValue V, SmallVectorImpl<SILInstruction *> &Users);

  /// Like getRCUses except uses a callback to prevent the need for an
  /// intermediate array.
  void visitRCUses(SILValue V, function_ref<void(Operand *)> Visitor);

private:
  SILValue getRCIdentityRootInner(SILValue V, unsigned RecursionDepth);
  SILValue stripRCIdentityPreservingOps(SILValue V, unsigned RecursionDepth);
  SILValue stripRCIdentityPreservingArgs(SILValue V, unsigned RecursionDepth);
  SILValue stripOneRCIdentityIncomingValue(SILArgument *Arg, SILValue V);
  bool findDominatingNonPayloadedEdge(SILBasicBlock *IncomingEdgeBB,
                                      SILValue RCIdentity);
};

class RCIdentityAnalysis : public FunctionAnalysisBase<RCIdentityFunctionInfo> {
  DominanceAnalysis *DA;

public:
  RCIdentityAnalysis(SILModule *)
      : FunctionAnalysisBase<RCIdentityFunctionInfo>(
            SILAnalysisKind::RCIdentity),
        DA(nullptr) {}

  RCIdentityAnalysis(const RCIdentityAnalysis &) = delete;
  RCIdentityAnalysis &operator=(const RCIdentityAnalysis &) = delete;

  static bool classof(const SILAnalysis *S) {
    return S->getKind() == SILAnalysisKind::RCIdentity;
  }

  virtual void initialize(SILPassManager *PM) override;
  
  virtual std::unique_ptr<RCIdentityFunctionInfo>
  newFunctionAnalysis(SILFunction *F) override {
    return std::make_unique<RCIdentityFunctionInfo>(DA);
  }

  virtual bool shouldInvalidate(SILAnalysis::InvalidationKind K) override {
    return true;
  }

 };

} // end language namespace

#endif
