//===--- Extract.h - Clang refactoring library ----------------------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_EXTRACT_EXTRACT_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_EXTRACT_EXTRACT_H

#include "language/Core/Tooling/Refactoring/ASTSelection.h"
#include "language/Core/Tooling/Refactoring/RefactoringActionRules.h"
#include <optional>

namespace language::Core {
namespace tooling {

/// An "Extract Function" refactoring moves code into a new function that's
/// then called from the place where the original code was.
class ExtractFunction final : public SourceChangeRefactoringRule {
public:
  /// Initiates the extract function refactoring operation.
  ///
  /// \param Code     The selected set of statements.
  /// \param DeclName The name of the extract function. If None,
  ///                 "extracted" is used.
  static Expected<ExtractFunction>
  initiate(RefactoringRuleContext &Context, CodeRangeASTSelection Code,
           std::optional<std::string> DeclName);

  static const RefactoringDescriptor &describe();

private:
  ExtractFunction(CodeRangeASTSelection Code,
                  std::optional<std::string> DeclName)
      : Code(std::move(Code)),
        DeclName(DeclName ? std::move(*DeclName) : "extracted") {}

  Expected<AtomicChanges>
  createSourceReplacements(RefactoringRuleContext &Context) override;

  CodeRangeASTSelection Code;

  // FIXME: Account for naming collisions:
  //  - error when name is specified by user.
  //  - rename to "extractedN" when name is implicit.
  std::string DeclName;
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_EXTRACT_EXTRACT_H
