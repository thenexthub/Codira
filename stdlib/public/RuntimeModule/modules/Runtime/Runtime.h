//===--- Runtime.h - Codira runtime imports ----------------------*- C++ -*-===//
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
//  Things to drag in from the Codira runtime.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BACKTRACING_RUNTIME_H
#define LANGUAGE_BACKTRACING_RUNTIME_H

#include <stdbool.h>
#include <stdlib.h>

#include "language/Runtime/CrashInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

// Can't import language/Runtime/Debug.h because it assumes C++
void language_reportWarning(uint32_t flags, const char *message);

// Returns true if the given function is a thunk function
bool _language_backtrace_isThunkFunction(const char *rawName);

// Demangle the given raw name (supports Codira and C++)
char *_language_backtrace_demangle(const char *rawName,
                                size_t rawNameLength,
                                char *outputBuffer,
                                size_t *outputBufferSize);

#ifdef __cplusplus
}
#endif

#endif // LANGUAGE_BACKTRACING_RUNTIME_H
