//===--- AnalysisConsumer.h - Front-end Analysis Engine Hooks ---*- C++ -*-===//
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
// This header contains the functions necessary for a front-end to run various
// analyses.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_FRONTEND_ANALYSISCONSUMER_H
#define LANGUAGE_CORE_STATICANALYZER_FRONTEND_ANALYSISCONSUMER_H

#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/Basic/LLVM.h"
#include <functional>
#include <memory>

namespace language::Core {

class CompilerInstance;

namespace ento {
class PathDiagnosticConsumer;
class CheckerRegistry;

class AnalysisASTConsumer : public ASTConsumer {
public:
  virtual void
  AddDiagnosticConsumer(std::unique_ptr<PathDiagnosticConsumer> Consumer) = 0;

  /// This method allows registering statically linked custom checkers that are
  /// not a part of the Clang tree. It employs the same mechanism that is used
  /// by plugins.
  ///
  /// Example:
  ///
  ///   Consumer->AddCheckerRegistrationFn([] (CheckerRegistry& Registry) {
  ///     Registry.addChecker<MyCustomChecker>("example.MyCustomChecker",
  ///                                          "Description");
  ///   });
  virtual void
  AddCheckerRegistrationFn(std::function<void(CheckerRegistry &)> Fn) = 0;
};

/// CreateAnalysisConsumer - Creates an ASTConsumer to run various code
/// analysis passes.  (The set of analyses run is controlled by command-line
/// options.)
std::unique_ptr<AnalysisASTConsumer>
CreateAnalysisConsumer(CompilerInstance &CI);

} // namespace ento

} // end clang namespace

#endif
