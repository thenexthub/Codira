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

#ifndef SWIFT_RUNTIME_ONCE_H
#define SWIFT_RUNTIME_ONCE_H

#include "language/Threading/Once.h"

namespace language {

typedef swift::once_t swift_once_t;

/// Runs the given function with the given context argument exactly once.
/// The predicate argument must point to a global or static variable of static
/// extent of type swift_once_t.
///
/// Within the runtime, you should be using swift::once, which will be faster;
/// this is exposed so that the compiler can generate calls to it.
SWIFT_RUNTIME_EXPORT
void swift_once(swift_once_t *predicate, void (*fn)(void *), void *context);

}

#endif
