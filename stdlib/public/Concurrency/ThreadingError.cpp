//===--- ThreadingError.cpp - Error handling support code -----------------===//
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

#include "language/Threading/Errors.h"
#include <cstdio>

#include "Error.h"

// Handle fatal errors from the threading library
LANGUAGE_ATTRIBUTE_NORETURN
LANGUAGE_FORMAT(1, 2)
void language::threading::fatal(const char *format, ...) {
  va_list val;

  va_start(val, format);
  language_Concurrency_fatalErrorv(0, format, val);
}
