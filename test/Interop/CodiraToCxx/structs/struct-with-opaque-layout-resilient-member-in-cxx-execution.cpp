// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/resilient-struct-in-cxx.code -enable-library-evolution -module-name Structs -emit-module -emit-module-path %t/Structs.codemodule

// RUN: %target-language-frontend %S/struct-with-opaque-layout-resilient-member-in-cxx.code -module-name UseStructs -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/useStructs.h -I %t


// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-structs-execution.o

// RUN: %target-interop-build-language -c %S/resilient-struct-in-cxx.code -enable-library-evolution -module-name Structs -o %t/resilient-struct-in-cxx.o -Xfrontend -entry-point-function-name -Xfrontend languageMain2

// RUN: %target-interop-build-language %S/struct-with-opaque-layout-resilient-member-in-cxx.code -o %t/language-structs-execution -Xlinker %t/resilient-struct-in-cxx.o -Xlinker %t/language-structs-execution.o -module-name UseStructs -Xfrontend -entry-point-function-name -Xfrontend languageMain -I %t

// RUN: %target-codesign %t/language-structs-execution
// RUN: %target-run %t/language-structs-execution | %FileCheck --check-prefixes=CHECK,CURRENT %s

// REQUIRES: executable_test

#include <assert.h>
#include "useStructs.h"

int main() {
  using namespace UseStructs;
  auto s = createUsesResilientSmallStruct();
  s.dump();
// CHECK: UsesResilientSmallStruct(97,FirstSmallStruct(x: 65)
  return 0;
}
