//===--- FoundationBridge.h -------------------------------------*- C++ -*-===//
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

typedef NS_ENUM(NSInteger, ObjectBehaviorAction) {
    ObjectBehaviorActionRetain,
    ObjectBehaviorActionCopy,
    ObjectBehaviorActionMutableCopy
};

// NOTE: this class is NOT meant to be used in threaded contexts.
@interface ObjectBehaviorVerifier : NSObject
@property (readonly) BOOL wasRetained;
@property (readonly) BOOL wasCopied;
@property (readonly) BOOL wasMutableCopied;

- (void)appendAction:(ObjectBehaviorAction)action;
- (void)enumerate:(void (^)(ObjectBehaviorAction))block;
- (void)reset;
- (void)dump;
@end

#pragma mark - NSData verification

@interface ImmutableDataVerifier : NSData {
    ObjectBehaviorVerifier *_verifier;
    NSData *_data;
}
@property (readonly) ObjectBehaviorVerifier *verifier;
@end

@interface MutableDataVerifier : NSMutableData {
    ObjectBehaviorVerifier *_verifier;
    NSMutableData *_data;
}
@property (readonly) ObjectBehaviorVerifier *verifier;
@end

void takesData(NSData *object);
NSData *returnsData();
BOOL identityOfData(NSData *data);

#pragma mark - NSCalendar verification

@interface CalendarBridgingTester : NSObject
- (NSCalendar *)autoupdatingCurrentCalendar;
- (BOOL)verifyAutoupdatingCalendar:(NSCalendar *)calendar;
@end

@interface TimeZoneBridgingTester : NSObject
- (NSTimeZone *)autoupdatingCurrentTimeZone;
- (BOOL)verifyAutoupdatingTimeZone:(NSTimeZone *)tz;
@end

@interface LocaleBridgingTester : NSObject
- (NSLocale *)autoupdatingCurrentLocale;
- (BOOL)verifyAutoupdatingLocale:(NSLocale *)locale;
@end

#pragma mark - NSNumber verification

@interface NumberBridgingTester : NSObject
- (BOOL)verifyKeysInRange:(NSRange)range existInDictionary:(NSDictionary *)dictionary;
@end

#pragma mark - NSString bridging

static inline NSString *getNSStringEqualTestString() {
  return [NSString stringWithUTF8String:"2166002315@874404110.1042078977"];
}

static inline BOOL NSStringBridgeTestEqual(NSString * _Nonnull a, NSString * _Nonnull b) {
  return [a isEqual:b];
}

static inline NSString *getNSStringWithUnpairedSurrogate() {
  unichar chars[16] = {
    0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
    0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
    0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
    0xD800 };
  return [NSString stringWithCharacters:chars length:1];
}

NS_ASSUME_NONNULL_END
