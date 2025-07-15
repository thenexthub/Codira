//===--- ContextualTypeInfo.h - Contextual Type Info ------------*- C++ -*-===//
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
// This file provides the \c ContextualTypeInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SEMA_CONTEXTUAL_TYPE_INFO_H
#define LANGUAGE_SEMA_CONTEXTUAL_TYPE_INFO_H

#include "language/AST/TypeLoc.h"
#include "language/Sema/ConstraintLocator.h"

namespace language {
namespace constraints {

/// Describes contextual type information about a particular element
/// (expression, statement etc.) within a constraint system.
struct ContextualTypeInfo {
  TypeLoc typeLoc;
  ContextualTypePurpose purpose;

  /// The locator for the contextual type conversion constraint, or
  /// \c nullptr to use the default locator which is anchored directly on
  /// the expression.
  ConstraintLocator *locator;

  ContextualTypeInfo()
      : typeLoc(TypeLoc()), purpose(CTP_Unused), locator(nullptr) {}

  ContextualTypeInfo(Type contextualTy, ContextualTypePurpose purpose,
                     ConstraintLocator *locator = nullptr)
      : typeLoc(TypeLoc::withoutLoc(contextualTy)), purpose(purpose),
        locator(locator) {}

  ContextualTypeInfo(TypeLoc typeLoc, ContextualTypePurpose purpose,
                     ConstraintLocator *locator = nullptr)
      : typeLoc(typeLoc), purpose(purpose), locator(locator) {}

  Type getType() const { return typeLoc.getType(); }
};

} // end namespace constraints
} // end namespace language

#endif // LANGUAGE_SEMA_CONTEXTUAL_TYPE_INFO_H
