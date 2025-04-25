//===--- SwiftTLSContext.cpp ----------------------------------------------===//
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

#include "languageTLSContext.h"

#include "language/Threading/Once.h"
#include "language/Threading/ThreadLocalStorage.h"

using namespace language;
using namespace language::runtime;

SwiftTLSContext &SwiftTLSContext::get() {

#if SWIFT_THREADING_USE_RESERVED_TLS_KEYS

  // If we have reserved keys, use those
  SwiftTLSContext *ctx =
      static_cast<SwiftTLSContext *>(swift::tls_get(swift::tls_key::runtime));
  if (ctx)
    return *ctx;

  static swift::once_t token;
  swift::tls_init_once(token, swift::tls_key::runtime, [](void *pointer) {
    swift_cxx_deleteObject(static_cast<SwiftTLSContext *>(pointer));
  });

  ctx = swift_cxx_newObject<SwiftTLSContext>();
  swift::tls_set(swift::tls_key::runtime, ctx);
  return *ctx;

#elif defined(SWIFT_THREAD_LOCAL)

  // If we have the thread local attribute, use that
  // (note that this happens for the no-threads case too)
  static SWIFT_THREAD_LOCAL SwiftTLSContext TLSContext;
  return TLSContext;

#else

  // Otherwise, allocate ourselves a key and use that
  static swift::tls_key_t runtimeKey;
  static swift::once_t token;

  swift::tls_alloc_once(token, runtimeKey, [](void *pointer) {
    swift_cxx_deleteObject(static_cast<SwiftTLSContext *>(pointer));
  });

  SwiftTLSContext *ctx =
      static_cast<SwiftTLSContext *>(swift::tls_get(runtimeKey));
  if (ctx)
    return *ctx;

  ctx = swift_cxx_newObject<SwiftTLSContext>();
  swift::tls_set(runtimeKey, ctx);
  return *ctx;

#endif
}
