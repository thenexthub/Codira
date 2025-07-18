//===--- ThreadLocalStorage.h - Thread-local storage interface. -*- C++ -*-===//
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

#ifndef LANGUAGE_THREADING_THREADLOCALSTORAGE_H
#define LANGUAGE_THREADING_THREADLOCALSTORAGE_H

#include <type_traits>

#include "Errors.h"
#include "Impl.h"
#include "Once.h"
#include "TLSKeys.h"

namespace language {

// -- Low-level TLS functions -------------------------------------------------

#if !LANGUAGE_THREADING_NONE
using tls_key_t = threading_impl::tls_key_t;
using tls_dtor_t = threading_impl::tls_dtor_t;

#if LANGUAGE_THREADING_USE_RESERVED_TLS_KEYS
using threading_impl::tls_get_key;
using threading_impl::tls_init;

/// tls_init_once() - Initialize TLS, once only
inline void tls_init_once(once_t &token, tls_key_t key, tls_dtor_t dtor) {
  const struct tls_init_info {
    tls_key_t &k;
    tls_dtor_t d;
  } info = {key, dtor};
  once(
      token,
      [](void *ctx) {
        const struct tls_init_info *pinfo =
            static_cast<const struct tls_init_info *>(ctx);
        if (!tls_init(pinfo->k, pinfo->d))
          language::threading::fatal("tls_init_once() failed to set destructor");
      },
      (void *)&info);
}

inline void tls_init_once(once_t &token, tls_key key, tls_dtor_t dtor) {
  tls_init_once(token, tls_get_key(key), dtor);
}
#endif // LANGUAGE_THREADING_USE_RESERVED_TLS_KEYS

using threading_impl::tls_alloc;
using threading_impl::tls_get;
using threading_impl::tls_set;

/// tls_alloc_once() - Allocate TLS key, once only
inline void tls_alloc_once(once_t &token, tls_key_t &key, tls_dtor_t dtor) {
  const struct tls_init_info {
    tls_key_t &k;
    tls_dtor_t d;
  } info = {key, dtor};
  once(
      token,
      [](void *ctx) {
        const struct tls_init_info *pinfo =
            static_cast<const struct tls_init_info *>(ctx);
        if (!tls_alloc(pinfo->k, pinfo->d))
          language::threading::fatal("tls_alloc_once() failed to allocate key");
      },
      (void *)&info);
}
#endif // !LANGUAGE_THREADING_NONE

// -- High-level TLS classes --------------------------------------------------

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
// - On platforms that define LANGUAGE_THREAD_LOCAL, an object of this type
//   is declared with LANGUAGE_THREAD_LOCAL.  This makes the object
//   itself thread-local, and no internal support is required.
//
//   Note that this includes platforms that don't support threading,
//   for which LANGUAGE_THREAD_LOCAL is empty; thread-local declarations
//   then create an ordinary global.
//
// - On platforms that don't define LANGUAGE_THREAD_LOCAL, we have to simulate
//   thread-local storage.  Fortunately, all of these platforms (at least
//   for now) support pthread_getspecific or similar.
#ifdef LANGUAGE_THREAD_LOCAL
template <class T>
class ThreadLocal {
  VALIDATE_THREAD_LOCAL_TYPE(T)

  T value;

public:
  constexpr ThreadLocal() {}

  T get() { return value; }

  void set(T newValue) { value = newValue; }

  T swap(T newValue) {
    // There's an implicit optimization here because we implicitly
    // materialize the address of the thread-local once instead of
    // separately for two calls to get and set.
    auto curValue = get();
    set(newValue);
    return curValue;
  }
};
#else
// A wrapper around a TLS key that is lazily initialized using language::once.
class ThreadLocalKey {
  // We rely on the zero-initialization of objects with static storage
  // duration.
  once_t onceFlag;
  tls_key_t key;

public:
  threading_impl::tls_key_t getKey() {
    once(
        onceFlag,
        [](void *ctx) {
          tls_key_t *pkey = reinterpret_cast<tls_key_t *>(ctx);
          tls_alloc(*pkey, nullptr);
        },
        &key);
    return key;
  }
};

#if LANGUAGE_THREADING_USE_RESERVED_TLS_KEYS
// A type representing a constant TLS key, for use on platforms
// that provide reserved keys.
template <tls_key constantKey>
class ConstantThreadLocalKey {
public:
  tls_key_t getKey() { return tls_get_key(constantKey); }
};
#endif

template <class T, class Key>
class ThreadLocal {
  VALIDATE_THREAD_LOCAL_TYPE(T)

  Key key;

public:
  constexpr ThreadLocal() {}

  T get() {
    void *storedValue = tls_get(key.getKey());
    T value;
    memcpy(&value, &storedValue, sizeof(T));
    return value;
  }

  void set(T newValue) {
    void *storedValue;
    memcpy(&storedValue, &newValue, sizeof(T));
    tls_set(key.getKey(), storedValue);
  }

  T swap(T newValue) {
    auto curValue = get();
    set(newValue);
    return curValue;
  }
};
#endif

} // end namespace language

/// LANGUAGE_THREAD_LOCAL_TYPE(TYPE, KEY) - Declare a variable
/// to be a thread-local variable.  The declaration must have static
/// storage duration; it may be prefixed with "static".
///
/// For example
///
///   static LANGUAGE_THREAD_LOCAL_TYPE(int, LANGUAGE_RESERVED_TLS_KEY_T_9) frobble;
///
/// Because of the fallback path, the default-initialization of the
/// type must be equivalent to a bitwise zero-initialization, and the
/// type must be small and trivially copyable and destructible.
#ifdef LANGUAGE_THREAD_LOCAL
#define LANGUAGE_THREAD_LOCAL_TYPE(TYPE, KEY)                                     \
  LANGUAGE_THREAD_LOCAL language::ThreadLocal<TYPE>
#elif LANGUAGE_THREADING_USE_RESERVED_TLS_KEYS
#define LANGUAGE_THREAD_LOCAL_TYPE(TYPE, KEY)                                     \
  language::ThreadLocal<TYPE, language::ConstantThreadLocalKey<KEY>>
#else
#define LANGUAGE_THREAD_LOCAL_TYPE(TYPE, KEY)                                     \
  language::ThreadLocal<TYPE, language::ThreadLocalKey>
#endif

#endif // LANGUAGE_THREADING_THREADLOCALSTORAGE_H
