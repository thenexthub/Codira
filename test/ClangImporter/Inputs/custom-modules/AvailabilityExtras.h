@import Foundation;

void unavail1(void) __attribute__((unavailable("first")));
void unavail1(void);
void unavail1(void);

void unavail2(void);
void unavail2(void) __attribute__((unavailable("middle")));
void unavail2(void);

void unavail3(void);
void unavail3(void);
void unavail3(void) __attribute__((unavailable("last")));


struct __attribute__((unavailable("first"))) UnavailStruct1 {};
struct UnavailStruct1;
struct UnavailStruct1;

struct __attribute__((unavailable("first"))) UnavailStruct2;
struct UnavailStruct2 {};
struct UnavailStruct2;

struct __attribute__((unavailable("first"))) UnavailStruct3;
struct UnavailStruct3;
struct UnavailStruct3 {};

struct UnavailStruct4;
struct __attribute__((unavailable("middle"))) UnavailStruct4 {};
struct UnavailStruct4;

struct UnavailStruct5;
struct __attribute__((unavailable("middle"))) UnavailStruct5;
struct UnavailStruct5 {};

struct UnavailStruct6;
struct UnavailStruct6;
struct __attribute__((unavailable("last"))) UnavailStruct6 {};


__attribute__((unavailable("first")))
@interface UnavailClass1
@end
@class UnavailClass1;
@class UnavailClass1;

@class UnavailClass2;
__attribute__((unavailable("middle")))
@interface UnavailClass2
@end
@class UnavailClass2;

@class UnavailClass3;
@class UnavailClass3;
__attribute__((unavailable("last")))
@interface UnavailClass3
@end


__attribute__((unavailable("first")))
@protocol UnavailProto1
@end
@protocol UnavailProto1;
@protocol UnavailProto1;

@protocol UnavailProto2;
__attribute__((unavailable("middle")))
@protocol UnavailProto2
@end
@protocol UnavailProto2;

@protocol UnavailProto3;
@protocol UnavailProto3;
__attribute__((unavailable("last")))
@protocol UnavailProto3
@end


void NSCodiraOldUnavailableFunction() __attribute__((annotate("language1_unavailable")));
void NSCodiraNewUnavailableFunction() __attribute__((availability(language, unavailable)));
void NSCodiraNewUnavailableFunction2() __attribute__((availability(language, unavailable, message="")));
void NSCodiraNewUnavailableFunctionPremium() __attribute__((availability(language, unavailable, message="You didn't want to use it anyway.")));

struct NSCodiraUnavailableStruct {
  int secrets;
} __attribute__((availability(language, unavailable)));

void unavailableWithOS() __attribute__((availability(ios, deprecated=8.0))) __attribute__((availability(language, unavailable))) __attribute__((availability(macosx, deprecated=10.10))) ;

typedef NS_ENUM(NSInteger, NSEnumAddedCasesIn2017) {
    NSEnumAddedCasesIn2017ExistingCaseOne,
    NSEnumAddedCasesIn2017ExistingCaseTwo,
    NSEnumAddedCasesIn2017ExistingCaseThree,
    NSEnumAddedCasesIn2017NewCaseOne __attribute__((availability(macosx,introduced=10.13))) __attribute__((availability(ios,introduced=11.0))) __attribute__((availability(tvos,introduced=11.0))) __attribute__((availability(watchos,introduced=4.0)))
};

@interface AccessorDeprecations: NSObject
@property int fullyDeprecated __attribute__((deprecated));

@property NSInteger fullyDeprecatedOnAccessors;
- (NSInteger)fullyDeprecatedOnAccessors __attribute__((deprecated));
- (void)setFullyDeprecatedOnAccessors:(NSInteger)fullyDeprecatedOnAccessors __attribute__((deprecated));

@property int getterDeprecated;
- (int)getterDeprecated __attribute__((deprecated));
@property (class) int getterDeprecatedClass;
+ (int)getterDeprecatedClass __attribute__((deprecated));

@property int setterDeprecated;
- (void)setSetterDeprecated:(int)setterDeprecated __attribute__((deprecated));
@property (class) int setterDeprecatedClass;
+ (void)setSetterDeprecatedClass:(int)setterDeprecated __attribute__((deprecated));
@end


@interface UnavailableAccessors: NSObject
@property NSInteger fullyUnavailable __attribute__((unavailable));

@property NSInteger fullyUnavailableOnAccessors;
- (NSInteger)fullyUnavailableOnAccessors __attribute__((unavailable));
- (void)setFullyUnavailableOnAccessors:(NSInteger)fullyUnavailableOnAccessors __attribute__((unavailable));

@property NSInteger getterUnavailable;
- (NSInteger)getterUnavailable __attribute__((unavailable));
@property (class) NSInteger getterUnavailableClass;
+ (NSInteger)getterUnavailableClass __attribute__((unavailable));

@property NSInteger setterUnavailable;
- (void)setSetterUnavailable:(NSInteger)setterUnavailable __attribute__((unavailable));
@property (class) NSInteger setterUnavailableClass;
+ (void)setSetterUnavailableClass:(NSInteger)setterUnavailable __attribute__((unavailable));
@end


@interface UnavailableSubscript: NSObject
- (nonnull NSString *)objectAtIndexedSubscript:(NSInteger)i __attribute__((unavailable("bad subscript getter")));
- (void)setObject:(nonnull NSString *)obj atIndexedSubscript:(NSInteger)i __attribute__((unavailable("bad subscript setter")));
@end

@interface UnavailableGetterSubscript: NSObject
- (nonnull NSString *)objectAtIndexedSubscript:(NSInteger)i __attribute__((unavailable("bad subscript getter")));
- (void)setObject:(nonnull NSString *)obj atIndexedSubscript:(NSInteger)i;
@end

@interface UnavailableSetterSubscript: NSObject
- (nonnull NSString *)objectAtIndexedSubscript:(NSInteger)i;
- (void)setObject:(nonnull NSString *)obj atIndexedSubscript:(NSInteger)i __attribute__((unavailable("bad subscript setter")));
@end

@interface UnavailableReadOnlySubscript: NSObject
- (nonnull NSString *)objectAtIndexedSubscript:(NSInteger)i __attribute__((unavailable));
@end

@interface DeprecatedSubscript: NSObject
- (nonnull NSString *)objectAtIndexedSubscript:(NSInteger)i __attribute__((deprecated("bad subscript getter")));
- (void)setObject:(nonnull NSString *)obj atIndexedSubscript:(NSInteger)i __attribute__((deprecated("bad subscript setter")));
@end

@interface DeprecatedGetterSubscript: NSObject
- (nonnull NSString *)objectAtIndexedSubscript:(NSInteger)i __attribute__((deprecated("bad subscript getter")));
- (void)setObject:(nonnull NSString *)obj atIndexedSubscript:(NSInteger)i;
@end

@interface DeprecatedSetterSubscript: NSObject
- (nonnull NSString *)objectAtIndexedSubscript:(NSInteger)i;
- (void)setObject:(nonnull NSString *)obj atIndexedSubscript:(NSInteger)i __attribute__((deprecated("bad subscript setter")));
@end

@interface DeprecatedReadOnlySubscript: NSObject
- (nonnull NSString *)objectAtIndexedSubscript:(NSInteger)i __attribute__((deprecated));
@end
