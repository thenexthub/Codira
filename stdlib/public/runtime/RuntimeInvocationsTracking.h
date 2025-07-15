//===--- RuntimeInvocationsTracking.h ---------------------------*- C++ -*-===//
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
// Track invocations of Codira runtime functions. This can be used for
// performance analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_RUNTIME_INVOCATIONS_TRACKING_H
#define LANGUAGE_STDLIB_RUNTIME_INVOCATIONS_TRACKING_H

#include "language/Runtime/Config.h"

/// This API is only enabled if this define is set.
#if defined(LANGUAGE_ENABLE_RUNTIME_FUNCTION_COUNTERS)

#if defined(__cplusplus)

namespace language {
struct HeapObject;
struct RuntimeFunctionCountersState;
} // end namespace language

using namespace language;
#else

struct HeapObject;
struct RuntimeFunctionCountersState;

#endif

/// The name of a helper function for tracking the calls of a runtime function.
#define LANGUAGE_RT_TRACK_INVOCATION_NAME(RT_FUNCTION)                            \
  language_trackRuntimeInvocation_##RT_FUNCTION

/// Invoke a helper function for tracking the calls of a runtime function.
#define LANGUAGE_RT_TRACK_INVOCATION(OBJ, RT_FUNCTION)                            \
  LANGUAGE_RT_TRACK_INVOCATION_NAME(RT_FUNCTION)(OBJ)

#define FUNCTION_TO_TRACK(RT_FUNCTION)                                         \
  extern void LANGUAGE_RT_TRACK_INVOCATION_NAME(RT_FUNCTION)(HeapObject * OBJ);
/// Declarations of external functions for invocations tracking.
#include "RuntimeInvocationsTracking.def"

/// This type defines a callback to be called on any intercepted runtime
/// function.
using RuntimeFunctionCountersUpdateHandler =
  __attribute__((languagecall))
  void (*)(HeapObject *object, int64_t runtimeFunctionID);

/// Public APIs

/// Get the runtime object state associated with an object and store it
/// into the result.
LANGUAGE_RUNTIME_EXPORT void
_language_getObjectRuntimeFunctionCounters(HeapObject *object,
                                        RuntimeFunctionCountersState *result);

/// Get the global runtime state containing the total numbers of invocations for
/// each runtime function of interest and store it into the result.
LANGUAGE_RUNTIME_EXPORT void _language_getGlobalRuntimeFunctionCounters(
    language::RuntimeFunctionCountersState *result);

/// Return the names of the runtime functions being tracked.
/// Their order is the same as the order of the counters in the
/// RuntimeObjectState structure.
LANGUAGE_RUNTIME_EXPORT const char **_language_getRuntimeFunctionNames();

/// Return the offsets of the runtime function counters being tracked.
/// Their order is the same as the order of the counters in the
/// RuntimeFunctionCountersState structure.
LANGUAGE_RUNTIME_EXPORT const uint16_t *_language_getRuntimeFunctionCountersOffsets();

/// Return the number of runtime functions being tracked.
LANGUAGE_RUNTIME_EXPORT uint64_t _language_getNumRuntimeFunctionCounters();

/// Dump all per-object runtime function pointers.
LANGUAGE_RUNTIME_EXPORT void _language_dumpObjectsRuntimeFunctionPointers();

/// Set mode for global runtime function counters.
/// Return the old value of this flag.
LANGUAGE_RUNTIME_EXPORT int
_language_setPerObjectRuntimeFunctionCountersMode(int mode);

/// Set mode for per object runtime function counters.
/// Return the old value of this flag.
LANGUAGE_RUNTIME_EXPORT int _language_setGlobalRuntimeFunctionCountersMode(int mode);

/// Set the global runtime state of function pointers from a provided state.
LANGUAGE_RUNTIME_EXPORT void _language_setGlobalRuntimeFunctionCounters(
    language::RuntimeFunctionCountersState *state);

/// Set the runtime object state associated with an object from a provided
/// state.
LANGUAGE_RUNTIME_EXPORT void
_language_setObjectRuntimeFunctionCounters(HeapObject *object,
                                        RuntimeFunctionCountersState *state);

/// Set the global runtime function counters update handler.
LANGUAGE_RUNTIME_EXPORT RuntimeFunctionCountersUpdateHandler
_language_setGlobalRuntimeFunctionCountersUpdateHandler(
    RuntimeFunctionCountersUpdateHandler handler);

#else

/// Let runtime functions unconditionally use this define by making it a NOP if
/// counters are not enabled.
#define LANGUAGE_RT_TRACK_INVOCATION(OBJ, RT_FUNCTION)

#endif // LANGUAGE_ENABLE_RUNTIME_FUNCTION_COUNTERS

#endif
