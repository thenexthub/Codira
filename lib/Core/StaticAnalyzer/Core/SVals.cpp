//===-- SVals.cpp - Abstract RValues for Path-Sens. Value Tracking --------===//
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
//  This file defines SVal, Loc, and NonLoc, classes that represent
//  abstract r-values for use with path-sensitive value tracking.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/JsonSupport.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/BasicValueFactory.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SValBuilder.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SValVisitor.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymExpr.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymbolManager.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/raw_ostream.h"
#include <cassert>
#include <optional>

using namespace language::Core;
using namespace ento;

//===----------------------------------------------------------------------===//
// Symbol iteration within an SVal.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Utility methods.
//===----------------------------------------------------------------------===//

const FunctionDecl *SVal::getAsFunctionDecl() const {
  if (std::optional<loc::MemRegionVal> X = getAs<loc::MemRegionVal>()) {
    const MemRegion* R = X->getRegion();
    if (const FunctionCodeRegion *CTR = R->getAs<FunctionCodeRegion>())
      if (const auto *FD = dyn_cast<FunctionDecl>(CTR->getDecl()))
        return FD;
  }

  if (auto X = getAs<nonloc::PointerToMember>()) {
    if (const auto *MD = dyn_cast_or_null<CXXMethodDecl>(X->getDecl()))
      return MD;
  }
  return nullptr;
}

/// If this SVal is a location (subclasses Loc) and wraps a symbol,
/// return that SymbolRef.  Otherwise return 0.
///
/// Implicit casts (ex: void* -> char*) can turn Symbolic region into Element
/// region. If that is the case, gets the underlining region.
/// When IncludeBaseRegions is set to true and the SubRegion is non-symbolic,
/// the first symbolic parent region is returned.
SymbolRef SVal::getAsLocSymbol(bool IncludeBaseRegions) const {
  // FIXME: should we consider SymbolRef wrapped in CodeTextRegion?
  if (const MemRegion *R = getAsRegion())
    if (const SymbolicRegion *SymR =
            IncludeBaseRegions ? R->getSymbolicBase()
                               : dyn_cast<SymbolicRegion>(R->StripCasts()))
      return SymR->getSymbol();

  return nullptr;
}

/// Get the symbol in the SVal or its base region.
SymbolRef SVal::getLocSymbolInBase() const {
  std::optional<loc::MemRegionVal> X = getAs<loc::MemRegionVal>();

  if (!X)
    return nullptr;

  const MemRegion *R = X->getRegion();

  while (const auto *SR = dyn_cast<SubRegion>(R)) {
    if (const auto *SymR = dyn_cast<SymbolicRegion>(SR))
      return SymR->getSymbol();
    else
      R = SR->getSuperRegion();
  }

  return nullptr;
}

/// If this SVal wraps a symbol return that SymbolRef.
/// Otherwise, return 0.
///
/// Casts are ignored during lookup.
/// \param IncludeBaseRegions The boolean that controls whether the search
/// should continue to the base regions if the region is not symbolic.
SymbolRef SVal::getAsSymbol(bool IncludeBaseRegions) const {
  // FIXME: should we consider SymbolRef wrapped in CodeTextRegion?
  if (std::optional<nonloc::SymbolVal> X = getAs<nonloc::SymbolVal>())
    return X->getSymbol();

  return getAsLocSymbol(IncludeBaseRegions);
}

const toolchain::APSInt *SVal::getAsInteger() const {
  if (auto CI = getAs<nonloc::ConcreteInt>())
    return CI->getValue().get();
  if (auto CI = getAs<loc::ConcreteInt>())
    return CI->getValue().get();
  return nullptr;
}

const MemRegion *SVal::getAsRegion() const {
  if (std::optional<loc::MemRegionVal> X = getAs<loc::MemRegionVal>())
    return X->getRegion();

  if (std::optional<nonloc::LocAsInteger> X = getAs<nonloc::LocAsInteger>())
    return X->getLoc().getAsRegion();

  return nullptr;
}

namespace {
class TypeRetrievingVisitor
    : public FullSValVisitor<TypeRetrievingVisitor, QualType> {
private:
  const ASTContext &Context;

public:
  TypeRetrievingVisitor(const ASTContext &Context) : Context(Context) {}

  QualType VisitMemRegionVal(loc::MemRegionVal MRV) {
    return Visit(MRV.getRegion());
  }
  QualType VisitGotoLabel(loc::GotoLabel GL) {
    return QualType{Context.VoidPtrTy};
  }
  template <class ConcreteInt> QualType VisitConcreteInt(ConcreteInt CI) {
    const toolchain::APSInt &Value = CI.getValue();
    if (1 == Value.getBitWidth())
      return Context.BoolTy;
    return Context.getIntTypeForBitwidth(Value.getBitWidth(), Value.isSigned());
  }
  QualType VisitLocAsInteger(nonloc::LocAsInteger LI) {
    QualType NestedType = Visit(LI.getLoc());
    if (NestedType.isNull())
      return NestedType;

    return Context.getIntTypeForBitwidth(LI.getNumBits(),
                                         NestedType->isSignedIntegerType());
  }
  QualType VisitCompoundVal(nonloc::CompoundVal CV) {
    return CV.getValue()->getType();
  }
  QualType VisitLazyCompoundVal(nonloc::LazyCompoundVal LCV) {
    return LCV.getRegion()->getValueType();
  }
  QualType VisitSymbolVal(nonloc::SymbolVal SV) {
    return Visit(SV.getSymbol());
  }
  QualType VisitSymbolicRegion(const SymbolicRegion *SR) {
    return Visit(SR->getSymbol());
  }
  QualType VisitAllocaRegion(const AllocaRegion *) {
    return QualType{Context.VoidPtrTy};
  }
  QualType VisitTypedRegion(const TypedRegion *TR) {
    return TR->getLocationType();
  }
  QualType VisitSymExpr(const SymExpr *SE) { return SE->getType(); }
};
} // end anonymous namespace

QualType SVal::getType(const ASTContext &Context) const {
  TypeRetrievingVisitor TRV{Context};
  return TRV.Visit(*this);
}

const MemRegion *loc::MemRegionVal::stripCasts(bool StripBaseCasts) const {
  return getRegion()->StripCasts(StripBaseCasts);
}

const void *nonloc::LazyCompoundVal::getStore() const {
  return static_cast<const LazyCompoundValData*>(Data)->getStore();
}

const TypedValueRegion *nonloc::LazyCompoundVal::getRegion() const {
  return static_cast<const LazyCompoundValData*>(Data)->getRegion();
}

bool nonloc::PointerToMember::isNullMemberPointer() const {
  return getPTMData().isNull();
}

const NamedDecl *nonloc::PointerToMember::getDecl() const {
  const auto PTMD = this->getPTMData();
  if (PTMD.isNull())
    return nullptr;

  const NamedDecl *ND = nullptr;
  if (const auto *NDP = dyn_cast<const NamedDecl *>(PTMD))
    ND = NDP;
  else
    ND = cast<const PointerToMemberData *>(PTMD)->getDeclaratorDecl();

  return ND;
}

//===----------------------------------------------------------------------===//
// Other Iterators.
//===----------------------------------------------------------------------===//

nonloc::CompoundVal::iterator nonloc::CompoundVal::begin() const {
  return getValue()->begin();
}

nonloc::CompoundVal::iterator nonloc::CompoundVal::end() const {
  return getValue()->end();
}

nonloc::PointerToMember::iterator nonloc::PointerToMember::begin() const {
  const PTMDataType PTMD = getPTMData();
  if (isa<const NamedDecl *>(PTMD))
    return {};
  return cast<const PointerToMemberData *>(PTMD)->begin();
}

nonloc::PointerToMember::iterator nonloc::PointerToMember::end() const {
  const PTMDataType PTMD = getPTMData();
  if (isa<const NamedDecl *>(PTMD))
    return {};
  return cast<const PointerToMemberData *>(PTMD)->end();
}

//===----------------------------------------------------------------------===//
// Useful predicates.
//===----------------------------------------------------------------------===//

bool SVal::isConstant() const {
  return getAs<nonloc::ConcreteInt>() || getAs<loc::ConcreteInt>();
}

bool SVal::isConstant(int I) const {
  if (std::optional<loc::ConcreteInt> LV = getAs<loc::ConcreteInt>())
    return *LV->getValue() == I;
  if (std::optional<nonloc::ConcreteInt> NV = getAs<nonloc::ConcreteInt>())
    return *NV->getValue() == I;
  return false;
}

bool SVal::isZeroConstant() const {
  return isConstant(0);
}

//===----------------------------------------------------------------------===//
// Pretty-Printing.
//===----------------------------------------------------------------------===//

StringRef SVal::getKindStr() const {
  switch (getKind()) {
#define BASIC_SVAL(Id, Parent)                                                 \
  case Id##Kind:                                                               \
    return #Id;
#define LOC_SVAL(Id, Parent)                                                   \
  case Loc##Id##Kind:                                                          \
    return #Id;
#define NONLOC_SVAL(Id, Parent)                                                \
  case NonLoc##Id##Kind:                                                       \
    return #Id;
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.def"
#undef REGION
  }
  toolchain_unreachable("Unkown kind!");
}

LLVM_DUMP_METHOD void SVal::dump() const { dumpToStream(toolchain::errs()); }

void SVal::printJson(raw_ostream &Out, bool AddQuotes) const {
  std::string Buf;
  toolchain::raw_string_ostream TempOut(Buf);

  dumpToStream(TempOut);

  Out << JsonFormat(Buf, AddQuotes);
}

void SVal::dumpToStream(raw_ostream &os) const {
  if (isUndef()) {
    os << "Undefined";
    return;
  }
  if (isUnknown()) {
    os << "Unknown";
    return;
  }
  if (NonLoc::classof(*this)) {
    castAs<NonLoc>().dumpToStream(os);
    return;
  }
  if (Loc::classof(*this)) {
    castAs<Loc>().dumpToStream(os);
    return;
  }
  toolchain_unreachable("Unhandled SVal kind!");
}

void NonLoc::dumpToStream(raw_ostream &os) const {
  switch (getKind()) {
  case nonloc::ConcreteIntKind: {
    APSIntPtr Value = castAs<nonloc::ConcreteInt>().getValue();
    os << Value << ' ' << (Value->isSigned() ? 'S' : 'U')
       << Value->getBitWidth() << 'b';
    break;
  }
    case nonloc::SymbolValKind:
      os << castAs<nonloc::SymbolVal>().getSymbol();
      break;

    case nonloc::LocAsIntegerKind: {
      const nonloc::LocAsInteger& C = castAs<nonloc::LocAsInteger>();
      os << C.getLoc() << " [as " << C.getNumBits() << " bit integer]";
      break;
    }
    case nonloc::CompoundValKind: {
      const nonloc::CompoundVal& C = castAs<nonloc::CompoundVal>();
      os << "compoundVal{";
      bool first = true;
      for (const auto &I : C) {
        if (first) {
          os << ' '; first = false;
        }
        else
          os << ", ";

        I.dumpToStream(os);
      }
      os << "}";
      break;
    }
    case nonloc::LazyCompoundValKind: {
      const nonloc::LazyCompoundVal &C = castAs<nonloc::LazyCompoundVal>();
      os << "lazyCompoundVal{" << const_cast<void *>(C.getStore())
         << ',' << C.getRegion()
         << '}';
      break;
    }
    case nonloc::PointerToMemberKind: {
      os << "pointerToMember{";
      const nonloc::PointerToMember &CastRes =
          castAs<nonloc::PointerToMember>();
      if (CastRes.getDecl())
        os << "|" << CastRes.getDecl()->getQualifiedNameAsString() << "|";
      bool first = true;
      for (const auto &I : CastRes) {
        if (first) {
          os << ' '; first = false;
        }
        else
          os << ", ";

        os << I->getType();
      }

      os << '}';
      break;
    }
    default:
      assert(false && "Pretty-printed not implemented for this NonLoc.");
      break;
    }
}

void Loc::dumpToStream(raw_ostream &os) const {
  switch (getKind()) {
  case loc::ConcreteIntKind:
    os << castAs<loc::ConcreteInt>().getValue()->getZExtValue() << " (Loc)";
    break;
  case loc::GotoLabelKind:
    os << "&&" << castAs<loc::GotoLabel>().getLabel()->getName();
    break;
  case loc::MemRegionValKind:
    os << '&' << castAs<loc::MemRegionVal>().getRegion()->getString();
    break;
  default:
    toolchain_unreachable("Pretty-printing not implemented for this Loc.");
  }
}
