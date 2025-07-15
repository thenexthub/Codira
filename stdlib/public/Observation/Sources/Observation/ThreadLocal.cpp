//===----------------------------------------------------------------------===//
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

#include "language/Runtime/Config.h"
#include "language/Runtime/Once.h"
#include "language/Threading/ThreadLocalStorage.h"

static LANGUAGE_THREAD_LOCAL_TYPE(void *, language::tls_key::observation_transaction) Value;

extern "C" LANGUAGE_CC(language) __attribute__((visibility("hidden")))
void *_language_observation_tls_get() {
  return Value.get();
}

extern "C" LANGUAGE_CC(language) __attribute__((visibility("hidden")))
void _language_observation_tls_set(void *value) {
  Value.set(value);
}
