//===--- Analysis.cpp - Codira Analysis ------------------------------------===//
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

#define DEBUG_TYPE "sil-analysis"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/AST/Module.h"
#include "language/AST/SILOptions.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/Analysis/BasicCalleeAnalysis.h"
#include "language/SILOptimizer/Analysis/ClassHierarchyAnalysis.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/Analysis/IVAnalysis.h"
#include "language/SILOptimizer/Analysis/NonLocalAccessBlockAnalysis.h"
#include "language/SILOptimizer/Analysis/PostOrderAnalysis.h"
#include "language/SILOptimizer/Analysis/ProtocolConformanceAnalysis.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/Support/Debug.h"

using namespace language;

void SILAnalysis::verifyFunction(SILFunction *F) {
  // Only functions with bodies can be analyzed by the analysis.
  assert(F->isDefinition() && "Can't analyze external functions");
}

SILAnalysis *language::createDominanceAnalysis(SILModule *) {
  return new DominanceAnalysis();
}

SILAnalysis *language::createPostDominanceAnalysis(SILModule *) {
  return new PostDominanceAnalysis();
}

SILAnalysis *language::createInductionVariableAnalysis(SILModule *M) {
  return new IVAnalysis(M);
}

SILAnalysis *language::createPostOrderAnalysis(SILModule *M) {
  return new PostOrderAnalysis();
}

SILAnalysis *language::createClassHierarchyAnalysis(SILModule *M) {
  return new ClassHierarchyAnalysis(M);
}

SILAnalysis *language::createBasicCalleeAnalysis(SILModule *M) {
  return new BasicCalleeAnalysis(M);
}

SILAnalysis *language::createProtocolConformanceAnalysis(SILModule *M) {
  return new ProtocolConformanceAnalysis(M);
}

SILAnalysis *language::createNonLocalAccessBlockAnalysis(SILModule *M) {
  return new NonLocalAccessBlockAnalysis();
}
