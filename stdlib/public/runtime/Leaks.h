//===--- Leaks.h ------------------------------------------------*- C++ -*-===//
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
// This is a very simple leak detector implementation that detects objects that
// are allocated but not deallocated in a region. It is purposefully behind a
// flag since it is not meant to be used in production yet.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_RUNTIME_LEAKS_H
#define LANGUAGE_STDLIB_RUNTIME_LEAKS_H

#if LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER

#include "language/shims/Visibility.h"

#include "language/Runtime/Config.h"

namespace language {
struct HeapObject;
}

LANGUAGE_CC(language)
LANGUAGE_RUNTIME_EXPORT LANGUAGE_NOINLINE LANGUAGE_USED void
_language_leaks_startTrackingObjects(const char *);

LANGUAGE_CC(language)
LANGUAGE_RUNTIME_EXPORT LANGUAGE_NOINLINE LANGUAGE_USED int
_language_leaks_stopTrackingObjects(const char *);

LANGUAGE_RUNTIME_EXPORT LANGUAGE_NOINLINE LANGUAGE_USED void
_language_leaks_startTrackingObject(language::HeapObject *);

LANGUAGE_RUNTIME_EXPORT LANGUAGE_NOINLINE LANGUAGE_USED void
_language_leaks_stopTrackingObject(language::HeapObject *);

#define LANGUAGE_LEAKS_START_TRACKING_OBJECT(obj)                                 \
  _language_leaks_startTrackingObject(obj)
#define LANGUAGE_LEAKS_STOP_TRACKING_OBJECT(obj)                                  \
  _language_leaks_stopTrackingObject(obj)

// LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER
#else
// not LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER

#define LANGUAGE_LEAKS_START_TRACKING_OBJECT(obj)
#define LANGUAGE_LEAKS_STOP_TRACKING_OBJECT(obj)

#endif

#endif
