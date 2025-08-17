//===--- MatchConsumer.h - MatchConsumer abstraction ------------*- C++ -*-===//
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
///
/// \file This file defines the *MatchConsumer* abstraction: a computation over
/// match results, specifically the `ast_matchers::MatchFinder::MatchResult`
/// class.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_TRANSFORMER_MATCHCONSUMER_H
#define LANGUAGE_CORE_TOOLING_TRANSFORMER_MATCHCONSUMER_H

#include "language/Core/AST/ASTTypeTraits.h"
#include "language/Core/ASTMatchers/ASTMatchFinder.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Errc.h"
#include "toolchain/Support/Error.h"

namespace language::Core {
namespace transformer {
/// A failable computation over nodes bound by AST matchers.
///
/// The computation should report any errors though its return value (rather
/// than terminating the program) to enable usage in interactive scenarios like
/// clang-query.
///
/// This is a central abstraction of the Transformer framework.
template <typename T>
using MatchConsumer =
    std::function<Expected<T>(const ast_matchers::MatchFinder::MatchResult &)>;

/// Creates an error that signals that a `MatchConsumer` expected a certain node
/// to be bound by AST matchers, but it was not actually bound.
inline toolchain::Error notBoundError(toolchain::StringRef Id) {
  return toolchain::make_error<toolchain::StringError>(toolchain::errc::invalid_argument,
                                             "Id not bound: " + Id);
}

/// Chooses between the two consumers, based on whether \p ID is bound in the
/// match.
template <typename T>
MatchConsumer<T> ifBound(std::string ID, MatchConsumer<T> TrueC,
                         MatchConsumer<T> FalseC) {
  return [=](const ast_matchers::MatchFinder::MatchResult &Result) {
    auto &Map = Result.Nodes.getMap();
    return (Map.find(ID) != Map.end() ? TrueC : FalseC)(Result);
  };
}

/// A failable computation over nodes bound by AST matchers, with (limited)
/// reflection via the `toString` method.
///
/// The computation should report any errors though its return value (rather
/// than terminating the program) to enable usage in interactive scenarios like
/// clang-query.
///
/// This is a central abstraction of the Transformer framework. It is a
/// generalization of `MatchConsumer` and intended to replace it.
template <typename T> class MatchComputation {
public:
  virtual ~MatchComputation() = default;

  /// Evaluates the computation and (potentially) updates the accumulator \c
  /// Result.  \c Result is undefined in the case of an error. `Result` is an
  /// out parameter to optimize case where the computation involves composing
  /// the result of sub-computation evaluations.
  virtual toolchain::Error eval(const ast_matchers::MatchFinder::MatchResult &Match,
                           T *Result) const = 0;

  /// Convenience version of `eval`, for the case where the computation is being
  /// evaluated on its own.
  toolchain::Expected<T> eval(const ast_matchers::MatchFinder::MatchResult &R) const;

  /// Constructs a string representation of the computation, for informational
  /// purposes. The representation must be deterministic, but is not required to
  /// be unique.
  virtual std::string toString() const = 0;

protected:
  MatchComputation() = default;

  // Since this is an abstract class, copying/assigning only make sense for
  // derived classes implementing `clone()`.
  MatchComputation(const MatchComputation &) = default;
  MatchComputation &operator=(const MatchComputation &) = default;
};

template <typename T>
toolchain::Expected<T> MatchComputation<T>::eval(
    const ast_matchers::MatchFinder::MatchResult &R) const {
  T Output;
  if (auto Err = eval(R, &Output))
    return std::move(Err);
  return Output;
}
} // namespace transformer
} // namespace language::Core
#endif // LANGUAGE_CORE_TOOLING_TRANSFORMER_MATCHCONSUMER_H
