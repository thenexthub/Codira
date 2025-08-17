//===--- SARIFDiagnostic.h - SARIF Diagnostic Formatting -----*- C++ -*-===//
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
// This is a utility class that provides support for constructing a SARIF object
// containing diagnostics.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_FRONTEND_SARIFDIAGNOSTIC_H
#define LANGUAGE_CORE_FRONTEND_SARIFDIAGNOSTIC_H

#include "language/Core/Basic/Sarif.h"
#include "language/Core/Frontend/DiagnosticRenderer.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {

class SARIFDiagnostic : public DiagnosticRenderer {
public:
  SARIFDiagnostic(raw_ostream &OS, const LangOptions &LangOpts,
                  DiagnosticOptions &DiagOpts, SarifDocumentWriter *Writer);

  ~SARIFDiagnostic() = default;

  SARIFDiagnostic &operator=(const SARIFDiagnostic &&) = delete;
  SARIFDiagnostic(SARIFDiagnostic &&) = delete;
  SARIFDiagnostic &operator=(const SARIFDiagnostic &) = delete;
  SARIFDiagnostic(const SARIFDiagnostic &) = delete;

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
                       ArrayRef<FixItHint> Hints) override {}

  void emitIncludeLocation(FullSourceLoc Loc, PresumedLoc PLoc) override;

  void emitImportLocation(FullSourceLoc Loc, PresumedLoc PLoc,
                          StringRef ModuleName) override;

  void emitBuildingModuleLocation(FullSourceLoc Loc, PresumedLoc PLoc,
                                  StringRef ModuleName) override;

private:
  // Shared between SARIFDiagnosticPrinter and this renderer.
  SarifDocumentWriter *Writer;

  SarifResult addLocationToResult(SarifResult Result, FullSourceLoc Loc,
                                  PresumedLoc PLoc,
                                  ArrayRef<CharSourceRange> Ranges,
                                  const Diagnostic &Diag);

  SarifRule addDiagnosticLevelToRule(SarifRule Rule,
                                     DiagnosticsEngine::Level Level);

  toolchain::StringRef emitFilename(StringRef Filename, const SourceManager &SM);
};

} // end namespace language::Core

#endif
