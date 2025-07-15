//===--- LoopAnalysis.cpp - SIL Loop Analysis -----------------------------===//
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

#include "language/Basic/Assertions.h"
#include "language/SIL/Dominance.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/Analysis/LoopAnalysis.h"
#include "language/SILOptimizer/PassManager/PassManager.h"
#include "toolchain/Support/Debug.h"

using namespace language;

std::unique_ptr<SILLoopInfo>
SILLoopAnalysis::newFunctionAnalysis(SILFunction *F) {
  assert(DA != nullptr && "Expect a valid dominance analysis");
  DominanceInfo *DT = DA->get(F);
  assert(DT != nullptr && "Expect a valid dominance information");
  return std::make_unique<SILLoopInfo>(F, DT);
}

void SILLoopAnalysis::initialize(SILPassManager *PM) {
  DA = PM->getAnalysis<DominanceAnalysis>();
}

SILAnalysis *language::createLoopAnalysis(SILModule *M) {
  return new SILLoopAnalysis(M);
}
