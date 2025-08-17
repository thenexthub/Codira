//===--- RefactoringOptionVisitor.h - Clang refactoring library -----------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGOPTIONVISITOR_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGOPTIONVISITOR_H

#include "language/Core/Basic/LLVM.h"
#include <optional>
#include <type_traits>

namespace language::Core {
namespace tooling {

class RefactoringOption;

/// An interface that declares functions that handle different refactoring
/// option types.
///
/// A valid refactoring option type must have a corresponding \c visit
/// declaration in this interface.
class RefactoringOptionVisitor {
public:
  virtual ~RefactoringOptionVisitor() {}

  virtual void visit(const RefactoringOption &Opt,
                     std::optional<std::string> &Value) = 0;
};

namespace traits {
namespace internal {

template <typename T> struct HasHandle {
private:
  template <typename ClassT>
  static auto check(ClassT *) -> typename std::is_same<
      decltype(std::declval<RefactoringOptionVisitor>().visit(
          std::declval<RefactoringOption>(),
          *std::declval<std::optional<T> *>())),
      void>::type;

  template <typename> static std::false_type check(...);

public:
  using Type = decltype(check<RefactoringOptionVisitor>(nullptr));
};

} // end namespace internal

/// A type trait that returns true iff the given type is a type that can be
/// stored in a refactoring option.
template <typename T>
struct IsValidOptionType : internal::HasHandle<T>::Type {};

} // end namespace traits
} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_REFACTORINGOPTIONVISITOR_H
