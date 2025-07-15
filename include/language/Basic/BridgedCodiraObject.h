//===--- BridgedCodiraObject.h - C header which defines CodiraObject --------===//
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
// This is a C header, which defines the CodiraObject header. For the C++ version
// see LanguageObjectHeader.h.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_BRIDGEDLANGUAGEOBJECT_H
#define LANGUAGE_BASIC_BRIDGEDLANGUAGEOBJECT_H

#include "language/Basic/Nullability.h"

#if defined(__OpenBSD__)
#include <sys/stdint.h>
#else
#include <stdint.h>
#endif

#if !defined(__has_feature)
#define __has_feature(feature) 0
#endif

LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS

typedef const void * _Nonnull CodiraMetatype;

/// The header of a Codira object.
///
/// This must be in sync with HeapObject, which is defined in the runtime lib.
/// It must be layout compatible with the Codira object header.
struct BridgedCodiraObject {
  CodiraMetatype metatype;
  int64_t refCounts;
};

typedef struct BridgedCodiraObject * _Nonnull CodiraObject;
typedef struct BridgedCodiraObject * _Nullable OptionalCodiraObject;

LANGUAGE_END_NULLABILITY_ANNOTATIONS

#endif
