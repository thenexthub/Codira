// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/language-functions.code -module-name Functions -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/functions.h  -cxx-interoperability-mode=upcoming-language

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/language-functions-execution.o -g
// RUN: %target-interop-build-language %S/language-functions.code -o %t/language-functions-execution -Xlinker %t/language-functions-execution.o -module-name Functions -Xfrontend -entry-point-function-name -Xfrontend languageMain -g

// RUN: %target-codesign %t/language-functions-execution
// RUN: %target-run %t/language-functions-execution 2>&1 | %FileCheck %s

// REQUIRES: executable_test

#include <cassert>
#include <stdio.h>
#include "functions.h"

int main() {
  static_assert(noexcept(Functions::passVoidReturnVoid()), "noexcept function");
  static_assert(noexcept(Functions::_impl::$s9Functions014passVoidReturnC0yyF()),
                "noexcept function");

  Functions::passVoidReturnVoid();
  Functions::passIntReturnVoid(-1);
  assert(Functions::passTwoIntReturnIntNoArgLabel(1, 2) == 42);
  assert(Functions::passTwoIntReturnInt(1, 2) == 3);
  assert(Functions::passTwoIntReturnIntNoArgLabelParamName(1, 4) == 5);
  Functions::passVoidReturnNever();
  return 42;
}

// CHECK: passVoidReturnVoid
// CHECK-NEXT: passIntReturnVoid -1
// CHECK-NEXT: passTwoIntReturnIntNoArgLabel
// CHECK-NEXT: passVoidReturnNever
