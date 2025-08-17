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

#include "language/Core/Frontend/TextDiagnosticBuffer.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/ErrorHandling.h"

using namespace language::Core;

/// HandleDiagnostic - Store the errors, warnings, and notes that are
/// reported.
void TextDiagnosticBuffer::HandleDiagnostic(DiagnosticsEngine::Level Level,
                                            const Diagnostic &Info) {
  // Default implementation (Warnings/errors count).
  DiagnosticConsumer::HandleDiagnostic(Level, Info);

  SmallString<100> Buf;
  Info.FormatDiagnostic(Buf);
  switch (Level) {
  default: toolchain_unreachable(
                         "Diagnostic not handled during diagnostic buffering!");
  case DiagnosticsEngine::Note:
    All.emplace_back(Level, Notes.size());
    Notes.emplace_back(Info.getLocation(), std::string(Buf));
    break;
  case DiagnosticsEngine::Warning:
    All.emplace_back(Level, Warnings.size());
    Warnings.emplace_back(Info.getLocation(), std::string(Buf));
    break;
  case DiagnosticsEngine::Remark:
    All.emplace_back(Level, Remarks.size());
    Remarks.emplace_back(Info.getLocation(), std::string(Buf));
    break;
  case DiagnosticsEngine::Error:
  case DiagnosticsEngine::Fatal:
    All.emplace_back(Level, Errors.size());
    Errors.emplace_back(Info.getLocation(), std::string(Buf));
    break;
  }
}

void TextDiagnosticBuffer::FlushDiagnostics(DiagnosticsEngine &Diags) const {
  for (const auto &I : All) {
    auto Diag = Diags.Report(Diags.getCustomDiagID(I.first, "%0"));
    switch (I.first) {
    default: toolchain_unreachable(
                           "Diagnostic not handled during diagnostic flushing!");
    case DiagnosticsEngine::Note:
      Diag << Notes[I.second].second;
      break;
    case DiagnosticsEngine::Warning:
      Diag << Warnings[I.second].second;
      break;
    case DiagnosticsEngine::Remark:
      Diag << Remarks[I.second].second;
      break;
    case DiagnosticsEngine::Error:
    case DiagnosticsEngine::Fatal:
      Diag << Errors[I.second].second;
      break;
    }
  }
}
