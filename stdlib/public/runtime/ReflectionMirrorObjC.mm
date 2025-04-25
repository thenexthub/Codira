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

#if SWIFT_OBJC_INTEROP && defined(SWIFT_ENABLE_REFLECTION)

#include "Private.h"
#include <Foundation/Foundation.h>
#include <objc/objc.h>
#include <objc/runtime.h>

using namespace language;

// Declare the debugQuickLookObject selector.
@interface DeclareSelectors
- (id)debugQuickLookObject;
@end

SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_INTERNAL
id swift::_quickLookObjectForPointer(void *value) {
  id object = [*reinterpret_cast<const id *>(value) retain];
  if ([object respondsToSelector:@selector(debugQuickLookObject)]) {
    id quickLookObject = [object debugQuickLookObject];
    [quickLookObject retain];
    [object release];
    return quickLookObject;
  }

  return object;
}

#endif
