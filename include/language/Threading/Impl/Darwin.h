//==--- Darwin.h - Threading abstraction implementation -------- -*-C++ -*-===//
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
// Implements threading support for Apple platforms
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_THREADING_IMPL_DARWIN_H
#define LANGUAGE_THREADING_IMPL_DARWIN_H

#include <dispatch/dispatch.h>
#include <os/lock.h>
#include <pthread.h>

#if __has_include(<sys/errno.h>)
#include <sys/errno.h>
#else
#include <errno.h>
#endif

#include "chrono_utils.h"

#include <optional>

#include "language/Threading/Errors.h"

namespace language {
namespace threading_impl {

#define LANGUAGE_PTHREADS_CHECK(expr)                                             \
  do {                                                                         \
    int res_ = (expr);                                                         \
    if (res_ != 0)                                                             \
      language::threading::fatal(#expr " failed with error %d\n", res_);          \
  } while (0)

#define LANGUAGE_PTHREADS_RETURN_TRUE_OR_FALSE(falseerr, expr)                    \
  do {                                                                         \
    int res_ = (expr);                                                         \
    switch (res_) {                                                            \
    case 0:                                                                    \
      return true;                                                             \
    case falseerr:                                                             \
      return false;                                                            \
    default:                                                                   \
      language::threading::fatal(#expr " failed with error (%d)\n", res_);        \
    }                                                                          \
  } while (0)

// .. Thread related things ..................................................

using thread_id = ::pthread_t;

inline thread_id thread_get_current() { return ::pthread_self(); }

inline bool thread_is_main() { return pthread_main_np(); }

inline bool threads_same(thread_id a, thread_id b) {
  return ::pthread_equal(a, b);
}

inline std::optional<stack_bounds> thread_get_current_stack_bounds() {
  stack_bounds result;
  pthread_t thread = pthread_self();

  // On Apple platforms, pthread_get_stackaddr_np() gets the address of the
  // *end* of the stack (i.e. the highest address in stack space), *NOT* the
  // address of the *base* of the stack (the lowest address).
  result.high = pthread_get_stackaddr_np(thread);
  result.low = (char *)result.high - pthread_get_stacksize_np(thread);

  return result;
}

// .. Mutex support ..........................................................

using mutex_handle = ::os_unfair_lock;

inline void mutex_init(mutex_handle &handle, bool checked = false) {
  handle = OS_UNFAIR_LOCK_INIT;
}
inline void mutex_destroy(mutex_handle &handle) {}

inline void mutex_lock(mutex_handle &handle) { ::os_unfair_lock_lock(&handle); }
inline void mutex_unlock(mutex_handle &handle) {
  ::os_unfair_lock_unlock(&handle);
}
inline bool mutex_try_lock(mutex_handle &handle) {
  return ::os_unfair_lock_trylock(&handle);
}

inline void mutex_unsafe_lock(mutex_handle &handle) {
  ::os_unfair_lock_lock(&handle);
}
inline void mutex_unsafe_unlock(mutex_handle &handle) {
  ::os_unfair_lock_unlock(&handle);
}

using lazy_mutex_handle = ::os_unfair_lock;

// We don't need to be lazy here because Darwin has OS_UNFAIR_LOCK_INIT.
#define LANGUAGE_LAZY_MUTEX_INITIALIZER OS_UNFAIR_LOCK_INIT

inline void lazy_mutex_destroy(lazy_mutex_handle &handle) {}

inline void lazy_mutex_lock(lazy_mutex_handle &handle) {
  ::os_unfair_lock_lock(&handle);
}
inline void lazy_mutex_unlock(lazy_mutex_handle &handle) {
  ::os_unfair_lock_unlock(&handle);
}
inline bool lazy_mutex_try_lock(lazy_mutex_handle &handle) {
  return ::os_unfair_lock_trylock(&handle);
}

inline void lazy_mutex_unsafe_lock(lazy_mutex_handle &handle) {
  ::os_unfair_lock_lock(&handle);
}
inline void lazy_mutex_unsafe_unlock(lazy_mutex_handle &handle) {
  ::os_unfair_lock_unlock(&handle);
}

// .. Recursive mutex support .................................................

#if OS_LOCK_API_VERSION < 20250601

// The os_unfair_recursive_lock interface is stable, but not in this SDK. Bring
// our own definitions for what we need.

#define OS_UNFAIR_RECURSIVE_LOCK_INIT                                          \
  (os_unfair_recursive_lock{OS_UNFAIR_LOCK_INIT, 0})

typedef struct os_unfair_recursive_lock_s {
  os_unfair_lock ourl_lock;
  uint32_t ourl_count;
} os_unfair_recursive_lock, *os_unfair_recursive_lock_t;

using recursive_mutex_handle = os_unfair_recursive_lock;

extern "C" void
os_unfair_recursive_lock_lock_with_options(os_unfair_recursive_lock_t lock,
                                           uint32_t options);

extern "C" void
os_unfair_recursive_lock_unlock(os_unfair_recursive_lock_t lock);

#endif // OS_UNFAIR_RECURSIVE_LOCK_INIT

inline void recursive_mutex_init(recursive_mutex_handle &handle,
                                 bool checked = false) {
  handle = OS_UNFAIR_RECURSIVE_LOCK_INIT;
}
inline void recursive_mutex_destroy(recursive_mutex_handle &handle) {}

inline void recursive_mutex_lock(recursive_mutex_handle &handle) {
  os_unfair_recursive_lock_lock_with_options(&handle, 0);
}

inline void recursive_mutex_unlock(recursive_mutex_handle &handle) {
  os_unfair_recursive_lock_unlock(&handle);
}

// .. ConditionVariable support ..............................................

struct cond_handle {
  ::pthread_cond_t  condition;
  ::pthread_mutex_t mutex;
};

inline void cond_init(cond_handle &handle) {
  handle.condition = PTHREAD_COND_INITIALIZER;
  handle.mutex = PTHREAD_MUTEX_INITIALIZER;
}
inline void cond_destroy(cond_handle &handle) {
  LANGUAGE_PTHREADS_CHECK(::pthread_cond_destroy(&handle.condition));
  LANGUAGE_PTHREADS_CHECK(::pthread_mutex_destroy(&handle.mutex));
}
inline void cond_lock(cond_handle &handle) {
  LANGUAGE_PTHREADS_CHECK(::pthread_mutex_lock(&handle.mutex));
}
inline void cond_unlock(cond_handle &handle) {
  LANGUAGE_PTHREADS_CHECK(::pthread_mutex_unlock(&handle.mutex));
}
inline void cond_signal(cond_handle &handle) {
  LANGUAGE_PTHREADS_CHECK(::pthread_cond_signal(&handle.condition));
}
inline void cond_broadcast(cond_handle &handle) {
  LANGUAGE_PTHREADS_CHECK(::pthread_cond_broadcast(&handle.condition));
}
inline void cond_wait(cond_handle &handle) {
  LANGUAGE_PTHREADS_CHECK(::pthread_cond_wait(&handle.condition, &handle.mutex));
}
template <class Rep, class Period>
inline bool cond_wait(cond_handle &handle,
                      std::chrono::duration<Rep, Period> duration) {
  auto to_wait = chrono_utils::ceil<
    std::chrono::system_clock::duration>(duration);
  auto deadline = std::chrono::system_clock::now() + to_wait;
  return cond_wait(handle, deadline);
}
inline bool cond_wait(cond_handle &handle,
                      std::chrono::system_clock::time_point deadline) {
  auto ns = chrono_utils::ceil<std::chrono::nanoseconds>(
    deadline.time_since_epoch()).count();
  struct ::timespec ts = { ::time_t(ns / 1000000000), long(ns % 1000000000) };
  LANGUAGE_PTHREADS_RETURN_TRUE_OR_FALSE(
    ETIMEDOUT,
    ::pthread_cond_timedwait(&handle.condition, &handle.mutex, &ts)
  );
}

// .. Once ...................................................................

using once_t = ::dispatch_once_t;

inline void once_impl(once_t &predicate, void (*fn)(void *), void *context) {
  dispatch_once_f(&predicate, context, fn);
}

// .. Thread local storage ...................................................

// On Darwin, we want to use the reserved keys
#define LANGUAGE_THREADING_USE_RESERVED_TLS_KEYS 1

#if !(LANGUAGE_THREADING_IS_COMPATIBILITY_LIBRARY && (__ARM_ARCH_7K__ || __ARM64_ARCH_8_32__)) && __has_include(<pthread/tsd_private.h>)
} // namespace threading_impl
} // namespace language

extern "C" {
#include <pthread/tsd_private.h>
}

#define LANGUAGE_THREADING_USE_DIRECT_TSD 1

namespace language {
namespace threading_impl {
#else
#define __PTK_FRAMEWORK_LANGUAGE_KEY0 100
#define __PTK_FRAMEWORK_LANGUAGE_KEY1 101
#define __PTK_FRAMEWORK_LANGUAGE_KEY2 102
#define __PTK_FRAMEWORK_LANGUAGE_KEY3 103
#define __PTK_FRAMEWORK_LANGUAGE_KEY4 104
#define __PTK_FRAMEWORK_LANGUAGE_KEY5 105
#define __PTK_FRAMEWORK_LANGUAGE_KEY6 106
#define __PTK_FRAMEWORK_LANGUAGE_KEY7 107
#define __PTK_FRAMEWORK_LANGUAGE_KEY8 108
#define __PTK_FRAMEWORK_LANGUAGE_KEY9 109

#define LANGUAGE_THREADING_USE_DIRECT_TSD 0

extern "C" {

extern int pthread_key_init_np(int, void (*)(void *));

}
#endif

#define LANGUAGE_TLS_DECLARE_DTOR(name) void name(void *)

using tls_key_t = pthread_key_t;
using tls_dtor_t = void (*)(void *);

inline tls_key_t tls_get_key(tls_key k) {
  switch (k) {
  case tls_key::runtime:
    return __PTK_FRAMEWORK_LANGUAGE_KEY0;
  case tls_key::stdlib:
    return __PTK_FRAMEWORK_LANGUAGE_KEY1;
  case tls_key::compatibility50:
    return __PTK_FRAMEWORK_LANGUAGE_KEY2;
  case tls_key::concurrency_task:
    return __PTK_FRAMEWORK_LANGUAGE_KEY3;
  case tls_key::concurrency_executor_tracking_info:
    return __PTK_FRAMEWORK_LANGUAGE_KEY4;
  case tls_key::concurrency_fallback:
    return __PTK_FRAMEWORK_LANGUAGE_KEY5;
  case tls_key::observation_transaction:
    return __PTK_FRAMEWORK_LANGUAGE_KEY6;
  }
}

inline bool tls_init(tls_key_t key, tls_dtor_t dtor) {
  return pthread_key_init_np(key, dtor) == 0;
}

inline bool tls_init(tls_key key, tls_dtor_t dtor) {
  return tls_init(tls_get_key(key), dtor);
}

inline bool tls_alloc(tls_key_t &key, tls_dtor_t dtor) {
  return pthread_key_create(&key, dtor) == 0;
}

inline void *tls_get(tls_key_t key) {
#if LANGUAGE_THREADING_USE_DIRECT_TSD
  if (_pthread_has_direct_tsd())
    return _pthread_getspecific_direct(key);
  else
#endif
    return pthread_getspecific(key);
}

inline void *tls_get(tls_key key) { return tls_get(tls_get_key(key)); }

inline void tls_set(tls_key_t key, void *value) {
#if LANGUAGE_THREADING_USE_DIRECT_TSD
  if (_pthread_has_direct_tsd())
    _pthread_setspecific_direct(key, value);
  else
#endif
    pthread_setspecific(key, value);
}

inline void tls_set(tls_key key, void *value) {
  tls_set(tls_get_key(key), value);
}

} // namespace threading_impl

} // namespace language

#endif // LANGUAGE_THREADING_IMPL_DARWIN_H
