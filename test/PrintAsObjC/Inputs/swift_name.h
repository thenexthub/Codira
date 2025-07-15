// This file is meant to be used with the mock SDK, not the real one.
#import <Foundation.h>

#define LANGUAGE_NAME(x) __attribute__((language_name(#x)))

typedef NSString *ABCStringAlias LANGUAGE_NAME(ZZStringAlias);
struct ABCPoint {
  int x;
  int y;
} LANGUAGE_NAME(ZZPoint);
enum ABCAlignment {
  ABCAlignmentLeft,
  ABCAlignmentRight
} LANGUAGE_NAME(ZZAlignment);

LANGUAGE_NAME(ZZClass)
@interface ABCClass : NSObject
@end

LANGUAGE_NAME(ZZProto)
@protocol ABCProto
@end
