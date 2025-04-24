//===--- AccessStorageAnalysisDumper.cpp - accessed storage analysis ----===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-accessed-storage-analysis-dumper"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/AccessStorageAnalysis.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "llvm/Support/Debug.h"

using namespace language;

namespace {

/// Dumps per-function information on dynamically enforced formal accesses.
class AccessStorageAnalysisDumper : public SILModuleTransform {

  void run() override {
    auto *analysis = PM->getAnalysis<AccessStorageAnalysis>();

    for (auto &fn : *getModule()) {
      llvm::outs() << "@" << fn.getName() << "\n";
      if (fn.empty()) {
        llvm::outs() << "<unknown>\n";
        continue;
      }
      const FunctionAccessStorage &summary = analysis->getEffects(&fn);
      summary.print(llvm::outs());
    }
  }
};

} // end anonymous namespace

SILTransform *swift::createAccessStorageAnalysisDumper() {
  return new AccessStorageAnalysisDumper();
}
