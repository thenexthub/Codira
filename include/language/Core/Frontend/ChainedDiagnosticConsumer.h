//===- ChainedDiagnosticConsumer.h - Chain Diagnostic Clients ---*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_FRONTEND_CHAINEDDIAGNOSTICCONSUMER_H
#define LANGUAGE_CORE_FRONTEND_CHAINEDDIAGNOSTICCONSUMER_H

#include "language/Core/Basic/Diagnostic.h"
#include <memory>

namespace language::Core {
class LangOptions;

/// ChainedDiagnosticConsumer - Chain two diagnostic clients so that diagnostics
/// go to the first client and then the second. The first diagnostic client
/// should be the "primary" client, and will be used for computing whether the
/// diagnostics should be included in counts.
class ChainedDiagnosticConsumer : public DiagnosticConsumer {
  virtual void anchor();
  std::unique_ptr<DiagnosticConsumer> OwningPrimary;
  DiagnosticConsumer *Primary;
  std::unique_ptr<DiagnosticConsumer> Secondary;

public:
  ChainedDiagnosticConsumer(std::unique_ptr<DiagnosticConsumer> Primary,
                            std::unique_ptr<DiagnosticConsumer> Secondary)
      : OwningPrimary(std::move(Primary)), Primary(OwningPrimary.get()),
        Secondary(std::move(Secondary)) {}

  /// Construct without taking ownership of \c Primary.
  ChainedDiagnosticConsumer(DiagnosticConsumer *Primary,
                            std::unique_ptr<DiagnosticConsumer> Secondary)
      : Primary(Primary), Secondary(std::move(Secondary)) {}

  void BeginSourceFile(const LangOptions &LO,
                       const Preprocessor *PP) override {
    Primary->BeginSourceFile(LO, PP);
    Secondary->BeginSourceFile(LO, PP);
  }

  void EndSourceFile() override {
    Secondary->EndSourceFile();
    Primary->EndSourceFile();
  }

  void finish() override {
    Secondary->finish();
    Primary->finish();
  }

  bool IncludeInDiagnosticCounts() const override {
    return Primary->IncludeInDiagnosticCounts();
  }

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {
    // Default implementation (Warnings/errors count).
    DiagnosticConsumer::HandleDiagnostic(DiagLevel, Info);

    Primary->HandleDiagnostic(DiagLevel, Info);
    Secondary->HandleDiagnostic(DiagLevel, Info);
  }
};

} // end namspace clang

#endif
