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

#ifndef LANGUAGE_SILOPTIMIZER_ANALYSIS_FUNCTIONORDER_H
#define LANGUAGE_SILOPTIMIZER_ANALYSIS_FUNCTIONORDER_H

#include "language/SILOptimizer/Analysis/BasicCalleeAnalysis.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/TinyPtrVector.h"

namespace language {

class BasicCalleeAnalysis;
class SILFunction;
class SILModule;

class BottomUpFunctionOrder {
public:
  typedef TinyPtrVector<SILFunction *> SCC;

private:
  SILModule &M;
  toolchain::SmallVector<SCC, 32> TheSCCs;
  toolchain::SmallVector<SILFunction *, 32> TheFunctions;

  // The callee analysis we use to determine the callees at each call site.
  BasicCalleeAnalysis *BCA;

  unsigned NextDFSNum;
  toolchain::DenseMap<SILFunction *, unsigned> DFSNum;
  toolchain::DenseMap<SILFunction *, unsigned> MinDFSNum;
  toolchain::SmallSetVector<SILFunction *, 4> DFSStack;

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
