//===--- CurrentSourceLocExprScope.h ----------------------------*- C++ -*-===//
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
//  This file defines types used to track the current context needed to evaluate
//  a SourceLocExpr.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_CURRENTSOURCELOCEXPRSCOPE_H
#define LANGUAGE_CORE_AST_CURRENTSOURCELOCEXPRSCOPE_H

#include <cassert>

namespace language::Core {
class Expr;

/// Represents the current source location and context used to determine the
/// value of the source location builtins (ex. __builtin_LINE), including the
/// context of default argument and default initializer expressions.
class CurrentSourceLocExprScope {
  /// The CXXDefaultArgExpr or CXXDefaultInitExpr we're currently evaluating.
  const Expr *DefaultExpr = nullptr;

public:
  /// A RAII style scope guard used for tracking the current source
  /// location and context as used by the source location builtins
  /// (ex. __builtin_LINE).
  class SourceLocExprScopeGuard;

  const Expr *getDefaultExpr() const { return DefaultExpr; }

  explicit CurrentSourceLocExprScope() = default;

private:
  explicit CurrentSourceLocExprScope(const Expr *DefaultExpr)
      : DefaultExpr(DefaultExpr) {}

  CurrentSourceLocExprScope(CurrentSourceLocExprScope const &) = default;
  CurrentSourceLocExprScope &
  operator=(CurrentSourceLocExprScope const &) = default;
};

class CurrentSourceLocExprScope::SourceLocExprScopeGuard {
public:
  SourceLocExprScopeGuard(const Expr *DefaultExpr,
                          CurrentSourceLocExprScope &Current)
      : Current(Current), OldVal(Current), Enable(false) {
    assert(DefaultExpr && "the new scope should not be empty");
    if ((Enable = (Current.getDefaultExpr() == nullptr)))
      Current = CurrentSourceLocExprScope(DefaultExpr);
  }

  ~SourceLocExprScopeGuard() {
    if (Enable)
      Current = OldVal;
  }

private:
  SourceLocExprScopeGuard(SourceLocExprScopeGuard const &) = delete;
  SourceLocExprScopeGuard &operator=(SourceLocExprScopeGuard const &) = delete;

  CurrentSourceLocExprScope &Current;
  CurrentSourceLocExprScope OldVal;
  bool Enable;
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_AST_CURRENTSOURCELOCEXPRSCOPE_H
