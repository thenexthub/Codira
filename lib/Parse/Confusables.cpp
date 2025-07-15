//===--- Confusables.cpp - Codira Confusable Character Diagnostics ---------===//
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

#include "language/Parse/Confusables.h"

char language::confusable::tryConvertConfusableCharacterToASCII(uint32_t codepoint) {
  switch (codepoint) {
#define CONFUSABLE(CONFUSABLE_POINT, CONFUSABLE_NAME, BASE_POINT, BASE_NAME)   \
  case CONFUSABLE_POINT:                                                       \
    return BASE_POINT;
#include "language/Parse/Confusables.def"
  default: return 0;
  }
}

std::pair<toolchain::StringRef, toolchain::StringRef>
language::confusable::getConfusableAndBaseCodepointNames(uint32_t codepoint) {
  switch (codepoint) {
#define CONFUSABLE(CONFUSABLE_POINT, CONFUSABLE_NAME, BASE_POINT, BASE_NAME)   \
  case CONFUSABLE_POINT:                                                       \
    return std::make_pair(CONFUSABLE_NAME, BASE_NAME);
#include "language/Parse/Confusables.def"
  default:
    return std::make_pair("", "");
  }
}
