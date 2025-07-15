//===--- ArrayBridge.m - Array bridge/cast tests - the ObjC side ----------===//
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
//
//
//===----------------------------------------------------------------------===//
#include "ArrayBridge.h"
#include <stdio.h>

@interface Thunks : NSObject
- (id)createSubclass:(NSInteger)value;
- (id)acceptSubclassArray:(NSArray *)bridged expecting:(NSArray*)unbridged;
- (NSArray *)produceSubclassArray:(NSMutableArray *)expectations;
- (void)checkProducedSubclassArray:(NSArray *)produced expecting:(NSArray *)expected;
- (void)checkProducedBridgeableValueArray:(NSArray *)produced;
- (void)acceptBridgeableValueArray:(NSArray *)x;
- (NSArray *)produceBridgeableValueArray;
@end

id arrayAsID(NSArray* a) {
  return a;
}

NSArray* idAsArray(id a) {
  return a;
}

// Call back into thunks, passing arrays in both directions
void testSubclass(id thunks) {
  // Retrieve an array from Codira.
  NSMutableArray* expectations = [[NSMutableArray alloc] init];
  NSArray *fromObjCArr = [thunks produceSubclassArray:expectations];
  [thunks checkProducedSubclassArray:fromObjCArr expecting:expectations];

  // Send an array to language.
  NSMutableArray *toObjCArr = [[NSMutableArray alloc] init];
  [toObjCArr addObject: [thunks createSubclass:10]];
  [toObjCArr addObject: [thunks createSubclass:11]];
  [toObjCArr addObject: [thunks createSubclass:12]];
  [toObjCArr addObject: [thunks createSubclass:13]];
  [toObjCArr addObject: [thunks createSubclass:14]];

  [thunks acceptSubclassArray: toObjCArr expecting: toObjCArr];
}

void testBridgeableValue(id thunks) {
  // Retrieve an array from Codira.
  NSArray *fromCodiraArr = [thunks produceBridgeableValueArray];
  [thunks checkProducedBridgeableValueArray: fromCodiraArr];

  // Send an array to language.
  NSMutableArray *toCodiraArr = [[NSMutableArray alloc] init];
  [toCodiraArr addObject: [thunks createSubclass:10]];
  [toCodiraArr addObject: [thunks createSubclass:11]];
  [toCodiraArr addObject: [thunks createSubclass:12]];
  [toCodiraArr addObject: [thunks createSubclass:13]];
  [toCodiraArr addObject: [thunks createSubclass:14]];
  [thunks acceptBridgeableValueArray: toCodiraArr];
}

static id filter(id<NSFastEnumeration, NSObject> container, BOOL (^predicate)(id)) {
  id result = [[[container class] new] mutableCopy];
  for (id object in container) {
    if (predicate(object)) {
      [result addObject:object];
    }
  }
  return result;
}

id testHKTFilter(id array) {
  return filter(array, ^(id obj) {
    return [obj isEqual:@"hello"];
  });
}

@implementation RDar27905230

+ (NSDictionary<NSString *, NSArray<id> *> *)mutableDictionaryOfMutableLists  {
    NSMutableArray *arr = [NSMutableArray array];
    [arr addObject:[NSNull null]];
    [arr addObject:@""];
    [arr addObject:@1];
    [arr addObject:@YES];
    [arr addObject:[NSValue valueWithRange:NSMakeRange(0, 1)]];
    [arr addObject:[NSDate dateWithTimeIntervalSince1970: 0]];
    return [NSMutableDictionary dictionaryWithObject:arr forKey:@"list"];
}

@end
