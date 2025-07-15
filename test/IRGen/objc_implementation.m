// This isn't really a test--it's more like an example of how
// IRGen/objc_implementation.code would be written in Objective-C to compare
// the code clang and Codira generate. It's unlikely to ever pass, so it's
// disabled. If you want to temporarily use it, disable this REQUIRES line:
// REQUIRES: development_only

// RUN: %empty-directory(%t)
// RUN: %target-language-frontend(mock-sdk: %clang-importer-sdk) -I %S/Inputs/abi -F %clang-importer-sdk-path/frameworks %S/objc_implementation.code -import-objc-header %S/Inputs/objc_implementation.h -emit-ir > %t/language.ll
// RUN: %clang -S -emit-toolchain %target-cc-options -isysroot %clang-importer-sdk-path -I %S/Inputs/abi -F %clang-importer-sdk-path/frameworks -I %S/Inputs %s -o %t/clang.ll
// RUN: ksdiff %t/clang.ll %t/language.ll

#import "objc_implementation.h"
int printf(const char * restrict format, ...);

@implementation ImplClass

- (void)mainMethod:(int)value {
  printf("mainMethod");
}

@end

@implementation ImplClass (Category1)

- (void)category1Method:(int)value {
  printf("category1Method");
}

@end

void implFunc(int param) {
  printf("implFunc");
}

void implFuncCName(int param) {
  printf("implFuncCName");
}
