//===--- TextDiagnosticPrinter.h - Text Diagnostic Client -------*- C++ -*-===//
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
// This is a concrete diagnostic client. In terminals that support it, the
// diagnostics are pretty-printed (colors + bold). The printing/flushing
// happens in HandleDiagnostics (usually called at the point when the
// diagnostic is generated).
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_FRONTEND_TEXTDIAGNOSTICPRINTER_H
#define LANGUAGE_COMPABILITY_FRONTEND_TEXTDIAGNOSTICPRINTER_H

#include "language/Core/Basic/Diagnostic.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {
class DiagnosticOptions;
class DiagnosticsEngine;
} // namespace language::Core

using toolchain::IntrusiveRefCntPtr;
using toolchain::raw_ostream;

namespace language::Compability::frontend {
class TextDiagnostic;

class TextDiagnosticPrinter : public language::Core::DiagnosticConsumer {
  raw_ostream &os;
  language::Core::DiagnosticOptions &diagOpts;

  /// A string to prefix to error messages.
  std::string prefix;

public:
  TextDiagnosticPrinter(raw_ostream &os, language::Core::DiagnosticOptions &diags);
  ~TextDiagnosticPrinter() override;

  /// Set the diagnostic printer prefix string, which will be printed at the
  /// start of any diagnostics. If empty, no prefix string is used.
  void setPrefix(std::string value) { prefix = std::move(value); }

  void HandleDiagnostic(language::Core::DiagnosticsEngine::Level level,
                        const language::Core::Diagnostic &info) override;

  void printLocForRemarks(toolchain::raw_svector_ostream &diagMessageStream,
                          toolchain::StringRef &diagMsg);
};

} // namespace language::Compability::frontend

#endif
