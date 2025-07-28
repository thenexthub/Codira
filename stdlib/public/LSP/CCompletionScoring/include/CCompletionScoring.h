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

#ifndef SOURCEKITLSP_CCOMPLETIONSCORING_H
#define SOURCEKITLSP_CCOMPLETIONSCORING_H

#define _GNU_SOURCE
#include <string.h>

static inline void *sourcekitlsp_memmem(const void *haystack, size_t haystack_len, const void *needle, size_t needle_len) {
  #if defined(_WIN32) && !defined(__CYGWIN__)
  // memmem is not available on Windows
  if (!haystack || haystack_len == 0) {
    return NULL;
  }
  if (!needle || needle_len == 0) {
    return NULL;
  }
  if (needle_len > haystack_len) {
    return NULL;
  }

  for (size_t offset = 0; offset <= haystack_len - needle_len; ++offset) {
    if (memcmp(haystack + offset, needle, needle_len) == 0) {
      return (void *)haystack + offset;
    }
  }
  return NULL;
  #else
  return memmem(haystack, haystack_len, needle, needle_len);
  #endif
}

#endif // SOURCEKITLSP_CCOMPLETIONSCORING_H
