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
// This is a utility class that provides support for textual pretty-printing of
// diagnostics. It is used to implement the different code paths which require
// such functionality in a consistent way.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_FRONTEND_TEXTDIAGNOSTIC_H
#define LANGUAGE_CORE_FRONTEND_TEXTDIAGNOSTIC_H

#include "language/Core/Frontend/DiagnosticRenderer.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {

/// Class to encapsulate the logic for formatting and printing a textual
/// diagnostic message.
///
/// This class provides an interface for building and emitting a textual
/// diagnostic, including all of the macro backtraces, caret diagnostics, FixIt
/// Hints, and code snippets. In the presence of macros this involves
/// a recursive process, synthesizing notes for each macro expansion.
///
/// The purpose of this class is to isolate the implementation of printing
/// beautiful text diagnostics from any particular interfaces. The Clang
/// DiagnosticClient is implemented through this class as is diagnostic
/// printing coming out of libclang.
class TextDiagnostic : public DiagnosticRenderer {
  raw_ostream &OS;
  const Preprocessor *PP;

public:
  TextDiagnostic(raw_ostream &OS, const LangOptions &LangOpts,
                 DiagnosticOptions &DiagOpts, const Preprocessor *PP = nullptr);

  ~TextDiagnostic() override;

  struct StyleRange {
    unsigned Start;
    unsigned End;
    enum toolchain::raw_ostream::Colors Color;
    StyleRange(unsigned S, unsigned E, enum toolchain::raw_ostream::Colors C)
        : Start(S), End(E), Color(C){};
  };

  /// Print the diagonstic level to a raw_ostream.
  ///
  /// This is a static helper that handles colorizing the level and formatting
  /// it into an arbitrary output stream. This is used internally by the
  /// TextDiagnostic emission code, but it can also be used directly by
  /// consumers that don't have a source manager or other state that the full
  /// TextDiagnostic logic requires.
  static void printDiagnosticLevel(raw_ostream &OS,
                                   DiagnosticsEngine::Level Level,
                                   bool ShowColors);

  /// Pretty-print a diagnostic message to a raw_ostream.
  ///
  /// This is a static helper to handle the line wrapping, colorizing, and
  /// rendering of a diagnostic message to a particular ostream. It is
  /// publicly visible so that clients which do not have sufficient state to
  /// build a complete TextDiagnostic object can still get consistent
  /// formatting of their diagnostic messages.
  ///
  /// \param OS Where the message is printed
  /// \param IsSupplemental true if this is a continuation note diagnostic
  /// \param Message The text actually printed
  /// \param CurrentColumn The starting column of the first line, accounting
  ///                      for any prefix.
  /// \param Columns The number of columns to use in line-wrapping, 0 disables
  ///                all line-wrapping.
  /// \param ShowColors Enable colorizing of the message.
  static void printDiagnosticMessage(raw_ostream &OS, bool IsSupplemental,
                                     StringRef Message, unsigned CurrentColumn,
                                     unsigned Columns, bool ShowColors);

protected:
  void emitDiagnosticMessage(FullSourceLoc Loc, PresumedLoc PLoc,
                             DiagnosticsEngine::Level Level, StringRef Message,
                             ArrayRef<CharSourceRange> Ranges,
                             DiagOrStoredDiag D) override;

  void emitDiagnosticLoc(FullSourceLoc Loc, PresumedLoc PLoc,
                         DiagnosticsEngine::Level Level,
                         ArrayRef<CharSourceRange> Ranges) override;

  void emitCodeContext(FullSourceLoc Loc, DiagnosticsEngine::Level Level,
                       SmallVectorImpl<CharSourceRange> &Ranges,
                       ArrayRef<FixItHint> Hints) override {
    emitSnippetAndCaret(Loc, Level, Ranges, Hints);
  }

  void emitIncludeLocation(FullSourceLoc Loc, PresumedLoc PLoc) override;

  void emitImportLocation(FullSourceLoc Loc, PresumedLoc PLoc,
                          StringRef ModuleName) override;

  void emitBuildingModuleLocation(FullSourceLoc Loc, PresumedLoc PLoc,
                                  StringRef ModuleName) override;

private:
  void emitFilename(StringRef Filename, const SourceManager &SM);

  void emitSnippetAndCaret(FullSourceLoc Loc, DiagnosticsEngine::Level Level,
                           SmallVectorImpl<CharSourceRange> &Ranges,
                           ArrayRef<FixItHint> Hints);

  void emitSnippet(StringRef SourceLine, unsigned MaxLineNoDisplayWidth,
                   unsigned LineNo, unsigned DisplayLineNo,
                   ArrayRef<StyleRange> Styles);

  void emitParseableFixits(ArrayRef<FixItHint> Hints, const SourceManager &SM);
};

} // end namespace language::Core

#endif
