//==- DependentDiagnostic.h - Dependently-generated diagnostics --*- C++ -*-==//
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
//  This file defines interfaces for diagnostics which may or may
//  fire based on how a template is instantiated.
//
//  At the moment, the only consumer of this interface is access
//  control.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_DEPENDENTDIAGNOSTIC_H
#define LANGUAGE_CORE_AST_DEPENDENTDIAGNOSTIC_H

#include "language/Core/AST/DeclBase.h"
#include "language/Core/AST/DeclContextInternals.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/PartialDiagnostic.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/Specifiers.h"
#include <cassert>
#include <iterator>

namespace language::Core {

class ASTContext;
class CXXRecordDecl;
class NamedDecl;

/// A dependently-generated diagnostic.
class DependentDiagnostic {
public:
  enum AccessNonce { Access = 0 };

  static DependentDiagnostic *Create(ASTContext &Context,
                                     DeclContext *Parent,
                                     AccessNonce _,
                                     SourceLocation Loc,
                                     bool IsMemberAccess,
                                     AccessSpecifier AS,
                                     NamedDecl *TargetDecl,
                                     CXXRecordDecl *NamingClass,
                                     QualType BaseObjectType,
                                     const PartialDiagnostic &PDiag) {
    DependentDiagnostic *DD = Create(Context, Parent, PDiag);
    DD->AccessData.Loc = Loc;
    DD->AccessData.IsMember = IsMemberAccess;
    DD->AccessData.Access = AS;
    DD->AccessData.TargetDecl = TargetDecl;
    DD->AccessData.NamingClass = NamingClass;
    DD->AccessData.BaseObjectType = BaseObjectType.getAsOpaquePtr();
    return DD;
  }

  unsigned getKind() const {
    return Access;
  }

  bool isAccessToMember() const {
    assert(getKind() == Access);
    return AccessData.IsMember;
  }

  AccessSpecifier getAccess() const {
    assert(getKind() == Access);
    return AccessSpecifier(AccessData.Access);
  }

  SourceLocation getAccessLoc() const {
    assert(getKind() == Access);
    return AccessData.Loc;
  }

  NamedDecl *getAccessTarget() const {
    assert(getKind() == Access);
    return AccessData.TargetDecl;
  }

  NamedDecl *getAccessNamingClass() const {
    assert(getKind() == Access);
    return AccessData.NamingClass;
  }

  QualType getAccessBaseObjectType() const {
    assert(getKind() == Access);
    return QualType::getFromOpaquePtr(AccessData.BaseObjectType);
  }

  const PartialDiagnostic &getDiagnostic() const {
    return Diag;
  }

private:
  friend class DeclContext::ddiag_iterator;
  friend class DependentStoredDeclsMap;

  DependentDiagnostic(const PartialDiagnostic &PDiag,
                      DiagnosticStorage *Storage)
      : Diag(PDiag, Storage) {}

  static DependentDiagnostic *Create(ASTContext &Context,
                                     DeclContext *Parent,
                                     const PartialDiagnostic &PDiag);

  DependentDiagnostic *NextDiagnostic;

  PartialDiagnostic Diag;

  struct {
    SourceLocation Loc;
    LLVM_PREFERRED_TYPE(AccessSpecifier)
    unsigned Access : 2;
    LLVM_PREFERRED_TYPE(bool)
    unsigned IsMember : 1;
    NamedDecl *TargetDecl;
    CXXRecordDecl *NamingClass;
    void *BaseObjectType;
  } AccessData;
};

/// An iterator over the dependent diagnostics in a dependent context.
class DeclContext::ddiag_iterator {
public:
  ddiag_iterator() = default;
  explicit ddiag_iterator(DependentDiagnostic *Ptr) : Ptr(Ptr) {}

  using value_type = DependentDiagnostic *;
  using reference = DependentDiagnostic *;
  using pointer = DependentDiagnostic *;
  using difference_type = int;
  using iterator_category = std::forward_iterator_tag;

  reference operator*() const { return Ptr; }

  ddiag_iterator &operator++() {
    assert(Ptr && "attempt to increment past end of diag list");
    Ptr = Ptr->NextDiagnostic;
    return *this;
  }

  ddiag_iterator operator++(int) {
    ddiag_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(ddiag_iterator Other) const {
    return Ptr == Other.Ptr;
  }

  bool operator!=(ddiag_iterator Other) const {
    return Ptr != Other.Ptr;
  }

  ddiag_iterator &operator+=(difference_type N) {
    assert(N >= 0 && "cannot rewind a DeclContext::ddiag_iterator");
    while (N--)
      ++*this;
    return *this;
  }

  ddiag_iterator operator+(difference_type N) const {
    ddiag_iterator tmp = *this;
    tmp += N;
    return tmp;
  }

private:
  DependentDiagnostic *Ptr = nullptr;
};

inline DeclContext::ddiag_range DeclContext::ddiags() const {
  assert(isDependentContext()
         && "cannot iterate dependent diagnostics of non-dependent context");
  const DependentStoredDeclsMap *Map
    = static_cast<DependentStoredDeclsMap*>(getPrimaryContext()->getLookupPtr());

  if (!Map)
    // Return an empty range using the always-end default constructor.
    return ddiag_range(ddiag_iterator(), ddiag_iterator());

  return ddiag_range(ddiag_iterator(Map->FirstDiagnostic), ddiag_iterator());
}

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_DEPENDENTDIAGNOSTIC_H
