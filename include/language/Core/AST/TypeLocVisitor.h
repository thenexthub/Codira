//===--- TypeLocVisitor.h - Visitor for TypeLoc subclasses ------*- C++ -*-===//
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
//  This file defines the TypeLocVisitor interface.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_AST_TYPELOCVISITOR_H
#define LANGUAGE_CORE_AST_TYPELOCVISITOR_H

#include "language/Core/AST/TypeLoc.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language::Core {

#define DISPATCH(CLASSNAME) \
  return static_cast<ImplClass*>(this)-> \
    Visit##CLASSNAME(TyLoc.castAs<CLASSNAME>())

template<typename ImplClass, typename RetTy=void>
class TypeLocVisitor {
public:
  RetTy Visit(TypeLoc TyLoc) {
    switch (TyLoc.getTypeLocClass()) {
#define ABSTRACT_TYPELOC(CLASS, PARENT)
#define TYPELOC(CLASS, PARENT) \
    case TypeLoc::CLASS: DISPATCH(CLASS##TypeLoc);
#include "language/Core/AST/TypeLocNodes.def"
    }
    toolchain_unreachable("unexpected type loc class!");
  }

  RetTy Visit(UnqualTypeLoc TyLoc) {
    switch (TyLoc.getTypeLocClass()) {
#define ABSTRACT_TYPELOC(CLASS, PARENT)
#define TYPELOC(CLASS, PARENT) \
    case TypeLoc::CLASS: DISPATCH(CLASS##TypeLoc);
#include "language/Core/AST/TypeLocNodes.def"
    }
    toolchain_unreachable("unexpected type loc class!");
  }

#define TYPELOC(CLASS, PARENT)      \
  RetTy Visit##CLASS##TypeLoc(CLASS##TypeLoc TyLoc) { \
    DISPATCH(PARENT);               \
  }
#include "language/Core/AST/TypeLocNodes.def"

  RetTy VisitTypeLoc(TypeLoc TyLoc) { return RetTy(); }
};

#undef DISPATCH

}  // end namespace language::Core

#endif // LANGUAGE_CORE_AST_TYPELOCVISITOR_H
