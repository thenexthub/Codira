//===--- Confusables.h - Codira Confusable Character Diagnostics -*- C++ -*-===//
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

#ifndef LANGUAGE_CONFUSABLES_H
#define LANGUAGE_CONFUSABLES_H

#include "toolchain/ADT/StringRef.h"
#include <stdint.h>

namespace language {
namespace confusable {
  /// Given a UTF-8 codepoint, determines whether it appears on the Unicode
  /// specification table of confusable characters and maps to punctuation,
  /// and either returns either the expected ASCII character or 0.
  char tryConvertConfusableCharacterToASCII(uint32_t codepoint);

  /// Given a UTF-8 codepoint which is previously determined to be confusable,
  /// return the name of the confusable character and the name of the base
  /// character.
  std::pair<toolchain::StringRef, toolchain::StringRef>
  getConfusableAndBaseCodepointNames(uint32_t codepoint);
}
}

#endif
