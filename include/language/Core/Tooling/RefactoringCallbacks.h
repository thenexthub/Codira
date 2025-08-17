//===--- RefactoringCallbacks.h - Structural query framework ----*- C++ -*-===//
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
//  Provides callbacks to make common kinds of refactorings easy.
//
//  The general idea is to construct a matcher expression that describes a
//  subtree match on the AST and then replace the corresponding source code
//  either by some specific text or some other AST node.
//
//  Example:
//  int main(int argc, char **argv) {
//    ClangTool Tool(argc, argv);
//    MatchFinder Finder;
//    ReplaceStmtWithText Callback("integer", "42");
//    Finder.AddMatcher(id("integer", expression(integerLiteral())), Callback);
//    return Tool.run(newFrontendActionFactory(&Finder));
//  }
//
//  This will replace all integer literals with "42".
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_REFACTORINGCALLBACKS_H
#define LANGUAGE_CORE_TOOLING_REFACTORINGCALLBACKS_H

#include "language/Core/ASTMatchers/ASTMatchFinder.h"
#include "language/Core/Tooling/Refactoring.h"

namespace language::Core {
namespace tooling {

/// Base class for RefactoringCallbacks.
///
/// Collects \c tooling::Replacements while running.
class RefactoringCallback : public ast_matchers::MatchFinder::MatchCallback {
public:
  RefactoringCallback();
  Replacements &getReplacements();

protected:
  Replacements Replace;
};

/// Adaptor between \c ast_matchers::MatchFinder and \c
/// tooling::RefactoringTool.
///
/// Runs AST matchers and stores the \c tooling::Replacements in a map.
class ASTMatchRefactorer {
public:
  explicit ASTMatchRefactorer(
    std::map<std::string, Replacements> &FileToReplaces);

  template <typename T>
  void addMatcher(const T &Matcher, RefactoringCallback *Callback) {
    MatchFinder.addMatcher(Matcher, Callback);
    Callbacks.push_back(Callback);
  }

  void addDynamicMatcher(const ast_matchers::internal::DynTypedMatcher &Matcher,
                         RefactoringCallback *Callback);

  std::unique_ptr<ASTConsumer> newASTConsumer();

private:
  friend class RefactoringASTConsumer;
  std::vector<RefactoringCallback *> Callbacks;
  ast_matchers::MatchFinder MatchFinder;
  std::map<std::string, Replacements> &FileToReplaces;
};

/// Replace the text of the statement bound to \c FromId with the text in
/// \c ToText.
class ReplaceStmtWithText : public RefactoringCallback {
public:
  ReplaceStmtWithText(StringRef FromId, StringRef ToText);
  void run(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  std::string FromId;
  std::string ToText;
};

/// Replace the text of an AST node bound to \c FromId with the result of
/// evaluating the template in \c ToTemplate.
///
/// Expressions of the form ${NodeName} in \c ToTemplate will be
/// replaced by the text of the node bound to ${NodeName}. The string
/// "$$" will be replaced by "$".
class ReplaceNodeWithTemplate : public RefactoringCallback {
public:
  static toolchain::Expected<std::unique_ptr<ReplaceNodeWithTemplate>>
  create(StringRef FromId, StringRef ToTemplate);
  void run(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  struct TemplateElement {
    enum { Literal, Identifier } Type;
    std::string Value;
  };
  ReplaceNodeWithTemplate(toolchain::StringRef FromId,
                          std::vector<TemplateElement> Template);
  std::string FromId;
  std::vector<TemplateElement> Template;
};

/// Replace the text of the statement bound to \c FromId with the text of
/// the statement bound to \c ToId.
class ReplaceStmtWithStmt : public RefactoringCallback {
public:
  ReplaceStmtWithStmt(StringRef FromId, StringRef ToId);
  void run(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  std::string FromId;
  std::string ToId;
};

/// Replace an if-statement bound to \c Id with the outdented text of its
/// body, choosing the consequent or the alternative based on whether
/// \c PickTrueBranch is true.
class ReplaceIfStmtWithItsBody : public RefactoringCallback {
public:
  ReplaceIfStmtWithItsBody(StringRef Id, bool PickTrueBranch);
  void run(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  std::string Id;
  const bool PickTrueBranch;
};

} // end namespace tooling
} // end namespace language::Core

#endif
