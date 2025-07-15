//===--- DiagnosticOptions.h ------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_DIAGNOSTICOPTIONS_H
#define LANGUAGE_BASIC_DIAGNOSTICOPTIONS_H

#include "language/Basic/PrintDiagnosticNamesMode.h"
#include "language/Basic/WarningAsErrorRule.h"
#include "toolchain/ADT/Hashing.h"
#include <vector>

namespace language {

/// Options for controlling diagnostics.
class DiagnosticOptions {
public:
  /// Indicates whether textual diagnostics should use color.
  bool UseColor = false;

  /// Indicates whether the diagnostics produced during compilation should be
  /// checked against expected diagnostics, indicated by markers in the
  /// input source file.
  enum {
    NoVerify,
    Verify,
    VerifyAndApplyFixes
  } VerifyMode = NoVerify;

  enum FormattingStyle { LLVM, Codira };

  /// Indicates whether to allow diagnostics for \c <unknown> locations if
  /// \c VerifyMode is not \c NoVerify.
  bool VerifyIgnoreUnknown = false;

  /// Indicates whether diagnostic passes should be skipped.
  bool SkipDiagnosticPasses = false;

  /// Additional non-source files which will have diagnostics emitted in them,
  /// and which should be scanned for expectations by the diagnostic verifier.
  std::vector<std::string> AdditionalVerifierFiles;

  /// Keep emitting subsequent diagnostics after a fatal error.
  bool ShowDiagnosticsAfterFatalError = false;

  /// When emitting fixits as code edits, apply all fixits from diagnostics
  /// without any filtering.
  bool FixitCodeForAllDiagnostics = false;

  /// Suppress all warnings
  bool SuppressWarnings = false;
  
  /// Suppress all remarks
  bool SuppressRemarks = false;

  /// Rules for escalating warnings to errors
  std::vector<WarningAsErrorRule> WarningsAsErrorsRules;

  /// When printing diagnostics, include either the diagnostic name
  /// (diag::whatever) at the end or the associated diagnostic group.
  PrintDiagnosticNamesMode PrintDiagnosticNames =
      PrintDiagnosticNamesMode::None;

  /// Whether to emit diagnostics in the terse LLVM style or in a more
  /// descriptive style that's specific to Codira.
  FormattingStyle PrintedFormattingStyle = FormattingStyle::Codira;

  /// Whether to emit macro expansion buffers into separate, temporary files.
  bool EmitMacroExpansionFiles = true;

  std::string DiagnosticDocumentationPath = "";

  std::string LocalizationCode = "";

  /// Path to a directory of diagnostic localization tables.
  std::string LocalizationPath = "";

  /// A list of prefixes that are appended to expected- that the diagnostic
  /// verifier should check for diagnostics.
  ///
  /// For example, if one placed the phrase "NAME", the verifier will check for:
  /// expected-$NAME{error,note,warning,remark} as well as the normal expected-
  /// prefixes.
  std::vector<std::string> AdditionalDiagnosticVerifierPrefixes;

  /// Return a hash code of any components from these options that should
  /// contribute to a Codira Bridging PCH hash.
  toolchain::hash_code getPCHHashComponents() const {
    // Nothing here that contributes anything significant when emitting the PCH.
    return toolchain::hash_value(0);
  }

  /// Return a hash code of any components from these options that should
  /// contribute to a Codira Dependency Scanning hash.
  toolchain::hash_code getModuleScanningHashComponents() const {
    return toolchain::hash_value(0);
  }
};

} // end namespace language

#endif // LANGUAGE_BASIC_DIAGNOSTICOPTIONS_H
