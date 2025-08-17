//===--- Function.h - Bytecode function for the VM --------------*- C++ -*-===//
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

#include "Function.h"
#include "Program.h"
#include "language/Core/AST/ASTLambda.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"

using namespace language::Core;
using namespace language::Core::interp;

Function::Function(Program &P, FunctionDeclTy Source, unsigned ArgSize,
                   toolchain::SmallVectorImpl<PrimType> &&ParamTypes,
                   toolchain::DenseMap<unsigned, ParamDescriptor> &&Params,
                   toolchain::SmallVectorImpl<unsigned> &&ParamOffsets,
                   bool HasThisPointer, bool HasRVO, bool IsLambdaStaticInvoker)
    : P(P), Kind(FunctionKind::Normal), Source(Source), ArgSize(ArgSize),
      ParamTypes(std::move(ParamTypes)), Params(std::move(Params)),
      ParamOffsets(std::move(ParamOffsets)), IsValid(false),
      IsFullyCompiled(false), HasThisPointer(HasThisPointer), HasRVO(HasRVO),
      HasBody(false), Defined(false) {
  if (const auto *F = dyn_cast<const FunctionDecl *>(Source)) {
    Variadic = F->isVariadic();
    Immediate = F->isImmediateFunction();
    Constexpr = F->isConstexpr() || F->hasAttr<MSConstexprAttr>();
    if (const auto *CD = dyn_cast<CXXConstructorDecl>(F)) {
      Virtual = CD->isVirtual();
      Kind = FunctionKind::Ctor;
    } else if (const auto *CD = dyn_cast<CXXDestructorDecl>(F)) {
      Virtual = CD->isVirtual();
      Kind = FunctionKind::Dtor;
    } else if (const auto *MD = dyn_cast<CXXMethodDecl>(F)) {
      Virtual = MD->isVirtual();
      if (IsLambdaStaticInvoker)
        Kind = FunctionKind::LambdaStaticInvoker;
      else if (language::Core::isLambdaCallOperator(F))
        Kind = FunctionKind::LambdaCallOperator;
      else if (MD->isCopyAssignmentOperator() || MD->isMoveAssignmentOperator())
        Kind = FunctionKind::CopyOrMoveOperator;
    } else {
      Virtual = false;
    }
  } else {
    Variadic = false;
    Virtual = false;
    Immediate = false;
    Constexpr = false;
  }
}

Function::ParamDescriptor Function::getParamDescriptor(unsigned Offset) const {
  auto It = Params.find(Offset);
  assert(It != Params.end() && "Invalid parameter offset");
  return It->second;
}

SourceInfo Function::getSource(CodePtr PC) const {
  assert(PC >= getCodeBegin() && "PC does not belong to this function");
  assert(PC <= getCodeEnd() && "PC Does not belong to this function");
  assert(hasBody() && "Function has no body");
  unsigned Offset = PC - getCodeBegin();
  using Elem = std::pair<unsigned, SourceInfo>;
  auto It = toolchain::lower_bound(SrcMap, Elem{Offset, {}}, toolchain::less_first());
  if (It == SrcMap.end())
    return SrcMap.back().second;
  return It->second;
}
