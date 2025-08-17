//===- TypeErasedDataflowAnalysis.h -----------------------------*- C++ -*-===//
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
//  This file defines type-erased base types and functions for building dataflow
//  analyses that run over Control-Flow Graphs (CFGs).
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_TYPEERASEDDATAFLOWANALYSIS_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_TYPEERASEDDATAFLOWANALYSIS_H

#include <optional>
#include <utility>
#include <vector>

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/Analysis/CFG.h"
#include "language/Core/Analysis/FlowSensitive/AdornedCFG.h"
#include "language/Core/Analysis/FlowSensitive/DataflowAnalysisContext.h"
#include "language/Core/Analysis/FlowSensitive/DataflowEnvironment.h"
#include "language/Core/Analysis/FlowSensitive/DataflowLattice.h"
#include "toolchain/ADT/Any.h"
#include "toolchain/Support/Error.h"

namespace language::Core {
namespace dataflow {

struct DataflowAnalysisOptions {
  /// Options for the built-in model, or empty to not apply them.
  // FIXME: Remove this option once the framework supports composing analyses
  // (at which point the built-in transfer functions can be simply a standalone
  // analysis).
  std::optional<DataflowAnalysisContext::Options> BuiltinOpts =
      DataflowAnalysisContext::Options{};
};

/// Type-erased lattice element container.
///
/// Requirements:
///
///  The type of the object stored in the container must be a bounded
///  join-semilattice.
struct TypeErasedLattice {
  toolchain::Any Value;
};

/// Type-erased base class for dataflow analyses built on a single lattice type.
class TypeErasedDataflowAnalysis : public Environment::ValueModel {
  DataflowAnalysisOptions Options;

public:
  TypeErasedDataflowAnalysis() : Options({}) {}

  TypeErasedDataflowAnalysis(DataflowAnalysisOptions Options)
      : Options(Options) {}

  virtual ~TypeErasedDataflowAnalysis() {}

  /// Returns the `ASTContext` that is used by the analysis.
  virtual ASTContext &getASTContext() = 0;

  /// Returns a type-erased lattice element that models the initial state of a
  /// basic block.
  virtual TypeErasedLattice typeErasedInitialElement() = 0;

  /// Joins two type-erased lattice elements by computing their least upper
  /// bound. Places the join result in the left element and returns an effect
  /// indicating whether any changes were made to it.
  virtual TypeErasedLattice joinTypeErased(const TypeErasedLattice &,
                                           const TypeErasedLattice &) = 0;

  /// Chooses a lattice element that approximates the current element at a
  /// program point, given the previous element at that point. Places the
  /// widened result in the current element (`Current`). Widening is optional --
  /// it is only needed to either accelerate convergence (for lattices with
  /// non-trivial height) or guarantee convergence (for lattices with infinite
  /// height).
  ///
  /// Returns an indication of whether any changes were made to `Current` in
  /// order to widen. This saves a separate call to `isEqualTypeErased` after
  /// the widening.
  virtual LatticeJoinEffect
  widenTypeErased(TypeErasedLattice &Current,
                  const TypeErasedLattice &Previous) = 0;

  /// Returns true if and only if the two given type-erased lattice elements are
  /// equal.
  virtual bool isEqualTypeErased(const TypeErasedLattice &,
                                 const TypeErasedLattice &) = 0;

  /// Applies the analysis transfer function for a given control flow graph
  /// element and type-erased lattice element.
  virtual void transferTypeErased(const CFGElement &, TypeErasedLattice &,
                                  Environment &) = 0;

  /// Applies the analysis transfer function for a given edge from a CFG block
  /// of a conditional statement.
  /// @param Stmt The condition which is responsible for the split in the CFG.
  /// @param Branch True if the edge goes to the basic block where the
  /// condition is true.
  // FIXME: Change `Stmt` argument to a reference.
  virtual void transferBranchTypeErased(bool Branch, const Stmt *,
                                        TypeErasedLattice &, Environment &) = 0;

  /// If the built-in model is enabled, returns the options to be passed to
  /// them. Otherwise returns empty.
  const std::optional<DataflowAnalysisContext::Options> &
  builtinOptions() const {
    return Options.BuiltinOpts;
  }
};

/// Type-erased model of the program at a given program point.
struct TypeErasedDataflowAnalysisState {
  /// Type-erased model of a program property.
  TypeErasedLattice Lattice;

  /// Model of the state of the program (store and heap).
  Environment Env;

  TypeErasedDataflowAnalysisState(TypeErasedLattice Lattice, Environment Env)
      : Lattice(std::move(Lattice)), Env(std::move(Env)) {}

  TypeErasedDataflowAnalysisState fork() const {
    return TypeErasedDataflowAnalysisState(Lattice, Env.fork());
  }
};

/// A callback to be called with the state before or after visiting a CFG
/// element.
using CFGEltCallbackTypeErased = std::function<void(
    const CFGElement &, const TypeErasedDataflowAnalysisState &)>;

/// A pair of callbacks to be called with the state before and after visiting a
/// CFG element.
/// Either or both of the callbacks may be null.
struct CFGEltCallbacksTypeErased {
  CFGEltCallbackTypeErased Before;
  CFGEltCallbackTypeErased After;
};

/// Performs dataflow analysis and returns a mapping from basic block IDs to
/// dataflow analysis states that model the respective basic blocks. Indices of
/// the returned vector correspond to basic block IDs. Returns an error if the
/// dataflow analysis cannot be performed successfully. Otherwise, calls
/// `PostAnalysisCallbacks` on each CFG element with the final analysis results
/// before and after that program point.
///
/// `MaxBlockVisits` caps the number of block visits during analysis. It doesn't
/// distinguish between repeat visits to the same block and visits to distinct
/// blocks. This parameter is a backstop to prevent infinite loops, in the case
/// of bugs in the lattice and/or transfer functions that prevent the analysis
/// from converging.
toolchain::Expected<std::vector<std::optional<TypeErasedDataflowAnalysisState>>>
runTypeErasedDataflowAnalysis(
    const AdornedCFG &ACFG, TypeErasedDataflowAnalysis &Analysis,
    const Environment &InitEnv,
    const CFGEltCallbacksTypeErased &PostAnalysisCallbacks,
    std::int32_t MaxBlockVisits);

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_TYPEERASEDDATAFLOWANALYSIS_H
