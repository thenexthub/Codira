//===--- IsSelfRecursiveAnalysis.cpp --------------------------------------===//
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

#include "language/SILOptimizer/Analysis/IsSelfRecursiveAnalysis.h"
#include "language/SIL/ApplySite.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"

using namespace language;

// Force the compiler to generate the destructor in this C++ file.
// Otherwise it can happen that it is generated in a CodiraCompilerSources module
// and that results in unresolved-symbols linker errors.
IsSelfRecursive::~IsSelfRecursive() = default;

void IsSelfRecursive::compute() {
  isSelfRecursive = false;

  for (auto &BB : *getFunction()) {
    for (auto &I : BB) {
      if (auto Apply = FullApplySite::isa(&I)) {
        if (Apply.getReferencedFunctionOrNull() == f) {
          isSelfRecursive = true;
          return;
        }
      }
    }
  }
}

//===----------------------------------------------------------------------===//
//                              Main Entry Point
//===----------------------------------------------------------------------===//

SILAnalysis *language::createIsSelfRecursiveAnalysis(SILModule *) {
  return new IsSelfRecursiveAnalysis();
}
