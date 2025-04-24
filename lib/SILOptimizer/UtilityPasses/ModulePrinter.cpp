//===--- ModulePrinter.cpp - Module printer pass --------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2021 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
    SILPrintContext context(llvm::outs(), /*Verbose*/ true, /*SortedSIL*/ true,
                            /*PrintFullConvention*/ true);
    module->print(context);
  }
};

} // end anonymous namespace

SILTransform *swift::createModulePrinter() { return new SILModulePrinter(); }
