//===- TextDiagnosticBuffer.h - Buffer Text Diagnostics ---------*- C++ -*-===//
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
// This is a concrete diagnostic client. The diagnostics are buffered rather
// than printed. In order to print them, use the FlushDiagnostics method.
// Pretty-printing is not supported.
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_FRONTEND_TEXTDIAGNOSTICBUFFER_H
#define LANGUAGE_COMPABILITY_FRONTEND_TEXTDIAGNOSTICBUFFER_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/SourceLocation.h"
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace language::Compability::frontend {

class TextDiagnosticBuffer : public language::Core::DiagnosticConsumer {
public:
  using DiagList = std::vector<std::pair<language::Core::SourceLocation, std::string>>;
  using DiagnosticsLevelAndIndexPairs =
      std::vector<std::pair<language::Core::DiagnosticsEngine::Level, size_t>>;

private:
  DiagList errors, warnings, remarks, notes;

  /// All diagnostics in the order in which they were generated. That order
  /// likely doesn't correspond to user input order, but at least it keeps
  /// notes in the right places. Each pair is a diagnostic level and an index
  /// into the corresponding DiagList above.
  DiagnosticsLevelAndIndexPairs all;

public:
  void HandleDiagnostic(language::Core::DiagnosticsEngine::Level diagLevel,
                        const language::Core::Diagnostic &info) override;

  /// Flush the buffered diagnostics to a given diagnostic engine.
  void flushDiagnostics(language::Core::DiagnosticsEngine &diags) const;
};

} // namespace language::Compability::frontend

#endif // FORTRAN_FRONTEND_TEXTDIAGNOSTICBUFFER_H
