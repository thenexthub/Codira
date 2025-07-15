#include <Foundation/Foundation.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-extension"
#pragma clang assume_nonnull begin

struct CxxRefType {
} __attribute__((language_attr("import_reference")))
__attribute__((language_attr("retain:retainCxxRefType")))
__attribute__((language_attr("release:releaseCxxRefType")));

struct CxxValType {};

void retainCxxRefType(CxxRefType *_Nonnull b) {}
void releaseCxxRefType(CxxRefType *_Nonnull b) {}

@interface Bridge : NSObject

+ (struct CxxRefType *)objCMethodReturningFRTUnannotated;
+ (struct CxxRefType *)objCMethodReturningFRTUnowned
    __attribute__((language_attr("returns_unretained")));
+ (struct CxxRefType *)objCMethodReturningFRTOwned
    __attribute__((language_attr("returns_retained")));
+ (struct CxxRefType *)objCMethodReturningFRTBothAnnotations // expected-error {{'objCMethodReturningFRTBothAnnotations' cannot be annotated with both LANGUAGE_RETURNS_RETAINED and LANGUAGE_RETURNS_UNRETAINED}}
    __attribute__((language_attr("returns_unretained")))
    __attribute__((language_attr("returns_retained")));
+ (struct CxxValType *)objCMethodReturningNonCxxFrtAnannotated // expected-error {{'objCMethodReturningNonCxxFrtAnannotated' cannot be annotated with either LANGUAGE_RETURNS_RETAINED or LANGUAGE_RETURNS_UNRETAINED because it is not returning a LANGUAGE_SHARED_REFERENCE type}}
    __attribute__((language_attr("returns_retained")));

@end

@implementation Bridge
+ (struct CxxRefType *)objCMethodReturningFRTUnannotated {
}
+ (struct CxxRefType *)objCMethodReturningFRTUnowned
    __attribute__((language_attr("returns_unretained"))) {
}
+ (struct CxxRefType *)objCMethodReturningFRTOwned
    __attribute__((language_attr("returns_retained"))) {
}
+ (struct CxxRefType *)objCMethodReturningFRTBothAnnotations
    __attribute__((language_attr("returns_unretained")))
    __attribute__((language_attr("returns_retained"))) {
}
+ (struct CxxValType *)objCMethodReturningNonCxxFrtAnannotated
    __attribute__((language_attr("returns_retained"))) {
}

@end

#pragma clang diagnostic pop
#pragma clang assume_nonnull end
