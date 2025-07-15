//===--- SILParserFunctionBuilder.h ---------------------------------------===//
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

#ifndef LANGUAGE_PARSESIL_SILPARSERFUNCTIONBUILDER_H
#define LANGUAGE_PARSESIL_SILPARSERFUNCTIONBUILDER_H

#include "language/SIL/SILFunctionBuilder.h"

namespace language {

class TOOLCHAIN_LIBRARY_VISIBILITY SILParserFunctionBuilder {
  SILFunctionBuilder builder;

public:
  SILParserFunctionBuilder(SILModule &mod) : builder(mod) {}

  SILFunction *createFunctionForForwardReference(StringRef name,
                                                 CanSILFunctionType ty,
                                                 SILLocation loc) {
    auto *result = builder.createFunction(
        SILLinkage::Private, name, ty, nullptr, loc, IsNotBare,
        IsNotTransparent, IsNotSerialized, IsNotDynamic, IsNotDistributed,
        IsNotRuntimeAccessible);
    result->setDebugScope(new (builder.mod) SILDebugScope(loc, result));

    // If we did not have a declcontext set, as a fallback set the parent module
    // of our SILFunction to the parent module of our SILModule.
    //
    // DISCUSSION: This ensures that we can perform protocol conformance checks.
    if (!result->getDeclContext()) {
      result->setParentModule(result->getModule().getCodiraModule());
    }

    return result;
  }
};

} // namespace language

#endif
