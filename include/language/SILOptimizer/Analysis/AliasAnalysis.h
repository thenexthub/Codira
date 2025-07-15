//===--- AliasAnalysis.h - SIL Alias Analysis -------------------*- C++ -*-===//
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

#ifndef LANGUAGE_SILOPTIMIZER_ANALYSIS_ALIASANALYSIS_H
#define LANGUAGE_SILOPTIMIZER_ANALYSIS_ALIASANALYSIS_H

#include "language/SIL/ApplySite.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "toolchain/ADT/DenseMap.h"

namespace language {

/// This class is a simple wrapper around an alias analysis cache. This is
/// needed since we do not have an "analysis" infrastructure.
///
/// This wrapper sits above the CodiraCompilerSource implementation of
/// AliasAnalysis. The implementation calls into AliasAnalysis.code via
/// BridgedAliasAnalysis whenever the result may depend on escape analysis.
class AliasAnalysis {
private:
  void *languageSpecificData[4];

  SILPassManager *PM;

  void initCodiraSpecificData();

public:
  AliasAnalysis(SILPassManager *PM) : PM(PM) {
    initCodiraSpecificData();
  }

  ~AliasAnalysis();

  static SILAnalysisKind getAnalysisKind() { return SILAnalysisKind::Alias; }

  /// Convenience method that returns true if V1, V2 may alias.
  bool mayAlias(SILValue V1, SILValue V2);

  /// Compute the effects of Inst's memory behavior on the memory pointed to by
  /// the value V.
  MemoryBehavior computeMemoryBehavior(SILInstruction *Inst, SILValue V);

  /// Returns true if \p Inst may read from memory at address \p V.
  ///
  /// For details see MemoryBehavior::MayRead.
  bool mayReadFromMemory(SILInstruction *Inst, SILValue V) {
    auto B = computeMemoryBehavior(Inst, V);
    return B == MemoryBehavior::MayRead ||
           B == MemoryBehavior::MayReadWrite ||
           B == MemoryBehavior::MayHaveSideEffects;
  }

  /// Returns true if \p Inst may write to memory or deinitialize memory at
  /// address \p V.
  ///
  /// For details see MemoryBehavior::MayWrite.
  bool mayWriteToMemory(SILInstruction *Inst, SILValue V) {
    auto B = computeMemoryBehavior(Inst, V);
    return B == MemoryBehavior::MayWrite ||
           B == MemoryBehavior::MayReadWrite ||
           B == MemoryBehavior::MayHaveSideEffects;
  }

  /// Returns true if \p Inst may read from memory, write to memory or
  /// deinitialize memory at address \p V.
  ///
  /// For details see MemoryBehavior.
  bool mayReadOrWriteMemory(SILInstruction *Inst, SILValue V) {
    auto B = computeMemoryBehavior(Inst, V);
    return MemoryBehavior::None != B;
  }

  /// Returns true if \p Ptr may be released in the function call \p FAS.
  bool canApplyDecrementRefCount(FullApplySite FAS, SILValue Ptr);

  /// Returns true if \p Ptr may be released by the builtin \p BI.
  bool canBuiltinDecrementRefCount(BuiltinInst *BI, SILValue Ptr);

  /// Returns true if the object(s of) `obj` can escape to `toInst`.
  ///
  /// Special entry point into BridgedAliasAnalysis (escape analysis) for use in
  /// ARC analysis.
  bool isObjectReleasedByInst(SILValue obj, SILInstruction *toInst);

  /// Is the `addr` within all reachable objects/addresses, when start walking
  /// from `obj`?
  ///
  /// Special entry point into BridgedAliasAnalysis (escape analysis) for use in
  /// ARC analysis.
  bool isAddrVisibleFromObject(SILValue addr, SILValue obj);
};

} // end namespace language

#endif
