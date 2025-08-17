//===- CreateCheckerManager.cpp - Checker Manager constructor ---*- C++ -*-===//
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
// Defines the constructors and the destructor of the Static Analyzer Checker
// Manager which cannot be placed under 'Core' because they depend on the
// CheckerRegistry.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Frontend/CheckerRegistry.h"
#include <memory>

namespace language::Core {
namespace ento {

CheckerManager::CheckerManager(
    ASTContext &Context, AnalyzerOptions &AOptions, const Preprocessor &PP,
    ArrayRef<std::string> plugins,
    ArrayRef<std::function<void(CheckerRegistry &)>> checkerRegistrationFns)
    : Context(&Context), LangOpts(Context.getLangOpts()), AOptions(AOptions),
      PP(&PP), Diags(Context.getDiagnostics()),
      RegistryData(std::make_unique<CheckerRegistryData>()) {
  CheckerRegistry Registry(*RegistryData, plugins, Context.getDiagnostics(),
                           AOptions, checkerRegistrationFns);
  Registry.initializeRegistry(*this);
  Registry.initializeManager(*this);
}

CheckerManager::CheckerManager(AnalyzerOptions &AOptions,
                               const LangOptions &LangOpts,
                               DiagnosticsEngine &Diags,
                               ArrayRef<std::string> plugins)
    : LangOpts(LangOpts), AOptions(AOptions), Diags(Diags),
      RegistryData(std::make_unique<CheckerRegistryData>()) {
  CheckerRegistry Registry(*RegistryData, plugins, Diags, AOptions, {});
  Registry.initializeRegistry(*this);
}

// This is declared here to ensure that the destructors of `CheckerBase` and
// `CheckerRegistryData` are available.
CheckerManager::~CheckerManager() = default;

} // namespace ento
} // namespace language::Core
