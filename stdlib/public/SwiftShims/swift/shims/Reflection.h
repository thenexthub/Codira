//===--- Reflection.h - Types for access to reflection metadata. ----------===//
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

#ifndef SWIFT_STDLIB_SHIMS_REFLECTION_H
#define SWIFT_STDLIB_SHIMS_REFLECTION_H

#include "languageStdbool.h"
#include "languageStdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*NameFreeFunc)(const char*);

typedef struct _FieldReflectionMetadata {
  const char* name;
  NameFreeFunc freeFunc;
  __swift_bool isStrong;
  __swift_bool isVar;
} _FieldReflectionMetadata;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SWIFT_STDLIB_SHIMS_REFLECTION_H
