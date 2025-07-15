@import Foundation;

#define LANGUAGE_NAME(X) __attribute__((language_name(#X)))

#pragma clang assume_nonnull begin
@interface CodiraNameTest : NSObject
+ (instancetype)g:(id)x outParam:(int *)foo LANGUAGE_NAME(init(g:));
@end
#pragma clang assume_nonnull end
