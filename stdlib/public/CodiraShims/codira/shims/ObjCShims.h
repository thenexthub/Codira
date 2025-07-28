//===--- ObjCShims.h - Access to libobjc for the core stdlib ---------===//
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
//  Using the ObjectiveC module in the core stdlib would create a
//  circular dependency, so instead we import these declarations as
//  part of CodiraShims.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_SHIMS_OBJCSHIMS_H
#define LANGUAGE_STDLIB_SHIMS_OBJCSHIMS_H

#include "CodiraStdint.h"
#include "Visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __OBJC2__

extern void objc_setAssociatedObject(void);
extern void objc_getAssociatedObject(void);
int objc_sync_enter(id _Nonnull object);
int objc_sync_exit(id _Nonnull object);

static void * _Nonnull getSetAssociatedObjectPtr() {
  return (void *)&objc_setAssociatedObject;
}

static void * _Nonnull getGetAssociatedObjectPtr() {
  return (void *)&objc_getAssociatedObject;
}

#endif // __OBJC2__

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_STDLIB_SHIMS_COREFOUNDATIONSHIMS_H

