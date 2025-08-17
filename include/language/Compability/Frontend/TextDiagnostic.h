//===--- TextDiagnostic.h - Text Diagnostic Pretty-Printing -----*- C++ -*-===//
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
// A utility class that provides support for textual pretty-printing of
// diagnostics. Based on language::Core::TextDiagnostic (this is a trimmed version).
//
// TODO: If expanding, consider sharing the implementation with Clang.
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_FRONTEND_TEXTDIAGNOSTIC_H
#define LANGUAGE_COMPABILITY_FRONTEND_TEXTDIAGNOSTIC_H

#include "language/Core/Basic/Diagnostic.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"

namespace language::Compability::frontend {

/// Class to encapsulate the logic for formatting and printing a textual
/// diagnostic message.
///
/// The purpose of this class is to isolate the implementation of printing
/// beautiful text diagnostics from any particular interfaces. Currently only
/// simple diagnostics that lack source location information are supported (e.g.
/// Flang driver errors).
///
/// In the future we can extend this class (akin to Clang) to support more
/// complex diagnostics that would include macro backtraces, caret diagnostics,
/// FixIt Hints and code snippets.
///
class TextDiagnostic {
public:
  TextDiagnostic();

  ~TextDiagnostic();

  /// Print the diagnostic level to a toolchain::raw_ostream.
  ///
  /// This is a static helper that handles colorizing the level and formatting
  /// it into an arbitrary output stream.
  ///
  /// \param os Where the message is printed
  /// \param level The diagnostic level (e.g. error or warning)
  /// \param showColors Enable colorizing of the message.
  static void printDiagnosticLevel(toolchain::raw_ostream &os,
                                   language::Core::DiagnosticsEngine::Level level,
                                   bool showColors);

  /// Pretty-print a diagnostic message to a toolchain::raw_ostream.
  ///
  /// This is a static helper to handle the colorizing and rendering diagnostic
  /// message to a particular ostream. In the future we can
  /// extend it to support e.g. line wrapping. It is
  /// publicly visible as at this stage we don't require any state data to
  /// print a diagnostic.
  ///
  /// \param os Where the message is printed
  /// \param isSupplemental true if this is a continuation note diagnostic
  /// \param message The text actually printed
  /// \param showColors Enable colorizing of the message.
  static void printDiagnosticMessage(toolchain::raw_ostream &os, bool isSupplemental,
                                     toolchain::StringRef message, bool showColors);
};

} // namespace language::Compability::frontend

#endif
