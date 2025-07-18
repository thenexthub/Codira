// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/resilient-struct-in-cxx.code -enable-library-evolution -module-name Structs -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/structs.h

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-structs-execution.o

// RUN: %target-interop-build-language %S/resilient-struct-in-cxx.code -enable-library-evolution -o %t/language-structs-execution -Xlinker %t/language-structs-execution.o -module-name Structs -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/language-structs-execution
// RUN: %target-run %t/language-structs-execution | %FileCheck --check-prefixes=CHECK,CURRENT %s

// RUN: %target-interop-build-language %S/resilient-struct-in-cxx.code -enable-library-evolution -o %t/language-structs-execution-new -Xlinker %t/language-structs-execution.o -module-name Structs -Xfrontend -entry-point-function-name -Xfrontend languageMain -D CHANGE_LAYOUT

// RUN: %target-codesign %t/language-structs-execution-new
// RUN: %target-run %t/language-structs-execution-new | %FileCheck --check-prefixes=CHECK,CHANGE %s

// REQUIRES: executable_test

#include <assert.h>
#include "structs.h"

int main() {
  using namespace Structs;

  auto largeStruct = createLargeStruct(11);
  assert(largeStruct.getX1() == 11);
  largeStruct.dump();
// CHECK: x.1 = 11, .2 = -11, .3 = 22, .4 = 7, .5 = 0

  auto smallStruct = largeStruct.getFirstSmallStruct();
  assert(smallStruct.getX() == 65);
  smallStruct.dump();
// CHECK: find - small dump
// CURRENT-NEXT: x = 65
// CHANGE-NEXT: x&y = 65&0

  auto copySmallStruct = smallStruct;
  mutateSmall(copySmallStruct);
  copySmallStruct.dump();
// CHECK: find - small dump
// CURRENT-NEXT: x = 66
// CHANGE-NEXT: x&y = 0&65
  copySmallStruct.mutate();
  copySmallStruct.dump();
// CHECK: find - small dump
// CURRENT-NEXT: x = 132
// CHANGE-NEXT: x&y = 0&4294967230

  printSmallAndLarge(smallStruct, largeStruct);
// CHECK: find - small dump
// CURRENT-NEXT: x = 65
// CHANGE-NEXT: x&y = 65&0
// CHECK-NEXT: x.1 = 11, .2 = -11, .3 = 22, .4 = 7, .5 = 0

  {
    auto structWithRefCountStoredProp =
      createStructWithRefCountStoredProp();
    structWithRefCountStoredProp.dump();
    {
       StructWithRefCountStoredProp copy(structWithRefCountStoredProp);
    }
    structWithRefCountStoredProp.dump();
  }
// CHECK-NEXT: create RefCountedClass 0
// CHECK-NEXT: storedRef = 0
// CHECK-NEXT: storedRef = 0
// CHECK-NEXT: destroy RefCountedClass 0
  return 0;
}
