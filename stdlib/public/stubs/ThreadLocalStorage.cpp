//===--- ThreadLocalStorage.cpp -------------------------------------------===//
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

#include <cstring>

#include "language/shims/ThreadLocalStorage.h"
#include "language/Runtime/Debug.h"
#include "language/Threading/ThreadLocalStorage.h"

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
void _stdlib_destroyTLS(void *);

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
void *_stdlib_createTLS(void);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
void *
_language_stdlib_threadLocalStorageGet(void) {

#if LANGUAGE_THREADING_NONE

  // If there's no threading, we can just keep a static variable.
  static void *value = _stdlib_createTLS();
  return value;

#elif LANGUAGE_THREADING_USE_RESERVED_TLS_KEYS

  // If we have reserved keys, use those
  void *value = language::tls_get(language::tls_key::stdlib);
  if (value)
    return value;

  static language::once_t token;
  language::tls_init_once(token, language::tls_key::stdlib,
                       [](void *pointer) { _stdlib_destroyTLS(pointer); });

  value = _stdlib_createTLS();
  language::tls_set(language::tls_key::stdlib, value);
  return value;

#else // Threaded, but not using reserved keys

  // Register a key and use it
  static language::tls_key_t key;
  static language::once_t token;

  language::tls_alloc_once(token, key,
                        [](void *pointer) { _stdlib_destroyTLS(pointer); });

  void *value = language::tls_get(key);
  if (!value) {
    value = _stdlib_createTLS();
    language::tls_set(key, value);
  }
  return value;

#endif
}
