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

#ifndef LANGUAGE_STDLIB_SHIMS_THREADLOCALSTORAGE_H
#define LANGUAGE_STDLIB_SHIMS_THREADLOCALSTORAGE_H

#include "Visibility.h"

LANGUAGE_RUNTIME_STDLIB_INTERNAL
void * _Nonnull _language_stdlib_threadLocalStorageGet(void);

#endif // LANGUAGE_STDLIB_SHIMS_THREADLOCALSTORAGE_H
