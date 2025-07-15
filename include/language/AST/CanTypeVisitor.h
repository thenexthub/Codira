//===--- CanTypeVisitor.h - TypeVisitor specialization ----------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This file defines CanTypeVisitor, a specialized version of
// TypeVisitor for visiting fully type-checked canonical types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_CANTYPEVISITOR_H
#define LANGUAGE_AST_CANTYPEVISITOR_H

#include "language/AST/Types.h"

namespace language {

/// This is a specialization of language::TypeVisitor which:
///   - works only on canonical and fully-checked types and
///   - preserves the canonicality of the visited types in the
///     static (C++) type.
template<typename ImplClass, typename RetTy = void, typename... Args>
class CanTypeVisitor {
public:
  RetTy visit(CanType T, Args... args) {
    switch (T->getKind()) {
#define SUGARED_TYPE(CLASS, PARENT) \
    case TypeKind::CLASS:
#define TYPE(CLASS, PARENT)
#include "language/AST/TypeNodes.def"
      toolchain_unreachable("non-canonical type");

#define SUGARED_TYPE(CLASS, PARENT)
#define TYPE(CLASS, PARENT)                                  \
    case TypeKind::CLASS:                                    \
      return static_cast<ImplClass*>(this)                   \
        ->visit##CLASS##Type(cast<CLASS##Type>(T),           \
                             ::std::forward<Args>(args)...);
#include "language/AST/TypeNodes.def"
    }
    toolchain_unreachable("Not reachable, all cases handled");
  }
  
  // Provide default implementations of abstract "visit" implementations that
  // just chain to their base class.  This allows visitors to just implement
  // the base behavior and handle all subclasses if they desire.  Since this is
  // a template, it will only instantiate cases that are used and thus we still
  // require full coverage of the AST nodes by the visitor.
#define ABSTRACT_TYPE(CLASS, PARENT)                           \
  RetTy visit##CLASS##Type(Can##CLASS##Type T, Args... args) { \
     return static_cast<ImplClass*>(this)                      \
              ->visit##PARENT(T, std::forward<Args>(args)...); \
  }
#define TYPE(CLASS, PARENT) ABSTRACT_TYPE(CLASS, PARENT)
#define ABSTRACT_SUGARED_TYPE(CLASS, PARENT)
#define SUGARED_TYPE(CLASS, PARENT)
  // Don't allow unchecked types by default, but allow visitors to opt-in to
  // handling them.
#define UNCHECKED_TYPE(CLASS, PARENT)                          \
  RetTy visit##CLASS##Type(Can##CLASS##Type T, Args... args) { \
     toolchain_unreachable("unchecked type");                       \
  }
#include "language/AST/TypeNodes.def"
};

} // end namespace language
  
#endif
