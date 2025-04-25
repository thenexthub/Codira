//===--- UnavailableFoundationMethodThunks.mm -----------------------------===//
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

#include <Foundation/Foundation.h>
#include "language/Runtime/Config.h"

SWIFT_CC(swift) SWIFT_RUNTIME_LIBRARY_VISIBILITY
extern "C" void
NSArray_getObjects(NSArray *_Nonnull nsArray,
                   id *objects, NSUInteger rangeLocation,
                   NSUInteger rangeLength) {
  [nsArray getObjects:objects range:NSMakeRange(rangeLocation, rangeLength)];
}

SWIFT_CC(swift) SWIFT_RUNTIME_LIBRARY_VISIBILITY
extern "C" void
NSDictionary_getObjectsAndKeysWithCount(NSDictionary *_Nonnull nsDictionary,
                                        id *objects, id *keys,
                                        NSInteger count) {
  [nsDictionary getObjects:objects andKeys:keys count:count];
}

