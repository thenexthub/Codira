//===- Solver.h -------------------------------------------------*- C++ -*-===//
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
//
//  This file defines an interface for a SAT solver that can be used by
//  dataflow analyses.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_SOLVER_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_SOLVER_H

#include "language/Core/Analysis/FlowSensitive/Formula.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include <optional>
#include <vector>

namespace language::Core {
namespace dataflow {

/// An interface for a SAT solver that can be used by dataflow analyses.
class Solver {
public:
  struct Result {
    enum class Status {
      /// Indicates that there exists a satisfying assignment for a boolean
      /// formula.
      Satisfiable,

      /// Indicates that there is no satisfying assignment for a boolean
      /// formula.
      Unsatisfiable,

      /// Indicates that the solver gave up trying to find a satisfying
      /// assignment for a boolean formula.
      TimedOut,
    };

    /// A boolean value is set to true or false in a truth assignment.
    enum class Assignment : uint8_t { AssignedFalse = 0, AssignedTrue = 1 };

    /// Constructs a result indicating that the queried boolean formula is
    /// satisfiable. The result will hold a solution found by the solver.
    static Result Satisfiable(toolchain::DenseMap<Atom, Assignment> Solution) {
      return Result(Status::Satisfiable, std::move(Solution));
    }

    /// Constructs a result indicating that the queried boolean formula is
    /// unsatisfiable.
    static Result Unsatisfiable() { return Result(Status::Unsatisfiable, {}); }

    /// Constructs a result indicating that satisfiability checking on the
    /// queried boolean formula was not completed.
    static Result TimedOut() { return Result(Status::TimedOut, {}); }

    /// Returns the status of satisfiability checking on the queried boolean
    /// formula.
    Status getStatus() const { return SATCheckStatus; }

    /// Returns a truth assignment to boolean values that satisfies the queried
    /// boolean formula if available. Otherwise, an empty optional is returned.
    std::optional<toolchain::DenseMap<Atom, Assignment>> getSolution() const {
      return Solution;
    }

  private:
    Result(Status SATCheckStatus,
           std::optional<toolchain::DenseMap<Atom, Assignment>> Solution)
        : SATCheckStatus(SATCheckStatus), Solution(std::move(Solution)) {}

    Status SATCheckStatus;
    std::optional<toolchain::DenseMap<Atom, Assignment>> Solution;
  };

  virtual ~Solver() = default;

  /// Checks if the conjunction of `Vals` is satisfiable and returns the
  /// corresponding result.
  ///
  /// Requirements:
  ///
  ///  All elements in `Vals` must not be null.
  virtual Result solve(toolchain::ArrayRef<const Formula *> Vals) = 0;

  // Did the solver reach its resource limit?
  virtual bool reachedLimit() const = 0;
};

toolchain::raw_ostream &operator<<(toolchain::raw_ostream &, const Solver::Result &);
toolchain::raw_ostream &operator<<(toolchain::raw_ostream &, Solver::Result::Assignment);

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_SOLVER_H
