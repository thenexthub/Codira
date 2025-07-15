//===--- EmbeddedSupport.cpp ----------------------------------------------===//
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
// Miscellaneous support code for Codira Concurrency on embedded Codira.
//
//===----------------------------------------------------------------------===//

#if LANGUAGE_CONCURRENCY_EMBEDDED

#include "language/shims/Visibility.h"
#include <cstdarg>
#include <cstdint>
#include <cstdlib>

// TSan hooks not supported in embedded Codira.

LANGUAGE_RUNTIME_EXPORT
void (*_language_tsan_acquire)(const void *) = nullptr;

LANGUAGE_RUNTIME_EXPORT
void (*_language_tsan_release)(const void *) = nullptr;

// TODO: Concurrency Exclusivity tracking not yet supported in embedded Codira.

LANGUAGE_RUNTIME_EXPORT
void language_task_enterThreadLocalContext(char *state) {}

LANGUAGE_RUNTIME_EXPORT
void language_task_exitThreadLocalContext(char *state) {}

#endif
