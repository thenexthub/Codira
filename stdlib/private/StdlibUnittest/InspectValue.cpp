//===----------------------------------------------------------------------===//
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

#include "language/Runtime/Config.h"
#include "language/Runtime/Metadata.h"

using namespace language;

LANGUAGE_CC(language)
LANGUAGE_LIBRARY_VISIBILITY extern "C" const
    char *getMetadataKindOf(OpaqueValue *value, const Metadata *type) {
  switch (type->getKind()) {
#define METADATAKIND(NAME, VALUE) \
  case MetadataKind::NAME: return #NAME;
#include "language/ABI/MetadataKind.def"

  default: return "none of your business";
  }
}
