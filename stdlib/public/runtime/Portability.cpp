//===--- Portability.cpp - ------------------------------------------------===//
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
// Implementations of the stub APIs that make portable runtime easier to write.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Portability.h"
#include <cstring>

size_t _swift_strlcpy(char *dst, const char *src, size_t maxlen) {
  const size_t srclen = std::strlen(src);
  if (srclen < maxlen) {
    std::memmove(dst, src, srclen + 1);
  } else if (maxlen != 0) {
    std::memmove(dst, src, maxlen - 1);
    dst[maxlen - 1] = '\0';
  }
  return srclen;
}
