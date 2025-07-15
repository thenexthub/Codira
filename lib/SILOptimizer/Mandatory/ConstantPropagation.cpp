//===--- ConstantPropagation.cpp - Constant fold and diagnose overflows ---===//
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

#define DEBUG_TYPE "constant-propagation"
#include "language/Basic/Assertions.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/SILOptFunctionBuilder.h"
#include "language/SILOptimizer/Utils/ConstantFolding.h"

using namespace language;

//===----------------------------------------------------------------------===//
//                              Top Level Driver
//===----------------------------------------------------------------------===//

namespace {

class ConstantPropagation : public SILFunctionTransform {
  bool EnableDiagnostics;

public:
  ConstantPropagation(bool EnableDiagnostics) :
    EnableDiagnostics(EnableDiagnostics) {}

private:
  /// The entry point to the transformation.
  void run() override {
    SILOptFunctionBuilder FuncBuilder(*this);
    ConstantFolder Folder(FuncBuilder, getOptions().AssertConfig,
                          EnableDiagnostics);
    Folder.initializeWorklist(*getFunction());
    auto Invalidation = Folder.processWorkList();

    if (Invalidation != SILAnalysis::InvalidationKind::Nothing) {
      invalidateAnalysis(Invalidation);
    }
  }
};

} // end anonymous namespace

SILTransform *language::createDiagnosticConstantPropagation() {
  // Diagnostic propagation is rerun on deserialized SIL because it is sensitive
  // to assert configuration.
  return new ConstantPropagation(true /*enable diagnostics*/);
}

SILTransform *language::createPerformanceConstantPropagation() {
  return new ConstantPropagation(false /*disable diagnostics*/);
}
