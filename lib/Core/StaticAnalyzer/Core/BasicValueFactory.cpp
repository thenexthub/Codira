//===- BasicValueFactory.cpp - Basic values for Path Sens analysis --------===//
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

#include "language/Core/StaticAnalyzer/Core/PathSensitive/BasicValueFactory.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/APSIntType.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/Store.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/StoreRef.h"
#include "toolchain/ADT/APSInt.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/ImmutableList.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include <cassert>
#include <cstdint>
#include <utility>

using namespace language::Core;
using namespace ento;

void CompoundValData::Profile(toolchain::FoldingSetNodeID& ID, QualType T,
                              toolchain::ImmutableList<SVal> L) {
  T.Profile(ID);
  ID.AddPointer(L.getInternalPointer());
}

void LazyCompoundValData::Profile(toolchain::FoldingSetNodeID& ID,
                                  const StoreRef &store,
                                  const TypedValueRegion *region) {
  ID.AddPointer(store.getStore());
  ID.AddPointer(region);
}

void PointerToMemberData::Profile(
    toolchain::FoldingSetNodeID &ID, const NamedDecl *D,
    toolchain::ImmutableList<const CXXBaseSpecifier *> L) {
  ID.AddPointer(D);
  ID.AddPointer(L.getInternalPointer());
}

using SValData = std::pair<SVal, uintptr_t>;
using SValPair = std::pair<SVal, SVal>;

namespace toolchain {

template<> struct FoldingSetTrait<SValData> {
  static inline void Profile(const SValData& X, toolchain::FoldingSetNodeID& ID) {
    X.first.Profile(ID);
    ID.AddPointer( (void*) X.second);
  }
};

template<> struct FoldingSetTrait<SValPair> {
  static inline void Profile(const SValPair& X, toolchain::FoldingSetNodeID& ID) {
    X.first.Profile(ID);
    X.second.Profile(ID);
  }
};

} // namespace toolchain

using PersistentSValsTy =
    toolchain::FoldingSet<toolchain::FoldingSetNodeWrapper<SValData>>;

using PersistentSValPairsTy =
    toolchain::FoldingSet<toolchain::FoldingSetNodeWrapper<SValPair>>;

BasicValueFactory::~BasicValueFactory() {
  // Note that the dstor for the contents of APSIntSet will never be called,
  // so we iterate over the set and invoke the dstor for each APSInt.  This
  // frees an aux. memory allocated to represent very large constants.
  for (const auto &I : APSIntSet)
    I.getValue().~APSInt();

  delete (PersistentSValsTy*) PersistentSVals;
  delete (PersistentSValPairsTy*) PersistentSValPairs;
}

APSIntPtr BasicValueFactory::getValue(const toolchain::APSInt &X) {
  toolchain::FoldingSetNodeID ID;
  void *InsertPos;

  using FoldNodeTy = toolchain::FoldingSetNodeWrapper<toolchain::APSInt>;

  X.Profile(ID);
  FoldNodeTy* P = APSIntSet.FindNodeOrInsertPos(ID, InsertPos);

  if (!P) {
    P = new (BPAlloc) FoldNodeTy(X);
    APSIntSet.InsertNode(P, InsertPos);
  }

  // We own the APSInt object. It's safe here.
  return APSIntPtr::unsafeConstructor(&P->getValue());
}

APSIntPtr BasicValueFactory::getValue(const toolchain::APInt &X, bool isUnsigned) {
  toolchain::APSInt V(X, isUnsigned);
  return getValue(V);
}

APSIntPtr BasicValueFactory::getValue(uint64_t X, unsigned BitWidth,
                                      bool isUnsigned) {
  toolchain::APSInt V(BitWidth, isUnsigned);
  V = X;
  return getValue(V);
}

APSIntPtr BasicValueFactory::getValue(uint64_t X, QualType T) {
  return getValue(getAPSIntType(T).getValue(X));
}

const CompoundValData*
BasicValueFactory::getCompoundValData(QualType T,
                                      toolchain::ImmutableList<SVal> Vals) {
  toolchain::FoldingSetNodeID ID;
  CompoundValData::Profile(ID, T, Vals);
  void *InsertPos;

  CompoundValData* D = CompoundValDataSet.FindNodeOrInsertPos(ID, InsertPos);

  if (!D) {
    D = new (BPAlloc) CompoundValData(T, Vals);
    CompoundValDataSet.InsertNode(D, InsertPos);
  }

  return D;
}

const LazyCompoundValData*
BasicValueFactory::getLazyCompoundValData(const StoreRef &store,
                                          const TypedValueRegion *region) {
  toolchain::FoldingSetNodeID ID;
  LazyCompoundValData::Profile(ID, store, region);
  void *InsertPos;

  LazyCompoundValData *D =
    LazyCompoundValDataSet.FindNodeOrInsertPos(ID, InsertPos);

  if (!D) {
    D = new (BPAlloc) LazyCompoundValData(store, region);
    LazyCompoundValDataSet.InsertNode(D, InsertPos);
  }

  return D;
}

const PointerToMemberData *BasicValueFactory::getPointerToMemberData(
    const NamedDecl *ND, toolchain::ImmutableList<const CXXBaseSpecifier *> L) {
  toolchain::FoldingSetNodeID ID;
  PointerToMemberData::Profile(ID, ND, L);
  void *InsertPos;

  PointerToMemberData *D =
      PointerToMemberDataSet.FindNodeOrInsertPos(ID, InsertPos);

  if (!D) {
    D = new (BPAlloc) PointerToMemberData(ND, L);
    PointerToMemberDataSet.InsertNode(D, InsertPos);
  }

  return D;
}

LLVM_ATTRIBUTE_UNUSED static bool hasNoRepeatedElements(
    toolchain::ImmutableList<const CXXBaseSpecifier *> BaseSpecList) {
  toolchain::SmallPtrSet<QualType, 16> BaseSpecSeen;
  for (const CXXBaseSpecifier *BaseSpec : BaseSpecList) {
    QualType BaseType = BaseSpec->getType();
    // Check whether inserted
    if (!BaseSpecSeen.insert(BaseType).second)
      return false;
  }
  return true;
}

const PointerToMemberData *BasicValueFactory::accumCXXBase(
    toolchain::iterator_range<CastExpr::path_const_iterator> PathRange,
    const nonloc::PointerToMember &PTM, const CastKind &kind) {
  assert((kind == CK_DerivedToBaseMemberPointer ||
          kind == CK_BaseToDerivedMemberPointer ||
          kind == CK_ReinterpretMemberPointer) &&
         "accumCXXBase called with wrong CastKind");
  nonloc::PointerToMember::PTMDataType PTMDT = PTM.getPTMData();
  const NamedDecl *ND = nullptr;
  toolchain::ImmutableList<const CXXBaseSpecifier *> BaseSpecList;

  if (PTMDT.isNull() || isa<const NamedDecl *>(PTMDT)) {
    if (const auto *NDP = dyn_cast_if_present<const NamedDecl *>(PTMDT))
      ND = NDP;

    BaseSpecList = CXXBaseListFactory.getEmptyList();
  } else {
    const auto *PTMD = cast<const PointerToMemberData *>(PTMDT);
    ND = PTMD->getDeclaratorDecl();

    BaseSpecList = PTMD->getCXXBaseList();
  }

  assert(hasNoRepeatedElements(BaseSpecList) &&
         "CXXBaseSpecifier list of PointerToMemberData must not have repeated "
         "elements");

  if (kind == CK_DerivedToBaseMemberPointer) {
    // Here we pop off matching CXXBaseSpecifiers from BaseSpecList.
    // Because, CK_DerivedToBaseMemberPointer comes from a static_cast and
    // serves to remove a matching implicit cast. Note that static_cast's that
    // are no-ops do not count since they produce an empty PathRange, a nice
    // thing about Clang AST.

    // Now we know that there are no repetitions in BaseSpecList.
    // So, popping the first element from it corresponding to each element in
    // PathRange is equivalent to only including elements that are in
    // BaseSpecList but not it PathRange
    auto ReducedBaseSpecList = CXXBaseListFactory.getEmptyList();
    for (const CXXBaseSpecifier *BaseSpec : BaseSpecList) {
      auto IsSameAsBaseSpec = [&BaseSpec](const CXXBaseSpecifier *I) -> bool {
        return BaseSpec->getType() == I->getType();
      };
      if (toolchain::none_of(PathRange, IsSameAsBaseSpec))
        ReducedBaseSpecList =
            CXXBaseListFactory.add(BaseSpec, ReducedBaseSpecList);
    }

    return getPointerToMemberData(ND, ReducedBaseSpecList);
  }
  // FIXME: Reinterpret casts on member-pointers are not handled properly by
  // this code
  for (const CXXBaseSpecifier *I : toolchain::reverse(PathRange))
    BaseSpecList = prependCXXBase(I, BaseSpecList);
  return getPointerToMemberData(ND, BaseSpecList);
}

std::optional<APSIntPtr>
BasicValueFactory::evalAPSInt(BinaryOperator::Opcode Op, const toolchain::APSInt &V1,
                              const toolchain::APSInt &V2) {
  switch (Op) {
    default:
      toolchain_unreachable("Invalid Opcode.");

    case BO_Mul:
      return getValue(V1 * V2);

    case BO_Div:
      if (V2 == 0) // Avoid division by zero
        return std::nullopt;
      return getValue(V1 / V2);

    case BO_Rem:
      if (V2 == 0) // Avoid division by zero
        return std::nullopt;
      return getValue(V1 % V2);

    case BO_Add:
      return getValue(V1 + V2);

    case BO_Sub:
      return getValue(V1 - V2);

    case BO_Shl: {
      // FIXME: This logic should probably go higher up, where we can
      // test these conditions symbolically.

      if (V2.isNegative() || V2.getBitWidth() > 64)
        return std::nullopt;

      uint64_t Amt = V2.getZExtValue();

      if (Amt >= V1.getBitWidth())
        return std::nullopt;

      return getValue(V1.operator<<((unsigned)Amt));
    }

    case BO_Shr: {
      // FIXME: This logic should probably go higher up, where we can
      // test these conditions symbolically.

      if (V2.isNegative() || V2.getBitWidth() > 64)
        return std::nullopt;

      uint64_t Amt = V2.getZExtValue();

      if (Amt >= V1.getBitWidth())
        return std::nullopt;

      return getValue(V1.operator>>((unsigned)Amt));
    }

    case BO_LT:
      return getTruthValue(V1 < V2);

    case BO_GT:
      return getTruthValue(V1 > V2);

    case BO_LE:
      return getTruthValue(V1 <= V2);

    case BO_GE:
      return getTruthValue(V1 >= V2);

    case BO_EQ:
      return getTruthValue(V1 == V2);

    case BO_NE:
      return getTruthValue(V1 != V2);

      // Note: LAnd, LOr, Comma are handled specially by higher-level logic.

    case BO_And:
      return getValue(V1 & V2);

    case BO_Or:
      return getValue(V1 | V2);

    case BO_Xor:
      return getValue(V1 ^ V2);
  }
}

const std::pair<SVal, uintptr_t>&
BasicValueFactory::getPersistentSValWithData(const SVal& V, uintptr_t Data) {
  // Lazily create the folding set.
  if (!PersistentSVals) PersistentSVals = new PersistentSValsTy();

  toolchain::FoldingSetNodeID ID;
  void *InsertPos;
  V.Profile(ID);
  ID.AddPointer((void*) Data);

  PersistentSValsTy& Map = *((PersistentSValsTy*) PersistentSVals);

  using FoldNodeTy = toolchain::FoldingSetNodeWrapper<SValData>;

  FoldNodeTy* P = Map.FindNodeOrInsertPos(ID, InsertPos);

  if (!P) {
    P = new (BPAlloc) FoldNodeTy(std::make_pair(V, Data));
    Map.InsertNode(P, InsertPos);
  }

  return P->getValue();
}

const std::pair<SVal, SVal>&
BasicValueFactory::getPersistentSValPair(const SVal& V1, const SVal& V2) {
  // Lazily create the folding set.
  if (!PersistentSValPairs) PersistentSValPairs = new PersistentSValPairsTy();

  toolchain::FoldingSetNodeID ID;
  void *InsertPos;
  V1.Profile(ID);
  V2.Profile(ID);

  PersistentSValPairsTy& Map = *((PersistentSValPairsTy*) PersistentSValPairs);

  using FoldNodeTy = toolchain::FoldingSetNodeWrapper<SValPair>;

  FoldNodeTy* P = Map.FindNodeOrInsertPos(ID, InsertPos);

  if (!P) {
    P = new (BPAlloc) FoldNodeTy(std::make_pair(V1, V2));
    Map.InsertNode(P, InsertPos);
  }

  return P->getValue();
}

const SVal* BasicValueFactory::getPersistentSVal(SVal X) {
  return &getPersistentSValWithData(X, 0).first;
}
