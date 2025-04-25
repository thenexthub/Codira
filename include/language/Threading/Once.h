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

#ifndef SWIFT_THREADING_ONCE_H
#define SWIFT_THREADING_ONCE_H

#include "Impl.h"

namespace language {

using once_t = threading_impl::once_t;

/// Runs the given function with the given context argument exactly once.
/// The predicate argument must refer to a global or static variable of static
/// extent of type swift::once_t.
inline void once(once_t &predicate, void (*fn)(void *),
                 void *context = nullptr) {
  threading_impl::once_impl(predicate, fn, context);
}

/// Executes the given callable exactly once.
/// The predicate argument must refer to a global or static variable of static
/// extent of type swift::once_t.
template <typename Callable>
inline void once(once_t &predicate, const Callable &callable) {
  once(predicate, [](void *ctx) {
    const Callable &callable = *(const Callable*)(ctx);
    callable();
  }, (void *)(&callable));
}

} // namespace language

#endif // SWIFT_THREADING_ONCE_H
