//===--- Analysis.cpp - Swift Analysis ------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"

using namespace language;

void SILAnalysis::verifyFunction(SILFunction *F) {
  // Only functions with bodies can be analyzed by the analysis.
  assert(F->isDefinition() && "Can't analyze external functions");
}

SILAnalysis *swift::createDominanceAnalysis(SILModule *) {
  return new DominanceAnalysis();
}

SILAnalysis *swift::createPostDominanceAnalysis(SILModule *) {
  return new PostDominanceAnalysis();
}

SILAnalysis *swift::createInductionVariableAnalysis(SILModule *M) {
  return new IVAnalysis(M);
}

SILAnalysis *swift::createPostOrderAnalysis(SILModule *M) {
  return new PostOrderAnalysis();
}

SILAnalysis *swift::createClassHierarchyAnalysis(SILModule *M) {
  return new ClassHierarchyAnalysis(M);
}

SILAnalysis *swift::createBasicCalleeAnalysis(SILModule *M) {
  return new BasicCalleeAnalysis(M);
}

SILAnalysis *swift::createProtocolConformanceAnalysis(SILModule *M) {
  return new ProtocolConformanceAnalysis(M);
}

SILAnalysis *swift::createNonLocalAccessBlockAnalysis(SILModule *M) {
  return new NonLocalAccessBlockAnalysis();
}
