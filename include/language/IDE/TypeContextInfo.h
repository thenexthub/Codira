//===--- TypeContextInfo.h --------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_IDE_TYPECONTEXTINFO_H
#define LANGUAGE_IDE_TYPECONTEXTINFO_H

#include "language/AST/Type.h"
#include "language/Basic/Toolchain.h"

namespace language {
class IDEInspectionCallbacksFactory;

namespace ide {

/// A result item for context info query.
class TypeContextInfoItem {
public:
  /// Possible expected type.
  Type ExpectedTy;
  /// Members of \c ExpectedTy which can be referenced by "Implicit Member
  /// Expression".
  SmallVector<ValueDecl *, 0> ImplicitMembers;

  TypeContextInfoItem(Type ExpectedTy) : ExpectedTy(ExpectedTy) {}
};

/// An abstract base class for consumers of context info results.
class TypeContextInfoConsumer {
public:
  virtual ~TypeContextInfoConsumer() {}
  virtual void handleResults(ArrayRef<TypeContextInfoItem>) = 0;
};

/// Create a factory for code completion callbacks.
IDEInspectionCallbacksFactory *
makeTypeContextInfoCallbacksFactory(TypeContextInfoConsumer &Consumer);

} // namespace ide
} // namespace language

#endif // LANGUAGE_IDE_TYPECONTEXTINFO_H
