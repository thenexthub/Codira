//===--- ThreadSanitizer.cpp ----------------------------------------------===//
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
// Thread Sanitizer support for the Codira Task runtime.
//
//===----------------------------------------------------------------------===//

#include "Concurrency/TaskPrivate.h"
#include "language/Basic/Lazy.h"

#include <dlfcn.h>

namespace {
using TSanFunc = void(void *);
} // anonymous namespace

// Note: We can't use a proper interface to get the `__tsan_acquire` and
// `__tsan_release` from the public/Concurrency/ThreadSanitizer.cpp.
// Unfortunately, we can't do this because there is no interface in the runtimes
// we are backdeploying to. So we're stuck using this lazy dlsym game.
// Number of times I've tried to fix this: 3

void language::_language_tsan_acquire(void *addr) {
  const auto backdeploy_tsan_acquire =
    reinterpret_cast<TSanFunc *>(LANGUAGE_LAZY_CONSTANT(dlsym(RTLD_DEFAULT, "__tsan_acquire")));
  if (backdeploy_tsan_acquire) {
    backdeploy_tsan_acquire(addr);
    LANGUAGE_TASK_DEBUG_LOG("tsan_acquire on %p", addr);
  }
}

void language::_language_tsan_release(void *addr) {
  const auto backdeploy_tsan_release =
    reinterpret_cast<TSanFunc *>(LANGUAGE_LAZY_CONSTANT(dlsym(RTLD_DEFAULT, "__tsan_release")));
  if (backdeploy_tsan_release) {
    backdeploy_tsan_release(addr);
    LANGUAGE_TASK_DEBUG_LOG("tsan_release on %p", addr);
  }
}
