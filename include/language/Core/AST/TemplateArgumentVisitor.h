//===- TemplateArgumentVisitor.h - Visitor for TArg subclasses --*- C++ -*-===//
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
//  This file defines the TemplateArgumentVisitor interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_TEMPLATEARGUMENTVISITOR_H
#define LANGUAGE_CORE_AST_TEMPLATEARGUMENTVISITOR_H

#include "language/Core/AST/TemplateBase.h"

namespace language::Core {

namespace templateargumentvisitor {

/// A simple visitor class that helps create template argument visitors.
template <template <typename> class Ref, typename ImplClass,
          typename RetTy = void, typename... ParamTys>
class Base {
public:
#define REF(CLASS) typename Ref<CLASS>::type
#define DISPATCH(NAME)                                                         \
  case TemplateArgument::NAME:                                                 \
    return static_cast<ImplClass *>(this)->Visit##NAME##TemplateArgument(      \
        TA, std::forward<ParamTys>(P)...)

  RetTy Visit(REF(TemplateArgument) TA, ParamTys... P) {
    switch (TA.getKind()) {
      DISPATCH(Null);
      DISPATCH(Type);
      DISPATCH(Declaration);
      DISPATCH(NullPtr);
      DISPATCH(Integral);
      DISPATCH(StructuralValue);
      DISPATCH(Template);
      DISPATCH(TemplateExpansion);
      DISPATCH(Expression);
      DISPATCH(Pack);
    }
    toolchain_unreachable("TemplateArgument is not covered in switch!");
  }

  // If the implementation chooses not to implement a certain visit
  // method, fall back to the parent.

#define VISIT_METHOD(CATEGORY)                                                 \
  RetTy Visit##CATEGORY##TemplateArgument(REF(TemplateArgument) TA,            \
                                          ParamTys... P) {                     \
    return static_cast<ImplClass *>(this)->VisitTemplateArgument(              \
        TA, std::forward<ParamTys>(P)...);                                     \
  }

  VISIT_METHOD(Null);
  VISIT_METHOD(Type);
  VISIT_METHOD(Declaration);
  VISIT_METHOD(NullPtr);
  VISIT_METHOD(Integral);
  VISIT_METHOD(StructuralValue);
  VISIT_METHOD(Template);
  VISIT_METHOD(TemplateExpansion);
  VISIT_METHOD(Expression);
  VISIT_METHOD(Pack);

  RetTy VisitTemplateArgument(REF(TemplateArgument), ParamTys...) {
    return RetTy();
  }

#undef REF
#undef DISPATCH
#undef VISIT_METHOD
};

} // namespace templateargumentvisitor

/// A simple visitor class that helps create template argument visitors.
///
/// This class does not preserve constness of TemplateArgument references (see
/// also ConstTemplateArgumentVisitor).
template <typename ImplClass, typename RetTy = void, typename... ParamTys>
class TemplateArgumentVisitor
    : public templateargumentvisitor::Base<std::add_lvalue_reference, ImplClass,
                                           RetTy, ParamTys...> {};

/// A simple visitor class that helps create template argument visitors.
///
/// This class preserves constness of TemplateArgument references (see also
/// TemplateArgumentVisitor).
template <typename ImplClass, typename RetTy = void, typename... ParamTys>
class ConstTemplateArgumentVisitor
    : public templateargumentvisitor::Base<toolchain::make_const_ref, ImplClass,
                                           RetTy, ParamTys...> {};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_TEMPLATEARGUMENTVISITOR_H
