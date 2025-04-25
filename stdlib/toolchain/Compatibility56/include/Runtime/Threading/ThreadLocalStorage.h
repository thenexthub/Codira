//===--- ThreadLocalStorage.h - Thread-local storage interface. --*- C++ -*-===//
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

#ifndef SWIFT_RUNTIME_THREADLOCALSTORAGE_BACKDEPLOY56_H
#define SWIFT_RUNTIME_THREADLOCALSTORAGE_BACKDEPLOY56_H

#include "language/Runtime/Config.h"

// Depending on the target, we may be able to use dedicated TSD keys or
// thread_local variables. When dedicated TSD keys aren't available,
// wrap the target's API for thread-local data for things that don't want
// to use thread_local.

// On Apple platforms, we have dedicated TSD keys.
#if defined(__APPLE__)
# define SWIFT_TLS_HAS_RESERVED_PTHREAD_SPECIFIC 1
#endif

#if SWIFT_TLS_HAS_RESERVED_PTHREAD_SPECIFIC
// Use reserved TSD keys.
# if __has_include(<pthread/tsd_private.h>)
#  include <pthread/tsd_private.h>
# else
// We still need to use the SPI for setting the destructor, so declare it here.
extern "C" int pthread_key_init_np(int key, void (*destructor)(void *));
# endif

// If the keys are not available from the header, define them ourselves. The values match
// what tsd_private.h provides.
# ifndef __PTK_FRAMEWORK_SWIFT_KEY0
#  define __PTK_FRAMEWORK_SWIFT_KEY0 100
# endif
# ifndef __PTK_FRAMEWORK_SWIFT_KEY1
#  define __PTK_FRAMEWORK_SWIFT_KEY1 101
# endif
# ifndef __PTK_FRAMEWORK_SWIFT_KEY2
#  define __PTK_FRAMEWORK_SWIFT_KEY2 102
# endif
# ifndef __PTK_FRAMEWORK_SWIFT_KEY3
#  define __PTK_FRAMEWORK_SWIFT_KEY3 103
# endif
# ifndef __PTK_FRAMEWORK_SWIFT_KEY4
#  define __PTK_FRAMEWORK_SWIFT_KEY4 104
# endif
# ifndef __PTK_FRAMEWORK_SWIFT_KEY5
#  define __PTK_FRAMEWORK_SWIFT_KEY5 105
# endif
# ifndef __PTK_FRAMEWORK_SWIFT_KEY6
#  define __PTK_FRAMEWORK_SWIFT_KEY6 106
# endif
# ifndef __PTK_FRAMEWORK_SWIFT_KEY7
#  define __PTK_FRAMEWORK_SWIFT_KEY7 107
# endif
# ifndef __PTK_FRAMEWORK_SWIFT_KEY8
#  define __PTK_FRAMEWORK_SWIFT_KEY8 108
# endif
# ifndef __PTK_FRAMEWORK_SWIFT_KEY9
#  define __PTK_FRAMEWORK_SWIFT_KEY9 109
# endif


# define SWIFT_RUNTIME_TLS_KEY __PTK_FRAMEWORK_SWIFT_KEY0
# define SWIFT_STDLIB_TLS_KEY __PTK_FRAMEWORK_SWIFT_KEY1
# define SWIFT_COMPATIBILITY_50_TLS_KEY __PTK_FRAMEWORK_SWIFT_KEY2
# define SWIFT_CONCURRENCY_TASK_KEY __PTK_FRAMEWORK_SWIFT_KEY3
# define SWIFT_CONCURRENCY_EXECUTOR_TRACKING_INFO_KEY __PTK_FRAMEWORK_SWIFT_KEY4
# define SWIFT_CONCURRENCY_FALLBACK_TASK_LOCAL_STORAGE_KEY \
  __PTK_FRAMEWORK_SWIFT_KEY5

#endif

// If the reserved key path didn't already provide get/setspecific macros,
// wrap the platform's APIs.
#ifndef SWIFT_THREAD_GETSPECIFIC

// Pick the right typedef for the key.
# if defined(__linux__)
#  if defined(__ANDROID__)
typedef int __swift_thread_key_t;
#  else
typedef unsigned int __swift_thread_key_t;
#  endif
# elif defined(__FreeBSD__)
typedef int __swift_thread_key_t;
# elif defined(__OpenBSD__)
typedef int __swift_thread_key_t;
# elif defined(_WIN32)
typedef unsigned long __swift_thread_key_t;
# elif defined(__HAIKU__)
typedef int __swift_thread_key_t;
# else
typedef unsigned long __swift_thread_key_t;
# endif

# if defined(_WIN32) && !defined(__CYGWIN__)
// Windows has its own flavor of API.
#  include <io.h>
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>

#include <type_traits>

static_assert(std::is_same<__swift_thread_key_t, DWORD>::value,
              "__swift_thread_key_t is not a DWORD");

#  define SWIFT_THREAD_KEY_CREATE _stdlib_thread_key_create
#  define SWIFT_THREAD_GETSPECIFIC FlsGetValue
#  define SWIFT_THREAD_SETSPECIFIC(key, value) (FlsSetValue(key, value) == FALSE)

# elif !defined(SWIFT_STDLIB_SINGLE_THREADED_RUNTIME)
// Otherwise use the pthread API.
#  include <pthread.h>
#  define SWIFT_THREAD_KEY_CREATE pthread_key_create
#  define SWIFT_THREAD_GETSPECIFIC pthread_getspecific
#  define SWIFT_THREAD_SETSPECIFIC pthread_setspecific
# endif
#endif

#endif // SWIFT_RUNTIME_THREADLOCALSTORAGE_BACKDEPLOY56_H
