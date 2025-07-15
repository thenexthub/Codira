// RUN: %empty-directory(%t)

// RUN: %target-language-frontend %S/cdecl.code -module-name CdeclFunctions -clang-header-expose-decls=all-public -typecheck -verify -emit-clang-header-path %t/functions.h

// RUN: %target-interop-build-clangxx -c %s -I %t -o %t/cdecl-execution.o
// RUN: %target-interop-build-language %S/cdecl.code -o %t/cdecl-execution -Xlinker %t/cdecl-execution.o -module-name Functions -Xfrontend -entry-point-function-name -Xfrontend languageMain

// RUN: %target-codesign %t/cdecl-execution
// RUN: %target-run %t/cdecl-execution

// REQUIRES: executable_test

#include <cassert>
#include "functions.h"

int main() {
  assert(CdeclFunctions::differentCDeclName(1, 1) == 2);
  return 0;
}
