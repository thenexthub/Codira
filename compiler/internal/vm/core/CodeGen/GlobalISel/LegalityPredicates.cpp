//===- lib/CodeGen/GlobalISel/LegalizerPredicates.cpp - Predicates --------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// A library of predicate factories to use for LegalityPredicate.
//
//===----------------------------------------------------------------------===//

// Enable optimizations to work around MSVC debug mode bug in 32-bit:
// https://developercommunity.visualstudio.com/content/problem/1179643/msvc-copies-overaligned-non-trivially-copyable-par.html
// FIXME: Remove this when the issue is closed.
#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_IX86)
// We have to disable runtime checks in order to enable optimizations. This is
// done for the entire file because the problem is actually observed in STL
// template functions.
#pragma runtime_checks("", off)
#pragma optimize("gs", on)
#endif

#include "vm/core/CodeGen/GlobalISel/LegalizerInfo.h"

using namespace vm::core;

LegalityPredicate LegalityPredicates::typeIs(unsigned TypeIdx, LLT Type) {
  return
      [=](const LegalityQuery &Query) { return Query.Types[TypeIdx] == Type; };
}

LegalityPredicate
LegalityPredicates::typeInSet(unsigned TypeIdx,
                              std::initializer_list<LLT> TypesInit) {
  SmallVector<LLT, 4> Types = TypesInit;
  return [=](const LegalityQuery &Query) {
    return toolchain::is_contained(Types, Query.Types[TypeIdx]);
  };
}

LegalityPredicate LegalityPredicates::typePairInSet(
    unsigned TypeIdx0, unsigned TypeIdx1,
    std::initializer_list<std::pair<LLT, LLT>> TypesInit) {
  SmallVector<std::pair<LLT, LLT>, 4> Types = TypesInit;
  return [=](const LegalityQuery &Query) {
    std::pair<LLT, LLT> Match = {Query.Types[TypeIdx0], Query.Types[TypeIdx1]};
    return toolchain::is_contained(Types, Match);
  };
}

LegalityPredicate LegalityPredicates::typeTupleInSet(
    unsigned TypeIdx0, unsigned TypeIdx1, unsigned TypeIdx2,
    std::initializer_list<std::tuple<LLT, LLT, LLT>> TypesInit) {
  SmallVector<std::tuple<LLT, LLT, LLT>, 4> Types = TypesInit;
  return [=](const LegalityQuery &Query) {
    std::tuple<LLT, LLT, LLT> Match = {
        Query.Types[TypeIdx0], Query.Types[TypeIdx1], Query.Types[TypeIdx2]};
    return toolchain::is_contained(Types, Match);
  };
}

LegalityPredicate LegalityPredicates::typePairAndMemDescInSet(
    unsigned TypeIdx0, unsigned TypeIdx1, unsigned MMOIdx,
    std::initializer_list<TypePairAndMemDesc> TypesAndMemDescInit) {
  SmallVector<TypePairAndMemDesc, 4> TypesAndMemDesc = TypesAndMemDescInit;
  return [=](const LegalityQuery &Query) {
    TypePairAndMemDesc Match = {Query.Types[TypeIdx0], Query.Types[TypeIdx1],
                                Query.MMODescrs[MMOIdx].MemoryTy,
                                Query.MMODescrs[MMOIdx].AlignInBits};
    return toolchain::any_of(TypesAndMemDesc,
                        [=](const TypePairAndMemDesc &Entry) -> bool {
                          return Match.isCompatible(Entry);
                        });
  };
}

LegalityPredicate LegalityPredicates::isScalar(unsigned TypeIdx) {
  return [=](const LegalityQuery &Query) {
    return Query.Types[TypeIdx].isScalar();
  };
}

LegalityPredicate LegalityPredicates::isVector(unsigned TypeIdx) {
  return [=](const LegalityQuery &Query) {
    return Query.Types[TypeIdx].isVector();
  };
}

LegalityPredicate LegalityPredicates::isPointer(unsigned TypeIdx) {
  return [=](const LegalityQuery &Query) {
    return Query.Types[TypeIdx].isPointer();
  };
}

LegalityPredicate LegalityPredicates::isPointer(unsigned TypeIdx,
                                                unsigned AddrSpace) {
  return [=](const LegalityQuery &Query) {
    LLT Ty = Query.Types[TypeIdx];
    return Ty.isPointer() && Ty.getAddressSpace() == AddrSpace;
  };
}

LegalityPredicate LegalityPredicates::isPointerVector(unsigned TypeIdx) {
  return [=](const LegalityQuery &Query) {
    return Query.Types[TypeIdx].isPointerVector();
  };
}

LegalityPredicate LegalityPredicates::elementTypeIs(unsigned TypeIdx,
                                                    LLT EltTy) {
  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.isVector() && QueryTy.getElementType() == EltTy;
  };
}

LegalityPredicate LegalityPredicates::scalarNarrowerThan(unsigned TypeIdx,
                                                         unsigned Size) {
  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.isScalar() && QueryTy.getSizeInBits() < Size;
  };
}

LegalityPredicate LegalityPredicates::scalarWiderThan(unsigned TypeIdx,
                                                      unsigned Size) {
  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.isScalar() && QueryTy.getSizeInBits() > Size;
  };
}

LegalityPredicate LegalityPredicates::smallerThan(unsigned TypeIdx0,
                                                  unsigned TypeIdx1) {
  return [=](const LegalityQuery &Query) {
    return Query.Types[TypeIdx0].getSizeInBits() <
           Query.Types[TypeIdx1].getSizeInBits();
  };
}

LegalityPredicate LegalityPredicates::largerThan(unsigned TypeIdx0,
                                                  unsigned TypeIdx1) {
  return [=](const LegalityQuery &Query) {
    return Query.Types[TypeIdx0].getSizeInBits() >
           Query.Types[TypeIdx1].getSizeInBits();
  };
}

LegalityPredicate LegalityPredicates::scalarOrEltNarrowerThan(unsigned TypeIdx,
                                                              unsigned Size) {
  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.getScalarSizeInBits() < Size;
  };
}

LegalityPredicate
LegalityPredicates::vectorElementCountIsGreaterThan(unsigned TypeIdx,
                                                    unsigned Size) {

  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.isFixedVector() && QueryTy.getNumElements() > Size;
  };
}

LegalityPredicate
LegalityPredicates::vectorElementCountIsLessThanOrEqualTo(unsigned TypeIdx,
                                                          unsigned Size) {

  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.isFixedVector() && QueryTy.getNumElements() <= Size;
  };
}

LegalityPredicate LegalityPredicates::scalarOrEltWiderThan(unsigned TypeIdx,
                                                           unsigned Size) {
  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.getScalarSizeInBits() > Size;
  };
}

LegalityPredicate LegalityPredicates::scalarOrEltSizeNotPow2(unsigned TypeIdx) {
  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return !isPowerOf2_32(QueryTy.getScalarSizeInBits());
  };
}

LegalityPredicate LegalityPredicates::sizeNotMultipleOf(unsigned TypeIdx,
                                                        unsigned Size) {
  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.isScalar() && QueryTy.getSizeInBits() % Size != 0;
  };
}

LegalityPredicate LegalityPredicates::sizeNotPow2(unsigned TypeIdx) {
  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.isScalar() &&
           !toolchain::has_single_bit<uint32_t>(QueryTy.getSizeInBits());
  };
}

LegalityPredicate LegalityPredicates::sizeIs(unsigned TypeIdx, unsigned Size) {
  return [=](const LegalityQuery &Query) {
    return Query.Types[TypeIdx].getSizeInBits() == Size;
  };
}

LegalityPredicate LegalityPredicates::sameSize(unsigned TypeIdx0,
                                               unsigned TypeIdx1) {
  return [=](const LegalityQuery &Query) {
    return Query.Types[TypeIdx0].getSizeInBits() ==
           Query.Types[TypeIdx1].getSizeInBits();
  };
}

LegalityPredicate LegalityPredicates::memSizeInBytesNotPow2(unsigned MMOIdx) {
  return [=](const LegalityQuery &Query) {
    return !toolchain::has_single_bit<uint32_t>(
        Query.MMODescrs[MMOIdx].MemoryTy.getSizeInBytes());
  };
}

LegalityPredicate LegalityPredicates::memSizeNotByteSizePow2(unsigned MMOIdx) {
  return [=](const LegalityQuery &Query) {
    const LLT MemTy = Query.MMODescrs[MMOIdx].MemoryTy;
    return !MemTy.isByteSized() ||
           !toolchain::has_single_bit<uint32_t>(
               MemTy.getSizeInBytes().getKnownMinValue());
  };
}

LegalityPredicate LegalityPredicates::numElementsNotPow2(unsigned TypeIdx) {
  return [=](const LegalityQuery &Query) {
    const LLT QueryTy = Query.Types[TypeIdx];
    return QueryTy.isFixedVector() && !isPowerOf2_32(QueryTy.getNumElements());
  };
}

LegalityPredicate LegalityPredicates::atomicOrderingAtLeastOrStrongerThan(
    unsigned MMOIdx, AtomicOrdering Ordering) {
  return [=](const LegalityQuery &Query) {
    return isAtLeastOrStrongerThan(Query.MMODescrs[MMOIdx].Ordering, Ordering);
  };
}
