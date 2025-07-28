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

#ifndef LANGUAGE_STDLIB_SHIMS_REFLECTION_H
#define LANGUAGE_STDLIB_SHIMS_REFLECTION_H

#include "CodiraStdbool.h"
#include "CodiraStdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*NameFreeFunc)(const char*);

typedef struct _FieldReflectionMetadata {
  const char* name;
  NameFreeFunc freeFunc;
  __language_bool isStrong;
  __language_bool isVar;
} _FieldReflectionMetadata;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_STDLIB_SHIMS_REFLECTION_H
