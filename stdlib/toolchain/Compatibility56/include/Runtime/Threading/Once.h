//===--- Once.h - Runtime support for lazy initialization -------*- C++ -*-===//
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
// Swift runtime functions in support of lazy initialization.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_RUNTIME_ONCE_BACKDEPLOY56_H
#define SWIFT_RUNTIME_ONCE_BACKDEPLOY56_H

#include "language/Runtime/HeapObject.h"
#include <mutex>

namespace language {

#ifdef SWIFT_STDLIB_SINGLE_THREADED_RUNTIME

typedef bool swift_once_t;

#elif defined(__APPLE__)

// On OS X and iOS, swift_once_t matches dispatch_once_t.
typedef long swift_once_t;

#elif defined(__CYGWIN__)

// On Cygwin, std::once_flag can not be used because it is larger than the
// platform word.
typedef uintptr_t swift_once_t;
#else

// On other platforms swift_once_t is std::once_flag
typedef std::once_flag swift_once_t;

#endif

/// Runs the given function with the given context argument exactly once.
/// The predicate argument must point to a global or static variable of static
/// extent of type swift_once_t.
void swift_once(swift_once_t *predicate, void (*fn)(void *), void *context);

}

#endif // SWIFT_RUNTIME_ONCE_BACKDEPLOY56_H
