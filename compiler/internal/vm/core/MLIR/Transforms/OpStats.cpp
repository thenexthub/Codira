//===- OpStats.cpp - Prints stats of operations in module -----------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "mlir/Transforms/Passes.h"

#include "mlir/IR/Operation.h"
#include "mlir/IR/OperationSupport.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"

namespace mlir {
#define GEN_PASS_DEF_PRINTOPSTATS
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
struct PrintOpStatsPass : public impl::PrintOpStatsBase<PrintOpStatsPass> {
  explicit PrintOpStatsPass(raw_ostream &os) : os(os) {}

  explicit PrintOpStatsPass(raw_ostream &os, bool printAsJSON) : os(os) {
    this->printAsJSON = printAsJSON;
  }

  // Prints the resultant operation statistics post iterating over the module.
  void runOnOperation() override;

  // Print summary of op stats.
  void printSummary();

  // Print symmary of op stats in JSON.
  void printSummaryInJSON();

private:
  toolchain::StringMap<int64_t> opCount;
  raw_ostream &os;
};
} // namespace

void PrintOpStatsPass::runOnOperation() {
  opCount.clear();

  // Compute the operation statistics for the currently visited operation.
  getOperation()->walk(
      [&](Operation *op) { ++opCount[op->getName().getStringRef()]; });
  if (printAsJSON)
    printSummaryInJSON();
  else
    printSummary();
  markAllAnalysesPreserved();
}

void PrintOpStatsPass::printSummary() {
  os << "Operations encountered:\n";
  os << "-----------------------\n";
  SmallVector<StringRef, 64> sorted(opCount.keys());
  toolchain::sort(sorted);

  // Split an operation name from its dialect prefix.
  auto splitOperationName = [](StringRef opName) {
    auto splitName = opName.split('.');
    return splitName.second.empty() ? std::make_pair("", splitName.first)
                                    : splitName;
  };

  // Compute the largest dialect and operation name.
  size_t maxLenOpName = 0, maxLenDialect = 0;
  for (const auto &key : sorted) {
    auto [dialectName, opName] = splitOperationName(key);
    maxLenDialect = std::max(maxLenDialect, dialectName.size());
    maxLenOpName = std::max(maxLenOpName, opName.size());
  }

  for (const auto &key : sorted) {
    auto [dialectName, opName] = splitOperationName(key);

    // Left-align the names (aligning on the dialect) and right-align the count
    // below. The alignment is for readability and does not affect CSV/FileCheck
    // parsing.
    if (dialectName.empty())
      os.indent(maxLenDialect + 3);
    else
      os << toolchain::right_justify(dialectName, maxLenDialect + 2) << '.';

    // Left justify the operation name.
    os << toolchain::left_justify(opName, maxLenOpName) << " , " << opCount[key]
       << '\n';
  }
}

void PrintOpStatsPass::printSummaryInJSON() {
  SmallVector<StringRef, 64> sorted(opCount.keys());
  toolchain::sort(sorted);

  os << "{\n";

  for (unsigned i = 0, e = sorted.size(); i != e; ++i) {
    const auto &key = sorted[i];
    os << "  \"" << key << "\" : " << opCount[key];
    if (i != e - 1)
      os << ",\n";
    else
      os << "\n";
  }
  os << "}\n";
}

std::unique_ptr<Pass> mlir::createPrintOpStatsPass(raw_ostream &os) {
  return std::make_unique<PrintOpStatsPass>(os);
}

std::unique_ptr<Pass> mlir::createPrintOpStatsPass(raw_ostream &os,
                                                   bool printAsJSON) {
  return std::make_unique<PrintOpStatsPass>(os, printAsJSON);
}
