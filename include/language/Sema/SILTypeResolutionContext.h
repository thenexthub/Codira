//===--- SILTypeResolutionContext.h -----------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_SEMA_SILTYPERESOLUTIONCONTEXT_H
#define LANGUAGE_SEMA_SILTYPERESOLUTIONCONTEXT_H

#include "toolchain/ADT/DenseMap.h"
#include "language/Basic/UUID.h"

namespace language {
class GenericParamList;
class GenericEnvironment;

class SILTypeResolutionContext {
public:
  struct OpenedPackElement {
    SourceLoc DefinitionPoint;
    GenericParamList *Params;
    GenericEnvironment *Environment;
  };
  using OpenedPackElementsMap = toolchain::DenseMap<UUID, OpenedPackElement>;

  /// Are we requesting a SIL type?
  bool IsSILType;

  /// Look up types in the given parameter list.
  GenericParamList *GenericParams;

  /// Look up @pack_element environments in this map.
  OpenedPackElementsMap *OpenedPackElements;

  SILTypeResolutionContext(bool isSILType,
                           GenericParamList *genericParams,
                           OpenedPackElementsMap *openedPackElements)
    : IsSILType(isSILType),
      GenericParams(genericParams),
      OpenedPackElements(openedPackElements) {}
};

}

#endif
