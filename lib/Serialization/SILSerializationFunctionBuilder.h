//===--- SILSerializationFunctionBuilder.h --------------------------------===//
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

#ifndef LANGUAGE_SERIALIZATION_SERIALIZATIONFUNCTIONBUILDER_H
#define LANGUAGE_SERIALIZATION_SERIALIZATIONFUNCTIONBUILDER_H

#include "language/SIL/SILFunctionBuilder.h"

namespace language {

class TOOLCHAIN_LIBRARY_VISIBILITY SILSerializationFunctionBuilder {
  SILFunctionBuilder builder;

public:
  SILSerializationFunctionBuilder(SILModule &mod) : builder(mod) {}

  /// Create a SILFunction declaration for use either as a forward reference or
  /// for the eventual deserialization of a function body.
  SILFunction *createDeclaration(StringRef name, SILType type,
                                 SILLocation loc) {
    return builder.createFunction(
        SILLinkage::Private, name, type.getAs<SILFunctionType>(), nullptr,
        loc, IsNotBare, IsNotTransparent,
        IsNotSerialized, IsNotDynamic, IsNotDistributed, IsNotRuntimeAccessible,
        ProfileCounter(), IsNotThunk, SubclassScope::NotApplicable);
  }

  void setHasOwnership(SILFunction *f, bool newValue) {
    builder.setHasOwnership(f, newValue);
  }
};

} // namespace language

#endif
