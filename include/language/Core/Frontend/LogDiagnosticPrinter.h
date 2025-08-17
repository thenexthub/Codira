//===--- LogDiagnosticPrinter.h - Log Diagnostic Client ---------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_FRONTEND_LOGDIAGNOSTICPRINTER_H
#define LANGUAGE_CORE_FRONTEND_LOGDIAGNOSTICPRINTER_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
class DiagnosticOptions;
class LangOptions;

class LogDiagnosticPrinter : public DiagnosticConsumer {
  struct DiagEntry {
    /// The primary message line of the diagnostic.
    std::string Message;

    /// The source file name, if available.
    std::string Filename;

    /// The source file line number, if available.
    unsigned Line;

    /// The source file column number, if available.
    unsigned Column;

    /// The ID of the diagnostic.
    unsigned DiagnosticID;

    /// The Option Flag for the diagnostic
    std::string WarningOption;

    /// The level of the diagnostic.
    DiagnosticsEngine::Level DiagnosticLevel;
  };

  void EmitDiagEntry(toolchain::raw_ostream &OS,
                     const LogDiagnosticPrinter::DiagEntry &DE);

  // Conditional ownership (when StreamOwner is non-null, it's keeping OS
  // alive). We might want to replace this with a wrapper for conditional
  // ownership eventually - it seems to pop up often enough.
  raw_ostream &OS;
  std::unique_ptr<raw_ostream> StreamOwner;
  const LangOptions *LangOpts;

  SourceLocation LastWarningLoc;
  FullSourceLoc LastLoc;

  SmallVector<DiagEntry, 8> Entries;

  std::string MainFilename;
  std::string DwarfDebugFlags;

public:
  LogDiagnosticPrinter(raw_ostream &OS, DiagnosticOptions &DiagOpts,
                       std::unique_ptr<raw_ostream> StreamOwner);

  void setDwarfDebugFlags(StringRef Value) {
    DwarfDebugFlags = std::string(Value);
  }

  void BeginSourceFile(const LangOptions &LO, const Preprocessor *PP) override {
    LangOpts = &LO;
  }

  void EndSourceFile() override;

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override;
};

} // end namespace language::Core

#endif
