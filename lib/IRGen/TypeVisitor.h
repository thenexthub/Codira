//===--- TypeVisitor.h - IR-gen TypeVisitor specialization ------*- C++ -*-===//
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
// This file defines various type visitors that are useful in
// IR-generation.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_TYPEVISITOR_H
#define SWIFT_IRGEN_TYPEVISITOR_H

#include "language/AST/CanTypeVisitor.h"

namespace language {
namespace irgen {

/// ReferenceTypeVisitor - This is a specialization of CanTypeVisitor
/// which automatically ignores non-reference types.
template <typename ImplClass, typename RetTy = void, typename... Args>
class ReferenceTypeVisitor : public CanTypeVisitor<ImplClass, RetTy, Args...> {
#define TYPE(Id) \
  RetTy visit##Id##Type(Can##Id##Type T, Args... args) { \
    llvm_unreachable(#Id "Type is not a reference type"); \
  }
  TYPE(BoundGenericEnum)
  TYPE(BoundGenericStruct)
  TYPE(BuiltinFloat)
  TYPE(BuiltinInteger)
  TYPE(BuiltinRawPointer)
  TYPE(BuiltinVector)
  TYPE(LValue)
  TYPE(Metatype)
  TYPE(Module)
  TYPE(Enum)
  TYPE(ReferenceStorage)
  TYPE(Struct)
  TYPE(Tuple)
#undef TYPE

  // BuiltinNativeObject
  // BuiltinBridgeObject
  // Class
  // BoundGenericClass
  // Protocol
  // ProtocolComposition
  // Archetype
  // Function
};
  
} // end namespace irgen
} // end namespace language
  
#endif
