//===--- ASTSelectionRequirements.cpp - Clang refactoring library ---------===//
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

#include "language/Core/Tooling/Refactoring/RefactoringActionRuleRequirements.h"
#include <optional>

using namespace language::Core;
using namespace tooling;

Expected<SelectedASTNode>
ASTSelectionRequirement::evaluate(RefactoringRuleContext &Context) const {
  // FIXME: Memoize so that selection is evaluated only once.
  Expected<SourceRange> Range =
      SourceRangeSelectionRequirement::evaluate(Context);
  if (!Range)
    return Range.takeError();

  std::optional<SelectedASTNode> Selection =
      findSelectedASTNodes(Context.getASTContext(), *Range);
  if (!Selection)
    return Context.createDiagnosticError(
        Range->getBegin(), diag::err_refactor_selection_invalid_ast);
  return std::move(*Selection);
}

Expected<CodeRangeASTSelection> CodeRangeASTSelectionRequirement::evaluate(
    RefactoringRuleContext &Context) const {
  // FIXME: Memoize so that selection is evaluated only once.
  Expected<SelectedASTNode> ASTSelection =
      ASTSelectionRequirement::evaluate(Context);
  if (!ASTSelection)
    return ASTSelection.takeError();
  std::unique_ptr<SelectedASTNode> StoredSelection =
      std::make_unique<SelectedASTNode>(std::move(*ASTSelection));
  std::optional<CodeRangeASTSelection> CodeRange =
      CodeRangeASTSelection::create(Context.getSelectionRange(),
                                    *StoredSelection);
  if (!CodeRange)
    return Context.createDiagnosticError(
        Context.getSelectionRange().getBegin(),
        diag::err_refactor_selection_invalid_ast);
  Context.setASTSelection(std::move(StoredSelection));
  return std::move(*CodeRange);
}
