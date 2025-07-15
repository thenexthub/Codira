//===--- ModulePrinter.cpp - Module printer pass --------------------------===//
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
//
// A utility module pass to print the module as textual SIL.
//
//===----------------------------------------------------------------------===//

#include "language/SIL/SILPrintContext.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using namespace language;

namespace {

class SILModulePrinter : public SILModuleTransform {

  /// The entry point.
  void run() override {
    auto *module = getModule();
    SILPrintContext context(toolchain::outs(), /*Verbose*/ true, /*SortedSIL*/ true,
                            /*PrintFullConvention*/ true);
    module->print(context);
  }
};

} // end anonymous namespace

SILTransform *language::createModulePrinter() { return new SILModulePrinter(); }
