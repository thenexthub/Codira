//===--- IgnoreExpr.h - Ignore intermediate Expressions -----------------===//
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
// This file defines common functions to ignore intermediate expression nodes
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_IGNOREEXPR_H
#define LANGUAGE_CORE_AST_IGNOREEXPR_H

#include "language/Core/AST/Expr.h"
#include "language/Core/AST/ExprCXX.h"

namespace language::Core {
namespace detail {
/// Given an expression E and functions Fn_1,...,Fn_n : Expr * -> Expr *,
/// Return Fn_n(...(Fn_1(E)))
inline Expr *IgnoreExprNodesImpl(Expr *E) { return E; }
template <typename FnTy, typename... FnTys>
Expr *IgnoreExprNodesImpl(Expr *E, FnTy &&Fn, FnTys &&... Fns) {
  return IgnoreExprNodesImpl(std::forward<FnTy>(Fn)(E),
                             std::forward<FnTys>(Fns)...);
}
} // namespace detail

/// Given an expression E and functions Fn_1,...,Fn_n : Expr * -> Expr *,
/// Recursively apply each of the functions to E until reaching a fixed point.
/// Note that a null E is valid; in this case nothing is done.
template <typename... FnTys> Expr *IgnoreExprNodes(Expr *E, FnTys &&... Fns) {
  Expr *LastE = nullptr;
  while (E != LastE) {
    LastE = E;
    E = detail::IgnoreExprNodesImpl(E, std::forward<FnTys>(Fns)...);
  }
  return E;
}

template <typename... FnTys>
const Expr *IgnoreExprNodes(const Expr *E, FnTys &&...Fns) {
  return IgnoreExprNodes(const_cast<Expr *>(E), std::forward<FnTys>(Fns)...);
}

inline Expr *IgnoreImplicitCastsSingleStep(Expr *E) {
  if (auto *ICE = dyn_cast<ImplicitCastExpr>(E))
    return ICE->getSubExpr();

  if (auto *FE = dyn_cast<FullExpr>(E))
    return FE->getSubExpr();

  return E;
}

inline Expr *IgnoreImplicitCastsExtraSingleStep(Expr *E) {
  // FIXME: Skip MaterializeTemporaryExpr and SubstNonTypeTemplateParmExpr in
  // addition to what IgnoreImpCasts() skips to account for the current
  // behaviour of IgnoreParenImpCasts().
  Expr *SubE = IgnoreImplicitCastsSingleStep(E);
  if (SubE != E)
    return SubE;

  if (auto *MTE = dyn_cast<MaterializeTemporaryExpr>(E))
    return MTE->getSubExpr();

  if (auto *NTTP = dyn_cast<SubstNonTypeTemplateParmExpr>(E))
    return NTTP->getReplacement();

  return E;
}

inline Expr *IgnoreCastsSingleStep(Expr *E) {
  if (auto *CE = dyn_cast<CastExpr>(E))
    return CE->getSubExpr();

  if (auto *FE = dyn_cast<FullExpr>(E))
    return FE->getSubExpr();

  if (auto *MTE = dyn_cast<MaterializeTemporaryExpr>(E))
    return MTE->getSubExpr();

  if (auto *NTTP = dyn_cast<SubstNonTypeTemplateParmExpr>(E))
    return NTTP->getReplacement();

  return E;
}

inline Expr *IgnoreLValueCastsSingleStep(Expr *E) {
  // Skip what IgnoreCastsSingleStep skips, except that only
  // lvalue-to-rvalue casts are skipped.
  if (auto *CE = dyn_cast<CastExpr>(E))
    if (CE->getCastKind() != CK_LValueToRValue)
      return E;

  return IgnoreCastsSingleStep(E);
}

inline Expr *IgnoreBaseCastsSingleStep(Expr *E) {
  if (auto *CE = dyn_cast<CastExpr>(E))
    if (CE->getCastKind() == CK_DerivedToBase ||
        CE->getCastKind() == CK_UncheckedDerivedToBase ||
        CE->getCastKind() == CK_NoOp)
      return CE->getSubExpr();

  return E;
}

inline Expr *IgnoreImplicitSingleStep(Expr *E) {
  Expr *SubE = IgnoreImplicitCastsSingleStep(E);
  if (SubE != E)
    return SubE;

  if (auto *MTE = dyn_cast<MaterializeTemporaryExpr>(E))
    return MTE->getSubExpr();

  if (auto *BTE = dyn_cast<CXXBindTemporaryExpr>(E))
    return BTE->getSubExpr();

  return E;
}

inline Expr *IgnoreElidableImplicitConstructorSingleStep(Expr *E) {
  auto *CCE = dyn_cast<CXXConstructExpr>(E);
  if (CCE && CCE->isElidable() && !isa<CXXTemporaryObjectExpr>(CCE)) {
    unsigned NumArgs = CCE->getNumArgs();
    if ((NumArgs == 1 ||
         (NumArgs > 1 && CCE->getArg(1)->isDefaultArgument())) &&
        !CCE->getArg(0)->isDefaultArgument() && !CCE->isListInitialization())
      return CCE->getArg(0);
  }
  return E;
}

inline Expr *IgnoreImplicitAsWrittenSingleStep(Expr *E) {
  if (auto *ICE = dyn_cast<ImplicitCastExpr>(E))
    return ICE->getSubExprAsWritten();

  return IgnoreImplicitSingleStep(E);
}

inline Expr *IgnoreParensOnlySingleStep(Expr *E) {
  if (auto *PE = dyn_cast<ParenExpr>(E))
    return PE->getSubExpr();
  return E;
}

inline Expr *IgnoreParensSingleStep(Expr *E) {
  if (auto *PE = dyn_cast<ParenExpr>(E))
    return PE->getSubExpr();

  if (auto *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() == UO_Extension)
      return UO->getSubExpr();
  }

  else if (auto *GSE = dyn_cast<GenericSelectionExpr>(E)) {
    if (!GSE->isResultDependent())
      return GSE->getResultExpr();
  }

  else if (auto *CE = dyn_cast<ChooseExpr>(E)) {
    if (!CE->isConditionDependent())
      return CE->getChosenSubExpr();
  }

  else if (auto *PE = dyn_cast<PredefinedExpr>(E)) {
    if (PE->isTransparent() && PE->getFunctionName())
      return PE->getFunctionName();
  }

  return E;
}

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_IGNOREEXPR_H
