//===--- RefactoringActionRule.h - Clang refactoring library -------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGACTIONRULE_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGACTIONRULE_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
namespace tooling {

class RefactoringOptionVisitor;
class RefactoringResultConsumer;
class RefactoringRuleContext;

struct RefactoringDescriptor {
  /// A unique identifier for the specific refactoring.
  StringRef Name;
  /// A human readable title for the refactoring.
  StringRef Title;
  /// A human readable description of what the refactoring does.
  StringRef Description;
};

/// A common refactoring action rule interface that defines the 'invoke'
/// function that performs the refactoring operation (either fully or
/// partially).
class RefactoringActionRuleBase {
public:
  virtual ~RefactoringActionRuleBase() {}

  /// Initiates and performs a specific refactoring action.
  ///
  /// The specific rule will invoke an appropriate \c handle method on a
  /// consumer to propagate the result of the refactoring action.
  virtual void invoke(RefactoringResultConsumer &Consumer,
                      RefactoringRuleContext &Context) = 0;

  /// Returns the structure that describes the refactoring.
  // static const RefactoringDescriptor &describe() = 0;
};

/// A refactoring action rule is a wrapper class around a specific refactoring
/// action rule (SourceChangeRefactoringRule, etc) that, in addition to invoking
/// the action, describes the requirements that determine when the action can be
/// initiated.
class RefactoringActionRule : public RefactoringActionRuleBase {
public:
  /// Returns true when the rule has a source selection requirement that has
  /// to be fulfilled before refactoring can be performed.
  virtual bool hasSelectionRequirement() = 0;

  /// Traverses each refactoring option used by the rule and invokes the
  /// \c visit callback in the consumer for each option.
  ///
  /// Options are visited in the order of use, e.g. if a rule has two
  /// requirements that use options, the options from the first requirement
  /// are visited before the options in the second requirement.
  virtual void visitRefactoringOptions(RefactoringOptionVisitor &Visitor) = 0;
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGACTIONRULE_H
