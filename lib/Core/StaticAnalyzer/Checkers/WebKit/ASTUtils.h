//=======- ASTUtis.h ---------------------------------------------*- C++ -*-==//
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

#ifndef LANGUAGE_CORE_ANALYZER_WEBKIT_ASTUTILS_H
#define LANGUAGE_CORE_ANALYZER_WEBKIT_ASTUTILS_H

#include "language/Core/AST/Decl.h"
#include "toolchain/ADT/APInt.h"
#include "toolchain/Support/Casting.h"

#include <functional>
#include <string>
#include <utility>

namespace language::Core {
class Expr;

/// This function de-facto defines a set of transformations that we consider
/// safe (in heuristical sense). These transformation if passed a safe value as
/// an input should provide a safe value (or an object that provides safe
/// values).
///
/// For more context see Static Analyzer checkers documentation - specifically
/// webkit.UncountedCallArgsChecker checker. Allowed list of transformations:
/// - constructors of ref-counted types (including factory methods)
/// - getters of ref-counted types
/// - member overloaded operators
/// - casts
/// - unary operators like ``&`` or ``*``
///
/// If passed expression is of type uncounted pointer/reference we try to find
/// the "origin" of the pointer value.
/// Origin can be for example a local variable, nullptr, constant or
/// this-pointer.
///
/// Certain subexpression nodes represent transformations that don't affect
/// where the memory address originates from. We try to traverse such
/// subexpressions to get to the relevant child nodes. Whenever we encounter a
/// subexpression that either can't be ignored, we don't model its semantics or
/// that has multiple children we stop.
///
/// \p E is an expression of uncounted pointer/reference type.
/// If \p StopAtFirstRefCountedObj is true and we encounter a subexpression that
/// represents ref-counted object during the traversal we return relevant
/// sub-expression and true.
///
/// Calls \p callback with the subexpression that we traversed to and if \p
/// StopAtFirstRefCountedObj is true we also specify whether we stopped early.
/// Returns false if any of calls to callbacks returned false. Otherwise true.
bool tryToFindPtrOrigin(
    const language::Core::Expr *E, bool StopAtFirstRefCountedObj,
    std::function<bool(const language::Core::CXXRecordDecl *)> isSafePtr,
    std::function<bool(const language::Core::QualType)> isSafePtrType,
    std::function<bool(const language::Core::Expr *, bool)> callback);

/// For \p E referring to a ref-countable/-counted pointer/reference we return
/// whether it's a safe call argument. Examples: function parameter or
/// this-pointer. The logic relies on the set of recursive rules we enforce for
/// WebKit codebase.
///
/// \returns Whether \p E is a safe call arugment.
bool isASafeCallArg(const language::Core::Expr *E);

/// \returns true if E is a MemberExpr accessing a const smart pointer type.
bool isConstOwnerPtrMemberExpr(const language::Core::Expr *E);

/// \returns true if E is a MemberExpr accessing a member variable which
/// supports CheckedPtr.
bool isExprToGetCheckedPtrCapableMember(const language::Core::Expr *E);

/// \returns true if E is a CXXMemberCallExpr which returns a const smart
/// pointer type.
class EnsureFunctionAnalysis {
  using CacheTy = toolchain::DenseMap<const FunctionDecl *, bool>;
  mutable CacheTy Cache{};

public:
  bool isACallToEnsureFn(const Expr *E) const;
};

/// \returns name of AST node or empty string.
template <typename T> std::string safeGetName(const T *ASTNode) {
  const auto *const ND = toolchain::dyn_cast_or_null<language::Core::NamedDecl>(ASTNode);
  if (!ND)
    return "";

  // In case F is for example "operator|" the getName() method below would
  // assert.
  if (!ND->getDeclName().isIdentifier())
    return "";

  return ND->getName().str();
}

} // namespace language::Core

#endif
