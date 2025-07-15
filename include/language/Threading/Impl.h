//===--- Impl.h - Threading abstraction implementation -------- -*- C++ -*-===//
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
// Includes the relevant implementation file based on the selected threading
// package.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_THREADING_IMPL_H
#define LANGUAGE_THREADING_IMPL_H

#include "TLSKeys.h"

namespace language {
namespace threading_impl {

struct stack_bounds {
  void *low;
  void *high;
};

} // namespace language
} // namespace threading_impl


// Try to autodetect if we aren't told what to do
#if !LANGUAGE_THREADING_NONE && !LANGUAGE_THREADING_DARWIN &&                        \
    !LANGUAGE_THREADING_LINUX && !LANGUAGE_THREADING_PTHREADS &&                     \
    !LANGUAGE_THREADING_C11 && !LANGUAGE_THREADING_WIN32
#ifdef __APPLE__
#define LANGUAGE_THREADING_DARWIN 1
#elif defined(__linux__)
#define LANGUAGE_THREADING_LINUX 1
#elif defined(_WIN32)
#define LANGUAGE_THREADING_WIN32 1
#elif defined(__wasi__)
#define LANGUAGE_THREADING_NONE 1
#elif __has_include(<threads.h>)
#define LANGUAGE_THREADING_C11 1
#elif __has_include(<pthread.h>)
#define LANGUAGE_THREADING_PTHREADS 1
#else
#error Unable to autodetect threading package.  Please define LANGUAGE_THREADING_x as appropriate for your platform.
#endif
#endif

#if LANGUAGE_THREADING_NONE
#include "Impl/Nothreads.h"
#elif LANGUAGE_THREADING_DARWIN
#include "Impl/Darwin.h"
#elif LANGUAGE_THREADING_LINUX
#include "Impl/Linux.h"
#elif LANGUAGE_THREADING_PTHREADS
#include "Impl/Pthreads.h"
#elif LANGUAGE_THREADING_C11
#include "Impl/C11.h"
#elif LANGUAGE_THREADING_WIN32
#include "Impl/Win32.h"
#else
#error You need to implement Threading/Impl.h for your threading package.
#endif

#endif // LANGUAGE_THREADING_IMPL_H
