//===-- SARIFDiagnosticPrinter.h - SARIF Diagnostic Client -------*- C++-*-===//
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
// This is a concrete diagnostic client, which prints the diagnostics to
// standard error in SARIF format.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_FRONTEND_SARIFDIAGNOSTICPRINTER_H
#define LANGUAGE_CORE_FRONTEND_SARIFDIAGNOSTICPRINTER_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/Sarif.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/StringRef.h"
#include <memory>

namespace language::Core {
class DiagnosticOptions;
class LangOptions;
class SARIFDiagnostic;
class SarifDocumentWriter;

class SARIFDiagnosticPrinter : public DiagnosticConsumer {
public:
  SARIFDiagnosticPrinter(raw_ostream &OS, DiagnosticOptions &DiagOpts);
  ~SARIFDiagnosticPrinter() = default;

  SARIFDiagnosticPrinter &operator=(const SARIFDiagnosticPrinter &&) = delete;
  SARIFDiagnosticPrinter(SARIFDiagnosticPrinter &&) = delete;
  SARIFDiagnosticPrinter &operator=(const SARIFDiagnosticPrinter &) = delete;
  SARIFDiagnosticPrinter(const SARIFDiagnosticPrinter &) = delete;

  /// setPrefix - Set the diagnostic printer prefix string, which will be
  /// printed at the start of any diagnostics. If empty, no prefix string is
  /// used.
  void setPrefix(toolchain::StringRef Value) { Prefix = Value; }

  bool hasSarifWriter() const { return Writer != nullptr; }

  SarifDocumentWriter &getSarifWriter() const {
    assert(Writer && "SarifWriter not set!");
    return *Writer;
  }

  void setSarifWriter(std::unique_ptr<SarifDocumentWriter> SarifWriter) {
    Writer = std::move(SarifWriter);
  }

  void BeginSourceFile(const LangOptions &LO, const Preprocessor *PP) override;
  void EndSourceFile() override;
  void HandleDiagnostic(DiagnosticsEngine::Level Level,
                        const Diagnostic &Info) override;

private:
  raw_ostream &OS;
  DiagnosticOptions &DiagOpts;

  /// Handle to the currently active SARIF diagnostic emitter.
  std::unique_ptr<SARIFDiagnostic> SARIFDiag;

  /// A string to prefix to error messages.
  std::string Prefix;

  std::unique_ptr<SarifDocumentWriter> Writer;
};

} // end namespace language::Core

#endif
