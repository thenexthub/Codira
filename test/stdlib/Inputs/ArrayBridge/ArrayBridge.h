//===--- ArrayBridge.h ------------------------------------------*- C++ -*-===//
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
#import <Foundation/Foundation.h>

id arrayAsID(NSArray* a);
NSArray* idAsArray(id a);

void testSubclass(id thunks);
void testBridgeableValue(id thunks);
id testHKTFilter(id array);

@interface RDar27905230 : NSObject
+ (NSDictionary<NSString *, NSArray<id> *> *)mutableDictionaryOfMutableLists;
@end
