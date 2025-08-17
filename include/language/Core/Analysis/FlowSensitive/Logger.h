//===-- Logger.h ------------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_LOGGER_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_LOGGER_H

#include "language/Core/Analysis/CFG.h"
#include "toolchain/Support/raw_ostream.h"
#include <memory>

namespace language::Core::dataflow {
// Forward declarations so we can use Logger anywhere in the framework.
class AdornedCFG;
class TypeErasedDataflowAnalysis;
struct TypeErasedDataflowAnalysisState;

/// A logger is notified as the analysis progresses.
/// It can produce a report of the analysis's findings and how it came to them.
///
/// The framework reports key structural events (e.g. traversal of blocks).
/// The specific analysis can add extra details to be presented in context.
class Logger {
public:
  /// Returns a dummy logger that does nothing.
  static Logger &null();
  /// A logger that simply writes messages to the specified ostream in real
  /// time.
  static std::unique_ptr<Logger> textual(toolchain::raw_ostream &);
  /// A logger that builds an HTML UI to inspect the analysis results.
  /// Each function's analysis is written to a stream obtained from the factory.
  static std::unique_ptr<Logger>
      html(std::function<std::unique_ptr<toolchain::raw_ostream>()>);

  virtual ~Logger() = default;

  /// Called by the framework as we start analyzing a new function or statement.
  /// Forms a pair with endAnalysis().
  virtual void beginAnalysis(const AdornedCFG &, TypeErasedDataflowAnalysis &) {
  }
  virtual void endAnalysis() {}

  // At any time during the analysis, we're computing the state for some target
  // program point.

  /// Called when we start (re-)processing a block in the CFG.
  /// The target program point is the entry to the specified block.
  /// Calls to log() describe transferBranch(), join() etc.
  /// `PostVisit` specifies whether we're processing the block for the
  /// post-visit callback.
  virtual void enterBlock(const CFGBlock &, bool PostVisit) {}
  /// Called when we start processing an element in the current CFG block.
  /// The target program point is after the specified element.
  /// Calls to log() describe the transfer() function.
  virtual void enterElement(const CFGElement &) {}

  /// Records the analysis state computed for the current program point.
  virtual void recordState(TypeErasedDataflowAnalysisState &) {}
  /// Records that the analysis state for the current block is now final.
  virtual void blockConverged() {}

  /// Called by the framework or user code to report some event.
  /// The event is associated with the current context (program point).
  /// The Emit function produces the log message. It may or may not be called,
  /// depending on if the logger is interested; it should have no side effects.
  void log(toolchain::function_ref<void(toolchain::raw_ostream &)> Emit) {
    if (!ShouldLogText)
      return;
    std::string S;
    toolchain::raw_string_ostream OS(S);
    Emit(OS);
    logText(S);
  }

protected:
  /// ShouldLogText should be false for trivial loggers that ignore logText().
  /// This allows log() to skip evaluating its Emit function.
  Logger(bool ShouldLogText = true) : ShouldLogText(ShouldLogText) {}

private:
  bool ShouldLogText;
  virtual void logText(toolchain::StringRef) {}
};

} // namespace language::Core::dataflow

#endif
