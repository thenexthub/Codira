//===------- SARIFDiagnosticPrinter.cpp - Diagnostic Printer---------------===//
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
// This diagnostic client prints out their diagnostic messages in SARIF format.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Frontend/SARIFDiagnosticPrinter.h"
#include "language/Core/Basic/DiagnosticOptions.h"
#include "language/Core/Basic/Sarif.h"
#include "language/Core/Frontend/DiagnosticRenderer.h"
#include "language/Core/Frontend/SARIFDiagnostic.h"
#include "language/Core/Lex/Lexer.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/JSON.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {

SARIFDiagnosticPrinter::SARIFDiagnosticPrinter(raw_ostream &OS,
                                               DiagnosticOptions &DiagOpts)
    : OS(OS), DiagOpts(DiagOpts) {}

void SARIFDiagnosticPrinter::BeginSourceFile(const LangOptions &LO,
                                             const Preprocessor *PP) {
  // Build the SARIFDiagnostic utility.
  assert(hasSarifWriter() && "Writer not set!");
  assert(!SARIFDiag && "SARIFDiagnostic already set.");
  SARIFDiag = std::make_unique<SARIFDiagnostic>(OS, LO, DiagOpts, &*Writer);
  // Initialize the SARIF object.
  Writer->createRun("clang", Prefix);
}

void SARIFDiagnosticPrinter::EndSourceFile() {
  assert(SARIFDiag && "SARIFDiagnostic has not been set.");
  Writer->endRun();
  toolchain::json::Value Value(Writer->createDocument());
  OS << "\n" << Value << "\n\n";
  OS.flush();
  SARIFDiag.reset();
}

void SARIFDiagnosticPrinter::HandleDiagnostic(DiagnosticsEngine::Level Level,
                                              const Diagnostic &Info) {
  assert(SARIFDiag && "SARIFDiagnostic has not been set.");
  // Default implementation (Warnings/errors count). Keeps track of the
  // number of errors.
  DiagnosticConsumer::HandleDiagnostic(Level, Info);

  // Render the diagnostic message into a temporary buffer eagerly. We'll use
  // this later as we add the diagnostic to the SARIF object.
  SmallString<100> OutStr;
  Info.FormatDiagnostic(OutStr);

  toolchain::raw_svector_ostream DiagMessageStream(OutStr);

  // Use a dedicated, simpler path for diagnostics without a valid location.
  // This is important as if the location is missing, we may be emitting
  // diagnostics in a context that lacks language options, a source manager, or
  // other infrastructure necessary when emitting more rich diagnostics.
  if (Info.getLocation().isInvalid()) {
    // FIXME: Enable diagnostics without a source manager
    return;
  }

  // Assert that the rest of our infrastructure is setup properly.
  assert(Info.hasSourceManager() &&
         "Unexpected diagnostic with no source manager");

  SARIFDiag->emitDiagnostic(
      FullSourceLoc(Info.getLocation(), Info.getSourceManager()), Level,
      DiagMessageStream.str(), Info.getRanges(), Info.getFixItHints(), &Info);
}
} // namespace language::Core
