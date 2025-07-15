// This file is meant to be included with modules turned off, compiled against
// the fake clang-importer-sdk.
#import <Foundation.h>

@interface Outer : NSObject
@end

__attribute__((language_name("Outer.Inner")))
@interface InnerClass : NSObject
@end

struct __attribute__((language_name("Outer.InnerV"))) InnerStruct {
  int value;
};

typedef struct {
  int value;
} InnerAnonStruct __attribute__((language_name("Outer.InnerAS")));

typedef int InnerAlias __attribute__((language_name("Outer.InnerA")));


@interface NullabilityBase : NSObject
- (nonnull instancetype)initFormerlyFailableValue:(NSInteger)value __attribute__((objc_designated_initializer));
- (void)processNowNullableArgument:(nullable NSObject *)object;
@end
