//===- TextDiagnosticBuffer.cpp - Buffer Text Diagnostics -----------------===//
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
// This is a concrete diagnostic client, which buffers the diagnostic messages.
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Frontend/TextDiagnosticBuffer.h"
#include "language/Core/Basic/Diagnostic.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/ErrorHandling.h"

using namespace language::Compability::frontend;

/// HandleDiagnostic - Store the errors, warnings, and notes that are
/// reported.
void TextDiagnosticBuffer::HandleDiagnostic(
    language::Core::DiagnosticsEngine::Level level, const language::Core::Diagnostic &info) {
  // Default implementation (warnings_/errors count).
  DiagnosticConsumer::HandleDiagnostic(level, info);

  toolchain::SmallString<100> buf;
  info.FormatDiagnostic(buf);
  switch (level) {
  default:
    toolchain_unreachable("Diagnostic not handled during diagnostic buffering!");
  case language::Core::DiagnosticsEngine::Note:
    all.emplace_back(level, notes.size());
    notes.emplace_back(info.getLocation(), std::string(buf));
    break;
  case language::Core::DiagnosticsEngine::Warning:
    all.emplace_back(level, warnings.size());
    warnings.emplace_back(info.getLocation(), std::string(buf));
    break;
  case language::Core::DiagnosticsEngine::Remark:
    all.emplace_back(level, remarks.size());
    remarks.emplace_back(info.getLocation(), std::string(buf));
    break;
  case language::Core::DiagnosticsEngine::Error:
  case language::Core::DiagnosticsEngine::Fatal:
    all.emplace_back(level, errors.size());
    errors.emplace_back(info.getLocation(), std::string(buf));
    break;
  }
}

void TextDiagnosticBuffer::flushDiagnostics(
    language::Core::DiagnosticsEngine &diags) const {
  for (const auto &i : all) {
    auto diag = diags.Report(diags.getCustomDiagID(i.first, "%0"));
    switch (i.first) {
    default:
      toolchain_unreachable("Diagnostic not handled during diagnostic flushing!");
    case language::Core::DiagnosticsEngine::Note:
      diag << notes[i.second].second;
      break;
    case language::Core::DiagnosticsEngine::Warning:
      diag << warnings[i.second].second;
      break;
    case language::Core::DiagnosticsEngine::Remark:
      diag << remarks[i.second].second;
      break;
    case language::Core::DiagnosticsEngine::Error:
    case language::Core::DiagnosticsEngine::Fatal:
      diag << errors[i.second].second;
      break;
    }
  }
}
