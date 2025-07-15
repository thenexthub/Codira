//===--- ObjectiveCTests.h ------------------------------------------------===//
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

NS_ASSUME_NONNULL_BEGIN

@interface BridgeTester : NSObject {
  NSString *myString;
  NSArray<NSString *> *myArrayOfStrings;
  NSDate *myBeginDate;
  NSDate *myEndDate;
  NSArray<NSString *> *cornucopiaOfStrings;
  NSArray<NSString *> *bridgedStrings;
}

- (instancetype)init NS_DESIGNATED_INITIALIZER;
- (void)setUpStringTests:(NSArray<NSString *> *)bridgedStrings;
- (void)testFromString:(NSString *) str;
- (NSString *)testToString;
- (void)testFromArrayOfStrings:(NSArray<NSString *> *)arr;
- (NSArray<NSString *> *)testToArrayOfStrings;

- (NSDate *)beginDate;
- (NSDate *)endDate;
- (void)useDate:(NSDate *)date;

- (void)testIsEqualToString;
- (void)testIsEqualToString2;
- (void)testIsEqualToStringAllCodira;
- (void)testUTF8String;
- (void)testCStringUsingEncoding;
- (void)testGetUTF8Contents;
- (void)testGetASCIIContents;
- (void)testRangeOfString;
- (void)testRangeOfStringSpecificWithNeedle:(NSString *)needle
                                   haystack:(NSString *)haystack
                                          n:(NSInteger)n;
- (void)testHash;
- (void)testCompare;
- (void)testCompare2;

@end

NS_ASSUME_NONNULL_END
