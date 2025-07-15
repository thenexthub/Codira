//===--- SolutionResult.h - Constraint System Solution ----------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file defines the SolutionResult class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_TYPECHECK_SOLUTION_RESULT_H
#define LANGUAGE_TYPECHECK_SOLUTION_RESULT_H

#include "toolchain/ADT/ArrayRef.h"
#include <optional>

namespace language {

using toolchain::ArrayRef;

namespace constraints {

class Solution;

/// Describes the result of solving a constraint system, after
/// potentially taking various corrective actions.
class SolutionResult {
public:
  enum Kind : unsigned char {
    /// The constraint system was successfully solved, and one can
    /// retrieve the resulting solution.
    Success,
    /// The constraint system had multiple solutions, none of which
    /// was better than the others.
    Ambiguous,
    /// The constraint system had no solution, and a diagnostic has
    /// already been emitted.
    Error,
    /// The constraint system had no solution, but no diagnostic has
    /// been emitted yet.
    UndiagnosedError,
    /// The constraint system was too complex to solve, but no
    /// diagnostic has been emitted yet.
    TooComplex,
  };

private:
  /// The kind of solution result.
  Kind kind;

  /// Whether the client has emitted a diagnostic.
  unsigned emittedDiagnostic : 1;

  /// The number of solutions owned by this result.
  unsigned numSolutions = 0;

  /// A pointer to the set of solutions, of which there are
  /// \c numSolutions entries.
  Solution *solutions = nullptr;

  /// A source range that was too complex to solve.
  std::optional<SourceRange> TooComplexAt = std::nullopt;

  /// General constructor for the named constructors.
  SolutionResult(Kind kind) : kind(kind) {
    emittedDiagnostic = false;
  }

public:
  SolutionResult(const SolutionResult &other) = delete;

  SolutionResult(SolutionResult &&other)
      : kind(other.kind), numSolutions(other.numSolutions),
        solutions(other.solutions) {
    emittedDiagnostic = false;
    other.kind = Error;
    other.numSolutions = 0;
    other.solutions = nullptr;
  }

  SolutionResult &operator=(const SolutionResult &other) = delete;
  SolutionResult &operator=(SolutionResult &&other) = delete;

  ~SolutionResult();

  /// Produce a "solved" result, embedding the given solution.
  static SolutionResult forSolved(Solution &&solution);

  /// Produce an "ambiguous" result, providing the set of
  /// potential solutions.
  static SolutionResult forAmbiguous(MutableArrayRef<Solution> solutions);

  /// Produce a "too complex" failure, which was not yet been
  /// diagnosed.
  static SolutionResult forTooComplex(std::optional<SourceRange> affected);

  /// Produce a failure that has already been diagnosed.
  static SolutionResult forError() {
    return SolutionResult(Error);
  }

  /// Produce a failure that has not yet been diagnosed.
  static SolutionResult forUndiagnosedError() {
    return SolutionResult(UndiagnosedError);
  }

  Kind getKind() const{ return kind; }

  /// Retrieve the solution, where there is one.
  const Solution &getSolution() const;

  /// Retrieve the solution, where there is one.
  Solution &&takeSolution() &&;

  /// Retrieve the set of solutions when there is an ambiguity.
  ArrayRef<Solution> getAmbiguousSolutions() const;

  /// Take the set of solutions when there is an ambiguity.
  MutableArrayRef<Solution> takeAmbiguousSolutions() &&;

  /// Retrieve a range of source that has been determined to be too
  /// complex to solve in a reasonable time.
  std::optional<SourceRange> getTooComplexAt() const { return TooComplexAt; }

  /// Whether this solution requires the client to produce a diagnostic.
  bool requiresDiagnostic() const {
    switch (kind) {
    case Success:
    case Ambiguous:
    case Error:
      return false;

    case UndiagnosedError:
    case TooComplex:
      return true;
    }
    toolchain_unreachable("invalid diagnostic kind");
  }

  /// Note that the failure has been diagnosed.
  void markAsDiagnosed() {
    emittedDiagnostic = true;
  }
};

} }

#endif /* LANGUAGE_TYPECHECK_SOLUTION_RESULT_H */
