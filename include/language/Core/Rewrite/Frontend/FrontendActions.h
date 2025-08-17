//===-- FrontendActions.h - Useful Frontend Actions -------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_REWRITE_FRONTEND_FRONTENDACTIONS_H
#define LANGUAGE_CORE_REWRITE_FRONTEND_FRONTENDACTIONS_H

#include "language/Core/Frontend/FrontendAction.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {
class FixItRewriter;
class FixItOptions;

//===----------------------------------------------------------------------===//
// AST Consumer Actions
//===----------------------------------------------------------------------===//

class HTMLPrintAction : public ASTFrontendAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;
};

class FixItAction : public ASTFrontendAction {
protected:
  std::unique_ptr<FixItRewriter> Rewriter;
  std::unique_ptr<FixItOptions> FixItOpts;

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;

  bool BeginSourceFileAction(CompilerInstance &CI) override;

  void EndSourceFileAction() override;

  bool hasASTFileSupport() const override { return false; }

public:
  FixItAction();
  ~FixItAction() override;
};

/// Emits changes to temporary files and uses them for the original
/// frontend action.
class FixItRecompile : public WrapperFrontendAction {
public:
  FixItRecompile(std::unique_ptr<FrontendAction> WrappedAction)
    : WrapperFrontendAction(std::move(WrappedAction)) {}

protected:
  bool BeginInvocation(CompilerInstance &CI) override;
};

class RewriteObjCAction : public ASTFrontendAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override;
};

class RewriteMacrosAction : public PreprocessorFrontendAction {
protected:
  void ExecuteAction() override;
};

class RewriteTestAction : public PreprocessorFrontendAction {
protected:
  void ExecuteAction() override;
};

class RewriteIncludesAction : public PreprocessorFrontendAction {
  std::shared_ptr<raw_ostream> OutputStream;
  class RewriteImportsListener;
protected:
  bool BeginSourceFileAction(CompilerInstance &CI) override;
  void ExecuteAction() override;
};

}  // end namespace language::Core

#endif
