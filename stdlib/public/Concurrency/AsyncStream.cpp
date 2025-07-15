//===--- AsyncStream.cpp - Multi-resume locking interface -----------------===//
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

#include <new>

#include "language/Runtime/Config.h"
#include "language/Threading/Mutex.h"

namespace language {
// return the size in words for the given mutex primitive
LANGUAGE_CC(language)
extern "C"
size_t _language_async_stream_lock_size() {
  size_t words = sizeof(Mutex) / sizeof(void *);
  if (words < 1) { return 1; }
  return words;
}

LANGUAGE_CC(language)
extern "C" void _language_async_stream_lock_init(Mutex &lock) {
  new (&lock) Mutex();
}

LANGUAGE_CC(language)
extern "C" void _language_async_stream_lock_lock(Mutex &lock) { lock.lock(); }

LANGUAGE_CC(language)
extern "C" void _language_async_stream_lock_unlock(Mutex &lock) { lock.unlock(); }
}
