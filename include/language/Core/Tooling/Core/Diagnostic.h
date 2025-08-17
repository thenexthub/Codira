//===--- Diagnostic.h - Framework for clang diagnostics tools --*- C++ -*-===//
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
// \file
//  Structures supporting diagnostics and refactorings that span multiple
//  translation units. Indicate diagnostics reports and replacements
//  suggestions for the analyzed sources.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_CORE_DIAGNOSTIC_H
#define LANGUAGE_CORE_TOOLING_CORE_DIAGNOSTIC_H

#include "Replacement.h"
#include "language/Core/Basic/Diagnostic.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include <string>

namespace language::Core {
namespace tooling {

/// Represents a range within a specific source file.
struct FileByteRange {
  FileByteRange() = default;

  FileByteRange(const SourceManager &Sources, CharSourceRange Range);

  std::string FilePath;
  unsigned FileOffset;
  unsigned Length;
};

/// Represents the diagnostic message with the error message associated
/// and the information on the location of the problem.
struct DiagnosticMessage {
  DiagnosticMessage(toolchain::StringRef Message = "");

  /// Constructs a diagnostic message with anoffset to the diagnostic
  /// within the file where the problem occurred.
  ///
  /// \param Loc Should be a file location, it is not meaningful for a macro
  /// location.
  ///
  DiagnosticMessage(toolchain::StringRef Message, const SourceManager &Sources,
                    SourceLocation Loc);

  std::string Message;
  std::string FilePath;
  unsigned FileOffset;

  /// Fixes for this diagnostic, grouped by file path.
  toolchain::StringMap<Replacements> Fix;

  /// Extra source ranges associated with the note, in addition to the location
  /// of the Message itself.
  toolchain::SmallVector<FileByteRange, 1> Ranges;
};

/// Represents the diagnostic with the level of severity and possible
/// fixes to be applied.
struct Diagnostic {
  enum Level {
    Remark = DiagnosticsEngine::Remark,
    Warning = DiagnosticsEngine::Warning,
    Error = DiagnosticsEngine::Error
  };

  Diagnostic() = default;

  Diagnostic(toolchain::StringRef DiagnosticName, Level DiagLevel,
             StringRef BuildDirectory);

  Diagnostic(toolchain::StringRef DiagnosticName, const DiagnosticMessage &Message,
             const SmallVector<DiagnosticMessage, 1> &Notes, Level DiagLevel,
             toolchain::StringRef BuildDirectory);

  /// Name identifying the Diagnostic.
  std::string DiagnosticName;

  /// Message associated to the diagnostic.
  DiagnosticMessage Message;

  /// Potential notes about the diagnostic.
  SmallVector<DiagnosticMessage, 1> Notes;

  /// Diagnostic level. Can indicate either an error or a warning.
  Level DiagLevel;

  /// A build directory of the diagnostic source file.
  ///
  /// It's an absolute path which is `directory` field of the source file in
  /// compilation database. If users don't specify the compilation database
  /// directory, it is the current directory where clang-tidy runs.
  ///
  /// Note: it is empty in unittest.
  std::string BuildDirectory;
};

/// Collection of Diagnostics generated from a single translation unit.
struct TranslationUnitDiagnostics {
  /// Name of the main source for the translation unit.
  std::string MainSourceFile;
  std::vector<Diagnostic> Diagnostics;
};

/// Get the first fix to apply for this diagnostic.
/// \returns nullptr if no fixes are attached to the diagnostic.
const toolchain::StringMap<Replacements> *selectFirstFix(const Diagnostic& D);

} // end namespace tooling
} // end namespace language::Core
#endif // LANGUAGE_CORE_TOOLING_CORE_DIAGNOSTIC_H
