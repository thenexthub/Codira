//===--- SValVisitor.h - Visitor for SVal subclasses ------------*- C++ -*-===//
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
//  This file defines the SValVisitor, SymExprVisitor, and MemRegionVisitor
//  interfaces, and also FullSValVisitor, which visits all three hierarchies.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SVALVISITOR_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SVALVISITOR_H

#include "language/Core/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymbolManager.h"

namespace language::Core {

namespace ento {

/// SValVisitor - this class implements a simple visitor for SVal
/// subclasses.
template <typename ImplClass, typename RetTy = void> class SValVisitor {
  ImplClass &derived() { return *static_cast<ImplClass *>(this); }

public:
  RetTy Visit(SVal V) {
    // Dispatch to VisitFooVal for each FooVal.
    switch (V.getKind()) {
#define BASIC_SVAL(Id, Parent)                                                 \
  case SVal::Id##Kind:                                                         \
    return derived().Visit##Id(V.castAs<Id>());
#define LOC_SVAL(Id, Parent)                                                   \
  case SVal::Loc##Id##Kind:                                                    \
    return derived().Visit##Id(V.castAs<loc::Id>());
#define NONLOC_SVAL(Id, Parent)                                                \
  case SVal::NonLoc##Id##Kind:                                                 \
    return derived().Visit##Id(V.castAs<nonloc::Id>());
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.def"
    }
    toolchain_unreachable("Unknown SVal kind!");
  }

  // Dispatch to the more generic handler as a default implementation.
#define BASIC_SVAL(Id, Parent)                                                 \
  RetTy Visit##Id(Id V) { return derived().Visit##Parent(V.castAs<Id>()); }
#define ABSTRACT_SVAL(Id, Parent) BASIC_SVAL(Id, Parent)
#define LOC_SVAL(Id, Parent)                                                   \
  RetTy Visit##Id(loc::Id V) { return derived().VisitLoc(V.castAs<Loc>()); }
#define NONLOC_SVAL(Id, Parent)                                                \
  RetTy Visit##Id(nonloc::Id V) {                                              \
    return derived().VisitNonLoc(V.castAs<NonLoc>());                          \
  }
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.def"

  // Base case, ignore it. :)
  RetTy VisitSVal(SVal V) { return RetTy(); }
};

/// SymExprVisitor - this class implements a simple visitor for SymExpr
/// subclasses.
template <typename ImplClass, typename RetTy = void> class SymExprVisitor {
public:

#define DISPATCH(CLASS) \
    return static_cast<ImplClass *>(this)->Visit ## CLASS(cast<CLASS>(S))

  RetTy Visit(SymbolRef S) {
    // Dispatch to VisitSymbolFoo for each SymbolFoo.
    switch (S->getKind()) {
#define SYMBOL(Id, Parent) \
    case SymExpr::Id ## Kind: DISPATCH(Id);
#include "language/Core/StaticAnalyzer/Core/PathSensitive/Symbols.def"
    }
    toolchain_unreachable("Unknown SymExpr kind!");
  }

  // If the implementation chooses not to implement a certain visit method, fall
  // back on visiting the superclass.
#define SYMBOL(Id, Parent) RetTy Visit ## Id(const Id *S) { DISPATCH(Parent); }
#define ABSTRACT_SYMBOL(Id, Parent) SYMBOL(Id, Parent)
#include "language/Core/StaticAnalyzer/Core/PathSensitive/Symbols.def"

  // Base case, ignore it. :)
  RetTy VisitSymExpr(SymbolRef S) { return RetTy(); }

#undef DISPATCH
};

/// MemRegionVisitor - this class implements a simple visitor for MemRegion
/// subclasses.
template <typename ImplClass, typename RetTy = void> class MemRegionVisitor {
public:

#define DISPATCH(CLASS) \
  return static_cast<ImplClass *>(this)->Visit ## CLASS(cast<CLASS>(R))

  RetTy Visit(const MemRegion *R) {
    // Dispatch to VisitFooRegion for each FooRegion.
    switch (R->getKind()) {
#define REGION(Id, Parent) case MemRegion::Id ## Kind: DISPATCH(Id);
#include "language/Core/StaticAnalyzer/Core/PathSensitive/Regions.def"
    }
    toolchain_unreachable("Unknown MemRegion kind!");
  }

  // If the implementation chooses not to implement a certain visit method, fall
  // back on visiting the superclass.
#define REGION(Id, Parent) \
  RetTy Visit ## Id(const Id *R) { DISPATCH(Parent); }
#define ABSTRACT_REGION(Id, Parent) \
  REGION(Id, Parent)
#include "language/Core/StaticAnalyzer/Core/PathSensitive/Regions.def"

  // Base case, ignore it. :)
  RetTy VisitMemRegion(const MemRegion *R) { return RetTy(); }

#undef DISPATCH
};

/// FullSValVisitor - a convenient mixed visitor for all three:
/// SVal, SymExpr and MemRegion subclasses.
template <typename ImplClass, typename RetTy = void>
class FullSValVisitor : public SValVisitor<ImplClass, RetTy>,
                        public SymExprVisitor<ImplClass, RetTy>,
                        public MemRegionVisitor<ImplClass, RetTy> {
public:
  using SValVisitor<ImplClass, RetTy>::Visit;
  using SymExprVisitor<ImplClass, RetTy>::Visit;
  using MemRegionVisitor<ImplClass, RetTy>::Visit;
};

} // end namespace ento

} // end namespace language::Core

#endif
