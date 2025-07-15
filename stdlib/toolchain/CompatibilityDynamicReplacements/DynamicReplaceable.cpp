//===--- ProtocolConformance.cpp - Codira protocol conformance checking ----===//
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
// Runtime support for dynamic replaceable functions.
//
// This implementation is intended to be backward-deployed into Codira 5.0
// runtimes.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Exclusivity.h"
#include "language/Runtime/FunctionReplacement.h"
#include "language/Threading/ThreadLocalStorage.h"

using namespace language;

namespace {

// Some threading libraries will need a global constructor here; that is
// unavoidable in the general case, so turn off the warning just for this line.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
LANGUAGE_THREAD_LOCAL_TYPE(uintptr_t, tls_key::compatibility50) compat50Key;
#pragma clang diagnostic pop

} // namespace

__attribute__((visibility("hidden"), weak))
extern "C" char *language_getFunctionReplacement50(char **ReplFnPtr, char *CurrFn) {
  // Call the current implementation if it is available.
  if (language_getFunctionReplacement)
    return language_getFunctionReplacement(ReplFnPtr, CurrFn);

  char *ReplFn = *ReplFnPtr;
  char *RawReplFn = ReplFn;

#if LANGUAGE_PTRAUTH
  RawReplFn = ptrauth_strip(RawReplFn, ptrauth_key_function_pointer);
#endif

  if (RawReplFn == CurrFn)
    return nullptr;

  auto origKey = compat50Key.get();
  if ((origKey & 0x1) != 0) {
    auto mask = ((uintptr_t)-1) << 1;
    auto resetKey = origKey & mask;
    compat50Key.set(resetKey);
    return nullptr;
  }
  return ReplFn;
}

__attribute__((visibility("hidden"), weak))
extern "C" char *language_getOrigOfReplaceable50(char **OrigFnPtr) {
  // Call the current implementation if it is available.
  if (language_getOrigOfReplaceable)
    return language_getOrigOfReplaceable(OrigFnPtr);

  char *OrigFn = *OrigFnPtr;
  auto origKey = compat50Key.get();
  auto newKey = origKey | 0x1;
  compat50Key.set(newKey);
  return OrigFn;
}

// Allow this library to get force-loaded by autolinking
__attribute__((weak, visibility("hidden")))
extern "C"
char _language_FORCE_LOAD_$_languageCompatibilityDynamicReplacements = 0;
