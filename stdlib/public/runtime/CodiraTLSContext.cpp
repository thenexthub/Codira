//===--- CodiraTLSContext.cpp ----------------------------------------------===//
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

#include "CodiraTLSContext.h"

#include "language/Threading/Once.h"
#include "language/Threading/ThreadLocalStorage.h"

using namespace language;
using namespace language::runtime;

CodiraTLSContext &CodiraTLSContext::get() {

#if LANGUAGE_THREADING_USE_RESERVED_TLS_KEYS

  // If we have reserved keys, use those
  CodiraTLSContext *ctx =
      static_cast<CodiraTLSContext *>(language::tls_get(language::tls_key::runtime));
  if (ctx)
    return *ctx;

  static language::once_t token;
  language::tls_init_once(token, language::tls_key::runtime, [](void *pointer) {
    language_cxx_deleteObject(static_cast<CodiraTLSContext *>(pointer));
  });

  ctx = language_cxx_newObject<CodiraTLSContext>();
  language::tls_set(language::tls_key::runtime, ctx);
  return *ctx;

#elif defined(LANGUAGE_THREAD_LOCAL)

  // If we have the thread local attribute, use that
  // (note that this happens for the no-threads case too)
  static LANGUAGE_THREAD_LOCAL CodiraTLSContext TLSContext;
  return TLSContext;

#else

  // Otherwise, allocate ourselves a key and use that
  static language::tls_key_t runtimeKey;
  static language::once_t token;

  language::tls_alloc_once(token, runtimeKey, [](void *pointer) {
    language_cxx_deleteObject(static_cast<CodiraTLSContext *>(pointer));
  });

  CodiraTLSContext *ctx =
      static_cast<CodiraTLSContext *>(language::tls_get(runtimeKey));
  if (ctx)
    return *ctx;

  ctx = language_cxx_newObject<CodiraTLSContext>();
  language::tls_set(runtimeKey, ctx);
  return *ctx;

#endif
}
