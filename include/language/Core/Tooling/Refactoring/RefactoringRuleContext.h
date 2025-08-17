//===--- RefactoringRuleContext.h - Clang refactoring library -------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGRULECONTEXT_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGRULECONTEXT_H

#include "language/Core/Basic/DiagnosticError.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Tooling/Refactoring/ASTSelection.h"

namespace language::Core {

class ASTContext;

namespace tooling {

/// The refactoring rule context stores all of the inputs that might be needed
/// by a refactoring action rule. It can create the specialized
/// \c ASTRefactoringOperation or \c PreprocessorRefactoringOperation values
/// that can be used by the refactoring action rules.
///
/// The following inputs are stored by the operation:
///
///   - SourceManager: a reference to a valid source manager.
///
///   - SelectionRange: an optional source selection ranges that can be used
///     to represent a selection in an editor.
class RefactoringRuleContext {
public:
  RefactoringRuleContext(const SourceManager &SM) : SM(SM) {}

  const SourceManager &getSources() const { return SM; }

  /// Returns the current source selection range as set by the
  /// refactoring engine. Can be invalid.
  SourceRange getSelectionRange() const { return SelectionRange; }

  void setSelectionRange(SourceRange R) { SelectionRange = R; }

  bool hasASTContext() const { return AST; }

  ASTContext &getASTContext() const {
    assert(AST && "no AST!");
    return *AST;
  }

  void setASTContext(ASTContext &Context) { AST = &Context; }

  /// Creates an toolchain::Error value that contains a diagnostic.
  ///
  /// The errors should not outlive the context.
  toolchain::Error createDiagnosticError(SourceLocation Loc, unsigned DiagID) {
    return DiagnosticError::create(Loc, PartialDiagnostic(DiagID, DiagStorage));
  }

  toolchain::Error createDiagnosticError(unsigned DiagID) {
    return createDiagnosticError(SourceLocation(), DiagID);
  }

  void setASTSelection(std::unique_ptr<SelectedASTNode> Node) {
    ASTNodeSelection = std::move(Node);
  }

private:
  /// The source manager for the translation unit / file on which a refactoring
  /// action might operate on.
  const SourceManager &SM;
  /// An optional source selection range that's commonly used to represent
  /// a selection in an editor.
  SourceRange SelectionRange;
  /// An optional AST for the translation unit on which a refactoring action
  /// might operate on.
  ASTContext *AST = nullptr;
  /// The allocator for diagnostics.
  PartialDiagnostic::DiagStorageAllocator DiagStorage;

  // FIXME: Remove when memoized.
  std::unique_ptr<SelectedASTNode> ASTNodeSelection;
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGRULECONTEXT_H
