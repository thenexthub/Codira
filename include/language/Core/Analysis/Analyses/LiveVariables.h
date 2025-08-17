//===- LiveVariables.h - Live Variable Analysis for Source CFGs -*- C++ --*-//
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
// This file implements Live Variables analysis for source-level CFGs.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_ANALYSES_LIVEVARIABLES_H
#define LANGUAGE_CORE_ANALYSIS_ANALYSES_LIVEVARIABLES_H

#include "language/Core/AST/Decl.h"
#include "language/Core/Analysis/AnalysisDeclContext.h"
#include "toolchain/ADT/ImmutableSet.h"

namespace language::Core {

class CFG;
class CFGBlock;
class Stmt;
class DeclRefExpr;
class SourceManager;

class LiveVariables : public ManagedAnalysis {
public:
  class LivenessValues {
  public:

    toolchain::ImmutableSet<const Expr *> liveExprs;
    toolchain::ImmutableSet<const VarDecl *> liveDecls;
    toolchain::ImmutableSet<const BindingDecl *> liveBindings;

    bool equals(const LivenessValues &V) const;

    LivenessValues()
      : liveExprs(nullptr), liveDecls(nullptr), liveBindings(nullptr) {}

    LivenessValues(toolchain::ImmutableSet<const Expr *> liveExprs,
                   toolchain::ImmutableSet<const VarDecl *> LiveDecls,
                   toolchain::ImmutableSet<const BindingDecl *> LiveBindings)
        : liveExprs(liveExprs), liveDecls(LiveDecls),
          liveBindings(LiveBindings) {}

    bool isLive(const Expr *E) const;
    bool isLive(const VarDecl *D) const;

    friend class LiveVariables;
  };

  class Observer {
    virtual void anchor();
  public:
    virtual ~Observer() {}

    /// A callback invoked right before invoking the
    ///  liveness transfer function on the given statement.
    virtual void observeStmt(const Stmt *S,
                             const CFGBlock *currentBlock,
                             const LivenessValues& V) {}

    /// Called when the live variables analysis registers
    /// that a variable is killed.
    virtual void observerKill(const DeclRefExpr *DR) {}
  };

  ~LiveVariables() override;

  /// Compute the liveness information for a given CFG.
  static std::unique_ptr<LiveVariables>
  computeLiveness(AnalysisDeclContext &analysisContext, bool killAtAssign);

  /// Return true if a variable is live at the end of a
  /// specified block.
  bool isLive(const CFGBlock *B, const VarDecl *D);

  /// Returns true if a variable is live at the beginning of the
  ///  the statement.  This query only works if liveness information
  ///  has been recorded at the statement level (see runOnAllBlocks), and
  ///  only returns liveness information for block-level expressions.
  bool isLive(const Stmt *S, const VarDecl *D);

  /// Returns true the block-level expression value is live
  ///  before the given block-level expression (see runOnAllBlocks).
  bool isLive(const Stmt *Loc, const Expr *Val);

  /// Print to stderr the variable liveness information associated with
  /// each basic block.
  void dumpBlockLiveness(const SourceManager &M);

  /// Print to stderr the expression liveness information associated with
  /// each basic block.
  void dumpExprLiveness(const SourceManager &M);

  void runOnAllBlocks(Observer &obs);

  static std::unique_ptr<LiveVariables>
  create(AnalysisDeclContext &analysisContext) {
    return computeLiveness(analysisContext, true);
  }

  static const void *getTag();

private:
  LiveVariables(void *impl);
  void *impl;
};

class RelaxedLiveVariables : public LiveVariables {
public:
  static std::unique_ptr<LiveVariables>
  create(AnalysisDeclContext &analysisContext) {
    return computeLiveness(analysisContext, false);
  }

  static const void *getTag();
};

} // end namespace language::Core

#endif
