#include "NestedClangTypesHelper.h"

struct Outer {
  int value;
};

struct __attribute__((language_name("Outer.InterestingValue"))) Inner {
  int value;
};

struct OuterFromOtherModule;
struct __attribute__((language_name("OuterFromOtherModule.InterestingValue"))) InnerCrossModule {
  int value;
};


#if __OBJC__

@import Foundation;

extern NSString * const ErrorCodeDomain;
enum __attribute__((ns_error_domain(ErrorCodeDomain))) ErrorCodeEnum {
  ErrorCodeEnumA
};

#endif // __OBJC__
