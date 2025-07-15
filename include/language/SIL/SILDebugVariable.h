//===--- SILDebugVariable.h -----------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILDEBUGVARIABLE_H
#define LANGUAGE_SIL_SILDEBUGVARIABLE_H

#include "language/Basic/Toolchain.h"
#include "language/SIL/SILDebugInfoExpression.h"
#include "language/SIL/SILLocation.h"
#include "language/SIL/SILType.h"
#include "toolchain/ADT/StringRef.h"

namespace language {

class AllocationInst;

/// Holds common debug information about local variables and function
/// arguments that are needed by DebugValueInst, AllocStackInst,
/// and AllocBoxInst.
struct SILDebugVariable;
inline toolchain::hash_code hash_value(const SILDebugVariable &P);

/// Holds common debug information about local variables and function
/// arguments that are needed by DebugValueInst, AllocStackInst,
/// and AllocBoxInst.
struct SILDebugVariable {
  friend toolchain::hash_code hash_value(const SILDebugVariable &P);

  StringRef Name;
  unsigned ArgNo : 16;
  unsigned Constant : 1;
  unsigned isDenseMapSingleton : 2;
  std::optional<SILType> Type;
  std::optional<SILLocation> Loc;
  const SILDebugScope *Scope;
  SILDebugInfoExpression DIExpr;

  // Use vanilla copy ctor / operator
  SILDebugVariable(const SILDebugVariable &) = default;
  SILDebugVariable &operator=(const SILDebugVariable &) = default;

  enum class IsDenseMapSingleton { No, IsEmpty, IsTombstone };
  SILDebugVariable(IsDenseMapSingleton inputIsDenseMapSingleton)
      : SILDebugVariable() {
    assert(inputIsDenseMapSingleton != IsDenseMapSingleton::No &&
           "Should only pass IsEmpty or IsTombstone");
    isDenseMapSingleton = unsigned(inputIsDenseMapSingleton);
  }

  SILDebugVariable()
      : ArgNo(0), Constant(false),  isDenseMapSingleton(0),
        Scope(nullptr) {}
  SILDebugVariable(bool Constant, uint16_t ArgNo)
      : ArgNo(ArgNo), Constant(Constant),
        isDenseMapSingleton(0), Scope(nullptr) {}
  SILDebugVariable(StringRef Name, bool Constant, unsigned ArgNo,
                   std::optional<SILType> AuxType = {},
                   std::optional<SILLocation> DeclLoc = {},
                   const SILDebugScope *DeclScope = nullptr,
                   toolchain::ArrayRef<SILDIExprElement> ExprElements = {})
      : Name(Name), ArgNo(ArgNo), Constant(Constant),
        isDenseMapSingleton(0), Type(AuxType), Loc(DeclLoc), Scope(DeclScope),
        DIExpr(ExprElements) {}

  /// Created from either AllocStack or AllocBox instruction
  static std::optional<SILDebugVariable>
  createFromAllocation(const AllocationInst *AI);

  /// Return the underlying variable declaration that this denotes,
  /// or nullptr if we don't have one.
  VarDecl *getDecl() const;

  // We're not comparing DIExpr here because strictly speaking,
  // DIExpr is not part of the debug variable. We simply piggyback
  // it in this class so that's it's easier to carry DIExpr around.
  bool operator==(const SILDebugVariable &V) const {
    return ArgNo == V.ArgNo && Constant == V.Constant && Name == V.Name &&
           Type == V.Type && Loc == V.Loc && Scope == V.Scope &&
           isDenseMapSingleton == V.isDenseMapSingleton && DIExpr == V.DIExpr;
  }

  SILDebugVariable withoutDIExpr() const {
    auto result = *this;
    result.DIExpr = {};
    return result;
  }

  bool isLet() const { return Name.size() && Constant; }

  bool isVar() const { return Name.size() && !Constant; }
};

/// Returns the hashcode for the new projection path.
inline toolchain::hash_code hash_value(const SILDebugVariable &P) {
  return toolchain::hash_combine(P.ArgNo, P.Constant, P.Name,
                            P.isDenseMapSingleton, P.Type, P.Loc, P.Scope,
                            P.DIExpr);
}

} // namespace language

#endif
