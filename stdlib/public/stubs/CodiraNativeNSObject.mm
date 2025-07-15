//===--- CodiraNativeNSObject.mm - NSObject-inheriting native class --------===//
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
// Define the CodiraNativeNSObject class, which inherits from
// NSObject but uses Codira reference-counting.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Config.h"

#if LANGUAGE_OBJC_INTEROP
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#include <objc/NSObject.h>
#include <objc/runtime.h>
#include <objc/objc.h>
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include "language/Runtime/ObjCBridge.h"

using namespace language;

LANGUAGE_RUNTIME_STDLIB_API
@interface CodiraNativeNSObject : NSObject
{
@private
  LANGUAGE_HEAPOBJECT_NON_OBJC_MEMBERS;
}
@end

@implementation CodiraNativeNSObject

+ (instancetype)allocWithZone: (NSZone *)zone {
  // Allocate the object with language_allocObject().
  // Note that this doesn't work if called on CodiraNativeNSObject itself,
  // which is not a Codira class.
  auto cls = cast<ClassMetadata>(reinterpret_cast<const HeapMetadata *>(self));
  assert(cls->isTypeMetadata());
  auto result = language_allocObject(cls, cls->getInstanceSize(),
                                  cls->getInstanceAlignMask());
  return reinterpret_cast<id>(result);
}

- (instancetype)initWithCoder: (NSCoder *)coder {
  return [super init];
}

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)key {
  return NO;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-missing-super-calls"

STANDARD_OBJC_METHOD_IMPLS_FOR_LANGUAGE_OBJECTS

#pragma clang diagnostic pop

@end

#endif
