//===--- DispatchShims.h - Shims for dispatch vended APIs --------------------*- C++ -*-//
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

#ifndef LANGUAGE_CONCURRENCY_DISPATCHSHIMS_H
#define LANGUAGE_CONCURRENCY_DISPATCHSHIMS_H

#include "Concurrency.h"

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
#include <dispatch/language_concurrency_private.h>

#if LANGUAGE_CONCURRENCY_TASK_TO_THREAD_MODEL
#error Cannot use task-to-thread model with priority escalation
#endif

// Provide wrappers with runtime checks to make sure that the dispatch functions
// are only called on OS-es where they are supported
static inline dispatch_thread_override_info_s
language_dispatch_thread_get_current_override_qos_floor()
{
  if (__builtin_available(macOS 9998, iOS 9998, tvOS 9998, watchOS 9998, *)) {
    return dispatch_thread_get_current_override_qos_floor();
  }

  return (dispatch_thread_override_info_s){
      0,                     // can_override
      0,                     // unused
      QOS_CLASS_UNSPECIFIED, // override_qos_floor
  };
}

static inline int
language_dispatch_thread_override_self(qos_class_t override_qos) {

  if (__builtin_available(macOS 9998, iOS 9998, tvOS 9998, watchOS 9998, *)) {
    return dispatch_thread_override_self(override_qos);
  }

  return 0;
}

static inline int
language_dispatch_lock_override_start_with_debounce(dispatch_lock_t *lock_addr,
   dispatch_tid_t expected_thread, qos_class_t override_to_apply) {

  if (__builtin_available(macOS 9998, iOS 9998, tvOS 9998, watchOS 9998, *)) {
    return dispatch_lock_override_start_with_debounce(lock_addr, expected_thread, override_to_apply);
  }

  return 0;
}

static inline int
language_dispatch_lock_override_end(qos_class_t override_to_end) {
  if (__builtin_available(macOS 9998, iOS 9998, tvOS 9998, watchOS 9998, *)) {
    return dispatch_lock_override_end(override_to_end);
  }

  return 0;
}
#endif

#endif /* LANGUAGE_CONCURRENCY_DISPATCHSHIMS_H */
