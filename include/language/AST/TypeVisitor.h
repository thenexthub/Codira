//===--- TypeVisitor.h - Type Visitor ---------------------------*- C++ -*-===//
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
// This file defines the TypeVisitor class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_TYPEVISITOR_H
#define LANGUAGE_AST_TYPEVISITOR_H

#include "language/AST/Types.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language {
  
/// TypeVisitor - This is a simple visitor class for Codira types.
template<typename ImplClass, typename RetTy = void, typename... Args> 
class TypeVisitor {
public:

  RetTy visit(Type T, Args... args) {
    switch (T->getKind()) {
#define TYPE(CLASS, PARENT) \
    case TypeKind::CLASS: \
      return static_cast<ImplClass*>(this) \
        ->visit##CLASS##Type(static_cast<CLASS##Type*>(T.getPointer()), \
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
  RetTy visit##CLASS##Type(CLASS##Type *T, Args... args) {     \
     return static_cast<ImplClass*>(this)                      \
              ->visit##PARENT(T, std::forward<Args>(args)...); \
  }
#define TYPE(CLASS, PARENT) ABSTRACT_TYPE(CLASS, PARENT)
#include "language/AST/TypeNodes.def"

};
  
} // end namespace language
  
#endif
