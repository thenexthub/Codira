//===--- ManglingFlavor.h - Codira name mangling -----------------*- C++ -*-===//
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

#ifndef LANGUAGE_DEMANGLING_MANGLINGFLAVOR_H
#define LANGUAGE_DEMANGLING_MANGLINGFLAVOR_H

#include "language/Demangling/NamespaceMacros.h"

#include <cstdint>

namespace language {
namespace Mangle {
LANGUAGE_BEGIN_INLINE_NAMESPACE

/// Which mangling style and prefix to use.
enum class ManglingFlavor: uint8_t {
  /// Default mangling with the ABI stable $s prefix
  Default,
  /// Embedded Codira's mangling with $e prefix
  Embedded,
};

LANGUAGE_END_INLINE_NAMESPACE
} // end namespace Mangle
} // end namespace language

#endif // LANGUAGE_DEMANGLING_MANGLINGFLAVOR_H
