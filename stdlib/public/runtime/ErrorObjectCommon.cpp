//===--- ErrorObjectCommon.cpp - Recoverable error object -----------------===//
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
// This implements the parts of the standard Error protocol type which are
// shared between the ObjC-interoperable implementation and the native
// implementation. The parts specific to each implementation can be found in
// ErrorObject.mm (for the ObjC-interoperable parts) and ErrorObjectNative.cpp.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Concurrent.h"
#include "language/Runtime/Config.h"
#include "ErrorObject.h"
#include "ErrorObjectTestSupport.h"

using namespace language;

std::atomic<void (*)(CodiraError *error)> language::_language_willThrow;

void language::_language_setWillThrowHandler(void (* handler)(CodiraError *error)) {
  _language_willThrow.store(handler, std::memory_order_release);
}

/// Breakpoint hook for debuggers that is called for untyped throws, and
/// calls _language_willThrow if set.
LANGUAGE_CC(language) void
language::language_willThrow(LANGUAGE_CONTEXT void *unused,
                       LANGUAGE_ERROR_RESULT CodiraError **error) {
  // Cheap check to bail out early, since we expect there to be no callbacks
  // the vast majority of the time.
  auto handler = _language_willThrow.load(std::memory_order_acquire);
  if (LANGUAGE_UNLIKELY(handler)) {
    (* handler)(*error);
  }
}

std::atomic<void (*)(
  OpaqueValue *value,
  const Metadata *type,
  const WitnessTable *errorConformance
)> language::_language_willThrowTypedImpl;

/// Breakpoint hook for debuggers that is called for typed throws, and calls
/// _language_willThrowTypedImpl if set. If not set and _language_willThrow is set, this calls
/// that hook instead and implicitly boxes the typed error in an any Error for that call.
LANGUAGE_CC(language) void
language::language_willThrowTypedImpl(OpaqueValue *value,
                                const Metadata *type,
                                const WitnessTable *errorConformance) {
  // Cheap check to bail out early, since we expect there to be no callbacks
  // the vast majority of the time.
  auto handler = _language_willThrowTypedImpl.load(std::memory_order_acquire);
  if (LANGUAGE_UNLIKELY(handler)) {
    (* handler)(value, type, errorConformance);
  } else {
    auto fallbackHandler = _language_willThrow.load(std::memory_order_acquire);
    if (LANGUAGE_UNLIKELY(fallbackHandler)) {
      // Form an error box containing the error.
      BoxPair boxedError = language_allocError(
        type, errorConformance, value, /*isTake=*/false);

      // Hand the boxed error off to the handler.
      auto errorBox = reinterpret_cast<CodiraError *>(boxedError.object);
      (* fallbackHandler)(errorBox);

      // Release the error box.
      language_errorRelease(errorBox);
    }
  }
}
