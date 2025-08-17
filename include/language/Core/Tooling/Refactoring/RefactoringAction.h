//===--- RefactoringAction.h - Clang refactoring library ------------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGACTION_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGACTION_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Tooling/Refactoring/RefactoringActionRules.h"
#include <vector>

namespace language::Core {
namespace tooling {

/// A refactoring action is a class that defines a set of related refactoring
/// action rules. These rules get grouped under a common umbrella - a single
/// clang-refactor subcommand.
///
/// A subclass of \c RefactoringAction is responsible for creating the set of
/// grouped refactoring action rules that represent one refactoring operation.
/// Although the rules in one action may have a number of different
/// implementations, they should strive to produce a similar result. It should
/// be easy for users to identify which refactoring action produced the result
/// regardless of which refactoring action rule was used.
///
/// The distinction between actions and rules enables the creation of action
/// that uses very different rules, for example:
///   - local vs global: a refactoring operation like
///     "add missing switch cases" can be applied to one switch when it's
///     selected in an editor, or to all switches in a project when an enum
///     constant is added to an enum.
///   - tool vs editor: some refactoring operation can be initiated in the
///     editor when a declaration is selected, or in a tool when the name of
///     the declaration is passed using a command-line argument.
class RefactoringAction {
public:
  virtual ~RefactoringAction() {}

  /// Returns the name of the subcommand that's used by clang-refactor for this
  /// action.
  virtual StringRef getCommand() const = 0;

  virtual StringRef getDescription() const = 0;

  RefactoringActionRules createActiveActionRules();

protected:
  /// Returns a set of refactoring actions rules that are defined by this
  /// action.
  virtual RefactoringActionRules createActionRules() const = 0;
};

/// Returns the list of all the available refactoring actions.
std::vector<std::unique_ptr<RefactoringAction>> createRefactoringActions();

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGACTION_H
