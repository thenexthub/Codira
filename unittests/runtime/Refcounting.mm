//===--- Refcounting.mm - Reference-counting for ObjC ---------------------===//
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

#include <Foundation/NSObject.h>
#include <objc/runtime.h>
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include "gtest/gtest.h"

using namespace language;

static unsigned DestroyedObjCCount = 0;
/// A trivial class that increments DestroyedObjCCount when deallocated.
@interface ObjCTestClass : NSObject @end
@implementation ObjCTestClass
- (void) dealloc {
  DestroyedObjCCount++;
  [super dealloc];
}
@end
static HeapObject *make_objc_object() {
  return static_cast<HeapObject *>([ObjCTestClass new]);
}

TEST(RefcountingTest, objc_unknown_retain_release_n) {
  auto object = make_objc_object();
  language_unknownObjectRetain_n(object, 32);
  language_unknownObjectRetain(object);
  language_unknownObjectRelease_n(object, 31);
  language_unknownObjectRelease(object);
  language_unknownObjectRelease_n(object, 1);
  language_unknownObjectRelease(object);
  // The object should be destroyed by now.
  EXPECT_EQ(1u, DestroyedObjCCount);
}
