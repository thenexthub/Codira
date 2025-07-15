//===--- REPLCodeCompletion.h - Code completion for REPL --------*- C++ -*-===//
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
// This module provides completions to the immediate mode environment.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IDE_REPL_CODE_COMPLETION_H
#define LANGUAGE_IDE_REPL_CODE_COMPLETION_H

#include "language/Basic/Toolchain.h"
#include "language/IDE/CodeCompletionCache.h"
#include "language/IDE/CodeCompletionConsumer.h"
#include "language/Parse/IDEInspectionCallbacks.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace language {

/// State of a completion operation.
enum class CompletionState {
  /// An invalid completion set.
  Invalid,
  /// No matching completions were found. Subsequent <TAB>s just beep.
  Empty,
  /// A unique completion was found. Subsequent <TAB>s just beep.
  Unique,
  /// Completions have been looked up and the root has been completed. This is
  /// the state after the first <TAB> event. The next <TAB> will display
  /// available completions.
  CompletedRoot,
  /// Displayed the list of available completions. Subsequent <TAB> events will
  /// cycle through completing different stems.
  DisplayedCompletionList
};

/// Represents a completion set and maintains state for navigating through
/// a set of completions.
class REPLCompletions {
public:
  struct CookedResult {
    StringRef InsertableString;
    unsigned NumBytesToErase;
  };

private:
  friend class REPLCodeCompletionConsumer;
  CompletionState State;

  ide::CodeCompletionCache CompletionCache;
  ide::CodeCompletionContext CompletionContext;
  std::unique_ptr<ide::CodeCompletionConsumer> Consumer;
  std::unique_ptr<IDEInspectionCallbacksFactory> IDEInspectionCallbacksFactory;

  std::vector<StringRef> CompletionStrings;
  std::vector<CookedResult> CookedResults;

  std::string Prefix;
  mutable std::optional<std::string> Root;
  size_t CurrentCompletionIdx;

public:
  /// Create an invalid completion set.
  REPLCompletions();

  /// Create completion results for the given string.
  void populate(SourceFile &SF, StringRef EnteredCode);

  /// Returns true if this is a valid completion set.
  explicit operator bool() const { return State != CompletionState::Invalid; }
  bool isValid() const { return State != CompletionState::Invalid; }

  // True if no completions were found.
  bool isEmpty() const { return State == CompletionState::Empty; }

  /// True if the completion is unique.
  bool isUnique() const { return State == CompletionState::Unique; }

  /// Returns the current state of the completion.
  CompletionState getState() const { return State; }
  /// Sets the state of the completion.
  void setState(CompletionState S) { State = S; }

  /// Returns the common root of all the found completions (or the entire
  /// completion for a unique completion).
  StringRef getRoot() const;

  /// Returns the stem (if any) returned by the previous \c getNextStem() call.
  CookedResult getPreviousStem() const;

  /// Returns the next completion stem. Cycles to the beginning of the list if
  /// the end was reached.
  CookedResult getNextStem();

  /// Returns a list of all complete names for which completions were found.
  ArrayRef<StringRef> getCompletionList() const {
    return CompletionStrings;
  }

  /// Reset the completion set to an invalid state.
  void reset();
};

} // namespace language

#endif // TOOLCHAIN_LANGUAGE_IDE_REPL_CODE_COMPLETION_H

