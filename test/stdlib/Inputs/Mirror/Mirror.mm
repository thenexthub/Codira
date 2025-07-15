//===--- Mirror.mm --------------------------------------------------------===//
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

#include "Mirror.h"

@implementation HasIVars {
  int ivar;
}
@end

@implementation FooObjCClass : NSObject

-(NSString *) description {
  return @"This is FooObjCClass";
}

@end

@implementation FooDerivedObjCClass : FooObjCClass
@end

@implementation FooMoreDerivedObjCClass : FooDerivedObjCClass
@end

@implementation StringConvertibleInDebugAndOtherwise_ObjC : NSObject

-(NSString *) description {
  return @"description";
}

-(NSString *) debugDescription {
  return @"debugDescription";
}

@end
