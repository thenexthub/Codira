//===----------------------------------------------------------------------===//
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

#include "language/Runtime/Config.h"

#if SWIFT_OBJC_INTEROP
#import <objc/runtime.h>
#import <objc/NSObject.h>

SWIFT_CC(swift)
SWIFT_RUNTIME_STDLIB_API
bool _swift_stdlib_NSObject_isKindOfClass(
    id _Nonnull object,
    char * _Nonnull className) {
  Class cls = objc_lookUpClass(className);
  return [object isKindOfClass:cls];
}
#endif

