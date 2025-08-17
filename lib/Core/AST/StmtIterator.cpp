//===- StmtIterator.cpp - Iterators for Statements ------------------------===//
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
// This file defines internal methods for StmtIterator.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/StmtIterator.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/LLVM.h"
#include <cassert>
#include <cstdint>

using namespace language::Core;

// FIXME: Add support for dependent-sized array types in C++?
// Does it even make sense to build a CFG for an uninstantiated template?
static inline const VariableArrayType *FindVA(const Type* t) {
  while (const ArrayType *vt = dyn_cast<ArrayType>(t)) {
    if (const VariableArrayType *vat = dyn_cast<VariableArrayType>(vt))
      if (vat->getSizeExpr())
        return vat;

    t = vt->getElementType().getTypePtr();
  }

  return nullptr;
}

void StmtIteratorBase::NextVA() {
  assert(getVAPtr());

  const VariableArrayType *p = getVAPtr();
  p = FindVA(p->getElementType().getTypePtr());
  setVAPtr(p);

  if (p)
    return;

  if (inDeclGroup()) {
    if (VarDecl* VD = dyn_cast<VarDecl>(*DGI))
      if (VD->hasInit())
        return;

    NextDecl();
  }
  else {
    assert(inSizeOfTypeVA());
    RawVAPtr = 0;
  }
}

void StmtIteratorBase::NextDecl(bool ImmediateAdvance) {
  assert(getVAPtr() == nullptr);
  assert(inDeclGroup());

  if (ImmediateAdvance)
    ++DGI;

  for ( ; DGI != DGE; ++DGI)
    if (HandleDecl(*DGI))
      return;

  RawVAPtr = 0;
}

bool StmtIteratorBase::HandleDecl(Decl* D) {
  if (VarDecl* VD = dyn_cast<VarDecl>(D)) {
    if (const VariableArrayType* VAPtr = FindVA(VD->getType().getTypePtr())) {
      setVAPtr(VAPtr);
      return true;
    }

    if (VD->getInit())
      return true;
  }
  else if (TypedefNameDecl* TD = dyn_cast<TypedefNameDecl>(D)) {
    if (const VariableArrayType* VAPtr =
        FindVA(TD->getUnderlyingType().getTypePtr())) {
      setVAPtr(VAPtr);
      return true;
    }
  }
  else if (EnumConstantDecl* ECD = dyn_cast<EnumConstantDecl>(D)) {
    if (ECD->getInitExpr())
      return true;
  }

  return false;
}

StmtIteratorBase::StmtIteratorBase(Decl** dgi, Decl** dge)
    : DGI(dgi), RawVAPtr(DeclGroupMode), DGE(dge) {
  NextDecl(false);
}

StmtIteratorBase::StmtIteratorBase(const VariableArrayType* t)
    : DGI(nullptr), RawVAPtr(SizeOfTypeVAMode) {
  RawVAPtr |= reinterpret_cast<uintptr_t>(t);
}

Stmt*& StmtIteratorBase::GetDeclExpr() const {
  if (const VariableArrayType* VAPtr = getVAPtr()) {
    assert(VAPtr->SizeExpr);
    return const_cast<Stmt*&>(VAPtr->SizeExpr);
  }

  assert(inDeclGroup());
  VarDecl* VD = cast<VarDecl>(*DGI);
  return *VD->getInitAddress();
}
