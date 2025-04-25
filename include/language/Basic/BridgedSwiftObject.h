//===--- BridgedSwiftObject.h - C header which defines SwiftObject --------===//
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
// This is a C header, which defines the SwiftObject header. For the C++ version
// see SwiftObjectHeader.h.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_BASIC_BRIDGEDSWIFTOBJECT_H
#define SWIFT_BASIC_BRIDGEDSWIFTOBJECT_H

#include "language/Basic/Nullability.h"

#if defined(__OpenBSD__)
#include <sys/stdint.h>
#else
#include <stdint.h>
#endif

#if !defined(__has_feature)
#define __has_feature(feature) 0
#endif

SWIFT_BEGIN_NULLABILITY_ANNOTATIONS

typedef const void * _Nonnull SwiftMetatype;

/// The header of a Swift object.
///
/// This must be in sync with HeapObject, which is defined in the runtime lib.
/// It must be layout compatible with the Swift object header.
struct BridgedSwiftObject {
  SwiftMetatype metatype;
  int64_t refCounts;
};

typedef struct BridgedSwiftObject * _Nonnull SwiftObject;
typedef struct BridgedSwiftObject * _Nullable OptionalSwiftObject;

SWIFT_END_NULLABILITY_ANNOTATIONS

#endif
