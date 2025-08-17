//===- WatchedLiteralsSolver.h ----------------------------------*- C++ -*-===//
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
//  This file defines a SAT solver implementation that can be used by dataflow
//  analyses.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_WATCHEDLITERALSSOLVER_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_WATCHEDLITERALSSOLVER_H

#include "language/Core/Analysis/FlowSensitive/Formula.h"
#include "language/Core/Analysis/FlowSensitive/Solver.h"
#include "toolchain/ADT/ArrayRef.h"

namespace language::Core {
namespace dataflow {

/// A SAT solver that is an implementation of Algorithm D from Knuth's The Art
/// of Computer Programming Volume 4: Satisfiability, Fascicle 6. It is based on
/// the Davis-Putnam-Logemann-Loveland (DPLL) algorithm [1], keeps references to
/// a single "watched" literal per clause, and uses a set of "active" variables
/// for unit propagation.
//
// [1] https://en.wikipedia.org/wiki/DPLL_algorithm
class WatchedLiteralsSolver : public Solver {
  // Count of the iterations of the main loop of the solver. This spans *all*
  // calls to the underlying solver across the life of this object. It is
  // reduced with every (non-trivial) call to the solver.
  //
  // We give control over the abstract count of iterations instead of concrete
  // measurements like CPU cycles or time to ensure deterministic results.
  std::int64_t MaxIterations = std::numeric_limits<std::int64_t>::max();

public:
  WatchedLiteralsSolver() = default;

  // `Work` specifies a computational limit on the solver. Units of "work"
  // roughly correspond to attempts to assign a value to a single
  // variable. Since the algorithm is exponential in the number of variables,
  // this is the most direct (abstract) unit to target.
  explicit WatchedLiteralsSolver(std::int64_t WorkLimit)
      : MaxIterations(WorkLimit) {}

  Result solve(toolchain::ArrayRef<const Formula *> Vals) override;

  bool reachedLimit() const override { return MaxIterations == 0; }
};

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_WATCHEDLITERALSSOLVER_H
