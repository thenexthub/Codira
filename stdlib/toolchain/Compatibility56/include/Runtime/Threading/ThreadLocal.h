//===--- ThreadLocal.h - Thread-local storage -------------------*- C++ -*-===//
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
// Declarations and macros for working with thread-local storage in the
// Codira runtime.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_THREADLOCAL_BACKDEPLOY56_H
#define LANGUAGE_RUNTIME_THREADLOCAL_BACKDEPLOY56_H

#include <type_traits>
#include "ThreadLocalStorage.h"

/// LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL - Does the current configuration
/// allow the use of LANGUAGE_RUNTIME_ATTRIBUTE_THREAD_LOCAL?
#if defined(__APPLE__)
// The pthread TLS APIs work better than C++ TLS on Apple platforms.
#define LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL 0
#elif defined(LANGUAGE_STDLIB_SINGLE_THREADED_RUNTIME)
// We define LANGUAGE_RUNTIME_ATTRIBUTE_THREAD_LOCAL to nothing in this
// configuration and just use a global variable, so this is okay.
#define LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL 1
#elif __has_feature(tls)
// If __has_feature reports that TLS is available, use it.
#define LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL 1
#elif !defined(__clang__)
// If we're not using Clang, assume that __has_feature is unreliable
// and that we can safely use TLS.
#else
// Otherwise we can't use TLS and have to fall back on something else.
#define LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL 0
#endif

/// LANGUAGE_RUNTIME_THREAD_LOCAL - Declare that something is a
/// thread-local variable in the runtime.
#if defined(LANGUAGE_STDLIB_SINGLE_THREADED_RUNTIME)
// In a single-threaded runtime, thread-locals are global.
#define LANGUAGE_RUNTIME_ATTRIBUTE_THREAD_LOCAL
#elif defined(__GNUC__)
// In GCC-compatible compilers, we prefer __thread because it's understood
// to guarantee a constant initializer, which permits more efficient access
// patterns.
#define LANGUAGE_RUNTIME_ATTRIBUTE_THREAD_LOCAL __thread
#else
// Otherwise, just fall back on the standard C++ feature.
#define LANGUAGE_RUNTIME_ATTRIBUTE_THREAD_LOCAL thread_local
#endif

// Implementation of LANGUAGE_RUNTIME_DECLARE_THREAD_LOCAL
#if !LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL
#include <pthread.h>
#include <dispatch/dispatch.h>
#endif

namespace language {
// Validate a type stored in thread-local storage, using static asserts. Such
// types must fit in a pointer and be trivially copyable/destructible.
#define VALIDATE_THREAD_LOCAL_TYPE(T)                                          \
  static_assert(sizeof(T) <= sizeof(void *),                                   \
                "cannot store more than a pointer");                           \
  static_assert(std::is_trivially_copyable<T>::value,                          \
                "ThreadLocal values must be trivially copyable");              \
  static_assert(std::is_trivially_destructible<T>::value,                      \
                "ThreadLocal cleanup is not supported, stored types must be "  \
                "trivially destructible");

// A wrapper class for thread-local storage.
//
// - On platforms that report LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL
//   above, an object of this type is declared with
//   LANGUAGE_RUNTIME_ATTRIBUTE_THREAD_LOCAL.  This makes the object
//   itself thread-local, and no internal support is required.
//
//   Note that this includes platforms that set
//   LANGUAGE_STDLIB_SINGLE_THREADED_RUNTIME, for which
//   LANGUAGE_RUNTIME_ATTRIBUTE_THREAD_LOCAL is empty;
//   thread-local declarations then create an ordinary global.
//
// - On platforms that don't report LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL,
//   we have to simulate thread-local storage.  Fortunately, all of
//   these platforms (at least for now) support pthread_getspecific.
#if LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL
template <class T>
class ThreadLocal {
  VALIDATE_THREAD_LOCAL_TYPE(T)

  T value;

public:
  constexpr ThreadLocal() {}

  T get() { return value; }

  void set(T newValue) { value = newValue; }
};
#else
// A wrapper around a pthread_key_t that is lazily initialized using
// dispatch_once.
class ThreadLocalKey {
  // We rely on the zero-initialization of objects with static storage
  // duration.
  dispatch_once_t once;
  pthread_key_t key;

public:
  pthread_key_t getKey() {
    dispatch_once_f(&once, &key, [](void *ctx) {
      pthread_key_create(reinterpret_cast<pthread_key_t *>(ctx), nullptr);
    });
    return key;
  }
};

// A type representing a constant pthread_key_t, for use on platforms that
// provide reserved keys.
template <pthread_key_t constantKey>
class ConstantThreadLocalKey {
public:
  pthread_key_t getKey() { return constantKey; }
};

template <class T, class Key>
class ThreadLocal {
  VALIDATE_THREAD_LOCAL_TYPE(T)

  Key key;

public:
  constexpr ThreadLocal() {}

  T get() {
    void *storedValue = LANGUAGE_THREAD_GETSPECIFIC(key.getKey());
    T value;
    memcpy(&value, &storedValue, sizeof(T));
    return value;
  }

  void set(T newValue) {
    void *storedValue;
    memcpy(&storedValue, &newValue, sizeof(T));
    LANGUAGE_THREAD_SETSPECIFIC(key.getKey(), storedValue);
  }
};
#endif

} // end namespace language

/// LANGUAGE_RUNTIME_DECLARE_THREAD_LOCAL(TYPE, NAME) - Declare a variable
/// to be a thread-local variable.  The declaration must have static
/// storage duration; it may be prefixed with "static".
///
/// Because of the fallback path, the default-initialization of the
/// type must be equivalent to a bitwise zero-initialization, and the
/// type must be small and trivially copyable and destructible.
#if LANGUAGE_RUNTIME_SUPPORTS_THREAD_LOCAL
#define LANGUAGE_RUNTIME_DECLARE_THREAD_LOCAL(TYPE, NAME, KEY) \
  LANGUAGE_RUNTIME_ATTRIBUTE_THREAD_LOCAL language::ThreadLocal<TYPE> NAME
#elif LANGUAGE_TLS_HAS_RESERVED_PTHREAD_SPECIFIC
#define LANGUAGE_RUNTIME_DECLARE_THREAD_LOCAL(TYPE, NAME, KEY) \
  language::ThreadLocal<TYPE, ConstantThreadLocalKey<KEY>> NAME
#else
#define LANGUAGE_RUNTIME_DECLARE_THREAD_LOCAL(TYPE, NAME, KEY) \
  language::ThreadLocal<TYPE, ThreadLocalKey> NAME
#endif

#endif // LANGUAGE_RUNTIME_THREADLOCAL_BACKDEPLOY56_H
