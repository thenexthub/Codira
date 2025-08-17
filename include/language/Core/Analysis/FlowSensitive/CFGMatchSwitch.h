//===---- CFGMatchSwitch.h --------------------------------------*- C++ -*-===//
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
//  This file defines the `CFGMatchSwitch` abstraction for building a "switch"
//  statement for control flow graph elements. Each case of the switch is
//  defined by an ASTMatcher which is applied on the AST node contained in the
//  input `CFGElement`.
//
//  Currently, the `CFGMatchSwitch` only handles `CFGElement`s of
//  `Kind::Statement` and `Kind::Initializer`.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_CFGMATCHSWITCH_H_
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_CFGMATCHSWITCH_H_

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/Analysis/CFG.h"
#include "language/Core/Analysis/FlowSensitive/MatchSwitch.h"
#include <functional>
#include <utility>

namespace language::Core {
namespace dataflow {

template <typename State, typename Result = void>
using CFGMatchSwitch =
    std::function<Result(const CFGElement &, ASTContext &, State &)>;

/// Collects cases of a "match switch": a collection of matchers paired with
/// callbacks, which together define a switch that can be applied to an AST node
/// contained in a CFG element.
template <typename State, typename Result = void> class CFGMatchSwitchBuilder {
public:
  /// Registers an action `A` for `CFGStmt`s that will be triggered by the match
  /// of the pattern `M` against the `Stmt` contained in the input `CFGStmt`.
  ///
  /// Requirements:
  ///
  ///  `NodeT` should be derived from `Stmt`.
  template <typename NodeT>
  CFGMatchSwitchBuilder &&
  CaseOfCFGStmt(MatchSwitchMatcher<Stmt> M,
                MatchSwitchAction<NodeT, State, Result> A) && {
    std::move(StmtBuilder).template CaseOf<NodeT>(M, A);
    return std::move(*this);
  }

  /// Registers an action `A` for `CFGInitializer`s that will be triggered by
  /// the match of the pattern `M` against the `CXXCtorInitializer` contained in
  /// the input `CFGInitializer`.
  ///
  /// Requirements:
  ///
  ///  `NodeT` should be derived from `CXXCtorInitializer`.
  template <typename NodeT>
  CFGMatchSwitchBuilder &&
  CaseOfCFGInit(MatchSwitchMatcher<CXXCtorInitializer> M,
                MatchSwitchAction<NodeT, State, Result> A) && {
    std::move(InitBuilder).template CaseOf<NodeT>(M, A);
    return std::move(*this);
  }

  CFGMatchSwitch<State, Result> Build() && {
    return [StmtMS = std::move(StmtBuilder).Build(),
            InitMS = std::move(InitBuilder).Build()](const CFGElement &Element,
                                                     ASTContext &Context,
                                                     State &S) -> Result {
      switch (Element.getKind()) {
      case CFGElement::Initializer:
        return InitMS(*Element.castAs<CFGInitializer>().getInitializer(),
                      Context, S);
      case CFGElement::Statement:
      case CFGElement::Constructor:
      case CFGElement::CXXRecordTypedCall:
        return StmtMS(*Element.castAs<CFGStmt>().getStmt(), Context, S);
      default:
        // FIXME: Handle other kinds of CFGElement.
        return Result();
      }
    };
  }

private:
  ASTMatchSwitchBuilder<Stmt, State, Result> StmtBuilder;
  ASTMatchSwitchBuilder<CXXCtorInitializer, State, Result> InitBuilder;
};

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_CFGMATCHSWITCH_H_
