//===--- DiagnosticError.h - Diagnostic payload for toolchain::Error -*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_BASIC_DIAGNOSTICERROR_H
#define LANGUAGE_CORE_BASIC_DIAGNOSTICERROR_H

#include "language/Core/Basic/PartialDiagnostic.h"
#include "toolchain/Support/Error.h"
#include <optional>

namespace language::Core {

/// Carries a Clang diagnostic in an toolchain::Error.
///
/// Users should emit the stored diagnostic using the DiagnosticsEngine.
class DiagnosticError : public toolchain::ErrorInfo<DiagnosticError> {
public:
  DiagnosticError(PartialDiagnosticAt Diag) : Diag(std::move(Diag)) {}

  void log(raw_ostream &OS) const override { OS << "clang diagnostic"; }

  PartialDiagnosticAt &getDiagnostic() { return Diag; }
  const PartialDiagnosticAt &getDiagnostic() const { return Diag; }

  /// Creates a new \c DiagnosticError that contains the given diagnostic at
  /// the given location.
  static toolchain::Error create(SourceLocation Loc, PartialDiagnostic Diag) {
    return toolchain::make_error<DiagnosticError>(
        PartialDiagnosticAt(Loc, std::move(Diag)));
  }

  /// Extracts and returns the diagnostic payload from the given \c Error if
  /// the error is a \c DiagnosticError. Returns std::nullopt if the given error
  /// is not a \c DiagnosticError.
  static std::optional<PartialDiagnosticAt> take(toolchain::Error &Err) {
    std::optional<PartialDiagnosticAt> Result;
    Err = toolchain::handleErrors(std::move(Err), [&](DiagnosticError &E) {
      Result = std::move(E.getDiagnostic());
    });
    return Result;
  }

  static char ID;

private:
  // Users are not expected to use error_code.
  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }

  PartialDiagnosticAt Diag;
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_BASIC_DIAGNOSTICERROR_H
