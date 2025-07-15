// RUN: %empty-directory(%t)
// RUN: %target-language-frontend(mock-sdk: %clang-importer-sdk) %S/../Inputs/empty.code -typecheck -verify -emit-objc-header-path %t/empty.h
// RUN: %clang -F %clang-importer-sdk-path/frameworks -E -fobjc-arc -fmodules -isysroot %clang-importer-sdk-path -I %t %s | %FileCheck %s

// REQUIRES: objc_interop

#import "empty.h"

@class ABC; // CHECK-LABEL: @class ABC;
LANGUAGE_CLASS(abc) // CHECK-NEXT: __attribute__((objc_runtime_name(abc)))
@interface ABC // CHECK-NEXT: @interface
@end

@class DEF; // CHECK-LABEL: @class DEF;
LANGUAGE_CLASS_NAMED(def) // CHECK-NEXT: __attribute__((language_name(def)))
@interface DEF // CHECK-NEXT: @interface
@end

@protocol AAA; // CHECK-LABEL: @protocol AAA;
LANGUAGE_PROTOCOL(aaa) // CHECK-NEXT: __attribute__((objc_runtime_name(aaa)))
@protocol AAA // CHECK-NEXT: @protocol
@end

@protocol BBB; // CHECK-LABEL: @protocol BBB;
LANGUAGE_PROTOCOL_NAMED(bbb) // CHECK-NEXT: __attribute__((language_name(bbb)))
@protocol BBB // CHECK-NEXT: @protocol
@end
