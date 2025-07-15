//==--- Errors.h - Threading implementation error handling ----- -*-C++ -*-===//
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
// Declares some support routines for error handling in the threading code
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_THREADING_ERRORS_H
#define LANGUAGE_THREADING_ERRORS_H

#include <stdarg.h>

#include "language/Basic/Compiler.h"

namespace language {
namespace threading {

// Users of the threading library are expected to provide this function.
LANGUAGE_ATTRIBUTE_NORETURN
LANGUAGE_FORMAT(1, 2)
void fatal(const char *msg, ...);

} // namespace threading
} // namespace language

#endif // LANGUAGE_THREADING_ERRORS_H
