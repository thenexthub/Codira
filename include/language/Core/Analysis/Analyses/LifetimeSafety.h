//===- LifetimeSafety.h - C++ Lifetime Safety Analysis -*----------- C++-*-===//
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
// This file defines the entry point for a dataflow-based static analysis
// that checks for C++ lifetime violations.
//
// The analysis is based on the concepts of "origins" and "loans" to track
// pointer lifetimes and detect issues like use-after-free and dangling
// pointers. See the RFC for more details:
// https://discourse.toolchain.org/t/rfc-intra-procedural-lifetime-analysis-in-clang/86291
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_ANALYSIS_ANALYSES_LIFETIMESAFETY_H
#define LANGUAGE_CORE_ANALYSIS_ANALYSES_LIFETIMESAFETY_H
#include "language/Core/Analysis/AnalysisDeclContext.h"
#include "language/Core/Analysis/CFG.h"
#include "toolchain/ADT/ImmutableSet.h"
#include "toolchain/ADT/StringMap.h"
#include <memory>

namespace language::Core::lifetimes {

/// The main entry point for the analysis.
void runLifetimeSafetyAnalysis(AnalysisDeclContext &AC);

namespace internal {
// Forward declarations of internal types.
class Fact;
class FactManager;
class LoanPropagationAnalysis;
class ExpiredLoansAnalysis;
struct LifetimeFactory;

/// A generic, type-safe wrapper for an ID, distinguished by its `Tag` type.
/// Used for giving ID to loans and origins.
template <typename Tag> struct ID {
  uint32_t Value = 0;

  bool operator==(const ID<Tag> &Other) const { return Value == Other.Value; }
  bool operator!=(const ID<Tag> &Other) const { return !(*this == Other); }
  bool operator<(const ID<Tag> &Other) const { return Value < Other.Value; }
  ID<Tag> operator++(int) {
    ID<Tag> Tmp = *this;
    ++Value;
    return Tmp;
  }
  void Profile(toolchain::FoldingSetNodeID &IDBuilder) const {
    IDBuilder.AddInteger(Value);
  }
};
template <typename Tag>
inline toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS, ID<Tag> ID) {
  return OS << ID.Value;
}

using LoanID = ID<struct LoanTag>;
using OriginID = ID<struct OriginTag>;

// Using LLVM's immutable collections is efficient for dataflow analysis
// as it avoids deep copies during state transitions.
// TODO(opt): Consider using a bitset to represent the set of loans.
using LoanSet = toolchain::ImmutableSet<LoanID>;
using OriginSet = toolchain::ImmutableSet<OriginID>;

/// A `ProgramPoint` identifies a location in the CFG by pointing to a specific
/// `Fact`. identified by a lifetime-related event (`Fact`).
///
/// A `ProgramPoint` has "after" semantics: it represents the location
/// immediately after its corresponding `Fact`.
using ProgramPoint = const Fact *;

/// Running the lifetime safety analysis and querying its results. It
/// encapsulates the various dataflow analyses.
class LifetimeSafetyAnalysis {
public:
  LifetimeSafetyAnalysis(AnalysisDeclContext &AC);
  ~LifetimeSafetyAnalysis();

  void run();

  /// Returns the set of loans an origin holds at a specific program point.
  LoanSet getLoansAtPoint(OriginID OID, ProgramPoint PP) const;

  /// Returns the set of loans that have expired at a specific program point.
  LoanSet getExpiredLoansAtPoint(ProgramPoint PP) const;

  /// Finds the OriginID for a given declaration.
  /// Returns a null optional if not found.
  std::optional<OriginID> getOriginIDForDecl(const ValueDecl *D) const;

  /// Finds the LoanID's for the loan created with the specific variable as
  /// their Path.
  std::vector<LoanID> getLoanIDForVar(const VarDecl *VD) const;

  /// Retrieves program points that were specially marked in the source code
  /// for testing.
  ///
  /// The analysis recognizes special function calls of the form
  /// `void("__lifetime_test_point_<name>")` as test points. This method returns
  /// a map from the annotation string (<name>) to the corresponding
  /// `ProgramPoint`. This allows test harnesses to query the analysis state at
  /// user-defined locations in the code.
  /// \note This is intended for testing only.
  toolchain::StringMap<ProgramPoint> getTestPoints() const;

private:
  AnalysisDeclContext &AC;
  std::unique_ptr<LifetimeFactory> Factory;
  std::unique_ptr<FactManager> FactMgr;
  std::unique_ptr<LoanPropagationAnalysis> LoanPropagation;
  std::unique_ptr<ExpiredLoansAnalysis> ExpiredLoans;
};
} // namespace internal
} // namespace language::Core::lifetimes

#endif // LANGUAGE_CORE_ANALYSIS_ANALYSES_LIFETIMESAFETY_H
