//===- LowLevelHelpers.cpp -------------------------------------*- C++ -*-===//
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

#include "language/Core/ASTMatchers/LowLevelHelpers.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/ExprCXX.h"
#include <type_traits>

namespace language::Core {
namespace ast_matchers {

static const FunctionDecl *getCallee(const CXXConstructExpr &D) {
  return D.getConstructor();
}
static const FunctionDecl *getCallee(const CallExpr &D) {
  return D.getDirectCallee();
}

template <class ExprNode>
static void matchEachArgumentWithParamTypeImpl(
    const ExprNode &Node,
    toolchain::function_ref<void(QualType /*Param*/, const Expr * /*Arg*/)>
        OnParamAndArg) {
  static_assert(std::is_same_v<CallExpr, ExprNode> ||
                std::is_same_v<CXXConstructExpr, ExprNode>);
  // The first argument of an overloaded member operator is the implicit object
  // argument of the method which should not be matched against a parameter, so
  // we skip over it here.
  unsigned ArgIndex = 0;
  if (const auto *CE = dyn_cast<CXXOperatorCallExpr>(&Node)) {
    const auto *MD = dyn_cast_or_null<CXXMethodDecl>(CE->getDirectCallee());
    if (MD && !MD->isExplicitObjectMemberFunction()) {
      // This is an overloaded operator call.
      // We need to skip the first argument, which is the implicit object
      // argument of the method which should not be matched against a
      // parameter.
      ++ArgIndex;
    }
  }

  const FunctionProtoType *FProto = nullptr;

  if (const auto *Call = dyn_cast<CallExpr>(&Node)) {
    if (const auto *Value =
            dyn_cast_or_null<ValueDecl>(Call->getCalleeDecl())) {
      QualType QT = Value->getType().getCanonicalType();

      // This does not necessarily lead to a `FunctionProtoType`,
      // e.g. K&R functions do not have a function prototype.
      if (QT->isFunctionPointerType())
        FProto = QT->getPointeeType()->getAs<FunctionProtoType>();

      if (QT->isMemberFunctionPointerType()) {
        const auto *MP = QT->getAs<MemberPointerType>();
        assert(MP && "Must be member-pointer if its a memberfunctionpointer");
        FProto = MP->getPointeeType()->getAs<FunctionProtoType>();
        assert(FProto &&
               "The call must have happened through a member function "
               "pointer");
      }
    }
  }

  unsigned ParamIndex = 0;
  unsigned NumArgs = Node.getNumArgs();
  if (FProto && FProto->isVariadic())
    NumArgs = std::min(NumArgs, FProto->getNumParams());

  for (; ArgIndex < NumArgs; ++ArgIndex, ++ParamIndex) {
    QualType ParamType;
    if (FProto && FProto->getNumParams() > ParamIndex)
      ParamType = FProto->getParamType(ParamIndex);
    else if (const FunctionDecl *FD = getCallee(Node);
             FD && FD->getNumParams() > ParamIndex)
      ParamType = FD->getParamDecl(ParamIndex)->getType();
    else
      continue;

    OnParamAndArg(ParamType, Node.getArg(ArgIndex)->IgnoreParenCasts());
  }
}

void matchEachArgumentWithParamType(
    const CallExpr &Node,
    toolchain::function_ref<void(QualType /*Param*/, const Expr * /*Arg*/)>
        OnParamAndArg) {
  matchEachArgumentWithParamTypeImpl(Node, OnParamAndArg);
}

void matchEachArgumentWithParamType(
    const CXXConstructExpr &Node,
    toolchain::function_ref<void(QualType /*Param*/, const Expr * /*Arg*/)>
        OnParamAndArg) {
  matchEachArgumentWithParamTypeImpl(Node, OnParamAndArg);
}

} // namespace ast_matchers

} // namespace language::Core
