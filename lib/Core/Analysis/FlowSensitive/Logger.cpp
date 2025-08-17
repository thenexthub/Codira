//===-- Logger.cpp --------------------------------------------------------===//
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

#include "language/Core/Analysis/FlowSensitive/Logger.h"
#include "language/Core/Analysis/FlowSensitive/AdornedCFG.h"
#include "language/Core/Analysis/FlowSensitive/TypeErasedDataflowAnalysis.h"
#include "toolchain/Support/WithColor.h"

namespace language::Core::dataflow {

Logger &Logger::null() {
  struct NullLogger final : Logger {};
  static auto *Instance = new NullLogger();
  return *Instance;
}

namespace {
struct TextualLogger final : Logger {
  toolchain::raw_ostream &OS;
  const CFG *CurrentCFG;
  const CFGBlock *CurrentBlock;
  const CFGElement *CurrentElement;
  unsigned CurrentElementIndex;
  bool ShowColors;
  toolchain::DenseMap<const CFGBlock *, unsigned> VisitCount;
  TypeErasedDataflowAnalysis *CurrentAnalysis;

  TextualLogger(toolchain::raw_ostream &OS)
      : OS(OS), ShowColors(toolchain::WithColor::defaultAutoDetectFunction()(OS)) {}

  virtual void beginAnalysis(const AdornedCFG &ACFG,
                             TypeErasedDataflowAnalysis &Analysis) override {
    {
      toolchain::WithColor Header(OS, toolchain::raw_ostream::Colors::RED, /*Bold=*/true);
      OS << "=== Beginning data flow analysis ===\n";
    }
    auto &D = ACFG.getDecl();
    D.print(OS);
    OS << "\n";
    D.dump(OS);
    CurrentCFG = &ACFG.getCFG();
    CurrentCFG->print(OS, Analysis.getASTContext().getLangOpts(), ShowColors);
    CurrentAnalysis = &Analysis;
  }
  virtual void endAnalysis() override {
    toolchain::WithColor Header(OS, toolchain::raw_ostream::Colors::RED, /*Bold=*/true);
    unsigned Blocks = 0, Steps = 0;
    for (const auto &E : VisitCount) {
      ++Blocks;
      Steps += E.second;
    }
    toolchain::errs() << "=== Finished analysis: " << Blocks << " blocks in "
                 << Steps << " total steps ===\n";
  }
  virtual void enterBlock(const CFGBlock &Block, bool PostVisit) override {
    unsigned Count = ++VisitCount[&Block];
    {
      toolchain::WithColor Header(OS, toolchain::raw_ostream::Colors::RED, /*Bold=*/true);
      OS << "=== Entering block B" << Block.getBlockID();
      if (PostVisit)
        OS << " (post-visit)";
      else
        OS << " (iteration " << Count << ")";
      OS << " ===\n";
    }
    Block.print(OS, CurrentCFG, CurrentAnalysis->getASTContext().getLangOpts(),
                ShowColors);
    CurrentBlock = &Block;
    CurrentElement = nullptr;
    CurrentElementIndex = 0;
  }
  virtual void enterElement(const CFGElement &Element) override {
    ++CurrentElementIndex;
    CurrentElement = &Element;
    {
      toolchain::WithColor Subheader(OS, toolchain::raw_ostream::Colors::CYAN,
                                /*Bold=*/true);
      OS << "Processing element B" << CurrentBlock->getBlockID() << "."
         << CurrentElementIndex << ": ";
      Element.dumpToStream(OS);
    }
  }
  void recordState(TypeErasedDataflowAnalysisState &State) override {
    {
      toolchain::WithColor Subheader(OS, toolchain::raw_ostream::Colors::CYAN,
                                /*Bold=*/true);
      OS << "Computed state for B" << CurrentBlock->getBlockID() << "."
         << CurrentElementIndex << ":\n";
    }
    // FIXME: currently the environment dump is verbose and unenlightening.
    // FIXME: dump the user-defined lattice, too.
    State.Env.dump(OS);
    OS << "\n";
  }
  void blockConverged() override {
    OS << "B" << CurrentBlock->getBlockID() << " has converged!\n";
  }
  virtual void logText(toolchain::StringRef S) override { OS << S << "\n"; }
};
} // namespace

std::unique_ptr<Logger> Logger::textual(toolchain::raw_ostream &OS) {
  return std::make_unique<TextualLogger>(OS);
}

} // namespace language::Core::dataflow
