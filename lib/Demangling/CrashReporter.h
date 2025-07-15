//===--- CrashReporter.h - Crash Reporter integration -----------*- C++ -*-===//
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
// Declares gCRAnnotations.  This lets us link with other static libraries
// that also declare gCRAnnotations, because we'll pull in their copy
// (assuming they're linked first).
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_DEMANGLING_CRASHREPORTER_H
#define LANGUAGE_DEMANGLING_CRASHREPORTER_H

#if LANGUAGE_HAVE_CRASHREPORTERCLIENT

// For LANGUAGE_LIBRARY_VISIBILITY
#include "language/shims/Visibility.h"

#include <inttypes.h>

#define CRASHREPORTER_ANNOTATIONS_VERSION 5
#define CRASHREPORTER_ANNOTATIONS_SECTION "__crash_info"

struct crashreporter_annotations_t {
  uint64_t version;          // unsigned long
  uint64_t message;          // char *
  uint64_t signature_string; // char *
  uint64_t backtrace;        // char *
  uint64_t message2;         // char *
  uint64_t thread;           // uint64_t
  uint64_t dialog_mode;      // unsigned int
  uint64_t abort_cause;      // unsigned int
};

extern "C" {
LANGUAGE_LIBRARY_VISIBILITY
extern struct crashreporter_annotations_t gCRAnnotations;
}

#endif // LANGUAGE_HAVE_CRASHREPORTERCLIENT

#endif // LANGUAGE_DEMANGLING_CRASHREPORTER_H
