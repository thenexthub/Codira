//===--- FunctionOrder.h - Utilities for function ordering  -----*- C++ -*-===//
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

#ifndef SWIFT_SILOPTIMIZER_ANALYSIS_FUNCTIONORDER_H
#define SWIFT_SILOPTIMIZER_ANALYSIS_FUNCTIONORDER_H

#include "language/SILOptimizer/Analysis/BasicCalleeAnalysis.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/TinyPtrVector.h"

namespace language {

class BasicCalleeAnalysis;
class SILFunction;
class SILModule;

class BottomUpFunctionOrder {
public:
  typedef TinyPtrVector<SILFunction *> SCC;

private:
  SILModule &M;
  llvm::SmallVector<SCC, 32> TheSCCs;
  llvm::SmallVector<SILFunction *, 32> TheFunctions;

  // The callee analysis we use to determine the callees at each call site.
  BasicCalleeAnalysis *BCA;

  unsigned NextDFSNum;
  llvm::DenseMap<SILFunction *, unsigned> DFSNum;
  llvm::DenseMap<SILFunction *, unsigned> MinDFSNum;
  llvm::SmallSetVector<SILFunction *, 4> DFSStack;

public:
  BottomUpFunctionOrder(SILModule &M, BasicCalleeAnalysis *BCA)
      : M(M), BCA(BCA), NextDFSNum(0) {}

  /// Get the SCCs in bottom-up order.
  ArrayRef<SCC> getSCCs() {
    if (!TheSCCs.empty())
      return TheSCCs;

    FindSCCs(M);
    return TheSCCs;
  }

  /// Get a flattened view of all functions in all the SCCs in
  /// bottom-up order
  ArrayRef<SILFunction *> getFunctions() {
    if (!TheFunctions.empty())
      return TheFunctions;

    for (auto SCC : getSCCs())
      for (auto *F : SCC)
        TheFunctions.push_back(F);

    return TheFunctions;
  }

private:
  void DFS(SILFunction *F);
  void FindSCCs(SILModule &M);
};

} // end namespace language

#endif
