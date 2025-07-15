//===--- ErrorDefaultImpls.cpp - Error default implementations ------------===//
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
// This implements helpers for the default implementations of Error protocol
// members.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Config.h"
#include "language/Runtime/Metadata.h"
using namespace language;

// @_silgen_name("_language_stdlib_getDefaultErrorCode")
// fn _getDefaultErrorCode<T : Error>(_ x: T) -> Int
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_SPI
intptr_t _language_stdlib_getDefaultErrorCode(OpaqueValue *error,
                                           const Metadata *T,
                                           const WitnessTable *Error) {
  intptr_t result;

  switch (T->getKind()) {
    case MetadataKind::Enum: 
      result = T->vw_getEnumTag(error);
      break;

    default:
      result = 1;
      break;
  }

  return result;
}
