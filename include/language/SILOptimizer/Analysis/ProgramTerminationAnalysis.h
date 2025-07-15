//===--- ProgramTerminationAnalysis.h ---------------------------*- C++ -*-===//
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
///
/// \file
///
/// This is an analysis which determines if a block is a "program terminating
/// block". Define a program terminating block is defined as follows:
///
/// 1. A block at whose end point according to the SIL model, the program must
/// end. An example of such a block is one that includes a call to fatalError.
/// 2. Any block that is joint post-dominated by program terminating blocks.
///
/// For now we only identify instances of 1. But the analysis could be extended
/// appropriately via simple dataflow or through the use of post-dominator
/// trees.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_ANALYSIS_PROGRAMTERMINATIONANALYSIS_H
#define LANGUAGE_SILOPTIMIZER_ANALYSIS_PROGRAMTERMINATIONANALYSIS_H

#include "language/SILOptimizer/Analysis/ARCAnalysis.h"
#include "toolchain/ADT/SmallPtrSet.h"

namespace language {

class ProgramTerminationFunctionInfo {
  toolchain::SmallPtrSet<const SILBasicBlock *, 4> ProgramTerminatingBlocks;

public:
  ProgramTerminationFunctionInfo(const SILFunction *F) {
    for (const auto &BB : *F) {
      if (!isARCInertTrapBB(&BB))
        continue;
      ProgramTerminatingBlocks.insert(&BB);
    }
  }

  bool isProgramTerminatingBlock(const SILBasicBlock *BB) const {
    return ProgramTerminatingBlocks.count(BB);
  }
};

} // end language namespace

#endif
