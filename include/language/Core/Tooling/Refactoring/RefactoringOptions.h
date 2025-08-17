//===--- RefactoringOptions.h - Clang refactoring library -----------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGOPTIONS_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGOPTIONS_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Tooling/Refactoring/RefactoringActionRuleRequirements.h"
#include "language/Core/Tooling/Refactoring/RefactoringOption.h"
#include "language/Core/Tooling/Refactoring/RefactoringOptionVisitor.h"
#include "toolchain/Support/Error.h"
#include <optional>
#include <type_traits>

namespace language::Core {
namespace tooling {

/// A refactoring option that stores a value of type \c T.
template <typename T,
          typename = std::enable_if_t<traits::IsValidOptionType<T>::value>>
class OptionalRefactoringOption : public RefactoringOption {
public:
  void passToVisitor(RefactoringOptionVisitor &Visitor) final {
    Visitor.visit(*this, Value);
  }

  bool isRequired() const override { return false; }

  using ValueType = std::optional<T>;

  const ValueType &getValue() const { return Value; }

protected:
  std::optional<T> Value;
};

/// A required refactoring option that stores a value of type \c T.
template <typename T,
          typename = std::enable_if_t<traits::IsValidOptionType<T>::value>>
class RequiredRefactoringOption : public OptionalRefactoringOption<T> {
public:
  using ValueType = T;

  const ValueType &getValue() const {
    return *OptionalRefactoringOption<T>::Value;
  }
  bool isRequired() const final { return true; }
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGOPTIONS_H
