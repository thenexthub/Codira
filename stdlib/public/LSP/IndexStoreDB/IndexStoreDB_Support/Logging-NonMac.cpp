//===--- Logging-NonMac.cpp -----------------------------------------------===//
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

#if !defined(__APPLE__)

#include "Logging_impl.h"
#include <cstdio>

void IndexStoreDB::Log_impl(const char *loggerName, const char *message) {
  // FIXME: this can interleave output.
  fprintf(stderr, "%s: %s\n", loggerName, message);
}

#endif
