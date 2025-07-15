//===--- Mirror.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_TEST_1_STDLIB_INPUTS_MIRROR_H
#define LANGUAGE_TEST_1_STDLIB_INPUTS_MIRROR_H

#import <Foundation/NSObject.h>

@interface HasIVars : NSObject
@end

@interface FooObjCClass : NSObject
@end

@interface FooDerivedObjCClass : FooObjCClass
@end

@interface FooMoreDerivedObjCClass : FooDerivedObjCClass
@end

@interface StringConvertibleInDebugAndOtherwise_ObjC : NSObject
@end

#endif
