//===--- RefactoringActionRules.h - Clang refactoring library -------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGACTIONRULES_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGACTIONRULES_H

#include "language/Core/Tooling/Refactoring/RefactoringActionRule.h"
#include "language/Core/Tooling/Refactoring/RefactoringActionRulesInternal.h"

namespace language::Core {
namespace tooling {

/// Creates a new refactoring action rule that constructs and invokes the
/// \c RuleType rule when all of the requirements are satisfied.
///
/// This function takes in a list of values whose type derives from
/// \c RefactoringActionRuleRequirement. These values describe the initiation
/// requirements that have to be satisfied by the refactoring engine before
/// the provided action rule can be constructed and invoked. The engine
/// verifies that the requirements are satisfied by evaluating them (using the
/// 'evaluate' member function) and checking that the results don't contain
/// any errors. Once all requirements are satisfied, the provided refactoring
/// rule is constructed by passing in the values returned by the requirements'
/// evaluate functions as arguments to the constructor. The rule is then invoked
/// immediately after construction.
///
/// The separation of requirements, their evaluation and the invocation of the
/// refactoring action rule allows the refactoring clients to:
///   - Disable refactoring action rules whose requirements are not supported.
///   - Gather the set of options and define a command-line / visual interface
///     that allows users to input these options without ever invoking the
///     action.
template <typename RuleType, typename... RequirementTypes>
std::unique_ptr<RefactoringActionRule>
createRefactoringActionRule(const RequirementTypes &... Requirements);

/// A set of refactoring action rules that should have unique initiation
/// requirements.
using RefactoringActionRules =
    std::vector<std::unique_ptr<RefactoringActionRule>>;

/// A type of refactoring action rule that produces source replacements in the
/// form of atomic changes.
///
/// This action rule is typically used for local refactorings that replace
/// source in a single AST unit.
class SourceChangeRefactoringRule : public RefactoringActionRuleBase {
public:
  void invoke(RefactoringResultConsumer &Consumer,
              RefactoringRuleContext &Context) final {
    Expected<AtomicChanges> Changes = createSourceReplacements(Context);
    if (!Changes)
      Consumer.handleError(Changes.takeError());
    else
      Consumer.handle(std::move(*Changes));
  }

private:
  virtual Expected<AtomicChanges>
  createSourceReplacements(RefactoringRuleContext &Context) = 0;
};

/// A type of refactoring action rule that finds a set of symbol occurrences
/// that reference a particular symbol.
///
/// This action rule is typically used for an interactive rename that allows
/// users to specify the new name and the set of selected occurrences during
/// the refactoring.
class FindSymbolOccurrencesRefactoringRule : public RefactoringActionRuleBase {
public:
  void invoke(RefactoringResultConsumer &Consumer,
              RefactoringRuleContext &Context) final {
    Expected<SymbolOccurrences> Occurrences = findSymbolOccurrences(Context);
    if (!Occurrences)
      Consumer.handleError(Occurrences.takeError());
    else
      Consumer.handle(std::move(*Occurrences));
  }

private:
  virtual Expected<SymbolOccurrences>
  findSymbolOccurrences(RefactoringRuleContext &Context) = 0;
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGACTIONRULES_H
