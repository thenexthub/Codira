//===- DeclVisitor.h - Visitor for Decl subclasses --------------*- C++ -*-===//
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
//  This file defines the DeclVisitor interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_DECLVISITOR_H
#define LANGUAGE_CORE_AST_DECLVISITOR_H

#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclFriend.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/AST/DeclOpenACC.h"
#include "language/Core/AST/DeclOpenMP.h"
#include "language/Core/AST/DeclTemplate.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language::Core {

namespace declvisitor {
/// A simple visitor class that helps create declaration visitors.
template<template <typename> class Ptr, typename ImplClass, typename RetTy=void>
class Base {
public:
#define PTR(CLASS) typename Ptr<CLASS>::type
#define DISPATCH(NAME, CLASS) \
  return static_cast<ImplClass*>(this)->Visit##NAME(static_cast<PTR(CLASS)>(D))

  RetTy Visit(PTR(Decl) D) {
    switch (D->getKind()) {
#define DECL(DERIVED, BASE) \
      case Decl::DERIVED: DISPATCH(DERIVED##Decl, DERIVED##Decl);
#define ABSTRACT_DECL(DECL)
#include "language/Core/AST/DeclNodes.inc"
    }
    toolchain_unreachable("Decl that isn't part of DeclNodes.inc!");
  }

  // If the implementation chooses not to implement a certain visit
  // method, fall back to the parent.
#define DECL(DERIVED, BASE) \
  RetTy Visit##DERIVED##Decl(PTR(DERIVED##Decl) D) { DISPATCH(BASE, BASE); }
#include "language/Core/AST/DeclNodes.inc"

  RetTy VisitDecl(PTR(Decl) D) { return RetTy(); }

#undef PTR
#undef DISPATCH
};

} // namespace declvisitor

/// A simple visitor class that helps create declaration visitors.
///
/// This class does not preserve constness of Decl pointers (see also
/// ConstDeclVisitor).
template <typename ImplClass, typename RetTy = void>
class DeclVisitor
    : public declvisitor::Base<std::add_pointer, ImplClass, RetTy> {};

/// A simple visitor class that helps create declaration visitors.
///
/// This class preserves constness of Decl pointers (see also DeclVisitor).
template <typename ImplClass, typename RetTy = void>
class ConstDeclVisitor
    : public declvisitor::Base<toolchain::make_const_ptr, ImplClass, RetTy> {};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_DECLVISITOR_H
