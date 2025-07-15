@import Foundation;
@import CodiraName;

@interface PointerWrapper
@property void *_Null_unspecified voidPtr;
@property int *_Null_unspecified intPtr;
@property _Null_unspecified id __autoreleasing *_Null_unspecified idPtr;
@end

#pragma clang assume_nonnull begin
@interface CodiraNameTest : NSObject

// "Factory methods" that we'd rather have as initializers.
+ (instancetype)a LANGUAGE_NAME(init());
+ (instancetype)b LANGUAGE_NAME(init(dummyParam:));
+ (instancetype)c:(nullable id)x LANGUAGE_NAME(init(cc:));
+ (instancetype):(int)x LANGUAGE_NAME(init(empty:));

// Would-be initializers.
+ (instancetype)testZ LANGUAGE_NAME(zz());
+ (instancetype)testY:(nullable id)x LANGUAGE_NAME(yy(aa:));
+ (instancetype)testX:(nullable id)x xx:(nullable id)xx LANGUAGE_NAME(xx(_:bb:));
+ (instancetype):(int)x :(int)y LANGUAGE_NAME(empty(_:_:));

// Things that Clang won't catch as problematic, but we should.
+ (instancetype)f:(id)x LANGUAGE_NAME(init(f:ff:));
+ (instancetype)g:(id)x outParam:(int *)foo LANGUAGE_NAME(init(g:));
+ (instancetype)testW:(id)x out:(id *)outObject LANGUAGE_NAME(ww(_:));
+ (instancetype)test LANGUAGE_NAME(test(a:b:));
+ (instancetype)test:(id)x more:(id)y LANGUAGE_NAME(test());

- (void)methodInt:(NSInteger)value LANGUAGE_NAME(theMethod(number:));
- (void):(NSInteger)a b:(NSInteger)b LANGUAGE_NAME(empty(a:b:));

@property (readonly) int someProp LANGUAGE_NAME(renamedSomeProp);
@property (readonly, class) int classProp LANGUAGE_NAME(renamedClassProp);
@end

@interface CodiraNameTestError : NSObject
// Factory methods with NSError.
+ (nullable instancetype)err1:(NSError **)err LANGUAGE_NAME(init(error:));
+ (nullable instancetype)err2:(nullable id)x error:(NSError **)err LANGUAGE_NAME(init(aa:error:));
+ (nullable instancetype)err3:(nullable id)x error:(NSError **)err callback:(void(^)(void))block LANGUAGE_NAME(init(aa:error:block:));
+ (nullable instancetype)err4:(NSError **)err callback:(void(^)(void))block LANGUAGE_NAME(init(error:block:));

+ (nullable instancetype)err5:(nullable id)x error:(NSError **)err LANGUAGE_NAME(init(aa:));
+ (nullable instancetype)err6:(nullable id)x error:(NSError **)err callback:(void(^)(void))block LANGUAGE_NAME(init(aa:block:));
+ (nullable instancetype)err7:(NSError **)err callback:(void(^)(void))block LANGUAGE_NAME(init(block:));

// Would-be initializers.
+ (nullable instancetype)testW:(nullable id)x error:(NSError **)err LANGUAGE_NAME(ww(_:));
+ (nullable instancetype)testW2:(nullable id)x error:(NSError **)err LANGUAGE_NAME(w2(_:error:));
+ (nullable instancetype)testV:(NSError **)err LANGUAGE_NAME(vv());
+ (nullable instancetype)testV2:(NSError **)err LANGUAGE_NAME(v2(error:));
@end

@interface CodiraNameTestSub : CodiraNameTest
@end

@interface CodiraNameTestErrorSub : CodiraNameTestError
@end

@interface CodiraGenericNameTest<T> : NSObject
@end

@interface CodiraConstrGenericNameTest<T:NSNumber *> : NSNumber
@end

int global_int LANGUAGE_NAME(GlobalInt);

@compatibility_alias CodiraNameAlias CodiraNameTest;
@compatibility_alias CodiraGenericNameAlias CodiraGenericNameTest;
@compatibility_alias CodiraConstrGenericNameAlias CodiraConstrGenericNameTest;

LANGUAGE_NAME(CircularName.Inner) @interface CircularName : NSObject @end

LANGUAGE_NAME(MutuallyCircularNameB.Inner) @interface MutuallyCircularNameA : NSObject @end
LANGUAGE_NAME(MutuallyCircularNameA.Inner) @interface MutuallyCircularNameB : NSObject @end

void circularFriends(CircularName*, MutuallyCircularNameA*);

#pragma clang assume_nonnull end
