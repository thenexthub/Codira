//===--- RefactoringOption.h - Clang refactoring library ------------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGOPTION_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGOPTION_H

#include "language/Core/Basic/LLVM.h"
#include <memory>
#include <type_traits>

namespace language::Core {
namespace tooling {

class RefactoringOptionVisitor;

/// A refactoring option is an interface that describes a value that
/// has an impact on the outcome of a refactoring.
///
/// Refactoring options can be specified using command-line arguments when
/// the clang-refactor tool is used.
class RefactoringOption {
public:
  virtual ~RefactoringOption() {}

  /// Returns the name of the refactoring option.
  ///
  /// Each refactoring option must have a unique name.
  virtual StringRef getName() const = 0;

  virtual StringRef getDescription() const = 0;

  /// True when this option must be specified before invoking the refactoring
  /// action.
  virtual bool isRequired() const = 0;

  /// Invokes the \c visit method in the option consumer that's appropriate
  /// for the option's value type.
  ///
  /// For example, if the option stores a string value, this method will
  /// invoke the \c visit method with a reference to an std::string value.
  virtual void passToVisitor(RefactoringOptionVisitor &Visitor) = 0;
};

/// Constructs a refactoring option of the given type.
///
/// The ownership of options is shared among requirements that use it because
/// one option can be used by multiple rules in a refactoring action.
template <typename OptionType>
std::shared_ptr<OptionType> createRefactoringOption() {
  static_assert(std::is_base_of<RefactoringOption, OptionType>::value,
                "invalid option type");
  return std::make_shared<OptionType>();
}

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGOPTION_H
