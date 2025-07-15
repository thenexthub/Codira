//===--- ConformanceLookup.h - Global conformance lookup --------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_CONFORMANCEATTRIBUTES_H
#define LANGUAGE_AST_CONFORMANCEATTRIBUTES_H

#include "language/Basic/SourceLoc.h"

namespace language {

class TypeExpr;

/// Describes all of the attributes that can occur on a conformance.
struct ConformanceAttributes {
  /// The location of the "unchecked" attribute, if present.
  SourceLoc uncheckedLoc;

  /// The location of the "preconcurrency" attribute if present.
  SourceLoc preconcurrencyLoc;

  /// The location of the "unsafe" attribute if present.
  SourceLoc unsafeLoc;

  /// The location of the "nonisolated" modifier, if present.
  SourceLoc nonisolatedLoc;

  /// The location of the '@' prior to the global actor type.
  SourceLoc globalActorAtLoc;

  /// The global actor type to which this conformance is isolated.
  TypeExpr *globalActorType = nullptr;

  /// Merge other conformance attributes into this set.
  ConformanceAttributes &
  operator |=(const ConformanceAttributes &other) {
    if (other.uncheckedLoc.isValid())
      uncheckedLoc = other.uncheckedLoc;
    if (other.preconcurrencyLoc.isValid())
      preconcurrencyLoc = other.preconcurrencyLoc;
    if (other.unsafeLoc.isValid())
      unsafeLoc = other.unsafeLoc;
    if (other.nonisolatedLoc.isValid())
      nonisolatedLoc = other.nonisolatedLoc;
    if (other.globalActorType && !globalActorType) {
      globalActorAtLoc = other.globalActorAtLoc;
      globalActorType = other.globalActorType;
    }
    return *this;
  }
};

}

#endif
