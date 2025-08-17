//== CheckerHelpers.h - Helper functions for checkers ------------*- C++ -*--=//
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
//  This file defines various utilities used by checkers.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_CHECKERHELPERS_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_CHECKERHELPERS_H

#include "ProgramState_Fwd.h"
#include "SVals.h"
#include "language/Core/AST/OperationKinds.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/Basic/OperatorKinds.h"
#include <optional>
#include <tuple>

namespace language::Core {

class Expr;
class VarDecl;
class QualType;
class Preprocessor;

namespace ento {

bool containsMacro(const Stmt *S);
bool containsEnum(const Stmt *S);
bool containsStaticLocal(const Stmt *S);
bool containsBuiltinOffsetOf(const Stmt *S);
template <class T> bool containsStmt(const Stmt *S) {
  if (isa<T>(S))
      return true;

  for (const Stmt *Child : S->children())
    if (Child && containsStmt<T>(Child))
      return true;

  return false;
}

std::pair<const language::Core::VarDecl *, const language::Core::Expr *>
parseAssignment(const Stmt *S);

// Do not reorder! The getMostNullable method relies on the order.
// Optimization: Most pointers expected to be unspecified. When a symbol has an
// unspecified or nonnull type non of the rules would indicate any problem for
// that symbol. For this reason only nullable and contradicted nullability are
// stored for a symbol. When a symbol is already contradicted, it can not be
// casted back to nullable.
enum class Nullability : char {
  Contradicted, // Tracked nullability is contradicted by an explicit cast. Do
                // not report any nullability related issue for this symbol.
                // This nullability is propagated aggressively to avoid false
                // positive results. See the comment on getMostNullable method.
  Nullable,
  Unspecified,
  Nonnull
};

/// Get nullability annotation for a given type.
Nullability getNullabilityAnnotation(QualType Type);

/// Try to parse the value of a defined preprocessor macro. We can only parse
/// simple expressions that consist of an optional minus sign token and then a
/// token for an integer. If we cannot parse the value then std::nullopt is
/// returned.
std::optional<int> tryExpandAsInteger(StringRef Macro, const Preprocessor &PP);

class OperatorKind {
  union {
    BinaryOperatorKind Bin;
    UnaryOperatorKind Un;
  } Op;
  bool IsBinary;

public:
  explicit OperatorKind(BinaryOperatorKind Bin) : Op{Bin}, IsBinary{true} {}
  explicit OperatorKind(UnaryOperatorKind Un) : IsBinary{false} { Op.Un = Un; }
  bool IsBinaryOp() const { return IsBinary; }

  BinaryOperatorKind GetBinaryOpUnsafe() const {
    assert(IsBinary && "cannot get binary operator - we have a unary operator");
    return Op.Bin;
  }

  std::optional<BinaryOperatorKind> GetBinaryOp() const {
    if (IsBinary)
      return Op.Bin;
    return {};
  }

  UnaryOperatorKind GetUnaryOpUnsafe() const {
    assert(!IsBinary &&
           "cannot get unary operator - we have a binary operator");
    return Op.Un;
  }

  std::optional<UnaryOperatorKind> GetUnaryOp() const {
    if (!IsBinary)
      return Op.Un;
    return {};
  }
};

OperatorKind operationKindFromOverloadedOperator(OverloadedOperatorKind OOK,
                                                 bool IsBinary);

std::optional<SVal> getPointeeVal(SVal PtrSVal, ProgramStateRef State);

/// Returns true if declaration \p D is in std namespace or any nested namespace
/// or class scope.
bool isWithinStdNamespace(const Decl *D);

} // namespace ento

} // namespace language::Core

#endif
