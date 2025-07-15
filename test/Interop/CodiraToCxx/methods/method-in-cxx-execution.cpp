// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/method-in-cxx.code -module-name Methods -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/methods.h

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-methods-execution.o
// RUN: %target-interop-build-language %S/method-in-cxx.code -o %t/language-methods-execution -Xlinker %t/language-methods-execution.o -module-name Methods -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-methods-execution
// RUN: %target-run %t/language-methods-execution | %FileCheck %s

// REQUIRES: executable_test

#include <assert.h>
#include "methods.h"

int main() {
  using namespace Methods;

  auto largeStruct = createLargeStruct();
  largeStruct.dump();
// CHECK: -1, 2, -100, 42, 67, -10101

  LargeStruct largeStructDoubled = largeStruct.doubled();
  largeStructDoubled.dump();
// CHECK-NEXT: -2, 4, -200, 84, 134, -20202

  auto largeStructScaled = largeStruct.scaled(2, -4);
  largeStructScaled.dump();
// CHECK-NEXT: -2, -8, -200, -168, 134, 40404

  auto largeStructAdded = largeStructDoubled.added(largeStruct);
  largeStructAdded.dump();
// CHECK-NEXT: -3, 6, -300, 126, 201, -30303

  {
    auto classValue = createClassWithMethods(42);
    classValue.dump();
// CHECK-NEXT: ClassWithMethods 42;
    auto classValueRef = classValue.sameRet();
    auto classValueOther = classValue.deepCopy(2);
    classValue.mutate();
    classValue.dump();
    classValueRef.dump();
    classValueOther.dump();
  }
// CHECK-NEXT: ClassWithMethods -42;
// CHECK-NEXT: ClassWithMethods -42;
// CHECK-NEXT: ClassWithMethods 44;
// CHECK-NEXT: ClassWithMethods 44 deinit
// CHECK-NEXT: ClassWithMethods -42 deinit

  auto classWithStruct = createPassStructInClassMethod();
  {
    auto largeStruct = classWithStruct.retStruct(-11);
    largeStruct.dump();
// CHECK-NEXT: PassStructInClassMethod.retStruct -11;
// CHECK-NEXT: 1, 2, 3, 4, 5, 6
  }
  classWithStruct.updateStruct(-578, largeStruct);
  {
    auto largeStruct = classWithStruct.retStruct(2);
    largeStruct.dump();
// CHECK-NEXT: PassStructInClassMethod.retStruct 2;
// CHECK-NEXT: -578, 2, -100, 42, 67, -10101
  }

  {
    LargeStruct::staticMethod();
// CHECK-NEXT: LargeStruct.staticMethod;
    auto largeStruct = ClassWithMethods::staticFinalClassMethod(9075);
// CHECK-NEXT: ClassWithMethods.staticFinalClassMethod;
    largeStruct.dump();
// CHECK-NEXT: 1, -1, -9075, -2, 9075, -456 
  }

  {
    auto x = ClassWithNonFinalMethods::classClassMethod(3);
    assert(x == 5);
    ClassWithNonFinalMethods::staticClassMethod();
  }
// CHECK-NEXT: ClassWithNonFinalMethods.classClassMethod;
// CHECK-NEXT: ClassWithNonFinalMethods.staticClassMethod;
  return 0;
}
