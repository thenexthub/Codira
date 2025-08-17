//=== Iterator.h - Common functions for iterator checkers. ---------*- C++ -*-//
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
// Defines common functions to be used by the itertor checkers .
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_ITERATOR_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_ITERATOR_H

#include "language/Core/StaticAnalyzer/Core/PathSensitive/DynamicType.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymExpr.h"

namespace language::Core {
namespace ento {
namespace iterator {

// Abstract position of an iterator. This helps to handle all three kinds
// of operators in a common way by using a symbolic position.
struct IteratorPosition {
private:

  // Container the iterator belongs to
  const MemRegion *Cont;

  // Whether iterator is valid
  const bool Valid;

  // Abstract offset
  const SymbolRef Offset;

  IteratorPosition(const MemRegion *C, bool V, SymbolRef Of)
      : Cont(C), Valid(V), Offset(Of) {}

public:
  const MemRegion *getContainer() const { return Cont; }
  bool isValid() const { return Valid; }
  SymbolRef getOffset() const { return Offset; }

  IteratorPosition invalidate() const {
    return IteratorPosition(Cont, false, Offset);
  }

  static IteratorPosition getPosition(const MemRegion *C, SymbolRef Of) {
    return IteratorPosition(C, true, Of);
  }

  IteratorPosition setTo(SymbolRef NewOf) const {
    return IteratorPosition(Cont, Valid, NewOf);
  }

  IteratorPosition reAssign(const MemRegion *NewCont) const {
    return IteratorPosition(NewCont, Valid, Offset);
  }

  bool operator==(const IteratorPosition &X) const {
    return Cont == X.Cont && Valid == X.Valid && Offset == X.Offset;
  }

  bool operator!=(const IteratorPosition &X) const { return !(*this == X); }

  void Profile(toolchain::FoldingSetNodeID &ID) const {
    ID.AddPointer(Cont);
    ID.AddInteger(Valid);
    ID.Add(Offset);
  }
};

// Structure to record the symbolic begin and end position of a container
struct ContainerData {
private:
  const SymbolRef Begin, End;

  ContainerData(SymbolRef B, SymbolRef E) : Begin(B), End(E) {}

public:
  static ContainerData fromBegin(SymbolRef B) {
    return ContainerData(B, nullptr);
  }

  static ContainerData fromEnd(SymbolRef E) {
    return ContainerData(nullptr, E);
  }

  SymbolRef getBegin() const { return Begin; }
  SymbolRef getEnd() const { return End; }

  ContainerData newBegin(SymbolRef B) const { return ContainerData(B, End); }

  ContainerData newEnd(SymbolRef E) const { return ContainerData(Begin, E); }

  bool operator==(const ContainerData &X) const {
    return Begin == X.Begin && End == X.End;
  }

  bool operator!=(const ContainerData &X) const { return !(*this == X); }

  void Profile(toolchain::FoldingSetNodeID &ID) const {
    ID.Add(Begin);
    ID.Add(End);
  }
};

class IteratorSymbolMap {};
class IteratorRegionMap {};
class ContainerMap {};

using IteratorSymbolMapTy =
  CLANG_ENTO_PROGRAMSTATE_MAP(SymbolRef, IteratorPosition);
using IteratorRegionMapTy =
  CLANG_ENTO_PROGRAMSTATE_MAP(const MemRegion *, IteratorPosition);
using ContainerMapTy =
  CLANG_ENTO_PROGRAMSTATE_MAP(const MemRegion *, ContainerData);

} // namespace iterator

template<>
struct ProgramStateTrait<iterator::IteratorSymbolMap>
  : public ProgramStatePartialTrait<iterator::IteratorSymbolMapTy> {
  static void *GDMIndex() { static int Index; return &Index; }
};

template<>
struct ProgramStateTrait<iterator::IteratorRegionMap>
  : public ProgramStatePartialTrait<iterator::IteratorRegionMapTy> {
  static void *GDMIndex() { static int Index; return &Index; }
};

template<>
struct ProgramStateTrait<iterator::ContainerMap>
  : public ProgramStatePartialTrait<iterator::ContainerMapTy> {
  static void *GDMIndex() { static int Index; return &Index; }
};

namespace iterator {

bool isIteratorType(const QualType &Type);
bool isIterator(const CXXRecordDecl *CRD);
bool isComparisonOperator(OverloadedOperatorKind OK);
bool isInsertCall(const FunctionDecl *Func);
bool isEraseCall(const FunctionDecl *Func);
bool isEraseAfterCall(const FunctionDecl *Func);
bool isEmplaceCall(const FunctionDecl *Func);
bool isAccessOperator(OverloadedOperatorKind OK);
bool isAccessOperator(UnaryOperatorKind OK);
bool isAccessOperator(BinaryOperatorKind OK);
bool isDereferenceOperator(OverloadedOperatorKind OK);
bool isDereferenceOperator(UnaryOperatorKind OK);
bool isDereferenceOperator(BinaryOperatorKind OK);
bool isIncrementOperator(OverloadedOperatorKind OK);
bool isIncrementOperator(UnaryOperatorKind OK);
bool isDecrementOperator(OverloadedOperatorKind OK);
bool isDecrementOperator(UnaryOperatorKind OK);
bool isRandomIncrOrDecrOperator(OverloadedOperatorKind OK);
bool isRandomIncrOrDecrOperator(BinaryOperatorKind OK);
const ContainerData *getContainerData(ProgramStateRef State,
                                      const MemRegion *Cont);
const IteratorPosition *getIteratorPosition(ProgramStateRef State, SVal Val);
ProgramStateRef setIteratorPosition(ProgramStateRef State, SVal Val,
                                    const IteratorPosition &Pos);
ProgramStateRef createIteratorPosition(ProgramStateRef State, SVal Val,
                                       const MemRegion *Cont,
                                       ConstCFGElementRef Elem,
                                       const LocationContext *LCtx,
                                       unsigned blockCount);
ProgramStateRef advancePosition(ProgramStateRef State, SVal Iter,
                                OverloadedOperatorKind Op, SVal Distance);
ProgramStateRef assumeNoOverflow(ProgramStateRef State, SymbolRef Sym,
                                 long Scale);
bool compare(ProgramStateRef State, SymbolRef Sym1, SymbolRef Sym2,
             BinaryOperator::Opcode Opc);
bool compare(ProgramStateRef State, NonLoc NL1, NonLoc NL2,
             BinaryOperator::Opcode Opc);

} // namespace iterator
} // namespace ento
} // namespace language::Core

#endif
