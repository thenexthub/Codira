//===- Timing.cpp - Execution time measurement facilities -----------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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
//
// Facilities to measure and provide statistics on execution time.
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Support/Timing.h"
#include "toolchain/Support/Format.h"

class OutputStrategyText : public mlir::OutputStrategy {
protected:
  static constexpr toolchain::StringLiteral header = "Flang execution timing report";

public:
  OutputStrategyText(toolchain::raw_ostream &os) : mlir::OutputStrategy(os) {}

  void printHeader(const mlir::TimeRecord &total) override {
    // Figure out how many spaces to description name.
    unsigned padding = (80 - header.size()) / 2;
    os << "===" << std::string(73, '-') << "===\n";
    os.indent(padding) << header << '\n';
    os << "===" << std::string(73, '-') << "===\n";

    // Print the total time followed by the section headers.
    os << toolchain::format("  Total Execution Time: %.4f seconds\n\n", total.wall);
    os << "  ----User Time----  ----Wall Time----  ----Name----\n";
  }

  void printFooter() override { os.flush(); }

  void printTime(
      const mlir::TimeRecord &time, const mlir::TimeRecord &total) override {
    os << toolchain::format(
        "  %8.4f (%5.1f%%)", time.user, 100.0 * time.user / total.user);
    os << toolchain::format(
        "  %8.4f (%5.1f%%)  ", time.wall, 100.0 * time.wall / total.wall);
  }

  void printListEntry(toolchain::StringRef name, const mlir::TimeRecord &time,
      const mlir::TimeRecord &total, bool lastEntry) override {
    printTime(time, total);
    os << name << "\n";
  }

  void printTreeEntry(unsigned indent, toolchain::StringRef name,
      const mlir::TimeRecord &time, const mlir::TimeRecord &total) override {
    printTime(time, total);
    os.indent(indent) << name << "\n";
  }

  void printTreeEntryEnd(unsigned indent, bool lastEntry) override {}
};

namespace language::Compability::support {

std::unique_ptr<mlir::OutputStrategy> createTimingFormatterText(
    toolchain::raw_ostream &os) {
  return std::make_unique<OutputStrategyText>(os);
}

} // namespace language::Compability::support
