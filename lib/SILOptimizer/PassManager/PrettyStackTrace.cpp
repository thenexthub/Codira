//===--- PrettyStackTrace.cpp ---------------------------------------------===//
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

#include "language/SILOptimizer/PassManager/PrettyStackTrace.h"
#include "language/SIL/SILFunction.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "llvm/Support/raw_ostream.h"

using namespace language;

PrettyStackTraceSILFunctionTransform::PrettyStackTraceSILFunctionTransform(
  SILFunctionTransform *SFT, unsigned PassNumber):
  PrettyStackTraceSILFunction("Running SIL Function Transform",
                              SFT->getFunction()),
  SFT(SFT), PassNumber(PassNumber) {}

void PrettyStackTraceSILFunctionTransform::print(llvm::raw_ostream &out) const {
  out << "While running pass #" << PassNumber
      << " SILFunctionTransform \"" << SFT->getID()
      << "\" on SILFunction ";
  if (!SFT->getFunction()) {
    out << " <<null>>";
    return;
  }
  printFunctionInfo(out);
}

void PrettyStackTraceSILModuleTransform::print(llvm::raw_ostream &out) const {
  out << "While running pass #" << PassNumber
      << " SILModuleTransform \"" << SMT->getID() << "\".\n";
}
