//===--- RuntimeInvocationsTracking.cpp - Track runtime invocations -------===//
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
// Track invocations of Codira runtime functions. This can be used for performance
// analysis.
//
//===----------------------------------------------------------------------===//

#include <cstdint>

#include "RuntimeInvocationsTracking.h"
#include "language/Basic/Lazy.h"
#include "language/Runtime/HeapObject.h"
#include "language/Threading/Mutex.h"
#include "toolchain/ADT/DenseMap.h"

#if defined(LANGUAGE_ENABLE_RUNTIME_FUNCTION_COUNTERS)

#define LANGUAGE_RT_FUNCTION_INVOCATION_COUNTER_NAME(RT_FUNCTION)                 \
  invocationCounter_##RT_FUNCTION

namespace language {

// Define counters used for tracking the total number of invocations of runtime
// functions.
struct RuntimeFunctionCountersState {
#define FUNCTION_TO_TRACK(RT_FUNCTION)                                         \
  std::uint32_t LANGUAGE_RT_FUNCTION_INVOCATION_COUNTER_NAME(RT_FUNCTION) = 0;
// Provide one counter per runtime function being tracked.
#include "RuntimeInvocationsTracking.def"
};

} // end namespace language


/// If set, global runtime function counters should be tracked.
static bool UpdatePerObjectRuntimeFunctionCounters = false;
/// If set, per object runtime function counters should be tracked.
static bool UpdateGlobalRuntimeFunctionCounters = false;
/// TODO: Add support for enabling/disabling counters on a per object basis?

/// Global set of counters tracking the total number of runtime invocations.
struct RuntimeFunctionCountersStateSentinel {
  RuntimeFunctionCountersState State;
  LazyMutex Lock;
};
static RuntimeFunctionCountersStateSentinel RuntimeGlobalFunctionCountersState;

/// The object state cache mapping objects to the collected state associated with
/// them.
struct RuntimeObjectCacheSentinel {
  toolchain::DenseMap<HeapObject *, RuntimeFunctionCountersState> Cache;
  Mutex Lock;
};
static Lazy<RuntimeObjectCacheSentinel> RuntimeObjectStateCache;

static const char *RuntimeFunctionNames[] {
/// Define names of runtime functions.
#define FUNCTION_TO_TRACK(RT_FUNCTION) #RT_FUNCTION,
#include "RuntimeInvocationsTracking.def"
  nullptr
};

#define RT_FUNCTION_ID(RT_FUNCTION) ID_##RT_FUNCTION

/// Define an enum where each enumerator corresponds to a runtime function being
/// tracked. Their order is the same as the order of the counters in the
/// RuntimeObjectState structure.
enum RuntimeFunctionNamesIDs : std::uint32_t {
/// Defines names of enum cases for each function being tracked.
#define FUNCTION_TO_TRACK(RT_FUNCTION) RT_FUNCTION_ID(RT_FUNCTION),
#include "RuntimeInvocationsTracking.def"
  ID_LastRuntimeFunctionName,
};

/// The global handler to be invoked on runtime function counters updates.
static RuntimeFunctionCountersUpdateHandler
    GlobalRuntimeFunctionCountersUpdateHandler;

/// The offsets of the runtime function counters being tracked inside the
/// RuntimeObjectState structure. The array is indexed by
/// the enumerators from RuntimeFunctionNamesIDs.
static std::uint16_t RuntimeFunctionCountersOffsets[] = {
/// Define offset for each function being tracked.
#define FUNCTION_TO_TRACK(RT_FUNCTION)                                         \
  (sizeof(std::uint16_t) * (unsigned)RT_FUNCTION_ID(RT_FUNCTION)),
#include "RuntimeInvocationsTracking.def"
};

/// Define implementations of tracking functions.
/// TODO: Track only objects that were registered for tracking?
/// TODO: Perform atomic increments?
#define FUNCTION_TO_TRACK(RT_FUNCTION)                                         \
  void LANGUAGE_RT_TRACK_INVOCATION_NAME(RT_FUNCTION)(HeapObject * object) {      \
    /* Update global counters. */                                              \
    if (UpdateGlobalRuntimeFunctionCounters) {                                 \
      LazyMutex::ScopedLock lock(RuntimeGlobalFunctionCountersState.Lock);     \
      RuntimeGlobalFunctionCountersState.State                                 \
          .code_RT_FUNCTION_INVOCATION_COUNTER_NAME(RT_FUNCTION)++;           \
      if (GlobalRuntimeFunctionCountersUpdateHandler) {                        \
        auto oldGlobalMode = _language_setGlobalRuntimeFunctionCountersMode(0);   \
        auto oldPerObjectMode =                                                \
            _language_setPerObjectRuntimeFunctionCountersMode(0);                 \
        GlobalRuntimeFunctionCountersUpdateHandler(                            \
            object, RT_FUNCTION_ID(RT_FUNCTION));                              \
        _language_setGlobalRuntimeFunctionCountersMode(oldGlobalMode);            \
        _language_setPerObjectRuntimeFunctionCountersMode(oldPerObjectMode);      \
      }                                                                        \
    }                                                                          \
    /* Update per object counters. */                                          \
    if (UpdatePerObjectRuntimeFunctionCounters && object) {                    \
      auto &theSentinel = RuntimeObjectStateCache.get();                       \
      Mutex::ScopedLock lock(theSentinel.Lock);                                \
      theSentinel.Cache[object].code_RT_FUNCTION_INVOCATION_COUNTER_NAME(     \
          RT_FUNCTION)++;                                                      \
      /* TODO: Remember the order/history of  operations? */                   \
    }                                                                          \
  }
#include "RuntimeInvocationsTracking.def"

/// Public APIs

/// Get the runtime object state associated with an object.
void _language_getObjectRuntimeFunctionCounters(
    HeapObject *object, RuntimeFunctionCountersState *result) {
  auto &theSentinel = RuntimeObjectStateCache.get();
  Mutex::ScopedLock lock(theSentinel.Lock);
  *result = theSentinel.Cache[object];
}

/// Set the runtime object state associated with an object from a provided
/// state.
void _language_setObjectRuntimeFunctionCounters(
    HeapObject *object, RuntimeFunctionCountersState *state) {
  auto &theSentinel = RuntimeObjectStateCache.get();
  Mutex::ScopedLock lock(theSentinel.Lock);
  theSentinel.Cache[object] = *state;
}

/// Get the global runtime state containing the total numbers of invocations for
/// each runtime function of interest.
void _language_getGlobalRuntimeFunctionCounters(
    RuntimeFunctionCountersState *result) {
  LazyMutex::ScopedLock lock(RuntimeGlobalFunctionCountersState.Lock);
  *result = RuntimeGlobalFunctionCountersState.State;
}

/// Set the global runtime state of function pointers from a provided state.
void _language_setGlobalRuntimeFunctionCounters(
    RuntimeFunctionCountersState *state) {
  LazyMutex::ScopedLock lock(RuntimeGlobalFunctionCountersState.Lock);
  RuntimeGlobalFunctionCountersState.State = *state;
}

/// Return the names of the runtime functions being tracked.
/// Their order is the same as the order of the counters in the
/// RuntimeObjectState structure. All these strings are null terminated.
const char **_language_getRuntimeFunctionNames() {
  return RuntimeFunctionNames;
}

/// Return the offsets of the runtime function counters being tracked.
/// Their order is the same as the order of the counters in the
/// RuntimeObjectState structure.
const std::uint16_t *_language_getRuntimeFunctionCountersOffsets() {
  return RuntimeFunctionCountersOffsets;
}

/// Return the number of runtime functions being tracked.
std::uint64_t _language_getNumRuntimeFunctionCounters() {
  return ID_LastRuntimeFunctionName;
}

static void _language_dumpRuntimeCounters(RuntimeFunctionCountersState *State) {
  std::uint32_t tmp;
/// Define how to dump the counter for a given runtime function.
#define FUNCTION_TO_TRACK(RT_FUNCTION)                                         \
  tmp = State->LANGUAGE_RT_FUNCTION_INVOCATION_COUNTER_NAME(RT_FUNCTION);         \
  if (tmp != 0)                                                                \
    printf("%s = %d\n",                                                        \
           RuntimeFunctionNames[(int)RT_FUNCTION_ID(RT_FUNCTION)], tmp);
#include "RuntimeInvocationsTracking.def"
}

/// Dump all per-object runtime function pointers.
void _language_dumpObjectsRuntimeFunctionPointers() {
  auto &theSentinel = RuntimeObjectStateCache.get();
  Mutex::ScopedLock lock(theSentinel.Lock);
  for (auto &Pair : theSentinel.Cache) {
    printf("\n\nRuntime counters for object at address %p:\n", Pair.getFirst());
    _language_dumpRuntimeCounters(&Pair.getSecond());
    printf("\n");
  }
}

/// Set mode for global runtime function counters.
/// Return the old value of this flag.
int _language_setGlobalRuntimeFunctionCountersMode(int mode) {
  int oldMode = UpdateGlobalRuntimeFunctionCounters;
  UpdateGlobalRuntimeFunctionCounters = mode ? 1 : 0;
  return oldMode;
}

/// Set mode for per object runtime function counters.
/// Return the old value of this flag.
int _language_setPerObjectRuntimeFunctionCountersMode(int mode) {
  int oldMode = UpdatePerObjectRuntimeFunctionCounters;
  UpdatePerObjectRuntimeFunctionCounters = mode ? 1 : 0;
  return oldMode;
}

/// Add the ability to call custom handlers when a counter
/// is being updated. The handler should take the object and may be
/// the name of the runtime function as parameters. And this handler
/// could e.g. check some conditions and stop the program under
/// a debugger if a certain condition is met, like a refcount has
/// reached a certain value.
/// We could allow for setting global handlers or even per-object
/// handlers.
RuntimeFunctionCountersUpdateHandler
_language_setGlobalRuntimeFunctionCountersUpdateHandler(
    RuntimeFunctionCountersUpdateHandler handler) {
  auto oldHandler = GlobalRuntimeFunctionCountersUpdateHandler;
  GlobalRuntimeFunctionCountersUpdateHandler = handler;
  return oldHandler;
}

/// TODO: Provide an API to remove any counters related to a specific object
/// or all objects.
/// This is useful if you want to reset the stats for some/all objects.

#endif
