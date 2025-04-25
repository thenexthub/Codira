//===--- ThreadLocalStorage.h - Wrapper for thread-local storage. --*- C++ -*-===//
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

#ifndef SWIFT_STDLIB_SHIMS_THREADLOCALSTORAGE_H
#define SWIFT_STDLIB_SHIMS_THREADLOCALSTORAGE_H

#include "Visibility.h"

SWIFT_RUNTIME_STDLIB_INTERNAL
void * _Nonnull _swift_stdlib_threadLocalStorageGet(void);

#endif // SWIFT_STDLIB_SHIMS_THREADLOCALSTORAGE_H
