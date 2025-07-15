//===--- BasicInstructionPropertyDumper.cpp -------------------------------===//
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

#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

namespace {

class BasicInstructionPropertyDumper : public SILModuleTransform {
  void run() override {
    for (auto &Fn : *getModule()) {
      unsigned Count = 0;
      toolchain::outs() << "@" << Fn.getName() << "\n";
      for (auto &BB : Fn) {
        for (auto &I : BB) {
          toolchain::outs() << "Inst #: " << Count++ << "\n    " << I;
          toolchain::outs() << "    Mem Behavior: " << I.getMemoryBehavior() << "\n";
          toolchain::outs() << "    Release Behavior: " << I.getReleasingBehavior()
                       << "\n";
        }
      }
    }
  }

};

} // end anonymous namespace

SILTransform *language::createBasicInstructionPropertyDumper() {
  return new BasicInstructionPropertyDumper();
}
