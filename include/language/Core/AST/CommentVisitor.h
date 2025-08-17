//===- CommentVisitor.h - Visitor for Comment subclasses --------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_COMMENTVISITOR_H
#define LANGUAGE_CORE_AST_COMMENTVISITOR_H

#include "language/Core/AST/Comment.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language::Core {
namespace comments {
template <template <typename> class Ptr, typename ImplClass,
          typename RetTy = void, class... ParamTys>
class CommentVisitorBase {
public:
#define PTR(CLASS) typename Ptr<CLASS>::type
#define DISPATCH(NAME, CLASS)                                                  \
  return static_cast<ImplClass *>(this)->visit##NAME(                          \
      static_cast<PTR(CLASS)>(C), std::forward<ParamTys>(P)...)

  RetTy visit(PTR(Comment) C, ParamTys... P) {
    if (!C)
      return RetTy();

    switch (C->getCommentKind()) {
    default: toolchain_unreachable("Unknown comment kind!");
#define ABSTRACT_COMMENT(COMMENT)
#define COMMENT(CLASS, PARENT)                                                 \
  case CommentKind::CLASS:                                                     \
    DISPATCH(CLASS, CLASS);
#include "language/Core/AST/CommentNodes.inc"
#undef ABSTRACT_COMMENT
#undef COMMENT
    }
  }

  // If the derived class does not implement a certain Visit* method, fall back
  // on Visit* method for the superclass.
#define ABSTRACT_COMMENT(COMMENT) COMMENT
#define COMMENT(CLASS, PARENT)                                                 \
  RetTy visit##CLASS(PTR(CLASS) C, ParamTys... P) { DISPATCH(PARENT, PARENT); }
#include "language/Core/AST/CommentNodes.inc"
#undef ABSTRACT_COMMENT
#undef COMMENT

  RetTy visitComment(PTR(Comment) C, ParamTys... P) { return RetTy(); }

#undef PTR
#undef DISPATCH
};

template <typename ImplClass, typename RetTy = void, class... ParamTys>
class CommentVisitor : public CommentVisitorBase<std::add_pointer, ImplClass,
                                                 RetTy, ParamTys...> {};

template <typename ImplClass, typename RetTy = void, class... ParamTys>
class ConstCommentVisitor
    : public CommentVisitorBase<toolchain::make_const_ptr, ImplClass, RetTy,
                                ParamTys...> {};

} // namespace comments
} // namespace language::Core

#endif // LANGUAGE_CORE_AST_COMMENTVISITOR_H
