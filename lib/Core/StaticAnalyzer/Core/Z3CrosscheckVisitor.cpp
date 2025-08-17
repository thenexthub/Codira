//===- Z3CrosscheckVisitor.cpp - Crosscheck reports with Z3 -----*- C++ -*-===//
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
//  This file declares the visitor and utilities around it for Z3 report
//  refutation.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/BugReporter/Z3CrosscheckVisitor.h"
#include "language/Core/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/EntryPointStats.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SMTConv.h"
#include "toolchain/Support/SMTAPI.h"
#include "toolchain/Support/Timer.h"

#define DEBUG_TYPE "Z3CrosscheckOracle"

// Queries attempted at most `Z3CrosscheckMaxAttemptsPerQuery` number of times.
// Multiple `check()` calls might be called on the same query if previous
// attempts of the same query resulted in UNSAT for any reason. Each query is
// only counted once for these statistics, the retries are not accounted for.
STAT_COUNTER(NumZ3QueriesDone, "Number of Z3 queries done");
STAT_COUNTER(NumTimesZ3TimedOut, "Number of times Z3 query timed out");
STAT_COUNTER(NumTimesZ3ExhaustedRLimit,
             "Number of times Z3 query exhausted the rlimit");
STAT_COUNTER(
    NumTimesZ3SpendsTooMuchTimeOnASingleEQClass,
    "Number of times report equivalenece class was cut because it spent "
    "too much time in Z3");

STAT_COUNTER(NumTimesZ3QueryAcceptsReport,
             "Number of Z3 queries accepting a report");
STAT_COUNTER(NumTimesZ3QueryRejectReport,
             "Number of Z3 queries rejecting a report");
STAT_COUNTER(NumTimesZ3QueryRejectEQClass,
             "Number of times rejecting an report equivalenece class");

STAT_COUNTER(TimeSpentSolvingZ3Queries,
             "Total time spent solving Z3 queries excluding retries");
STAT_MAX(MaxTimeSpentSolvingZ3Queries,
         "Max time spent solving a Z3 query excluding retries");

using namespace language::Core;
using namespace ento;

Z3CrosscheckVisitor::Z3CrosscheckVisitor(Z3CrosscheckVisitor::Z3Result &Result,
                                         const AnalyzerOptions &Opts)
    : Constraints(ConstraintMap::Factory().getEmptyMap()), Result(Result),
      Opts(Opts) {}

void Z3CrosscheckVisitor::finalizeVisitor(BugReporterContext &BRC,
                                          const ExplodedNode *EndPathNode,
                                          PathSensitiveBugReport &BR) {
  // Collect new constraints
  addConstraints(EndPathNode, /*OverwriteConstraintsOnExistingSyms=*/true);

  // Create a refutation manager
  toolchain::SMTSolverRef RefutationSolver = toolchain::CreateZ3Solver();
  if (Opts.Z3CrosscheckRLimitThreshold)
    RefutationSolver->setUnsignedParam("rlimit",
                                       Opts.Z3CrosscheckRLimitThreshold);
  if (Opts.Z3CrosscheckTimeoutThreshold)
    RefutationSolver->setUnsignedParam("timeout",
                                       Opts.Z3CrosscheckTimeoutThreshold); // ms

  ASTContext &Ctx = BRC.getASTContext();

  // Add constraints to the solver
  for (const auto &[Sym, Range] : Constraints) {
    auto RangeIt = Range.begin();

    toolchain::SMTExprRef SMTConstraints = SMTConv::getRangeExpr(
        RefutationSolver, Ctx, Sym, RangeIt->From(), RangeIt->To(),
        /*InRange=*/true);
    while ((++RangeIt) != Range.end()) {
      SMTConstraints = RefutationSolver->mkOr(
          SMTConstraints, SMTConv::getRangeExpr(RefutationSolver, Ctx, Sym,
                                                RangeIt->From(), RangeIt->To(),
                                                /*InRange=*/true));
    }
    RefutationSolver->addConstraint(SMTConstraints);
  }

  auto GetUsedRLimit = [](const toolchain::SMTSolverRef &Solver) {
    return Solver->getStatistics()->getUnsigned("rlimit count");
  };

  auto AttemptOnce = [&](const toolchain::SMTSolverRef &Solver) -> Z3Result {
    auto getCurrentTime = toolchain::TimeRecord::getCurrentTime;
    unsigned InitialRLimit = GetUsedRLimit(Solver);
    double Start = getCurrentTime(/*Start=*/true).getWallTime();
    std::optional<bool> IsSAT = Solver->check();
    double End = getCurrentTime(/*Start=*/false).getWallTime();
    return {
        IsSAT,
        static_cast<unsigned>((End - Start) * 1000),
        GetUsedRLimit(Solver) - InitialRLimit,
    };
  };

  // And check for satisfiability
  unsigned MinQueryTimeAcrossAttempts = std::numeric_limits<unsigned>::max();
  for (unsigned I = 0; I < Opts.Z3CrosscheckMaxAttemptsPerQuery; ++I) {
    Result = AttemptOnce(RefutationSolver);
    Result.Z3QueryTimeMilliseconds =
        std::min(MinQueryTimeAcrossAttempts, Result.Z3QueryTimeMilliseconds);
    if (Result.IsSAT.has_value())
      return;
  }
}

void Z3CrosscheckVisitor::addConstraints(
    const ExplodedNode *N, bool OverwriteConstraintsOnExistingSyms) {
  // Collect new constraints
  ConstraintMap NewCs = getConstraintMap(N->getState());
  ConstraintMap::Factory &CF = N->getState()->get_context<ConstraintMap>();

  // Add constraints if we don't have them yet
  for (auto const &[Sym, Range] : NewCs) {
    if (!Constraints.contains(Sym)) {
      // This symbol is new, just add the constraint.
      Constraints = CF.add(Constraints, Sym, Range);
    } else if (OverwriteConstraintsOnExistingSyms) {
      // Overwrite the associated constraint of the Symbol.
      Constraints = CF.remove(Constraints, Sym);
      Constraints = CF.add(Constraints, Sym, Range);
    }
  }
}

PathDiagnosticPieceRef
Z3CrosscheckVisitor::VisitNode(const ExplodedNode *N, BugReporterContext &,
                               PathSensitiveBugReport &) {
  addConstraints(N, /*OverwriteConstraintsOnExistingSyms=*/false);
  return nullptr;
}

void Z3CrosscheckVisitor::Profile(toolchain::FoldingSetNodeID &ID) const {
  static int Tag = 0;
  ID.AddPointer(&Tag);
}

Z3CrosscheckOracle::Z3Decision Z3CrosscheckOracle::interpretQueryResult(
    const Z3CrosscheckVisitor::Z3Result &Query) {
  ++NumZ3QueriesDone;
  AccumulatedZ3QueryTimeInEqClass += Query.Z3QueryTimeMilliseconds;
  TimeSpentSolvingZ3Queries += Query.Z3QueryTimeMilliseconds;
  MaxTimeSpentSolvingZ3Queries.updateMax(Query.Z3QueryTimeMilliseconds);

  if (Query.IsSAT && Query.IsSAT.value()) {
    ++NumTimesZ3QueryAcceptsReport;
    return AcceptReport;
  }

  // Suggest cutting the EQClass if certain heuristics trigger.
  if (Opts.Z3CrosscheckTimeoutThreshold &&
      Query.Z3QueryTimeMilliseconds >= Opts.Z3CrosscheckTimeoutThreshold) {
    ++NumTimesZ3TimedOut;
    ++NumTimesZ3QueryRejectEQClass;
    return RejectEQClass;
  }

  if (Opts.Z3CrosscheckRLimitThreshold &&
      Query.UsedRLimit >= Opts.Z3CrosscheckRLimitThreshold) {
    ++NumTimesZ3ExhaustedRLimit;
    ++NumTimesZ3QueryRejectEQClass;
    return RejectEQClass;
  }

  if (Opts.Z3CrosscheckEQClassTimeoutThreshold &&
      AccumulatedZ3QueryTimeInEqClass >
          Opts.Z3CrosscheckEQClassTimeoutThreshold) {
    ++NumTimesZ3SpendsTooMuchTimeOnASingleEQClass;
    ++NumTimesZ3QueryRejectEQClass;
    return RejectEQClass;
  }

  // If no cutoff heuristics trigger, and the report is "unsat" or "undef",
  // then reject the report.
  ++NumTimesZ3QueryRejectReport;
  return RejectReport;
}
