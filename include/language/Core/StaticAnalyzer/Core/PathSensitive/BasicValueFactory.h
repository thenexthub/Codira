//==- BasicValueFactory.h - Basic values for Path Sens analysis --*- C++ -*-==//
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
//  This file defines BasicValueFactory, a class that manages the lifetime
//  of APSInt objects and symbolic constraints used by ExprEngine
//  and related classes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_BASICVALUEFACTORY_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_BASICVALUEFACTORY_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/Type.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/APSIntPtr.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/APSIntType.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/StoreRef.h"
#include "toolchain/ADT/APSInt.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/ImmutableList.h"
#include "toolchain/ADT/iterator_range.h"
#include "toolchain/Support/Allocator.h"
#include <cassert>
#include <cstdint>
#include <utility>

namespace language::Core {

class CXXBaseSpecifier;

namespace ento {

class CompoundValData : public toolchain::FoldingSetNode {
  QualType T;
  toolchain::ImmutableList<SVal> L;

public:
  CompoundValData(QualType t, toolchain::ImmutableList<SVal> l) : T(t), L(l) {
    assert(NonLoc::isCompoundType(t));
  }

  using iterator = toolchain::ImmutableList<SVal>::iterator;

  iterator begin() const { return L.begin(); }
  iterator end() const { return L.end(); }

  QualType getType() const { return T; }

  static void Profile(toolchain::FoldingSetNodeID& ID, QualType T,
                      toolchain::ImmutableList<SVal> L);

  void Profile(toolchain::FoldingSetNodeID& ID) { Profile(ID, T, L); }
};

class LazyCompoundValData : public toolchain::FoldingSetNode {
  StoreRef store;
  const TypedValueRegion *region;

public:
  LazyCompoundValData(const StoreRef &st, const TypedValueRegion *r)
      : store(st), region(r) {
    assert(r);
    assert(NonLoc::isCompoundType(r->getValueType()));
  }

  /// It might return null.
  const void *getStore() const { return store.getStore(); }

  LLVM_ATTRIBUTE_RETURNS_NONNULL
  const TypedValueRegion *getRegion() const { return region; }

  static void Profile(toolchain::FoldingSetNodeID& ID,
                      const StoreRef &store,
                      const TypedValueRegion *region);

  void Profile(toolchain::FoldingSetNodeID& ID) { Profile(ID, store, region); }
};

class PointerToMemberData : public toolchain::FoldingSetNode {
  const NamedDecl *D;
  toolchain::ImmutableList<const CXXBaseSpecifier *> L;

public:
  PointerToMemberData(const NamedDecl *D,
                      toolchain::ImmutableList<const CXXBaseSpecifier *> L)
      : D(D), L(L) {}

  using iterator = toolchain::ImmutableList<const CXXBaseSpecifier *>::iterator;

  iterator begin() const { return L.begin(); }
  iterator end() const { return L.end(); }

  static void Profile(toolchain::FoldingSetNodeID &ID, const NamedDecl *D,
                      toolchain::ImmutableList<const CXXBaseSpecifier *> L);

  void Profile(toolchain::FoldingSetNodeID &ID) { Profile(ID, D, L); }

  /// It might return null.
  const NamedDecl *getDeclaratorDecl() const { return D; }

  toolchain::ImmutableList<const CXXBaseSpecifier *> getCXXBaseList() const {
    return L;
  }
};

class BasicValueFactory {
  using APSIntSetTy =
      toolchain::FoldingSet<toolchain::FoldingSetNodeWrapper<toolchain::APSInt>>;

  ASTContext &Ctx;
  toolchain::BumpPtrAllocator& BPAlloc;

  APSIntSetTy APSIntSet;
  void *PersistentSVals = nullptr;
  void *PersistentSValPairs = nullptr;

  toolchain::ImmutableList<SVal>::Factory SValListFactory;
  toolchain::ImmutableList<const CXXBaseSpecifier *>::Factory CXXBaseListFactory;
  toolchain::FoldingSet<CompoundValData>  CompoundValDataSet;
  toolchain::FoldingSet<LazyCompoundValData> LazyCompoundValDataSet;
  toolchain::FoldingSet<PointerToMemberData> PointerToMemberDataSet;

  // This is private because external clients should use the factory
  // method that takes a QualType.
  APSIntPtr getValue(uint64_t X, unsigned BitWidth, bool isUnsigned);

public:
  BasicValueFactory(ASTContext &ctx, toolchain::BumpPtrAllocator &Alloc)
      : Ctx(ctx), BPAlloc(Alloc), SValListFactory(Alloc),
        CXXBaseListFactory(Alloc) {}

  ~BasicValueFactory();

  ASTContext &getContext() const { return Ctx; }

  APSIntPtr getValue(const toolchain::APSInt &X);
  APSIntPtr getValue(const toolchain::APInt &X, bool isUnsigned);
  APSIntPtr getValue(uint64_t X, QualType T);

  /// Returns the type of the APSInt used to store values of the given QualType.
  APSIntType getAPSIntType(QualType T) const {
    // For the purposes of the analysis and constraints, we treat atomics
    // as their underlying types.
    if (const AtomicType *AT = T->getAs<AtomicType>()) {
      T = AT->getValueType();
    }

    if (T->isIntegralOrEnumerationType() || Loc::isLocType(T)) {
      return APSIntType(Ctx.getIntWidth(T),
                        !T->isSignedIntegerOrEnumerationType());
    } else {
      // implicitly handle case of T->isFixedPointType()
      return APSIntType(Ctx.getIntWidth(T), T->isUnsignedFixedPointType());
    }

    toolchain_unreachable("Unsupported type in getAPSIntType!");
  }

  /// Convert - Create a new persistent APSInt with the same value as 'From'
  ///  but with the bitwidth and signedness of 'To'.
  APSIntPtr Convert(const toolchain::APSInt &To, const toolchain::APSInt &From) {
    APSIntType TargetType(To);
    if (TargetType == APSIntType(From))
      return getValue(From);

    return getValue(TargetType.convert(From));
  }

  APSIntPtr Convert(QualType T, const toolchain::APSInt &From) {
    APSIntType TargetType = getAPSIntType(T);
    return Convert(TargetType, From);
  }

  APSIntPtr Convert(APSIntType TargetType, const toolchain::APSInt &From) {
    if (TargetType == APSIntType(From))
      return getValue(From);

    return getValue(TargetType.convert(From));
  }

  APSIntPtr getIntValue(uint64_t X, bool isUnsigned) {
    QualType T = isUnsigned ? Ctx.UnsignedIntTy : Ctx.IntTy;
    return getValue(X, T);
  }

  APSIntPtr getMaxValue(const toolchain::APSInt &v) {
    return getValue(APSIntType(v).getMaxValue());
  }

  APSIntPtr getMinValue(const toolchain::APSInt &v) {
    return getValue(APSIntType(v).getMinValue());
  }

  APSIntPtr getMaxValue(QualType T) { return getMaxValue(getAPSIntType(T)); }

  APSIntPtr getMinValue(QualType T) { return getMinValue(getAPSIntType(T)); }

  APSIntPtr getMaxValue(APSIntType T) { return getValue(T.getMaxValue()); }

  APSIntPtr getMinValue(APSIntType T) { return getValue(T.getMinValue()); }

  APSIntPtr Add1(const toolchain::APSInt &V) {
    toolchain::APSInt X = V;
    ++X;
    return getValue(X);
  }

  APSIntPtr Sub1(const toolchain::APSInt &V) {
    toolchain::APSInt X = V;
    --X;
    return getValue(X);
  }

  APSIntPtr getZeroWithTypeSize(QualType T) {
    assert(T->isScalarType());
    return getValue(0, Ctx.getTypeSize(T), true);
  }

  APSIntPtr getTruthValue(bool b, QualType T) {
    return getValue(b ? 1 : 0, Ctx.getIntWidth(T),
                    T->isUnsignedIntegerOrEnumerationType());
  }

  APSIntPtr getTruthValue(bool b) {
    return getTruthValue(b, Ctx.getLogicalOperationType());
  }

  const CompoundValData *getCompoundValData(QualType T,
                                            toolchain::ImmutableList<SVal> Vals);

  const LazyCompoundValData *getLazyCompoundValData(const StoreRef &store,
                                            const TypedValueRegion *region);

  const PointerToMemberData *
  getPointerToMemberData(const NamedDecl *ND,
                         toolchain::ImmutableList<const CXXBaseSpecifier *> L);

  toolchain::ImmutableList<SVal> getEmptySValList() {
    return SValListFactory.getEmptyList();
  }

  toolchain::ImmutableList<SVal> prependSVal(SVal X, toolchain::ImmutableList<SVal> L) {
    return SValListFactory.add(X, L);
  }

  toolchain::ImmutableList<const CXXBaseSpecifier *> getEmptyCXXBaseList() {
    return CXXBaseListFactory.getEmptyList();
  }

  toolchain::ImmutableList<const CXXBaseSpecifier *> prependCXXBase(
      const CXXBaseSpecifier *CBS,
      toolchain::ImmutableList<const CXXBaseSpecifier *> L) {
    return CXXBaseListFactory.add(CBS, L);
  }

  const PointerToMemberData *
  accumCXXBase(toolchain::iterator_range<CastExpr::path_const_iterator> PathRange,
               const nonloc::PointerToMember &PTM, const language::Core::CastKind &kind);

  std::optional<APSIntPtr> evalAPSInt(BinaryOperator::Opcode Op,
                                      const toolchain::APSInt &V1,
                                      const toolchain::APSInt &V2);

  const std::pair<SVal, uintptr_t>&
  getPersistentSValWithData(const SVal& V, uintptr_t Data);

  const std::pair<SVal, SVal>&
  getPersistentSValPair(const SVal& V1, const SVal& V2);

  const SVal* getPersistentSVal(SVal X);
};

} // namespace ento

} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_BASICVALUEFACTORY_H
