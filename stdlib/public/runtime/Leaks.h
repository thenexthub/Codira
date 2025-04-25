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

#ifndef SWIFT_STDLIB_RUNTIME_LEAKS_H
#define SWIFT_STDLIB_RUNTIME_LEAKS_H

#if SWIFT_RUNTIME_ENABLE_LEAK_CHECKER

#include "language/shims/Visibility.h"

#include "language/Runtime/Config.h"

namespace language {
struct HeapObject;
}

SWIFT_CC(swift)
SWIFT_RUNTIME_EXPORT SWIFT_NOINLINE SWIFT_USED void
_swift_leaks_startTrackingObjects(const char *);

SWIFT_CC(swift)
SWIFT_RUNTIME_EXPORT SWIFT_NOINLINE SWIFT_USED int
_swift_leaks_stopTrackingObjects(const char *);

SWIFT_RUNTIME_EXPORT SWIFT_NOINLINE SWIFT_USED void
_swift_leaks_startTrackingObject(swift::HeapObject *);

SWIFT_RUNTIME_EXPORT SWIFT_NOINLINE SWIFT_USED void
_swift_leaks_stopTrackingObject(swift::HeapObject *);

#define SWIFT_LEAKS_START_TRACKING_OBJECT(obj)                                 \
  _swift_leaks_startTrackingObject(obj)
#define SWIFT_LEAKS_STOP_TRACKING_OBJECT(obj)                                  \
  _swift_leaks_stopTrackingObject(obj)

// SWIFT_RUNTIME_ENABLE_LEAK_CHECKER
#else
// not SWIFT_RUNTIME_ENABLE_LEAK_CHECKER

#define SWIFT_LEAKS_START_TRACKING_OBJECT(obj)
#define SWIFT_LEAKS_STOP_TRACKING_OBJECT(obj)

#endif

#endif
