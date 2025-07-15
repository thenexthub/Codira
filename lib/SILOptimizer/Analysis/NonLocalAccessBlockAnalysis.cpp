//===--- NonLocalAccessBlockAnalysis.cpp  - Nonlocal end_access -----------===//
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

#include "language/SILOptimizer/Analysis/NonLocalAccessBlockAnalysis.h"
#include "language/SIL/SILFunction.h"

using namespace language;

// Populate this->accessBlocks with all blocks containing a non-local
// end_access.
void NonLocalAccessBlocks::compute() {
  for (SILBasicBlock &block : *this->function) {
    for (SILInstruction &inst : block) {
      if (auto *endAccess = dyn_cast<EndAccessInst>(&inst)) {
        if (endAccess->getBeginAccess()->getParent() != endAccess->getParent())
          this->accessBlocks.insert(&block);
      } else if (isa<EndUnpairedAccessInst>(inst)) {
        this->accessBlocks.insert(&block);
      }
    }
  }
}
