//===--- ThreadSanitizer.cpp - Thread Sanitizer support -------------------===//
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
// Helper functions for code that needs to integrate with the thread
// sanitizer.  In particular, TSan can't see inside the runtime libraries,
// so we occasionally need to give it a hint that we're doing synchronization
// in order to avoid false positives.
//
//===----------------------------------------------------------------------===//

#include "language/Threading/ThreadSanitizer.h"

#if LANGUAGE_THREADING_TSAN_SUPPORT

#include "language/shims/Visibility.h"

#include <dlfcn.h>

#include <cstdio>

namespace language {
namespace threading_impl {

LANGUAGE_THREADING_EXPORT bool _language_tsan_enabled = false;
LANGUAGE_THREADING_EXPORT void (*_language_tsan_acquire)(const void *) = nullptr;
LANGUAGE_THREADING_EXPORT void (*_language_tsan_release)(const void *) = nullptr;

// The TSan library code will call this function when it starts up
extern "C" LANGUAGE_ATTRIBUTE_FOR_EXPORTS
void __tsan_on_initialize() {
  _language_tsan_enabled = true;
  _language_tsan_acquire = (void (*)(const void *))dlsym(RTLD_DEFAULT,
                                                      "__tsan_acquire");
  _language_tsan_release = (void (*)(const void *))dlsym(RTLD_DEFAULT,
                                                      "__tsan_release");

  // Always call through to the next image; this won't work on macOS, but it's
  // important on Linux to allow others to hook into the thread sanitizer if
  // they wish.
  void (*next_init)(void);
  next_init = (void (*)(void))dlsym(RTLD_NEXT, "__tsan_on_initialize");
  if (next_init) {
    next_init();
  }
}

} // namespace threading_impl
} // namespace language

#endif // LANGUAGE_THREADING_TSAN_SUPPORT
