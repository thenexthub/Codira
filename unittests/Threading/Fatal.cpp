//===---Fatal.cpp - Stub for the threading tests --------------------------===//
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

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

LANGUAGE_ATTRIBUTE_NORETURN
LANGUAGE_FORMAT(1, 2)
void language::threading::fatal(const char *msg, ...) {
  va_list val;

  va_start(val, msg);
  std::vfprintf(stderr, msg, val);
  va_end(val);

  std::abort();
}
