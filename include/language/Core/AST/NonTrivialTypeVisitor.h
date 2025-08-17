//===-- NonTrivialTypeVisitor.h - Visitor for non-trivial Types -*- C++ -*-===//
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
//  This file defines the visitor classes that are used to traverse non-trivial
//  structs.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_NONTRIVIALTYPEVISITOR_H
#define LANGUAGE_CORE_AST_NONTRIVIALTYPEVISITOR_H

#include "language/Core/AST/Type.h"

namespace language::Core {

template <class Derived, class RetTy = void> struct DestructedTypeVisitor {
  template <class... Ts> RetTy visit(QualType FT, Ts &&... Args) {
    return asDerived().visitWithKind(FT.isDestructedType(), FT,
                                     std::forward<Ts>(Args)...);
  }

  template <class... Ts>
  RetTy visitWithKind(QualType::DestructionKind DK, QualType FT,
                      Ts &&... Args) {
    switch (DK) {
    case QualType::DK_objc_strong_lifetime:
      return asDerived().visitARCStrong(FT, std::forward<Ts>(Args)...);
    case QualType::DK_nontrivial_c_struct:
      return asDerived().visitStruct(FT, std::forward<Ts>(Args)...);
    case QualType::DK_none:
      return asDerived().visitTrivial(FT, std::forward<Ts>(Args)...);
    case QualType::DK_cxx_destructor:
      return asDerived().visitCXXDestructor(FT, std::forward<Ts>(Args)...);
    case QualType::DK_objc_weak_lifetime:
      return asDerived().visitARCWeak(FT, std::forward<Ts>(Args)...);
    }

    toolchain_unreachable("unknown destruction kind");
  }

  Derived &asDerived() { return static_cast<Derived &>(*this); }
};

template <class Derived, class RetTy = void>
struct DefaultInitializedTypeVisitor {
  template <class... Ts> RetTy visit(QualType FT, Ts &&... Args) {
    return asDerived().visitWithKind(
        FT.isNonTrivialToPrimitiveDefaultInitialize(), FT,
        std::forward<Ts>(Args)...);
  }

  template <class... Ts>
  RetTy visitWithKind(QualType::PrimitiveDefaultInitializeKind PDIK,
                      QualType FT, Ts &&... Args) {
    switch (PDIK) {
    case QualType::PDIK_ARCStrong:
      return asDerived().visitARCStrong(FT, std::forward<Ts>(Args)...);
    case QualType::PDIK_ARCWeak:
      return asDerived().visitARCWeak(FT, std::forward<Ts>(Args)...);
    case QualType::PDIK_Struct:
      return asDerived().visitStruct(FT, std::forward<Ts>(Args)...);
    case QualType::PDIK_Trivial:
      return asDerived().visitTrivial(FT, std::forward<Ts>(Args)...);
    }

    toolchain_unreachable("unknown default-initialize kind");
  }

  Derived &asDerived() { return static_cast<Derived &>(*this); }
};

template <class Derived, bool IsMove, class RetTy = void>
struct CopiedTypeVisitor {
  template <class... Ts> RetTy visit(QualType FT, Ts &&... Args) {
    QualType::PrimitiveCopyKind PCK =
        IsMove ? FT.isNonTrivialToPrimitiveDestructiveMove()
               : FT.isNonTrivialToPrimitiveCopy();
    return asDerived().visitWithKind(PCK, FT, std::forward<Ts>(Args)...);
  }

  template <class... Ts>
  RetTy visitWithKind(QualType::PrimitiveCopyKind PCK, QualType FT,
                      Ts &&... Args) {
    asDerived().preVisit(PCK, FT, std::forward<Ts>(Args)...);

    switch (PCK) {
    case QualType::PCK_ARCStrong:
      return asDerived().visitARCStrong(FT, std::forward<Ts>(Args)...);
    case QualType::PCK_ARCWeak:
      return asDerived().visitARCWeak(FT, std::forward<Ts>(Args)...);
    case QualType::PCK_PtrAuth:
      return asDerived().visitPtrAuth(FT, std::forward<Ts>(Args)...);
    case QualType::PCK_Struct:
      return asDerived().visitStruct(FT, std::forward<Ts>(Args)...);
    case QualType::PCK_Trivial:
      return asDerived().visitTrivial(FT, std::forward<Ts>(Args)...);
    case QualType::PCK_VolatileTrivial:
      return asDerived().visitVolatileTrivial(FT, std::forward<Ts>(Args)...);
    }

    toolchain_unreachable("unknown primitive copy kind");
  }

  Derived &asDerived() { return static_cast<Derived &>(*this); }
};

} // end namespace language::Core

#endif
