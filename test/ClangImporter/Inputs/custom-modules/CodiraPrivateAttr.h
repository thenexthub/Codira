@import Foundation;

#define NS_REFINED_FOR_LANGUAGE __attribute__((language_private))

NS_REFINED_FOR_LANGUAGE
@protocol PrivProto
@end
  
@interface Foo : NSObject <PrivProto>
@property id privValue NS_REFINED_FOR_LANGUAGE;
@property (class) id privClassValue NS_REFINED_FOR_LANGUAGE;

- (void)noArgs NS_REFINED_FOR_LANGUAGE;
- (void)oneArg:(int)arg NS_REFINED_FOR_LANGUAGE;
- (void)twoArgs:(int)arg other:(int)arg2 NS_REFINED_FOR_LANGUAGE;

+ (instancetype)foo NS_REFINED_FOR_LANGUAGE;
+ (instancetype)fooWithNoArgs NS_REFINED_FOR_LANGUAGE;
+ (instancetype)fooWithOneArg:(int)arg NS_REFINED_FOR_LANGUAGE;
+ (instancetype)fooWithTwoArgs:(int)arg other:(int)arg2 NS_REFINED_FOR_LANGUAGE;
+ (instancetype)foo:(int)arg NS_REFINED_FOR_LANGUAGE;

- (id)objectForKeyedSubscript:(id)index;
- (void)setObject:(id)object forKeyedSubscript:(id)index NS_REFINED_FOR_LANGUAGE;

- (id)objectAtIndexedSubscript:(int)index NS_REFINED_FOR_LANGUAGE;
- (void)setObject:(id)object atIndexedSubscript:(int)index;

@end

@interface Bar : NSObject
- (instancetype)init NS_REFINED_FOR_LANGUAGE;
- (instancetype)initWithNoArgs NS_REFINED_FOR_LANGUAGE;
- (instancetype)initWithOneArg:(int)arg NS_REFINED_FOR_LANGUAGE;
- (instancetype)initWithTwoArgs:(int)arg other:(int)arg2 NS_REFINED_FOR_LANGUAGE;
- (instancetype)init:(int)arg NS_REFINED_FOR_LANGUAGE;
@end

NS_REFINED_FOR_LANGUAGE
@interface PrivFooSub : Foo
@end

void privTest() NS_REFINED_FOR_LANGUAGE;

struct S0 {
  int privValue NS_REFINED_FOR_LANGUAGE;
};

struct PrivS1 {
  int value;
} NS_REFINED_FOR_LANGUAGE;

typedef struct {
  int value;
} PrivS2 NS_REFINED_FOR_LANGUAGE;

enum /* anonymous */ {
  PrivAnonymousA NS_REFINED_FOR_LANGUAGE
};

enum E0 {
  E0PrivA NS_REFINED_FOR_LANGUAGE
};

enum PrivE1 {
  PrivE1A
} NS_REFINED_FOR_LANGUAGE;

typedef enum {
  PrivE2A
} PrivE2 NS_REFINED_FOR_LANGUAGE;

typedef NS_ENUM(long, PrivNSEnum) {
  PrivNSEnumA
} NS_REFINED_FOR_LANGUAGE;

typedef NS_ENUM(long, NSEnum) {
  NSEnumPrivA NS_REFINED_FOR_LANGUAGE,
  NSEnumB
};

typedef NS_OPTIONS(long, PrivNSOptions) {
  PrivNSOptionsA = 1
} NS_REFINED_FOR_LANGUAGE;

typedef NS_OPTIONS(long, NSOptions) {
  NSOptionsPrivA NS_REFINED_FOR_LANGUAGE = 1,
  NSOptionsB = 2
};

typedef struct __attribute__((objc_bridge(id))) _PrivCFTypeRef *PrivCFTypeRef NS_REFINED_FOR_LANGUAGE;
typedef PrivCFTypeRef PrivCFSubRef NS_REFINED_FOR_LANGUAGE;
typedef int PrivInt NS_REFINED_FOR_LANGUAGE;
