//===- AttrVisitor.h - Visitor for Attr subclasses --------------*- C++ -*-===//
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
//  This file defines the AttrVisitor interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_ATTRVISITOR_H
#define LANGUAGE_CORE_AST_ATTRVISITOR_H

#include "language/Core/AST/Attr.h"

namespace language::Core {

namespace attrvisitor {

/// A simple visitor class that helps create attribute visitors.
template <template <typename> class Ptr, typename ImplClass,
          typename RetTy = void, class... ParamTys>
class Base {
public:
#define PTR(CLASS) typename Ptr<CLASS>::type
#define DISPATCH(NAME)                                                         \
  return static_cast<ImplClass *>(this)->Visit##NAME(static_cast<PTR(NAME)>(A))

  RetTy Visit(PTR(Attr) A) {
    switch (A->getKind()) {

#define ATTR(NAME)                                                             \
  case attr::NAME:                                                             \
    DISPATCH(NAME##Attr);
#include "language/Core/Basic/AttrList.inc"
    }
    toolchain_unreachable("Attr that isn't part of AttrList.inc!");
  }

  // If the implementation chooses not to implement a certain visit
  // method, fall back to the parent.
#define ATTR(NAME)                                                             \
  RetTy Visit##NAME##Attr(PTR(NAME##Attr) A) { DISPATCH(Attr); }
#include "language/Core/Basic/AttrList.inc"

  RetTy VisitAttr(PTR(Attr)) { return RetTy(); }

#undef PTR
#undef DISPATCH
};

} // namespace attrvisitor

/// A simple visitor class that helps create attribute visitors.
///
/// This class does not preserve constness of Attr pointers (see
/// also ConstAttrVisitor).
template <typename ImplClass, typename RetTy = void, typename... ParamTys>
class AttrVisitor : public attrvisitor::Base<std::add_pointer, ImplClass, RetTy,
                                             ParamTys...> {};

/// A simple visitor class that helps create attribute visitors.
///
/// This class preserves constness of Attr pointers (see also
/// AttrVisitor).
template <typename ImplClass, typename RetTy = void, typename... ParamTys>
class ConstAttrVisitor
    : public attrvisitor::Base<toolchain::make_const_ptr, ImplClass, RetTy,
                               ParamTys...> {};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_ATTRVISITOR_H
